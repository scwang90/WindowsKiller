#pragma once


#define WM_NOTIFYICON WM_USER+1

class CMainTray : public CObject
{
public:
	~CMainTray()
	{
		DelTrayIcon();
	}
	CMainTray();
public:
	void SetOwner(HWND _hOwner)
	{
		if (m_IsCreated)return;
		m_NotifyData.hWnd = _hOwner;
		m_IsComplete |= 0x00000001;
	}
	void SetIcoid(UINT _hIcoid)
	{
		if (m_IsCreated)return;
		m_NotifyData.uID = _hIcoid;
		m_IsComplete |= 0x00000002;
	}
	void SetHicon(HICON _hIcon)
	{
		if (m_IsCreated)return;
		m_NotifyData.hIcon = _hIcon;
		m_IsComplete |= 0x00000004;
	}
	void SetTipText(LPCTSTR _lpTipStr)
	{
		if (m_IsCreated)return;
		lstrcpy(m_NotifyData.szTip, _lpTipStr);
		m_IsComplete |= 0x00000008;
	}
	void SetMessage(UINT _CallbackMessage)
	{
		if (m_IsCreated)return;
		m_NotifyData.uCallbackMessage = _CallbackMessage;
	}
	BOOL AddTrayIcon()
	{
		if (!m_IsCreated && ((m_IsComplete & 0x0000000F) == 0x0000000F))
			return BOOL(m_IsCreated = !!Shell_NotifyIcon(NIM_ADD, (PNOTIFYICONDATA)&m_NotifyData));
		else
			return FALSE;
	}
	BOOL DelTrayIcon()
	{
		if (!m_IsCreated)return FALSE;
		return !BOOL(m_IsCreated = !Shell_NotifyIcon(NIM_DELETE, (PNOTIFYICONDATA)&m_NotifyData));
	}
	BOOL ChangeTip(LPCTSTR newTipText)
	{
		if (!m_IsCreated)return FALSE;
		m_NotifyData.uFlags = NIF_TIP;
		lstrcpy(m_NotifyData.szTip, newTipText);
		return Shell_NotifyIcon(NIM_MODIFY, (PNOTIFYICONDATA)&m_NotifyData);
	}
	BOOL ChangeIcon(HICON hNewIcon)
	{
		if (!m_IsCreated || !hNewIcon)return FALSE;
		m_NotifyData.hIcon = hNewIcon;
		m_NotifyData.uFlags = NIF_ICON;
		return Shell_NotifyIcon(NIM_MODIFY, (PNOTIFYICONDATA)&m_NotifyData);
	}
public:
	BOOL ShowBalloon(LPCTSTR title, LPCTSTR text);
	void SetTrayInfo(HWND _hOwner, UINT _hIcoid, HICON _hIcon = 0,
		LPCTSTR _lpTipStr = 0, UINT _CallbackMessage = 0);
private:
	NOTIFYICONDATA m_NotifyData;

private:
	bool	m_IsCreated;
	BOOL	m_IsComplete;
};


