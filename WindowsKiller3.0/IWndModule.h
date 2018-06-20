#pragma once

#include <Unknwn.h>

#include "../FileEngine/IFileEngine.h"


// {74420AE8-B2C6-4216-9305-A5CA77DC38AA}
extern "C" const GUID IID_IWndModule;
// {E0E54DE5-A585-4FF5-8839-AE4F4DD42B06}
extern "C" const GUID CLSID_IWndModule;

interface IWndModuleCallback
{
	//If the normal calls, should return true
	virtual bool OnCheckCallback() = 0;
};

interface IWndModule : public IUnknown
{
	virtual HWND GetWndHandle() = 0;
	virtual bool CreateWndModule(CWnd* pParentWnd = 0) = 0;
	virtual	bool UpdateWndModuleInfo(LPCTSTR lpTstr, HWND hFwnd = NULL, DWORD dwPid = 0) = 0;
	virtual bool SetCallback(IWndModuleCallback* pRWndModule) = 0;
	virtual bool SetIFileEngine(IFileEngine* pIFileEngine) = 0;
	virtual LPVOID GetProcEngine() = 0;
};

