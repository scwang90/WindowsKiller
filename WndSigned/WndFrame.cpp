#include "StdAfx.h"
#include "WndFrame.h"

#include <stdio.h>
//#include <tchar.h>

// Global Variables:
HINSTANCE hInst;							// current instance
TCHAR szTitle[] = TEXT("");					// The title bar text
TCHAR szWindowClass[] = TEXT("SignWindow");		// the class name

DWORD	g_SignColor = RGB(255, 0, 0);
HPEN	g_WhitePen = NULL;
HPEN	g_hSignPen = NULL;
BOOL	g_blShow = FALSE;
ULONG	g_WndFrameNumber = 0;
BOOL	g_bIsSetRect = FALSE;

// Foward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
void OnWinDraw(HWND hwnd);
void OnMove(HWND hwnd, int x, int y);
void OnSize(HWND hwnd, UINT state, int cx, int cy);
void OnTimer(HWND hwnd, UINT id);
void OnDestroy(HWND hwnd);

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. 
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.lpszMenuName = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = GetStockBrush(WHITE_BRUSH);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowEx(WS_EX_TOPMOST/*|WS_EX_LAYERED*/ | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE/*|WS_EX_TRANSPARENT*/,
		szWindowClass, szTitle, WS_POPUP, CW_USEDEFAULT,
		0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_PAINT, OnWinDraw);
		HANDLE_MSG(hWnd, WM_CTLCOLOREDIT, OnCtlColor);
		HANDLE_MSG(hWnd, WM_TIMER, OnTimer);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_MOVE, OnMove);
		HANDLE_MSG(hWnd, WM_SIZE, OnSize);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	g_WndFrameNumber++;

#ifdef _DEBUG
	printf("SignWnd::OnCreate::g_WndFrameNumber = %d\n", g_WndFrameNumber);
#endif
	if (g_WndFrameNumber == 1)
	{
		g_WhitePen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
		g_hSignPen = CreatePen(PS_SOLID, 4, g_SignColor);
	}

	//	BOOL ret = SetLayeredWindowAttributes(hwnd,RGB(255,255,255),0,LWA_COLORKEY);
	//
	//#ifdef _DEBUG
	//		printf("SetLayeredWindowAttributes = %d \n",ret);
	//#endif
	//
	//	if(ret == FALSE)
	//	{
	//		AfxMessageBox(::GetLastErrorInfo("Call SetLayeredWindowAttributes Fail!"));
	//	}

	return TRUE;
}


void OnWinDraw(HWND hwnd)
{
	PAINTSTRUCT ps;
	RECT	rect;
	HDC		hdc = BeginPaint(hwnd, &ps);

	GetWindowRect(hwnd, &rect);

	SelectObject(hdc, g_blShow ? g_hSignPen : g_WhitePen);
	Rectangle(hdc, 1, 1, rect.right - rect.left, rect.bottom - rect.top);

	EndPaint(hwnd, &ps);
}
void OnTimer(HWND hwnd, UINT id)
{
	if (IDT_SHOW == id)
	{
		RECT cRect;
		HDC	hdc = GetDC(hwnd);

		GetWindowRect(hwnd, &cRect);

		SelectObject(hdc, g_blShow ? g_hSignPen : g_WhitePen);
		Rectangle(hdc, 1, 1, cRect.right - cRect.left, cRect.bottom - cRect.top);
		g_blShow = !g_blShow;

		ReleaseDC(hwnd, hdc);

		//#ifdef _DEBUG
		//		printf("OnTimer::IsWindowUnicode = %d Rect = {%d,%d,%d,%d}\n",
		//			::IsWindowVisible(hwnd),
		//			cRect.left,cRect.top,cRect.Width(),cRect.Height());
		//#endif
	}
	else if (IDT_TIMEOUT == id)
	{
		KillTimer(hwnd, IDT_SHOW);
		KillTimer(hwnd, IDT_TIMEOUT);
		ShowWindow(hwnd, SW_HIDE);
	}
}

void OnSizeOrMove(HWND hwnd)
{
	if (g_bIsSetRect == TRUE)
	{
		g_blShow = TRUE;

		RECT rtWnd;
		::GetClientRect(hwnd, &rtWnd);

		HRGN cSignRgn = ::CreateRectRgn(0, 0, 0, 0);
		HRGN bSignRgn = ::CreateRectRgn(0, 0, rtWnd.right, rtWnd.bottom);
		HRGN sSignRgn = ::CreateRectRgn(3, 3, rtWnd.right - 3, rtWnd.bottom - 3);

		::CombineRgn(cSignRgn, bSignRgn, sSignRgn, RGN_XOR);
		::SetWindowRgn(hwnd, cSignRgn, TRUE);
		g_bIsSetRect = FALSE;

		::DeleteRgn(cSignRgn);
		::DeleteRgn(bSignRgn);
		::DeleteRgn(sSignRgn);
	}
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	OnSizeOrMove(hwnd);
}
void OnMove(HWND hwnd, int x, int y)
{
	OnSizeOrMove(hwnd);
}


void OnDestroy(HWND hwnd)
{
	g_WndFrameNumber--;
#ifdef _DEBUG
	printf("SignWnd::OnDestroy::g_WndFrameNumber = %d\n", g_WndFrameNumber);
#endif
	if (g_WndFrameNumber == 0)
	{
		DeleteObject(g_WhitePen);
		DeleteObject(g_hSignPen);
		g_WhitePen = NULL;
		g_hSignPen = NULL;
	}
}


HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
#ifdef _DEBUG
	printf("SignWnd::OnCtlColor(%#x,%#x,%#x,%d)\n", (UINT)hwnd, (UINT)hdc, (UINT)hwndChild, type);
#endif
	return (HBRUSH)GetStockObject(WHITE_BRUSH);
}