
// SingleSeverQSys.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSingleSeverQSysApp:
// �йش����ʵ�֣������ SingleSeverQSys.cpp
//

class CSingleSeverQSysApp : public CWinApp
{
public:
	CSingleSeverQSysApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSingleSeverQSysApp theApp;