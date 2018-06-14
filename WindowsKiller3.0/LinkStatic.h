#pragma once


class CLinkStatic : public CStatic
{
public:
	CLinkStatic();
	virtual ~CLinkStatic();

public:
	void SetTextColor(COLORREF textcolor, COLORREF hovercolor);
	void SetToolTipText(CString str);
	void SetLinkStaticCursor(HCURSOR hCursor);

	//{{AFX_VIRTUAL(CLinkStatic)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();

protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

public:
	BOOL			m_bLinkStatic;

protected:
	BOOL			m_bOver;
	HCURSOR			m_hLinkCursor;
	CToolTipCtrl	m_ToolTip;

protected:
	COLORREF clr_hoverColor;
	COLORREF clr_Textcolor;

	//{{AFX_MSG(CLinkStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

