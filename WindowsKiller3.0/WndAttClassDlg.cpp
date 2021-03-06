// WndAttClassDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndAttClassDlg.h"
#include "afxdialogex.h"


// CWndAttClassDlg 对话框

IMPLEMENT_DYNAMIC(CWndAttClassDlg, CDialog)

CWndAttClassDlg::CWndAttClassDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_CLASS, pParent)
{

}

CWndAttClassDlg::~CWndAttClassDlg()
{
}

BOOL CWndAttClassDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndAttribute.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}

void CWndAttClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CLASSSTYLES, m_ComBoxStyle);

}

BEGIN_MESSAGE_MAP(CWndAttClassDlg, CDialog)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CWndAttClassDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndAttClassDlg::OnBnClickedButtonOkCancel)

END_MESSAGE_MAP()


// CWndAttClassDlg 消息处理程序

BOOL CWndAttClassDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndAttClassDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndAttClassDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));
	if (code == EN_SETFOCUS)
	{
		TCHAR	szClassName[128] = TEXT("");
		GetClassName(hCtl, szClassName, 128);
		if (lstrcmpi(szClassName, TEXT("edit")) == 0)
		{
			CEdit	cEdit;
			cEdit.Attach(hCtl);
			cEdit.SetSel(0, -1);
			cEdit.Detach();
		}
	}

	return CWnd::OnCommand(wParam, lParam);
}

void CWndAttClassDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值

}

void CWndAttClassDlg::OnBnClickedButtonOkCancel()
{
}
bool CWndAttClassDlg::SetIWndEngine(IWndEngine* pIWndEngine)
{
	if (pIWndEngine != NULL)
	{
		mPtrWndEngine = pIWndEngine;
		return true;
	}
	return false;
}
void CWndAttClassDlg::UpdateWindowInfo()
{
#ifdef _DEBUG
	ASSERT(mPtrWndEngine != NULL);
#endif
	static DWORD dwItemIds[] = {
		IDC_EDIT_CLASSNAME,//IDC_EDIT_CLASSSTYLE,
		IDC_EDIT_CLASSBYTES,
		IDC_EDIT_CLASSATOM,
		IDC_EDIT_CLASSINSTHANDLE,
		IDC_EDIT_CLASSWINBYTES,
		IDC_EDIT_CLASSWINPROC,
		IDC_EDIT_CLASSMENUNAME,
		IDC_EDIT_ICONHANDLE,
		IDC_EDIT_CLASSCURSOR,
		IDC_EDIT_CLASSBKGNDBRUSH,
		IDC_COMBO_CLASSSTYLES
	};

	static DWORD dwFunIds[] = {
		IWndEngine::CLS_NAME,//IWndEngine::CLS_STYLE,
		IWndEngine::CLS_BYTE,
		IWndEngine::CLS_ATOM,
		IWndEngine::CLS_INSTANCE,
		IWndEngine::CLS_WNDBYTE,
		IWndEngine::CLS_WINPROCESS,
		IWndEngine::CLS_MEUN,
		IWndEngine::CLS_ICON,
		IWndEngine::CLS_CURSOR,
		IWndEngine::CLS_HBRUSH
	};
	for (int i = 0, len = sizeof(dwFunIds) / sizeof(dwFunIds[0]); i<len; i++)
	{
		SetDlgItemText(dwItemIds[i], mPtrWndEngine->GetStageInfo(dwFunIds[i]));
	}
	CString	szClsStyle = mPtrWndEngine->GetStageInfo(IWndEngine::CLS_STYLE);

	szClsStyle += TEXT('|');
	int n = szClsStyle.Find('|');
	SetDlgItemText(IDC_EDIT_CLASSSTYLE, szClsStyle.Left(n));

	m_ComBoxStyle.ResetContent();
	for (int i = 0; (i = szClsStyle.Find(TEXT('|'), n)) != -1; n = i + 1)
	{
		if (i > n)
			m_ComBoxStyle.AddString(szClsStyle.Mid(n, i - n));
	}
	if (m_ComBoxStyle.GetCount() == 0)
		m_ComBoxStyle.AddString(TEXT("[无]"));
	m_ComBoxStyle.SetCurSel(0);
}
