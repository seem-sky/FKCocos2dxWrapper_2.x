//*************************************************************************
//	��������:	2014-8-13   15:16
//	�ļ�����:	Event.h
//  �� �� ��:   ������ FreeKnight	
//	��Ȩ����:	MIT
//	˵    ��:	
//*************************************************************************

#pragma once

//-------------------------------------------------------------------------
#include "BaseMacro.h"
#include <string>
using std::string;
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

class CEvent
{
public:
	string				m_szType;
public:
	CEvent( const string& p_szType )
	{
		m_szType = p_szType;
	}
	virtual ~CEvent()
	{

	}
};

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------