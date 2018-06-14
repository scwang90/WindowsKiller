#pragma once


#include <Unknwn.h>

// {D7AE683E-35FC-434b-B856-B5A8B01D0F2D}
extern "C" const GUID IID_IWndExtract;
// {CB260DCF-4553-488d-9E8A-0343C0DC6786}
extern "C"  const GUID CLSID_IWndExtract;

interface IWndExtract : public IUnknown
{
	virtual	bool AddBorderWnd(HWND hwnd) = 0;
	virtual	bool DelBorderWnd(void* lpVoid) = 0;
};

