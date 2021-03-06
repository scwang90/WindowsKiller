// TrayIcon.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "MainTray.h"

CMainTray::CMainTray()
{
	m_IsCreated = false;
	m_IsComplete = FALSE;
	m_NotifyData.cbSize = sizeof(m_NotifyData);
	m_NotifyData.uCallbackMessage = WM_NOTIFYICON;
	//操作系统发出的trayicon消息
	m_NotifyData.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
}
void CMainTray::SetTrayInfo(
	HWND _hOwner, UINT _hIcoid,
	HICON _hIcon, LPCTSTR _lpTipStr,
	UINT _CallbackMessage)
{
	SetOwner(_hOwner);
	SetIcoid(_hIcoid);
	if (_hIcon)SetHicon(_hIcon);
	if (_lpTipStr)SetTipText(_lpTipStr);
	if (_CallbackMessage)SetMessage(_CallbackMessage);
}
BOOL CMainTray::ShowBalloon(LPCTSTR title, LPCTSTR text)
{
	if (!m_IsCreated)return FALSE;
	m_NotifyData.uFlags = NIF_INFO;
	m_NotifyData.dwInfoFlags = 0x01;
	//m_NotifyData.cbSize
	//m_NotifyData.DUMMYUNIONNAME.uTimeout=   (显示的毫秒数)
	lstrcpy(m_NotifyData.szInfoTitle, title);
	lstrcpy(m_NotifyData.szInfo, text);
	return Shell_NotifyIcon(NIM_MODIFY, (PNOTIFYICONDATA)&m_NotifyData);
}
