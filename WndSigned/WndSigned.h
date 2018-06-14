#pragma once

#include "IWndSigned.h"

class WndSigned :public IWndSigned
{
public:
	WndSigned(void);
public:
	~WndSigned(void);

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IWndSigned member function	
	virtual bool SignHide();
	virtual bool SignRect(LPRECT lpRect, int iTime);
	virtual bool IsSignHide();
	virtual bool SetSignColor(DWORD color);
	virtual LONG GetSignColor();

private:
	// IUnknown member data	
	int		m_Ref;

	// IWndSigned member function
	HWND	m_hWnd;
};
