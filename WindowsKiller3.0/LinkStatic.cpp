#include "stdafx.h"
#include "LinkStatic.h"


#define TIP_ID 1

CLinkStatic::CLinkStatic()
{
	m_bLinkStatic = false;
	m_bOver = false;
	m_hLinkCursor = NULL; //初值设置为NULL，这样如果没有在程序中设置该值，就会使用默认的手形光标
	clr_Textcolor = RGB(0, 0, 255);
	clr_hoverColor = RGB(255, 0, 0);
}

CLinkStatic::~CLinkStatic()
{
}

/////////////////////////////////////////////////////////////////////////////

BOOL CLinkStatic::DestroyWindow()
{
	KillTimer(1);
	return CStatic::DestroyWindow();
}

BOOL CLinkStatic::PreTranslateMessage(MSG* pMsg)
{
	m_ToolTip.RelayEvent(pMsg);
	return CStatic::PreTranslateMessage(pMsg);
}


void CLinkStatic::PreSubclassWindow()
{
	DWORD dwStyle = GetStyle(); //修改属性，使之可接受消息
	::SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_NOTIFY);

	m_hLinkCursor = ::LoadCursor(NULL, IDC_HAND);     //设置默认光标为手形光标

	CRect rect;
	GetClientRect(rect);
	m_ToolTip.Create(this);
	m_ToolTip.SetDelayTime(100);
	m_ToolTip.SetMaxTipWidth(200);

	m_ToolTip.AddTool(this, TEXT(""), rect, TIP_ID);
	CStatic::PreSubclassWindow();
}

BEGIN_MESSAGE_MAP(CLinkStatic, CStatic)
	//{{AFX_MSG_MAP(CLinkStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

HBRUSH CLinkStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	HBRUSH hBrush = FORWARD_WM_CTLCOLORDLG(m_hWnd, pDC->m_hDC, m_hWnd, ::DefWindowProc);

	if (m_bLinkStatic)
		pDC->SetTextColor(m_bOver ? clr_hoverColor : clr_Textcolor);

	return hBrush;
}

void CLinkStatic::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bOver == false)
	{
		m_bOver = true;
		Invalidate();
		SetTimer(1, 100, NULL); //设置一个定时器，用于设置鼠标离开时的颜色
	}
	CStatic::OnMouseMove(nFlags, point);
}

void CLinkStatic::OnTimer(UINT_PTR nIDEvent)
{
	CPoint pt(::GetMessagePos());
	ScreenToClient(&pt);

	CRect rc;
	GetClientRect(rc);

	if (!rc.PtInRect(pt))
	{
		m_bOver = false;
		KillTimer(1);
		Invalidate();
	}
	CStatic::OnTimer(nIDEvent);
}

BOOL CLinkStatic::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	if (m_bLinkStatic) //如果设置了光标，就使用新设置的 光标
		::SetCursor(m_hLinkCursor);
	return m_bLinkStatic;
}
/////////////////////////////////////////////////////////////////////////////
void CLinkStatic::SetToolTipText(CString str)
{
	m_ToolTip.UpdateTipText(str, this, TIP_ID);
}

void CLinkStatic::SetLinkStaticCursor(HCURSOR hCursor)
{
	m_hLinkCursor = hCursor;
	if (m_hLinkCursor == NULL)
		m_hLinkCursor = ::LoadCursor(NULL, IDC_HAND);
}

void CLinkStatic::SetTextColor(COLORREF textcolor, COLORREF hovercolor)
{
	clr_Textcolor = textcolor;
	clr_hoverColor = hovercolor;
}

