#pragma once

#include "MainTray.h"

// CMainFrame 框架

class CMainFrame : public CFrameWndEx, public RViewWizard
{
	DECLARE_DYNCREATE(CMainFrame)
protected:
	CMainFrame();
	virtual ~CMainFrame();
public:
	CMainFrame(BOOL IsBackstage);           // 动态创建所使用的受保护的构造函数

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

protected:
	HICON		m_hIcon;
	CMainTray	m_TrayIcon;
	CMenu		m_TrayMenu;
	CBitmap		m_BmpMenuQuit;
	BOOL		m_bIsBackstage;

	// 实现
protected:
	IWndWizard * m_pIViewWizard;
	IFileEngine*	m_pIFileEngine;
	IWndPreview*	m_pIWndPreview;
	IWndModule*		m_pIWndModule;

public:
	BOOL Create();
	virtual void OnViewWizardEmpty();
	virtual bool OnCheckRViewWizard();

public:

	afx_msg	LRESULT OnNotifyMsg(WPARAM wParam, LPARAM lParam);
	afx_msg	LRESULT OnTaskBar(WPARAM wParam, LPARAM lParam);
	afx_msg	LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg	LRESULT OnReStartKiller(WPARAM wParam, LPARAM lParam);

	afx_msg int	 OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};

