//-------------------------------------------------------------------------
#include "FKMaskLayer.h"
//-------------------------------------------------------------------------
FKMaskLayer::FKMaskLayer()
	: FKMapLayer( s_MaskLayerID )
{
	SetLayerDepth( s_MaskLayerID );
}
//-------------------------------------------------------------------------
FKMaskLayer::~FKMaskLayer()
{

}
//-------------------------------------------------------------------------
// 获取地图层类型( 返回 ENUM_MapImageLayerType )
int FKMaskLayer::GetLayerType()
{
	return eMILT_Mask;
}
//-------------------------------------------------------------------------