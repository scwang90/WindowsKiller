#pragma once

#include "IWndExtract.h"
#include "WndExtBorderDlg.h"

class CWndExtractMgr : public IWndExtract
{
public:
	CWndExtractMgr();
	~CWndExtractMgr();

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IWndExtract member function	
	virtual	bool AddBorderWnd(HWND hwnd);
	virtual	bool DelBorderWnd(void* lpVoid);

private:
	// IUnknown member data	
	int		m_Ref;

	// IWndExtract member function
#define MAX_BORDER 64
	CWndExtBorderDlg*	m_lpBorder[MAX_BORDER];
	int			m_BorderNum;
};

