//*************************************************************************
//	创建日期:	2015-2-2
//	文件名称:	FKRenderSystem.h
//  创 建 人:   王宏张 FreeKnight	
//	版权所有:	MIT
//	说    明:	
//*************************************************************************

#pragma once

//-------------------------------------------------------------------------
#include "../FKEngineCommonHead.h"
//-------------------------------------------------------------------------
class FKRenderObjMgr;
class FKBannerReelMapMgr;
class FKRenderTextMgr;
class FKRenderSystem : public IRenderSystem
{
public:
	FKRenderSystem();
	~FKRenderSystem();
public:
	// 初始化渲染系统
	// 参数：tagDesignSize 渲染窗口大小
	virtual void		Init( CCSize& tagDesignSize );
	// 设置根渲染节点，所有场景全部挂接在本节点上
	virtual void		SetRoot( CCNode* pMapRootNode );
	// 获取当前运行中的地图场景
	virtual IMapObj*	GetCurrentMap();
	// 更新进入新场景
	// 参数：szFKMFileName 地图数据文件
	virtual RENDER_MAP_ID	EnterMap( const char* szFKMFileName );
	// 是否允许Update
	virtual void			SetEnableUpdate( bool bIsEnable );
	// 设置场景摄像机视点
	// 参数：tagLocPos 摄像机视点位置
	virtual void		SetViewPoint( const CCPoint& tagLocPos );
	// 获取场景摄像机视点
	// 参数：[out]tagLocPos 摄像机视点位置
	virtual bool		GetViewPoint( const CCPoint& tagLocPos );
	// 转换屏幕坐标到场景坐标
	// 参数：[in]tagScreenPos 屏幕坐标
	// 参数：[out]tagScenePos 场景坐标
	virtual bool		ConvertScreenPosToScenePos( const CCPoint& tagScreenPos,
		CCPoint& tagScenePos );

	// 获取碰撞对象
	// 参数：ObjList 被碰撞的对象列表
	virtual void		HitRenderObj( const CCPoint& tagScreenPos, list<IRenderObj*>& ObjList );
	// 创建渲染对象
	// 参数：szGameObjName 游戏对象名
	virtual IRenderObj*	CreateRenderObj( const char* szGameObjName );
	// 删除渲染对象
	// 参数: pRenderObj 需要删除的渲染对象
	virtual bool		RemoveRenderObj( IRenderObj* pRenderObj );
	// 删除渲染对象
	// 参数: id 需要删除的渲染对象ID
	virtual bool		RemoveRenderObj( RENDER_OBJ_ID id );
	// 获取渲染对象
	virtual IRenderObj*	GetRenderObj( RENDER_OBJ_ID id );
	// 添加渲染对象到当前场景
	// 参数：pRenderObj 添加渲染对象到当前场景
	virtual bool		AddRenderObjToScene( IRenderObj* pRenderObj );
	// 从当前场景删除渲染对象
	// 参数：pRenderObj 需要从当前场景删除的渲染对象
	virtual bool		RemoveRenderObjFromScene( IRenderObj* pRenderObj );

	// 创建渲染文字
	// 参数：szString 文字
	// 参数：szFontName 字体名称
	// 参数：nFontSize 字体大小
	// 参数：tagDimentions 字体尺寸
	// 参数：bIsBmpFont 是否是BMP文字
	virtual IRenderText* CreateRenderText( const char* szString, const char* szFontName, 
		int nFontSize,  CCSize tagDimentions = CCSizeZero, bool bIsBmpFont = false );
	// 删除渲染文字
	// 参数：pRenderText 需要删除的渲染文字
	virtual bool		RemoveRenderText( IRenderText* pRenderText );
	// 删除渲染文字
	// 参数：id 唯一ID
	virtual bool		RemoveRenderTextByID( RENDER_OBJ_ID id );
private:
	FKRenderObjMgr*				m_pRenderObjMgr;
	FKBannerReelMapMgr*			m_pMapMgr;
	FKRenderTextMgr*			m_pRenderTextMgr;
};

//-------------------------------------------------------------------------
extern FKRenderSystem gs_FKRenderSystem;
//-------------------------------------------------------------------------

