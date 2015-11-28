//*************************************************************************
//	创建日期:	2015-1-14
//	文件名称:	IUISystem.h
//  创 建 人:   王宏张 FreeKnight	
//	版权所有:	MIT
//	说    明:	
//*************************************************************************

#pragma once

//-------------------------------------------------------------------------
class FKCW_UIWidget_WidgetWindow;
class IUISystem
{
public:
	// 释放销毁
	virtual void							Release() = 0;
	// 获取UI系统根节点窗口
	virtual FKCW_UIWidget_WidgetWindow*		GetRoot() const = 0;
};
//-------------------------------------------------------------------------
// 获取UI管理器
extern IUISystem* GetUISystem();
//-------------------------------------------------------------------------