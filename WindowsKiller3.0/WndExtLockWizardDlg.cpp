// WndExtLockWizardDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndExtLockWizardDlg.h"
#include "afxdialogex.h"

#define	CUR_NULL	0
#define	CUR_ALL		1
#define CUR_WE		2
#define CUR_NS		3
#define CUR_NWSE    4
#define CUR_NESW    5

#ifdef _DEBUG

enum Style {

	STY_SRCCOPY,
	STY_SRCPAINT,
	STY_SRCAND,
	STY_SRCINVERT,
	STY_SRCERASE,
	STY_NOTSRCCOPY,
	STY_NOTSRCERASE,
	STY_MERGECOPY,
	STY_MERGEPAINT,
	STY_PATCOPY,
	STY_PATPAINT,
	STY_PATINVERT,
	STY_DSTINVERT,
	STY_BLACKNESS,
	STY_WHITENESS,
	STY_NOMIRROR,
	STY_CAPTUREBLT,
	STY_ALL
};

struct	StyleStr {
	DWORD	mStyle;
	DWORD	mRaster;
	LPSTR	mpStr;
}mStyleStr[] = {

	{ STY_SRCCOPY     ,SRCCOPY		,"SRCCOPY		" },
{ STY_SRCPAINT    ,SRCPAINT		,"SRCPAINT		" },
{ STY_SRCAND      ,SRCAND		,"SRCAND		" },
{ STY_SRCINVERT   ,SRCINVERT		,"SRCINVERT		" },
{ STY_SRCERASE    ,SRCERASE		,"SRCERASE		" },
{ STY_NOTSRCCOPY  ,NOTSRCCOPY	,"NOTSRCCOPY	" },
{ STY_NOTSRCERASE ,NOTSRCERASE	,"NOTSRCERASE	" },
{ STY_MERGECOPY   ,MERGECOPY	 	,"MERGECOPY	 	" },
{ STY_MERGEPAINT  ,MERGEPAINT	,"MERGEPAINT	" },
{ STY_PATCOPY     ,PATCOPY		,"PATCOPY		" },
{ STY_PATPAINT    ,PATPAINT		,"PATPAINT		" },
{ STY_PATINVERT   ,PATINVERT		,"PATINVERT		" },
{ STY_DSTINVERT   ,DSTINVERT		,"DSTINVERT		" },
{ STY_BLACKNESS   ,BLACKNESS		,"BLACKNESS		" },
{ STY_WHITENESS	 ,WHITENESS		,"WHITENESS		" },
{ STY_NOMIRROR	 ,NOMIRRORBITMAP,"NOMIRRORBITMAP" },
{ STY_CAPTUREBLT	 ,CAPTUREBLT	,"CAPTUREBLT	" },

};

static	int		mStyle = STY_SRCAND;
static	short	r = 190, g = 255, b = 255;

#endif

// CWndExtLockWizardDlg 对话框

IMPLEMENT_DYNAMIC(CWndExtLockWizardDlg, CDialog)

CWndExtLockWizardDlg::CWndExtLockWizardDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LOCKWIZARD_DIALOG, pParent)
{
	CDC cDC;

	HWND	hDWnd = ::GetDesktopWindow();
	HDC		hDC = ::GetDC(hDWnd);

	cDC.Attach(hDC);

	::GetWindowRect(hDWnd, &m_dRect);

	m_memHDC.CreateCompatibleDC(&cDC);
	m_DeskDC.CreateCompatibleDC(&cDC);
	m_hWndDC.CreateCompatibleDC(&cDC);
	m_CalHDC.CreateCompatibleDC(&cDC);

	m_memHMap.CreateCompatibleBitmap(&cDC, m_dRect.Width() * 2, m_dRect.Height() * 2);
	m_DeskMap.CreateCompatibleBitmap(&cDC, m_dRect.Width() * 2, m_dRect.Height() * 2);
	m_hWndMap.CreateCompatibleBitmap(&cDC, m_dRect.Width() * 2, m_dRect.Height() * 2);
	m_CalcMap.CreateCompatibleBitmap(&cDC, m_dRect.Width() * 2, m_dRect.Height() * 2);

	m_DeskDC.SelectObject(&m_DeskMap);
	m_hWndDC.SelectObject(&m_hWndMap);
	m_memHDC.SelectObject(&m_memHMap);
	m_CalHDC.SelectObject(&m_CalcMap);

	cDC.Detach();

	::ReleaseDC(hDWnd, hDC);

	m_cPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
#ifdef _DEBUG
	m_cBrush.CreateSolidBrush(RGB(r, g, b));
#else
	m_cBrush.CreateSolidBrush(RGB(190, 255, 255));
#endif
	m_cBrishPos.CreateSolidBrush(RGB(255, 0, 255));


	m_CurStyle = CUR_NULL;
	m_OppStyle = CUR_NULL;

	m_memHDC.SelectObject(&m_cPen);

	m_memHDC.SetBkMode(TRANSPARENT);
	m_memHDC.SetTextColor(RGB(255, 0, 255));
	m_memHDC.SetDCPenColor(RGB(255, 255, 255));
	m_memHDC.SetStretchBltMode(STRETCH_HALFTONE);

	m_pRLockWizardDlg = NULL;
}

CWndExtLockWizardDlg::~CWndExtLockWizardDlg()
{
}

BOOL CWndExtLockWizardDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndExtract.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
	/*
	CRect	cRect = m_CRect;
	AdjustWindowRect(&cRect,WS_CAPTION|WS_BORDER,FALSE);

	DLGTEMPLATE* pDlgTemplate = (DLGTEMPLATE*)lpDialogTemplate;
	printf("pDlgTemplate = {%d,%d,%d,%d}\n",lpDialogTemplate->x,lpDialogTemplate->y,lpDialogTemplate->cy,lpDialogTemplate->cy);
	pDlgTemplate->x		=	cRect.left;
	pDlgTemplate->y		=	cRect.top;
	pDlgTemplate->cx	=	cRect.Width();
	pDlgTemplate->cy	=	cRect.Height();
	printf("pDlgTemplate = {%d,%d,%d,%d}\n",lpDialogTemplate->x,lpDialogTemplate->y,lpDialogTemplate->cy,lpDialogTemplate->cy);
	*/
	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndExtLockWizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWndExtLockWizardDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDCANCEL, &CWndExtLockWizardDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDOK, &CWndExtLockWizardDlg::OnBnClickedButtonOk)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
#ifdef _DEBUG
	ON_WM_MOUSEWHEEL()
#endif
	ON_WM_CONTEXTMENU()

END_MESSAGE_MAP()


// CWndExtLockWizardDlg 消息处理程序

BOOL CWndExtLockWizardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: 在此添加额外的初始化代码

	m_cDlgBack.CreateSolidBrush(RGB(0, 0, 0));
	m_DeskDC.SelectObject(&m_cDlgBack);

	m_memHDC.SelectObject(this->GetFont());

	m_cMenu.Attach(::LoadMenu(::GetModuleHandle(NULL/*TEXT("WndExtract.dll")*/), LPCTSTR(IDR_MENU_FREE)));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWndExtLockWizardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndExtLockWizardDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CWnd::OnCommand(wParam, lParam);
}
bool CWndExtLockWizardDlg::WizardWndRect(HWND hCWnd, HWND hBWnd)
{
	if (::IsWindow(hCWnd))
	{
		m_hCWnd = hCWnd;
		m_hBWnd = hBWnd;

		HWND hDWnd = ::GetDesktopWindow();

		::GetWindowRect(hCWnd, &m_sRect);
		::GetWindowRect(hCWnd, &m_wRect);
		::GetWindowRect(hDWnd, &m_dRect);
		::GetClientRect(hBWnd, &m_bRect);

		::ClientToScreen(hBWnd, LPPOINT(&m_bRect));
		m_bRect.right += m_bRect.left;
		m_bRect.bottom += m_bRect.top;


		::PrintWindow(hCWnd, m_hWndDC, NULL);

		CDC	dDC;
		HDC	hDC = ::GetDC(hDWnd);

		dDC.Attach(hDC);
		m_DeskDC.BitBlt(0, 0, m_dRect.Width(), m_dRect.Height(), &dDC, 0, 0, SRCCOPY);
		m_DeskDC.Rectangle(m_bRect.left, m_bRect.top, m_bRect.right, m_bRect.bottom);
		dDC.Detach();

		::ReleaseDC(hDWnd, hDC);

		m_CalHDC.BitBlt(0, 0, m_sRect.Width(), m_sRect.Height(), &m_hWndDC, 0, 0, SRCCOPY);

		::MoveWindow(m_hWnd, 0, 0, m_dRect.Width(), m_dRect.Height(), TRUE);
		::ShowWindow(m_hWnd, SW_SHOW);
		::BringWindowToTop(m_hWnd);

		this->UpDateWindowPaint();

		return true;
	}
	return false;
}
void CWndExtLockWizardDlg::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	dc.BitBlt(0, 0, m_dRect.Width(), m_dRect.Height(), &m_memHDC, 0, 0, SRCCOPY);
	//CDialog::OnPaint();
}

void CWndExtLockWizardDlg::OnBnClickedButtonCancel()
{
	::AdjustWindowRect(&m_bRect, WS_CAPTION | WS_BORDER, FALSE);
	::MoveWindow(m_hBWnd, m_bRect.left, m_bRect.top, m_bRect.Width(), m_bRect.Height(), TRUE);

	this->ShowWindow(SW_HIDE);
}
void CWndExtLockWizardDlg::OnBnClickedButtonOk()
{
	::AdjustWindowRect(&m_bRect, WS_CAPTION | WS_BORDER, FALSE);
	::MoveWindow(m_hBWnd, m_bRect.left, m_bRect.top, m_bRect.Width(), m_bRect.Height(), TRUE);

	if (m_pRLockWizardDlg != NULL)
	{
		m_pRLockWizardDlg->OnLockRect(&m_sRect);
	}

	this->ShowWindow(SW_HIDE);
}

void CWndExtLockWizardDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
}

void CWndExtLockWizardDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码

}

void CWndExtLockWizardDlg::OnTimer(UINT_PTR nIDEvent)
{
	__super::OnTimer(nIDEvent);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

}
void CWndExtLockWizardDlg::UpDateWindowPaint()
{
	CWindowDC dc(this); // 用于绘制的设备上下文

	m_memHDC.SelectObject(&m_cBrush);

#ifdef _DEBUG
	//m_memHDC.Rectangle(0,0,m_dRect.Width(),m_dRect.Height());
	m_memHDC.BitBlt(0, 0, m_dRect.Width(), m_dRect.Height(), &m_DeskDC, 0, 0, mStyleStr[mStyle].mRaster);

#else
	m_memHDC.Rectangle(0, 0, m_dRect.Width(), m_dRect.Height());
	m_memHDC.BitBlt(0, 0, m_dRect.Width(), m_dRect.Height(), &m_DeskDC, 0, 0, MERGECOPY);
#endif

	m_memHDC.BitBlt(m_sRect.left, m_sRect.top, m_sRect.Width(), m_sRect.Height(),
		&m_DeskDC, m_sRect.left, m_sRect.top, SRCCOPY);


	CRect	pRect;
	pRect.left = max(m_sRect.left, m_bRect.left);
	pRect.top = max(m_sRect.top, m_bRect.top);
	pRect.right = min(m_sRect.right, m_bRect.right);
	pRect.bottom = min(m_sRect.bottom, m_bRect.bottom);

	CPoint pPoint;
	pPoint.x = m_sRect.left>m_bRect.left ? 0 : m_bRect.left - m_sRect.left;
	pPoint.y = m_sRect.top>m_bRect.top ? 0 : m_bRect.top - m_sRect.top;

	if (pRect.Width() > 0 && pRect.Height() > 0)
	{
		m_memHDC.BitBlt(pRect.left, pRect.top, pRect.Width(), pRect.Height(),
			&m_CalHDC, pPoint.x, pPoint.y, SRCCOPY);
	}


#ifdef _DEBUG
	{
		CString	srt;
		srt.Format(TEXT("Color(%d,%d,%d) mRaster = %s,m_OppStyle = %d,m_OffPos = {%d,%d}"), r, g, b, mStyleStr[mStyle].mpStr
			, m_OppStyle, m_OffPos.x, m_OffPos.y);
		m_memHDC.TextOut(0, 0, LPCTSTR(srt), srt.GetLength());
	}
#endif


	m_memHDC.MoveTo(m_sRect.left, m_sRect.top);
	m_memHDC.LineTo(m_sRect.right - 1, m_sRect.top);
	m_memHDC.LineTo(m_sRect.right - 1, m_sRect.bottom - 1);
	m_memHDC.LineTo(m_sRect.left, m_sRect.bottom - 1);
	m_memHDC.LineTo(m_sRect.left, m_sRect.top);

	m_memHDC.SelectObject(&m_cBrishPos);
	//m_memHDC.SetDCBrushColor(RGB(255,0,255));
	m_memHDC.Rectangle(m_sRect.left - 3, m_sRect.top - 3, m_sRect.left + 3, m_sRect.top + 3);
	m_memHDC.Rectangle(m_sRect.right - 3, m_sRect.top - 3, m_sRect.right + 3, m_sRect.top + 3);
	m_memHDC.Rectangle(m_sRect.left - 3, m_sRect.bottom - 3, m_sRect.left + 3, m_sRect.bottom + 3);
	m_memHDC.Rectangle(m_sRect.right - 3, m_sRect.bottom - 3, m_sRect.right + 3, m_sRect.bottom + 3);

	m_memHDC.Rectangle(m_sRect.left + m_sRect.Width() / 2 - 3, m_sRect.top - 3, m_sRect.left + m_sRect.Width() / 2 + 3, m_sRect.top + 3);
	m_memHDC.Rectangle(m_sRect.left + m_sRect.Width() / 2 - 3, m_sRect.bottom - 3, m_sRect.left + m_sRect.Width() / 2 + 3, m_sRect.bottom + 3);
	m_memHDC.Rectangle(m_sRect.left - 3, m_sRect.top + m_sRect.Height() / 2 - 3, m_sRect.left + 3, m_sRect.top + m_sRect.Height() / 2 + 3);
	m_memHDC.Rectangle(m_sRect.right - 3, m_sRect.top + m_sRect.Height() / 2 - 3, m_sRect.right + 3, m_sRect.top + m_sRect.Height() / 2 + 3);

	m_memHDC.TextOut(m_sRect.left, m_sRect.top - 20, LPCTSTR(m_szSize), m_szSize.GetLength());

	dc.BitBlt(0, 0, m_dRect.Width(), m_dRect.Height(), &m_memHDC, 0, 0, SRCCOPY);
}
void CWndExtLockWizardDlg::CalculateWindowRect(CPoint &point)
{
	CRect	tRect = m_sRect;
	switch (m_OppStyle)
	{
	case CUR_ALL:
		tRect.right -= tRect.left;
		tRect.bottom -= tRect.top;

		tRect.left = point.x - m_OffPos.x;
		tRect.top = point.y - m_OffPos.y;

		if (!AdsorptionVariable(&tRect.left, 0, 8)
			&& !AdsorptionVariable(&tRect.left, m_bRect.left, 8)
			&& !AdsorptionVariable(&tRect.left, m_bRect.right - tRect.right, 8))
			AdsorptionVariable(&tRect.left, GetSystemMetrics(SM_CXSCREEN) - tRect.right);


		if (!AdsorptionVariable(&tRect.top, 0, 8)
			&& !AdsorptionVariable(&tRect.top, m_bRect.top, 8)
			&& !AdsorptionVariable(&tRect.top, m_bRect.bottom - tRect.bottom, 8))
			AdsorptionVariable(&tRect.top, GetSystemMetrics(SM_CYSCREEN) - tRect.bottom);


		tRect.right += tRect.left;
		tRect.bottom += tRect.top;

		break;
	case CUR_WE:
		if (m_OffPos.x == 0)
		{
			if (point.x > tRect.right - 1)
				point.x = tRect.right - 1;

			tRect.left = point.x;
		}
		else
		{
			if (point.x < tRect.left + 1)
				point.x = tRect.left + 1;

			tRect.right = point.x;
		}

		break;
	case CUR_NS:
		if (m_OffPos.y == 0)
		{
			if (point.y > tRect.bottom - 1)
				point.y = tRect.bottom - 1;

			tRect.top = point.y;
		}
		else
		{
			if (point.y < tRect.top + 1)
				point.y = tRect.top + 1;

			tRect.bottom = point.y;
		}

		break;
	case CUR_NWSE:
		if (m_OffPos.y != 0)
		{
			if (point.x < tRect.left + 1)
				point.x = tRect.left + 1;

			if (point.y < tRect.top + 1)
				point.y = tRect.top + 1;

			tRect.right = point.x,
				tRect.bottom = point.y;
		}
		else
		{
			if (point.x > tRect.right - 1)
				point.x = tRect.right - 1;

			if (point.y > tRect.bottom - 1)
				point.y = tRect.bottom - 1;

			tRect.left = point.x,
				tRect.top = point.y;
		}
		break;
	case CUR_NESW:
		if (m_OffPos.y != 0)
		{
			if (point.x > tRect.right - 1)
				point.x = tRect.right - 1;
			if (point.y < tRect.top + 1)
				point.y = tRect.top + 1;
			tRect.left = point.x,
				tRect.bottom = point.y;
		}
		else
		{
			if (point.x < tRect.left + 1)
				point.x = tRect.left + 1;
			if (point.y > tRect.bottom - 1)
				point.y = tRect.bottom - 1;
			tRect.right = point.x,
				tRect.top = point.y;
		}

		break;
	default:m_OppStyle = CUR_NULL;
		return;
	}

	if (m_OppStyle != CUR_NULL && m_OppStyle != CUR_ALL)
	{
		if (!AdsorptionVariable(&tRect.top, 0))
			AdsorptionVariable(&tRect.top, m_bRect.top);

		if (!AdsorptionVariable(&tRect.left, 0))
			AdsorptionVariable(&tRect.left, m_bRect.left);

		if (!AdsorptionVariable(&tRect.bottom, m_bRect.bottom))
			AdsorptionVariable(&tRect.bottom, GetSystemMetrics(SM_CYSCREEN));

		if (!AdsorptionVariable(&tRect.right, m_bRect.right))
			AdsorptionVariable(&tRect.right, GetSystemMetrics(SM_CXSCREEN));


		CRect ttRect = tRect;

		::MoveWindow(m_hCWnd, 0, 0, tRect.Width(), tRect.Height(), FALSE);
		::GetWindowRect(m_hCWnd, &ttRect);

		if (m_OffPos.y != 0)
			tRect.bottom = tRect.top + ttRect.Height();
		else
			tRect.top = tRect.bottom - ttRect.Height();

		if (m_OffPos.x != 0)
			tRect.right = tRect.left + ttRect.Width();
		else
			tRect.left = tRect.right - ttRect.Width();

		::AdjustWindowRect(&ttRect, WS_CAPTION | WS_BORDER, FALSE);
		::MoveWindow(m_hBWnd, 0, 0, ttRect.Width(), ttRect.Height(), FALSE);
		::MoveWindow(m_hCWnd, 0, 0, tRect.Width(), tRect.Height(), FALSE);

		::PrintWindow(m_hCWnd, m_CalHDC, NULL);
	}

	m_sRect = tRect;
	m_szSize.Format(TEXT("[%d X %d]"), m_sRect.Width(), m_sRect.Height());
	this->UpDateWindowPaint();
}
void CWndExtLockWizardDlg::CursorStyleCheck(bool bl, bool bt, bool br, bool bb, CPoint &point)
{
	if ((bl && bt) || (br && bb))
	{
		m_OffPos.x = LONG(br);
		m_OffPos.y = LONG(bb);

		::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
		m_CurStyle = CUR_NWSE;
	}
	else if ((bl && bb) || (bt && br))
	{
		m_OffPos.x = LONG(br);
		m_OffPos.y = LONG(bb);

		::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
		m_CurStyle = CUR_NESW;
	}
	else if (m_sRect.PtInRect(point))
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		m_CurStyle = CUR_ALL;
	}
	else if (point.y > m_sRect.top && point.y < m_sRect.bottom)
	{
		if (bl || br)
		{
			if (br == true)
				m_OffPos.x = 1,
				m_OffPos.y = -1;
			else
				m_OffPos.x = 0,
				m_OffPos.y = -1;

			::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
			m_CurStyle = CUR_WE;
		}
	}
	else if (point.x > m_sRect.left && point.x < m_sRect.right)
	{
		if (bt || bb)
		{
			if (bb == true)
				m_OffPos.x = -1,
				m_OffPos.y = 1;
			else
				m_OffPos.x = -1,
				m_OffPos.y = 0;

			::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
			m_CurStyle = CUR_NS;
		}
	}

	if (m_CurStyle != CUR_NULL)
	{
		::SetCapture(m_hWnd);
	}
}
void CWndExtLockWizardDlg::CursorStyleCancel(bool bl, bool bt, bool br, bool bb, CPoint &point)
{
	switch (m_CurStyle)
	{
	case CUR_ALL:
		if (!m_sRect.PtInRect(point))
		{
			m_CurStyle = CUR_NULL;
		}
		break;
	case CUR_WE:
		if (point.y < m_sRect.top || point.y > m_sRect.bottom || (!bl && !br))
		{
			m_CurStyle = CUR_NULL;
		}
		break;
	case CUR_NS:
		if (point.x < m_sRect.left || point.x > m_sRect.right || (!bt && !bb))
		{
			m_CurStyle = CUR_NULL;
		}
		break;
	case CUR_NWSE:
		if ((!bl || !bt) && (!br || !bb))
		{
			m_CurStyle = CUR_NULL;
		}
		break;
	case CUR_NESW:
		if ((!bl || !bb) && (!bt || !br))
		{
			m_CurStyle = CUR_NULL;
		}
		break;
	}
	if (m_CurStyle == CUR_NULL)
	{
		::ReleaseCapture();
	}
}


void CWndExtLockWizardDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_OppStyle != CUR_NULL)
	{
		this->CalculateWindowRect(point);
		return;
	}

	bool bl = point.x < m_sRect.left + 4 && point.x > m_sRect.left - 4;
	bool br = point.x < m_sRect.right + 4 && point.x > m_sRect.right - 4;
	bool bt = point.y < m_sRect.top + 4 && point.y > m_sRect.top - 4;
	bool bb = point.y < m_sRect.bottom + 4 && point.y > m_sRect.bottom - 4;

	if (m_CurStyle == CUR_NULL)
	{
		this->CursorStyleCheck(bl, bt, br, bb, point);
	}
	else
	{
		this->CursorStyleCancel(bl, bt, br, bb, point);
	}
}

void CWndExtLockWizardDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonUp(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_OffPos.x = 0;
	m_OffPos.y = 0;
	m_CurStyle = CUR_NULL;
	m_OppStyle = CUR_NULL;
	::ReleaseCapture();
}

void CWndExtLockWizardDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_CurStyle != CUR_NULL)
	{
		if (m_CurStyle == CUR_ALL)
		{
			m_OffPos.x = point.x - m_sRect.left;
			m_OffPos.y = point.y - m_sRect.top;
		}
		m_OppStyle = m_CurStyle;
	}
#ifdef _DEBUG
	else
	{
		mStyle = (mStyle + 1) % STY_ALL;
		this->UpDateWindowPaint();
	}
#endif
}



bool CWndExtLockWizardDlg::SetRLockWizardDlg(RLockWizardDlg* pRLockWizardDlg)
{
	if (pRLockWizardDlg != NULL && pRLockWizardDlg->OnCheckRLockWizardDlg())
	{
		m_pRLockWizardDlg = pRLockWizardDlg;
		return true;
	}
	return false;
}
void CWndExtLockWizardDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	m_cMenu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
}


#ifdef _DEBUG
BOOL CWndExtLockWizardDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_cBrush.DeleteObject();


	if (pt.x < 455)
	{
		r += zDelta / 120;
		if (r < 0)r = 0;
		else if (r > 255)r = 255;
	}
	else if (pt.x < 910)
	{
		g += zDelta / 120;
		if (g < 0)g = 0;
		else if (g > 255)g = 255;
	}
	else
	{
		b += zDelta / 120;
		if (b < 0)b = 0;
		else if (b > 255)b = 255;
	}


	m_cBrush.CreateSolidBrush(RGB(r, g, b));


	this->UpDateWindowPaint();

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}
#endif
BOOL CWndExtLockWizardDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (pMsg->message == WM_KEYDOWN)
	{
		UINT	vk = UINT(pMsg->wParam);
#ifdef	_DEBUG
		UINT	cRepeat = UINT((short)LOWORD(pMsg->lParam));
		UINT	flags = UINT(HIWORD(pMsg->lParam));
#endif

		if (vk > VK_LEFT - 1 && vk < VK_DOWN + 1)
		{
			m_sRect.right -= m_sRect.left;
			m_sRect.bottom -= m_sRect.top;

			LPLONG	lpLong = LPLONG(&m_sRect);

			lpLong[!(vk & 1)] += 1 - 2 * (vk < VK_RIGHT);

			m_sRect.right += m_sRect.left;
			m_sRect.bottom += m_sRect.top;

			this->UpDateWindowPaint();
		}
#ifdef	_DEBUG
		CString	szStr;
		szStr.Format(TEXT("CWndExtLockWizardDlg::OnKeyDown(%d,%d,%d)             "), vk, cRepeat, flags);

		CWindowDC	cDC(this);
		cDC.SelectObject(this->GetFont());
		cDC.TextOut(0, 0, szStr, szStr.GetLength());
#endif
	}
	return CDialog::PreTranslateMessage(pMsg);
}
bool CWndExtLockWizardDlg::AdsorptionVariable(long *Var, long Ads, long offset)
{
	if (Var[0] >= Ads - offset && Var[0] <= Ads + offset)
	{
		Var[0] = Ads;
		return true;
	}
	return false;
}
