#pragma once

#include <Unknwn.h>

#include "../FileEngine/IFileEngine.h"

// {F8F96468-0E5B-4485-8029-326D88640EDC}
extern "C" const GUID IID_IWndPreview;
// {A7C6EAF4-6BDF-448f-A8EC-7BADC8A06549}
extern "C" const GUID CLSID_IWndPreview;

interface RWndPreview
{
	virtual LPCTSTR	OnGetLastError() = 0;
	virtual bool	OnQueryWindow(HWND hwnd) = 0;
	virtual bool	OnQueryProcess(DWORD dwPid) = 0;
	//If the normal calls, should return true
	virtual bool	OnCheckRWndPreview() = 0;
};

interface IWndPreview : public IUnknown
{
	virtual HWND GetWndHandle() = 0;
	virtual bool ShowChildWndView() = 0;
	virtual bool HideChildWndView() = 0;
	virtual bool UpdateChildWndsInfo(HWND hWnd) = 0;
	virtual bool CreateChildWndView(CWnd* pParentWnd = 0) = 0;
	virtual bool SetRWndPreview(RWndPreview* pRWndPreview) = 0;
	virtual bool DelRWndPreview(RWndPreview* pRWndPreview) = 0;
	virtual bool SetIFileEngine(IFileEngine* pIFileEngine) = 0;
};
