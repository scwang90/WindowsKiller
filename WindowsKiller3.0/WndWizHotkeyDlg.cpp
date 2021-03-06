// WinWizardHotkeyDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndWizHotkeyDlg.h"
#include "afxdialogex.h"


// CWndWizHotkeyDlg 对话框

IMPLEMENT_DYNAMIC(CWndWizHotkeyDlg, CDialogEx)

CWndWizHotkeyDlg::CWndWizHotkeyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_HOTKEY, pParent)
{
	m_wMainPanel1 = MOD_SHIFT;
	m_wMainPanel2 = VK_F1;
	m_wBackBegin1 = MOD_SHIFT;
	m_wBackBegin2 = VK_F2;
	m_wBackEnd1 = MOD_SHIFT;
	m_wBackEnd2 = VK_F3;
	m_wFindCpu1 = MOD_SHIFT;
	m_wFindCpu2 = VK_F4;
	m_wEndFind1 = MOD_SHIFT;
	m_wEndFind2 = VK_ESCAPE;
}

CWndWizHotkeyDlg::~CWndWizHotkeyDlg()
{
}

void CWndWizHotkeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOTKEY_BACKBEGIN, m_htBackBegin);
	DDX_Control(pDX, IDC_HOTKEY_BACKEND, m_htBackEnd);
	DDX_Control(pDX, IDC_HOTKEY_STOP, m_htEndFind);
	DDX_Control(pDX, IDC_HOTKEY_FINDCPU, m_htFindCpu);
	DDX_Control(pDX, IDC_HOTKEY_MAIN_PANEL, m_htMainPanel);
}


BEGIN_MESSAGE_MAP(CWndWizHotkeyDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CWndWizHotkeyDlg::OnBnClickedOk)
END_MESSAGE_MAP()

INT_PTR CWndWizHotkeyDlg::DoModal()
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndWizard.dll")*/);
	if (m_lpszTemplateName != NULL)
	{
		//hInst = ::GetModuleHandle(NULL/*TEXT("WndWizard.dll")*/);//AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)
	HWND hWndParent = PreModal();
	AfxUnhookWindowCreate();
	BOOL bEnableParent = FALSE;
#ifndef _AFX_NO_OLE_SUPPORT
	CWnd* pMainWnd = NULL;
	BOOL bEnableMainWnd = FALSE;
#endif

	if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
#ifndef _AFX_NO_OLE_SUPPORT
		pMainWnd = AfxGetMainWnd();
		if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled())
		{
			//
			// We are hosted by non-MFC container
			// 
			pMainWnd->EnableWindow(FALSE);
			bEnableMainWnd = TRUE;
		}
#endif
	}

	TRY
	{
		// create modeless dialog
		AfxHookWindowCreate(this);
	if (CreateDlgIndirect(lpDialogTemplate,
		CWnd::FromHandle(hWndParent), hInst))
	{
		if (m_nFlags & WF_CONTINUEMODAL)
		{
			// enter modal loop
			DWORD dwFlags = MLF_SHOWONIDLE;
			if (GetStyle() & DS_NOIDLEMSG)
				dwFlags |= MLF_NOIDLEMSG;
			VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
		}

		// hide the window before enabling the parent, etc.
		if (m_hWnd != NULL)
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW |
				SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
	}
		CATCH_ALL(e)
	{
		//DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL

#ifndef _AFX_NO_OLE_SUPPORT
		if (bEnableMainWnd)
			pMainWnd->EnableWindow(TRUE);
#endif
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return m_nModalResult;
}
// CWndWizHotkeyDlg 消息处理程序


BOOL CWndWizHotkeyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_htMainPanel.SetHotKey(m_wMainPanel2, m_wMainPanel1);
	m_htBackBegin.SetHotKey(m_wBackBegin2, m_wBackBegin1);
	m_htBackEnd.SetHotKey(m_wBackEnd2, m_wBackEnd1);
	m_htEndFind.SetHotKey(m_wEndFind2, m_wEndFind1);
	m_htFindCpu.SetHotKey(m_wFindCpu2, m_wFindCpu1);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWndWizHotkeyDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
	m_htMainPanel.GetHotKey(m_wMainPanel2, m_wMainPanel1);
	m_htBackBegin.GetHotKey(m_wBackBegin2, m_wBackBegin1);
	m_htBackEnd.GetHotKey(m_wBackEnd2, m_wBackEnd1);
	m_htEndFind.GetHotKey(m_wEndFind2, m_wEndFind1);
	m_htFindCpu.GetHotKey(m_wFindCpu2, m_wFindCpu1);
}
