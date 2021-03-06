#pragma once


// CWndWizFindOptionDlg 对话框

class CWndWizFindOptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CWndWizFindOptionDlg)

public:
	CWndWizFindOptionDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndWizFindOptionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FINDOPTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	// modal processing
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	// 实现
protected:
	HICON	m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

protected:
	DECLARE_MESSAGE_MAP()

public:
	UINT	m_uFlag;

public:
	void	SetFindOption(UINT flag);

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonOkCancel();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

};
