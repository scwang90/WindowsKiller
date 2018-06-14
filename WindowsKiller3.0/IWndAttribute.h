#pragma once

#include <Unknwn.h>

#include "..\\FileEngine\\IFileEngine.h"

// {03E1DA29-14FB-4D92-B37E-293BD6C6C81A}
extern "C" const GUID IID_IWndAttribute;
// {1A5DD9D7-255B-462C-93A6-7EBA3DA8254A}
extern "C" const GUID CLSID_IWndAttribute;

interface IWndAttribute : public IUnknown
{
	virtual HWND GetWndHandle() = 0;
	virtual bool QueryWindowAttrib(HWND hWnd) = 0;
	virtual bool CreateAttributeView(CWnd* pParentWnd = 0) = 0;
	virtual bool SetIFileEngine(IFileEngine* pIFileEngine) = 0;
};
