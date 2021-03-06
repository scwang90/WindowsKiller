#pragma once


// CWndWizHotkeyDlg 对话框

class CWndWizHotkeyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWndWizHotkeyDlg)

public:
	CWndWizHotkeyDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndWizHotkeyDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_HOTKEY };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CHotKeyCtrl m_htBackBegin;
	CHotKeyCtrl m_htMainPanel;
	CHotKeyCtrl m_htBackEnd;
	CHotKeyCtrl m_htEndFind;
	CHotKeyCtrl m_htFindCpu;

	WORD	m_wMainPanel1;
	WORD	m_wMainPanel2;
	WORD	m_wBackBegin1;
	WORD	m_wBackBegin2;
	WORD	m_wBackEnd1;
	WORD	m_wBackEnd2;
	WORD	m_wEndFind1;
	WORD	m_wEndFind2;
	WORD	m_wFindCpu1;
	WORD	m_wFindCpu2;

public:
	afx_msg void OnBnClickedOk();
	virtual INT_PTR DoModal();
};
