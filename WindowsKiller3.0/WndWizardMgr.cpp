#include "stdafx.h"
#include "WndWizardMgr.h"

// class IViewWizard implementation

CWndWizardMgr::CWndWizardMgr(void)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	//////////////////////////////////////////////////////////////////////////////
	//	IViewWizard
	m_WizardNum = 0;
	m_pRViewWizard = 0;
	m_lpBackWizard = NULL;
	m_bIsBackstage = FALSE;
	memset(m_lpWizard, 0, sizeof(m_lpWizard));
}

CWndWizardMgr::~CWndWizardMgr(void)
{
	for (int i = 0; i < m_WizardNum; i++)
		delete	m_lpWizard[i];
	if (m_bIsBackstage == TRUE && m_lpBackWizard) {
		delete m_lpBackWizard;
	}
}


bool CWndWizardMgr::LaunchViewWizard()
{
	if (m_WizardNum < MAX_WIZARD)
	{
		if (m_bIsBackstage == TRUE && m_lpBackWizard == NULL) {
			m_lpBackWizard = new CWndWizardDlg(this, true);

		}
		else {
			m_lpWizard[m_WizardNum++] = new CWndWizardDlg(this, false);
		}

		return true;
	}
	return false;
}
bool CWndWizardMgr::ClearViewWizard(void* lpVoid)
{
	for (int i = 0; i < m_WizardNum; i++)
	{
		if ((void*)m_lpWizard[i] == lpVoid)
		{
			delete	m_lpWizard[i];
			m_lpWizard[i] = m_lpWizard[--m_WizardNum];
			if (m_WizardNum == 0)
			{
				ASSERT(m_pRViewWizard != NULL);
				m_pRViewWizard->OnViewWizardEmpty();
			}
			return true;
		}
	}
	return false;
}
bool CWndWizardMgr::SetRViewWizard(RViewWizard*pRViewWizard)
{
	if (pRViewWizard->OnCheckRViewWizard())
	{
		m_pRViewWizard = pRViewWizard;
		return true;
	}
	return false;
}
bool CWndWizardMgr::SetIFileEngine(IFileEngine*pIFileEngine)
{
	return CWndWizardDlg::SetIFileEngine(pIFileEngine);
}
bool CWndWizardMgr::SetIWndModule(IWndModule*pIWndModule)
{
	return CWndWizardDlg::SetIWndModule(pIWndModule);
}
bool CWndWizardMgr::SetIWndPreview(IWndPreview*pIWndPreview)
{
	return CWndWizardDlg::SetIWndPreview(pIWndPreview);
}
HWND CWndWizardMgr::GetCurrentWindow()
{
	if (m_bIsBackstage == TRUE && m_lpBackWizard) {
		return m_lpBackWizard->m_hWnd;
	}
	else {
		if (m_WizardNum > 0)
			return m_lpWizard[m_WizardNum - 1]->m_hWnd;
		else
			return NULL;
	}
}

bool CWndWizardMgr::SetBackStage(bool value)
{
	m_bIsBackstage = value;
	return true;
}
HRESULT		CWndWizardMgr::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IViewWizard)
	{
		*ppv = (IWndWizard *)this;
		((IWndWizard *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG		CWndWizardMgr::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG		CWndWizardMgr::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}




