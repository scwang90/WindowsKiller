#pragma once

#include "IWndWizard.h"
#include "WndWizardDlg.h"

class CWndWizardMgr :public IWndWizard
{
public:
	CWndWizardMgr();
	~CWndWizardMgr();

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();
		
	// IViewWizard member function	
	virtual	HWND GetCurrentWindow();
	virtual bool LaunchViewWizard();
	virtual	bool ClearViewWizard(void* lpVoid);
	virtual bool SetIWndModule(IWndModule*pIWndModule);
	virtual bool SetIWndPreview(IWndPreview*pIWndPreview);
	virtual bool SetRViewWizard(RViewWizard*pRViewWizard);
	virtual bool SetIFileEngine(IFileEngine*pIFileEngine);
	virtual	bool SetBackStage(bool value);

private:
	// IUnknown member data	
	int		m_Ref;

	// IViewWizard member function
#define MAX_WIZARD 64
	CWndWizardDlg*		m_lpBackWizard;
	CWndWizardDlg*		m_lpWizard[MAX_WIZARD];
	int					m_WizardNum;
	RViewWizard*		m_pRViewWizard;
	BOOL				m_bIsBackstage;
};
