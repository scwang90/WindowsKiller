#pragma once

#define IDT_SHOW	0x01000
#define IDT_TIMEOUT	0x02000

ATOM	MyRegisterClass(HINSTANCE hInstance);
HWND	InitInstance(HINSTANCE, int);
