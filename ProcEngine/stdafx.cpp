// stdafx.cpp : 只包括标准包含文件的源文件
// ProcEngine.pch 将作为预编译标头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中引用任何所需的附加头文件，
//而不是在此文件中引用

// {8AA0D665-3C3A-4A7E-8FAE-F0A83AB474A2}
extern "C" const GUID CLSID_IProcEngine =
{ 0x8aa0d665, 0x3c3a, 0x4a7e,{ 0x8f, 0xae, 0xf0, 0xa8, 0x3a, 0xb4, 0x74, 0xa2 } };

// {D44CAC41-F0F3-4C2A-ADB4-0274D862B5DB}
extern "C" const GUID IID_IProcEngine =
{ 0xd44cac41, 0xf0f3, 0x4c2a,{ 0xad, 0xb4, 0x2, 0x74, 0xd8, 0x62, 0xb5, 0xdb } };
