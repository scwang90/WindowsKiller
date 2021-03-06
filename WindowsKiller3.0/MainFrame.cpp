// MainFrame.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "MainFrame.h"

#include <locale.h>


#define FM_NOTIFYMSG (WM_USER+100)


static const DWORD WM_TASKBARCREATED = RegisterWindowMessage(_T("TaskbarCreated")); //explorer进程重建消息

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

CMainFrame::CMainFrame() : CMainFrame(false){
}
CMainFrame::CMainFrame(BOOL IsBackstage) :CFrameWndEx(), m_bIsBackstage(IsBackstage)
{
	m_pIWndModule = NULL;
	m_pIViewWizard = NULL;
	m_pIFileEngine = NULL;
	m_pIWndPreview = NULL;

	m_TrayMenu.LoadMenu(IDR_MENU_TRAY);

	CMenu *pMenu = m_TrayMenu.GetSubMenu(0);
	m_BmpMenuQuit.LoadBitmap(IDB_MENU_QUIT);
	pMenu->SetMenuItemBitmaps(ID_MAIN_QUIT, MF_BYCOMMAND, &m_BmpMenuQuit, &m_BmpMenuQuit);


#ifdef _DEBUG
	CMainFrame::OnCommand(MAKEWPARAM(ID_MAIN_CONSOLE, 0), 0);
	_tprintf(TEXT("WM_TASKBARCREATED = %d\n"),WM_TASKBARCREATED);
#endif
}

CMainFrame::~CMainFrame()
{
	if (m_pIViewWizard != NULL)m_pIViewWizard->Release();
	if (m_pIFileEngine != NULL)m_pIFileEngine->Release();
	if (m_pIWndPreview != NULL)m_pIWndPreview->Release();
	if (m_pIWndModule != NULL)m_pIWndModule->Release();
}


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, CMainFrame::OnTaskBar)
	ON_MESSAGE(FM_NOTIFYMSG, CMainFrame::OnNotifyMsg)
	ON_MESSAGE(WM_NOTIFYICON, CMainFrame::OnNotifyIcon)
	ON_MESSAGE(WM_RESTARTKILLER, CMainFrame::OnReStartKiller)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CMainFrame 消息处理程序

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	//设置视觉管理器使用的视觉样式
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
	//设置用于绘制所有用户界面元素的视觉管理器
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

	try
	{
		if (FAILED(CWindowsKiller30App::DllCoCreateObject(CLSID_IWndWizard, IID_IViewWizard, (LPVOID *)&m_pIViewWizard)))
		{
			throw TEXT("Create CLSID_IWndWizard failed!\n");
		}
		if (FAILED(DllFileEngine::DllCoCreateObject(CLSID_IFileEngine, IID_IFileEngine, (LPVOID *)&m_pIFileEngine)))
		{
			throw TEXT("Create CLSID_IFileEngine failed!\n");
		}
		if (FAILED(CWindowsKiller30App::DllCoCreateObject(CLSID_IWndPreview, IID_IWndPreview, (LPVOID*)&m_pIWndPreview)))
		{
			throw TEXT("Create CLSID_IWndPreview failed!\n");
		}
		if (FAILED(CWindowsKiller30App::DllCoCreateObject(CLSID_IWndModule, IID_IWndModule, (LPVOID*)&m_pIWndModule)))
		{
			throw TEXT("Create CLSID_IWndModule failed!\n");
		}
	}
	catch (LPCTSTR lpError)
	{
		AfxMessageBox(lpError, MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}

	if (m_pIFileEngine != NULL)
	{
		m_pIFileEngine->OpenFile(TEXT("data.dat"));

#ifdef	_DEBUG
		printf("CMainFrame::OnCreate::OpenFile()\n");
		m_pIFileEngine->OutPutDataInfo();
#endif

		if (m_pIFileEngine->IsEmptyRecorde())
		{
			m_pIFileEngine->SetDepict(TEXT("窗口杀手3.0配置文件。"));
#ifdef _DEBUG
			printf("CMainFrame::OnCreate::m_pIFileEngine->IsEmptyRecorde()\n");
#endif
		}
	}

	m_pIViewWizard->SetBackStage(m_bIsBackstage == TRUE);
	m_pIWndModule->SetIFileEngine(m_pIFileEngine);
	m_pIWndPreview->SetIFileEngine(m_pIFileEngine);
	m_pIWndModule->CreateWndModule();
	m_pIWndPreview->CreateChildWndView();

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	this->SetIcon(m_hIcon, TRUE);			// 设置大图标
	this->SetIcon(m_hIcon, FALSE);			// 设置小图标
	this->SetWindowText(TEXT("WindowKiller III"));

	m_TrayIcon.SetTrayInfo(m_hWnd, 1000, m_hIcon, TEXT("窗口杀手3.0"));
	m_TrayIcon.AddTrayIcon();

	m_pIViewWizard->SetRViewWizard(this);
	m_pIViewWizard->SetIFileEngine(m_pIFileEngine);
	m_pIViewWizard->SetIWndPreview(m_pIWndPreview);
	m_pIViewWizard->SetIWndModule(m_pIWndModule);
	m_pIViewWizard->LaunchViewWizard();

	return 0;
}


BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	int nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));

	/*if(nID == ID_MAIN_HELP)
	{
	return CFrameWndEx::OnCommand(MAKEWPARAM(ID_HELP,0),0);
	}
	else */if (nID == ID_MAIN_QUIT)
	{
		return CFrameWndEx::OnCommand(MAKEWPARAM(ID_APP_EXIT, 0), 0);
	}
	else if (nID == ID_MAIN_CONSOLE)
	{
		CMenu	*pMenu = m_TrayMenu.GetSubMenu(0);
		if (pMenu->GetMenuState(nID, MF_BYCOMMAND))
		{
			::FreeConsole();
			pMenu->CheckMenuItem(nID, MF_UNCHECKED | MF_BYCOMMAND);
		}
		else
		{
			::AllocConsole();
			::SetConsoleTitle(TEXT("窗口杀手3.0"));
			FILE* file;
			::_tfreopen_s(&file,TEXT("CONOUT$"), TEXT("w+t"),stdout);
#ifdef UNICODE
			setlocale(LC_ALL,"Chs");
#endif // UNICODE


			HWND	hCwnd = ::GetConsoleWindow();
			HMENU	hMenu = ::GetSystemMenu(hCwnd, false);
			EnableMenuItem(hMenu, SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);

			CWnd	cWnd;
			cWnd.Attach(hCwnd);
			CMenu*	pCMenu = cWnd.GetSystemMenu(false);
			pCMenu->DestroyMenu();
			cWnd.Detach();

			pMenu->CheckMenuItem(nID, MF_CHECKED | MF_BYCOMMAND);
		}
		return 0;
	}
	else if (nID == ID_MAIN_PANEL)
	{
		HWND hCurWnd = m_pIViewWizard->GetCurrentWindow();

		::ShowWindow(hCurWnd, SW_NORMAL);
		::SwitchToThisWindow(hCurWnd, TRUE);

		return 0;
	}
	return CFrameWndEx::OnCommand(wParam, lParam);
}

LRESULT CMainFrame::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_RBUTTONUP:
	{
		CPoint cPoint;
		GetCursorPos(&cPoint);
		m_TrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTBUTTON, cPoint.x, cPoint.y, this, NULL);
	}
	break;
	case WM_LBUTTONDBLCLK:
	{
		CMainFrame::OnCommand(MAKEWPARAM(ID_MAIN_PANEL, 0), lParam);
	}
	break;
	}
	return 0;
}
LRESULT CMainFrame::OnNotifyMsg(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR	lpTitle = LPCTSTR(wParam);
	LPCTSTR	lpMessage = LPCTSTR(lParam);
	m_TrayIcon.ShowBalloon(lpTitle, lpMessage);
	return TRUE;
}
LRESULT CMainFrame::OnTaskBar(WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	printf("CMainFrame::OnTaskBar\n");
#endif
	m_TrayIcon.AddTrayIcon();
	return LONG(::DefWindowProc(m_hWnd, WM_TASKBARCREATED, wParam, lParam));
}
LRESULT CMainFrame::OnReStartKiller(WPARAM wParam, LPARAM lParam)
{
	if (!::IsIconic(m_pIViewWizard->GetCurrentWindow()))
	{
		m_pIViewWizard->LaunchViewWizard();
	}
	CMainFrame::OnCommand(MAKEWPARAM(ID_MAIN_PANEL, 0), lParam);
	return 0;
}
void CMainFrame::OnViewWizardEmpty()
{
	if (this->m_bIsBackstage == false) {
		CFrameWndEx::OnCommand(MAKEWPARAM(ID_APP_EXIT, 0), 0);
	}
}
bool CMainFrame::OnCheckRViewWizard()
{
	return true;
}

BOOL CMainFrame::Create()
{
	// TODO: 在此添加专用代码和/或调用基类

#ifdef _DEBUG
	return CFrameWndEx::Create(TEXT("#32768"), TEXT("WindowKiller"),0, CRect(200, 120, 640, 400));
#else
	return CFrameWndEx::Create(TEXT("#32768"), TEXT("WindowKiller"),0, CRect(0, 0, 0, 0));
#endif
}


void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: 在此处添加消息处理程序代码
}
