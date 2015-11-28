//-------------------------------------------------------------------------
#include "../Include/FKCW_TMX_Layer.h"
#include "../Include/FKCW_TMX_MapInfo.h"
#include "../Include/FKCW_TMX_LayerInfo.h"
#include "../Include/FKCW_TMX_TileSetInfo.h"
//-------------------------------------------------------------------------
FKCW_TMX_Layer::FKCW_TMX_Layer(int layerIndex, FKCW_TMX_MapInfo* mapInfo) :
	m_mapInfo(mapInfo),
	m_layerInfo(NULL),
	m_reusedTile(NULL),
	m_tileWidth(mapInfo->getTileWidth()),
	m_tileHeight(mapInfo->getTileHeight()),
	m_batchNodes(NULL),
	m_tiles(NULL),
	m_atlasInfos(NULL),
	m_useAutomaticVertexZ(false),
	m_minGid(MAX_INT),
	m_maxGid(0),
	m_vertexZ(0.0f),
	m_alphaFuncValue(0.0f) 
{
		// retain map info
		CC_SAFE_RETAIN(mapInfo);

		// get layer info
		m_layerInfo = (FKCW_TMX_LayerInfo*)mapInfo->getLayers().objectAtIndex(layerIndex);
		CC_SAFE_RETAIN(m_layerInfo);

		// init layer members
		m_layerWidth = m_layerInfo->getLayerWidth();
		m_layerHeight = m_layerInfo->getLayerHeight();
		m_tiles = m_layerInfo->getTiles();
		setOpacity(m_layerInfo->getAlpha());

		// allocate memory for atlas indices
		size_t size = m_layerInfo->getLayerWidth() * m_layerInfo->getLayerHeight() * sizeof(STileSetAtlasInfo);
		m_atlasInfos = (STileSetAtlasInfo*)malloc(size);
		memset(m_atlasInfos, 0xff, size);

		// memory for batch node array
		m_batchNodes = (FKCW_TMX_SpriteBatchNode**)calloc(mapInfo->getTileSets().count(), sizeof(FKCW_TMX_SpriteBatchNode*));

		// set anchor percent
		setAnchorPoint(CCPointZero);
		ignoreAnchorPointForPosition(false);

		// set content size
		switch(m_mapInfo->getOrientation()) 
		{
		case eTMXOrientationIsometric:
		case eTMXOrientationOrthogonal:
			setContentSize(CCSizeMake(m_layerWidth * m_tileWidth, m_layerHeight * m_tileHeight));
			break;
		case eTMXOrientationHexagonal:
			setContentSize(CCSizeMake(m_layerWidth * m_tileWidth * 3 / 4 + m_tileWidth / 4,
				m_layerHeight * m_tileHeight + (m_layerWidth > 1 ? (m_tileHeight / 2) : 0)));
			break;
		}

		// set visible or not
		setVisible(m_layerInfo->isVisible());

		// set position
		CCPoint offset = calculateLayerOffset(m_layerInfo->getOffsetX(), m_layerInfo->getOffsetY());
		setPosition(offset.x, offset.y);

		// setup tiles
		setupTiles();
}
//-------------------------------------------------------------------------
FKCW_TMX_Layer::~FKCW_TMX_Layer() 
{
	CC_SAFE_RELEASE(m_mapInfo);
	CC_SAFE_RELEASE(m_layerInfo);
	CC_SAFE_RELEASE(m_reusedTile);
	CC_SAFE_FREE(m_batchNodes);
	CC_SAFE_FREE(m_atlasInfos);
}
//-------------------------------------------------------------------------
FKCW_TMX_Layer* FKCW_TMX_Layer::create(int layerIndex, FKCW_TMX_MapInfo* mapInfo) 
{
	FKCW_TMX_Layer* l = new FKCW_TMX_Layer(layerIndex, mapInfo);
	if(l->init()) {
		return (FKCW_TMX_Layer*)l->autorelease();
	}
	l->release();
	return NULL;
}
//-------------------------------------------------------------------------
CCPoint FKCW_TMX_Layer::getPositionForOrthoAt(int posX, int posY) 
{
	float x = posX * m_tileWidth;
	float y = (m_layerHeight - posY - 1) * m_tileHeight;
	return ccp(x, y);
}
//-------------------------------------------------------------------------
CCPoint FKCW_TMX_Layer::getPositionForIsoAt(int posX, int posY) 
{
	float x = m_tileWidth / 2 * (m_layerWidth + posX - posY - 1);
	float y = m_tileHeight / 2 * (m_layerHeight * 2 - posX - posY - 2);
	return ccp(x, y);
}
//-------------------------------------------------------------------------
CCPoint FKCW_TMX_Layer::getPositionForHexAt(int posX, int posY) 
{
	float diffY = 0;
	if(posX % 2 == 0)
		diffY = m_tileHeight / 2;

	float x = posX * m_tileWidth * 3 / 4;
	float y = (m_layerHeight - posY - 1) * m_tileHeight + diffY;
	return ccp(x, y);
}
//-------------------------------------------------------------------------
CCPoint FKCW_TMX_Layer::getPositionAt(int x, int y)
{
	switch(m_mapInfo->getOrientation()) 
	{
	case eTMXOrientationOrthogonal:
		return getPositionForOrthoAt(x, y);
	case eTMXOrientationIsometric:
		return getPositionForIsoAt(x, y);
	case eTMXOrientationHexagonal:
		return getPositionForHexAt(x, y);
	default:
		return CCPointZero;
	}
}
//-------------------------------------------------------------------------
CCPoint FKCW_TMX_Layer::getTileCoordinateAt(float x, float y) 
{
	CCPoint d = ccp(-1, -1);
	if(x < 0 || y < 0)
		return d;

	switch(m_mapInfo->getOrientation()) 
	{
	case eTMXOrientationOrthogonal:
		{
			d.x = x / m_tileWidth;
			d.y = m_layerHeight - (int)(y / m_tileHeight) - 1;
			break;
		}
	case eTMXOrientationIsometric:
		{
			// convert position relative to left-bottom to a position relative to top rhombus
			CCPoint top = getPositionForIsoAt(0, 0);
			top.x += m_tileWidth / 2;
			top.y += m_tileHeight;
			x -= top.x;
			y = top.y - y;

			// map converted position to tile coordinate
			d.x = (int)((m_tileWidth * y + m_tileHeight * x) / m_tileWidth / m_tileHeight);
			d.y = (int)((m_tileWidth * y - m_tileHeight * x) / m_tileWidth / m_tileHeight);
			break;
		}
	case eTMXOrientationHexagonal:
		{
			// convert position relative to left-bottom to a position to top-left
			CCPoint top = getPositionForHexAt(0, 0);
			float newY = top.y + m_tileHeight - y;

			// map converted position to tile coordinate
			float xBelt = x / (m_tileWidth * 3 / 4);
			float yEvenBelt = newY / m_tileHeight;
			float yOddBelt = (newY - m_tileHeight / 2) / m_tileHeight;
			bool intermediate = fmod(x, m_tileWidth * 3 / 4) < m_tileWidth / 4;
			if(intermediate) {
				// get possible candidates, two
				int posX2 = (int)xBelt;
				int posX1 = posX2 - 1;
				int posY1, posY2;
				if(posX2 % 2 == 0) {
					posY1 = (int)yOddBelt;
					posY2 = (int)yEvenBelt;
				} else {
					posY1 = (int)yEvenBelt;
					posY2 = (int)yOddBelt;
				}

				// get center of two candidates
				CCPoint pos1 = getPositionForHexAt(posX1, posY1);
				CCPoint pos2 = getPositionForHexAt(posX2, posY2);
				pos1.x += m_tileWidth / 2;
				pos1.y += m_tileHeight / 2;
				pos2.x += m_tileWidth / 2;
				pos2.y += m_tileHeight / 2;

				// choose the one whose center is nearest to touch position
				CCPoint touch = ccp(x, y);
				float distance1 = ccpLength(ccpSub(pos1, touch));
				float distance2 = ccpLength(ccpSub(pos2, touch));
				if(distance1 < distance2) {
					d.x = posX1;
					d.y = posY1;
				} else {
					d.x = posX2;
					d.y = posY2;
				}
			} else {
				d.x = (int)xBelt;
				if( static_cast<int>(d.x) % 2 == 0)
					d.y = (int)yEvenBelt;
				else
					d.y = (int)yOddBelt;
			}
			break;
		}
	}

	if(d.x < 0 || d.x >= m_layerWidth)
		d.x = -1;
	if(d.y < 0 || d.y >= m_layerHeight)
		d.y = -1;
	return d;
}
//-------------------------------------------------------------------------
float FKCW_TMX_Layer::getVertexZAt(int x, int y)
{
	float ret = 0;
	int maxVal = 0;
	if(m_useAutomaticVertexZ) 
	{
		switch(m_mapInfo->getOrientation()) 
		{
		case eTMXOrientationIsometric:
			maxVal = m_layerWidth + m_layerHeight;
			ret = -(maxVal - (x + y));
			break;
		case eTMXOrientationOrthogonal:
			ret = -(m_layerHeight - y);
			break;
		case eTMXOrientationHexagonal:
			// TODO TMX Hexagonal zOrder not supported
			break;
		default:
			CCLOGWARN("TMX invalid value");
			break;
		}
	} else {
		ret = m_vertexZ;
	}

	return ret;
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::setTileColorAt(ccColor4B c, int x, int y)
{
	// get gid
	int gid = getGidAt(x, y);
	if(gid <= 0)
		return;

	// get tileset index and rect
	int tilesetIndex = m_mapInfo->getTileSetIndex(gid);
	FKCW_TMX_TileSetInfo* tileset = (FKCW_TMX_TileSetInfo*)m_mapInfo->getTileSets().objectAtIndex(tilesetIndex);
	CCRect rect = tileset->getRect(gid);
	FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[tilesetIndex];

	// get sprite
	CCSprite* tile = reusedTile(rect, bn);
	setupTileSprite(tile, ccp(x, y), gid);
	tile->setColor(ccc3(c.r, c.g, c.b));
	tile->setOpacity(c.a);

	// update
	int z = x + y * m_layerWidth;
	bn->updateQuadFromSprite(tile, m_atlasInfos[z].atlasIndex);
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::setupTileSprite(CCSprite* sprite, CCPoint pos, int gid) 
{
	CCPoint spritePos = getPositionAt(static_cast<int>(pos.x), static_cast<int>(pos.y));
	sprite->setPosition(ccp(spritePos.x + sprite->getContentSize().width / 2,
		spritePos.y + sprite->getContentSize().height / 2));
	sprite->setVertexZ(getVertexZAt(static_cast<int>(pos.x), static_cast<int>(pos.y)));
	sprite->setColor(getColor());
	sprite->setOpacity(getOpacity());

	//issue 1264, flip can be undone as well
	sprite->setFlipX(false);
	sprite->setFlipY(false);
	sprite->setRotation(0.0f);

	// Rotation in tiled is achieved using 3 flipped states, flipping across the horizontal, vertical, and diagonal axes of the tiles.
	if(gid & eTMXTileFlagFlipDiagonal) 
	{
		// get flag
		int flag = gid & (eTMXTileFlagFlipH | eTMXTileFlagFlipV);

		// handle the 4 diagonally flipped states.
		if (flag == eTMXTileFlagFlipH) {
			sprite->setRotation(90.0f);
		} else if(flag == eTMXTileFlagFlipV) {
			sprite->setRotation(270.0f);
		} else if (flag == (eTMXTileFlagFlipV | eTMXTileFlagFlipH)) {
			sprite->setRotation(90.0f);
			sprite->setFlipX(true);
		} else {
			sprite->setRotation(270.0f);
			sprite->setFlipX(true);
		}
	} 
	else 
	{
		if(gid & eTMXTileFlagFlipH) 
		{
			sprite->setFlipX(true);
		}
		if(gid & eTMXTileFlagFlipV) 
		{
			sprite->setFlipY(true);
		}
	}
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::appendTileForGid(int tilesetIndex, int gid, int x, int y) 
{
	// get next index
	FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[tilesetIndex];
	CCTextureAtlas* atlas = bn->getTextureAtlas();
	int index = atlas->getTotalQuads();

	// save atlas index
	int z = x + y * m_layerWidth;
	m_atlasInfos[z].atlasIndex = index;
	m_atlasInfos[z].tilesetIndex = tilesetIndex;

	// get tileset and rect of gid
	FKCW_TMX_TileSetInfo* tileset = (FKCW_TMX_TileSetInfo*)m_mapInfo->getTileSets().objectAtIndex(tilesetIndex);
	CCRect rect = tileset->getRect(gid);

	// create sprite
	CCSprite* tile = reusedTile(rect, bn);

	// setup tile
	setupTileSprite(tile, ccp(x, y), gid);

	// insert
	bn->insertQuadFromSprite(tile, index);
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::parseInternalProperties() 
{
	string vertexz = getProperty("cc_vertexz");
	if(!vertexz.empty()) 
	{
		// If "automatic" is on, then parse the "cc_alpha_func" too
		if (vertexz == "automatic") 
		{
			m_useAutomaticVertexZ = true;
			string alphaFuncVal = getProperty("cc_alpha_func");
			float alphaFuncValue = 0.0f;
			if(!alphaFuncVal.empty()) 
			{
				alphaFuncValue = static_cast<float>(atof(alphaFuncVal.c_str()));
			}
			setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColorAlphaTest));

			GLint alphaValueLocation = glGetUniformLocation(getShaderProgram()->getProgram(), kCCUniformAlphaTestValue);

			// NOTE: alpha test shader is hard-coded to use the equivalent of a glAlphaFunc(GL_GREATER) comparison
			getShaderProgram()->setUniformLocationWith1f(alphaValueLocation, alphaFuncValue);
		} 
		else 
		{
			m_vertexZ = static_cast<float>(atof(vertexz.c_str()));
		}
	}
}
//-------------------------------------------------------------------------
CCPoint FKCW_TMX_Layer::calculateLayerOffset(float x, float y)
{
	CCPoint ret = CCPointZero;
	switch(m_mapInfo->getOrientation()) {
	case eTMXOrientationOrthogonal:
		ret.x = x * m_tileWidth;
		ret.y = y * m_tileHeight;
		break;
	case eTMXOrientationIsometric:
		ret.x = (m_tileWidth / 2) * (x - y);
		ret.y = (m_tileHeight / 2) * (-x - y);
		break;
	case eTMXOrientationHexagonal:
		// TODO offset for hexagonal map not implemented yet
		break;
	}
	return ret;
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::setupTiles() 
{
	// Parse cocos2d properties
	parseInternalProperties();

	// add tiles coordinates
	for(int y = 0; y < m_layerHeight; y++) 
	{
		for(int x = 0; x < m_layerWidth; x++) 
		{
			int pos = x + m_layerWidth * y;
			int gid = m_tiles[pos];

			// gid == 0 -> empty tile
			if(gid != 0) 
			{
				// find tileset index
				int tilesetIndex = m_mapInfo->getTileSetIndex(gid);

				// if corresponded batch not is not created, create it and add it
				if(m_batchNodes[tilesetIndex] == NULL) 
				{
					FKCW_TMX_TileSetInfo* tileset = (FKCW_TMX_TileSetInfo*)m_mapInfo->getTileSets().objectAtIndex(tilesetIndex);
					FKCW_TMX_SpriteBatchNode* bn = FKCW_TMX_SpriteBatchNode::createWithTexture(tileset->getTexture());
					m_batchNodes[tilesetIndex] = bn;
					addChild(bn, tilesetIndex);
				}

				// append tile
				appendTileForGid(tilesetIndex, gid, x, y);
				m_minGid = MIN(m_minGid, gid);
				m_maxGid = MAX(m_maxGid, gid);
			}
		}
	}
}
//-------------------------------------------------------------------------
string FKCW_TMX_Layer::getProperty(const string& key) {
	return m_layerInfo->getProperty(key);
}
//-------------------------------------------------------------------------
int FKCW_TMX_Layer::getGidAt(int x, int y) {
	if(x < 0 || x >= m_layerWidth || y < 0 || y >= m_layerHeight)
		return 0;

	int pos = x + m_layerWidth * y;
	return m_tiles[pos];
}
//-------------------------------------------------------------------------
const int* FKCW_TMX_Layer::getGids() {
	return m_tiles;
}
//-------------------------------------------------------------------------
const int* FKCW_TMX_Layer::copyGids() 
{
	int totalNumberOfTiles = m_layerWidth * m_layerHeight;
	int* ret = (int*)malloc(totalNumberOfTiles * sizeof(int));
	memcpy(ret, m_tiles, totalNumberOfTiles * sizeof(int));
	return ret;
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::decreaseIndexIfMoreThan(int tilesetIndex, int index) 
{
	for(int x = 0; x < m_layerWidth; x++) {
		for(int y = 0; y < m_layerHeight; y++) {
			int z = x + y * m_layerWidth;
			if(m_atlasInfos[z].tilesetIndex == tilesetIndex && m_atlasInfos[z].atlasIndex > index) {
				m_atlasInfos[z].atlasIndex--;
			}
		}
	}
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::increaseIndexIfEqualOrMoreThan(int tilesetIndex, int index) 
{
	for(int x = 0; x < m_layerWidth; x++) {
		for(int y = 0; y < m_layerHeight; y++) {
			int z = x + y * m_layerWidth;
			if(m_atlasInfos[z].tilesetIndex == tilesetIndex && m_atlasInfos[z].atlasIndex >= index) {
				m_atlasInfos[z].atlasIndex++;
			}
		}
	}
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::removeTile(CCSprite* sprite)
{
	// basic validation
	if(!sprite)
		return;

	// get z
	int z = sprite->getTag();
	if(z < 0 || z >= m_layerWidth * m_layerHeight)
		return;

	// get batch node atlas
	FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[m_atlasInfos[z].tilesetIndex];

	// remove this sprite from batch node
	bn->removeChild(sprite, true);

	// adjust atlas indices
	decreaseIndexIfMoreThan(m_atlasInfos[z].tilesetIndex, m_atlasInfos[z].atlasIndex);
	m_atlasInfos[z].tilesetIndex = -1;
	m_atlasInfos[z].atlasIndex = -1;

	// remove gid
	m_tiles[z] = 0;
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::removeTileAt(int x, int y) 
{
	// basic validation
	if(x < 0 || x >= m_layerWidth || y < 0 || y >= m_layerHeight)
		return;

	// find index
	int z = x + y * m_layerWidth;
	int index = m_atlasInfos[z].atlasIndex;
	if(index < 0)
		return;

	// get batch node atlas
	FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[m_atlasInfos[z].tilesetIndex];
	CCTextureAtlas* atlas = bn->getTextureAtlas();

	// has sprite?
	CCSprite* sprite = (CCSprite*)bn->getChildByTag(z);
	if(sprite == NULL) {
		// remove quadratic
		atlas->removeQuadAtIndex(index);
	} else {
		// remove this sprite from batch node
		bn->removeChild(sprite, true);
	}

	// adjust atlas indices
	decreaseIndexIfMoreThan(m_atlasInfos[z].tilesetIndex, index);
	m_atlasInfos[z].tilesetIndex = -1;
	m_atlasInfos[z].atlasIndex = -1;

	// remove gid
	m_tiles[z] = 0;
}
//-------------------------------------------------------------------------
CCSprite* FKCW_TMX_Layer::reusedTile(CCRect rect, FKCW_TMX_SpriteBatchNode* bn) 
{
	if(!m_reusedTile) {
		m_reusedTile = new CCSprite();
		m_reusedTile->initWithTexture(bn->getTextureAtlas()->getTexture(), rect, false);
		m_reusedTile->setBatchNode(bn);
	} else {
		// XXX HACK: Needed because if "batch node" is nil,
		// then the Sprite'squad will be reset
		m_reusedTile->setBatchNode(NULL);

		// Re-init the sprite
		m_reusedTile->setTextureRect(rect, false, rect.size);

		// restore the batch node
		m_reusedTile->setBatchNode(bn);
	}

	return m_reusedTile;
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::setTileAt(int tilesetIndex, int gid, int x, int y, int z) 
{
	// get tileset
	FKCW_TMX_TileSetInfo* tileset = (FKCW_TMX_TileSetInfo*)m_mapInfo->getTileSets().objectAtIndex(tilesetIndex);
	CCRect rect = tileset->getRect(gid);

	// if coorespond batch not is not created, create it and add it
	if(m_batchNodes[tilesetIndex] == NULL) {
		FKCW_TMX_SpriteBatchNode* bn = FKCW_TMX_SpriteBatchNode::createWithTexture(tileset->getTexture());
		m_batchNodes[tilesetIndex] = bn;
		addChild(bn, tilesetIndex);
	}

	// get atlas and tileset
	FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[tilesetIndex];
	CCTextureAtlas* atlas = bn->getTextureAtlas();
	int index = atlas->getTotalQuads();

	// create and setup tile
	CCSprite* tile = reusedTile(rect, bn);
	setupTileSprite(tile, ccp(x, y), gid);

	// insert
	bn->insertQuadFromSprite(tile, index);

	// adjust index
	increaseIndexIfEqualOrMoreThan(tilesetIndex, index);

	// save atlas indices
	m_atlasInfos[z].atlasIndex = index;
	m_atlasInfos[z].tilesetIndex = tilesetIndex;

	// insert gid
	m_tiles[z] = gid;

	// update possible children
	CCObject* pObject = NULL;
	CCARRAY_FOREACH(bn->getChildren(), pObject) {
		CCSprite* pChild = (CCSprite*) pObject;
		if(pChild && pChild != tile) {
			unsigned int ai = pChild->getAtlasIndex();
			if(ai >= index) 
			{
				pChild->setAtlasIndex(ai + 1);
			}
		}
	}
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::setTileAt(int gid, int x, int y) 
{
	// basic validation
	if(x < 0 || x >= m_layerWidth || y < 0 || y >= m_layerHeight)
		return;

	// decide tileset index
	int tilesetIndex = m_mapInfo->getTileSetIndex(gid);

	// if same tileset, and index is not zero, just update it
	// if same tileset, but index is zero, set it
	// if not same tileset, first remove it and then set it
	int z = x + y * m_layerWidth;
	int index = m_atlasInfos[z].atlasIndex;
	if(m_atlasInfos[z].tilesetIndex == tilesetIndex) {
		if(index >= 0) {
			updateTileAt(gid, x, y);
		} else {
			setTileAt(tilesetIndex, gid, x, y, z);
		}
	} else {
		// first remove it
		if(index >= 0)
			removeTileAt(x, y);

		// then set it
		setTileAt(tilesetIndex, gid, x, y, z);
	}
}
//-------------------------------------------------------------------------
void FKCW_TMX_Layer::updateTileAt(int gid, int x, int y) 
{
	// basic validation
	if(x < 0 || x >= m_layerWidth || y < 0 || y >= m_layerHeight)
		return;

	// decide tileset index
	int tilesetIndex = m_mapInfo->getTileSetIndex(gid);

	// if same tileset, but index less than zero, just set it
	// if same tileset, but index >= 0, just update
	// if not same tileset, remove it and then set it
	int z = x + y * m_layerWidth;
	int index = m_atlasInfos[z].atlasIndex;
	if(m_atlasInfos[z].tilesetIndex == tilesetIndex)
	{
		if(index < 0) 
		{
			setTileAt(tilesetIndex, gid, x, y, z);
		} 
		else 
		{
			FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[tilesetIndex];
			FKCW_TMX_TileSetInfo* tileset = (FKCW_TMX_TileSetInfo*)m_mapInfo->getTileSets().objectAtIndex(tilesetIndex);
			CCRect rect = tileset->getRect(gid);

			// create and setup tile
			CCSprite* tile = reusedTile(rect, bn);
			setupTileSprite(tile, ccp(x, y), gid);

			// update
			int index = m_atlasInfos[z].atlasIndex;
			tile->setAtlasIndex(index);
			tile->setDirty(true);
			tile->updateTransform();

			// save gid
			m_tiles[z] = gid;
		}
	} 
	else 
	{
		// first remove it
		if(index >= 0)
			removeTileAt(x, y);

		// then set it
		setTileAt(tilesetIndex, gid, x, y, z);
	}
}
//-------------------------------------------------------------------------
CCSprite* FKCW_TMX_Layer::tileAt(int x, int y) 
{
	// get gid
	int gid = getGidAt(x, y);
	CCSprite* tile = NULL;

	// if gid is zero, no tile at that location
	if(gid != 0) {
		// decide tileset index
		int tilesetIndex = m_mapInfo->getTileSetIndex(gid);

		// get batch node and tileset
		FKCW_TMX_SpriteBatchNode* bn = m_batchNodes[tilesetIndex];
		FKCW_TMX_TileSetInfo* tileset = (FKCW_TMX_TileSetInfo*)m_mapInfo->getTileSets().objectAtIndex(tilesetIndex);
		CCRect rect = tileset->getRect(gid);

		// first check this child exist or not
		int z = x + y * m_layerWidth;
		tile = (CCSprite*)bn->getChildByTag(z);

		// if not exist, create a new
		if(!tile) {
			CCPoint origin = getPositionAt(x, y);
			tile = new CCSprite();
			tile->initWithTexture(bn->getTextureAtlas()->getTexture(), rect);
			tile->setBatchNode(bn);
			tile->setPosition(ccp(origin.x + tile->getContentSize().width / 2,
				origin.y + tile->getContentSize().height / 2));
			tile->setVertexZ(getVertexZAt(x, y));
			tile->setColor(getColor());
			tile->setOpacity(getOpacity());

			int index = m_atlasInfos[z].atlasIndex;
			bn->addSpriteWithoutQuad(tile, index, z);
			tile->release();
		}
	}

	return tile;
}
//-------------------------------------------------------------------------
// FKCW_TMX_SpriteBatchNode
//-------------------------------------------------------------------------
FKCW_TMX_SpriteBatchNode* FKCW_TMX_SpriteBatchNode::createWithTexture(CCTexture2D* tex, unsigned int capacity) 
{
	FKCW_TMX_SpriteBatchNode *batchNode = new FKCW_TMX_SpriteBatchNode();
	batchNode->initWithTexture(tex, capacity);
	batchNode->autorelease();

	return batchNode;
}
//-------------------------------------------------------------------------
FKCW_TMX_SpriteBatchNode* FKCW_TMX_SpriteBatchNode::create(const char *fileImage, unsigned int capacity) 
{
	FKCW_TMX_SpriteBatchNode *batchNode = new FKCW_TMX_SpriteBatchNode();
	batchNode->initWithFile(fileImage, capacity);
	batchNode->autorelease();

	return batchNode;
}
//-------------------------------------------------------------------------
FKCW_TMX_SpriteBatchNode::FKCW_TMX_SpriteBatchNode() {
}
//-------------------------------------------------------------------------
FKCW_TMX_SpriteBatchNode::~FKCW_TMX_SpriteBatchNode() {
}
//-------------------------------------------------------------------------