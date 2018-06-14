
// stdafx.cpp : 只包括标准包含文件的源文件
// WindowsKiller3.0.pch 将作为预编译标头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"


// {0387EC96-5C4D-44b0-B26F-7B32AF292285}
static const GUID CLSID_IWndEngine =
{ 0x387ec96, 0x5c4d, 0x44b0,{ 0xb2, 0x6f, 0x7b, 0x32, 0xaf, 0x29, 0x22, 0x85 } };

// {09980499-7D26-4260-8779-4ACDDA9B8E9C}
static const GUID IID_IWndEngine =
{ 0x9980499, 0x7d26, 0x4260,{ 0x87, 0x79, 0x4a, 0xcd, 0xda, 0x9b, 0x8e, 0x9c } };


// {8AA0D665-3C3A-4A7E-8FAE-F0A83AB474A2}
static const GUID CLSID_IProcEngine =
{ 0x8aa0d665, 0x3c3a, 0x4a7e,{ 0x8f, 0xae, 0xf0, 0xa8, 0x3a, 0xb4, 0x74, 0xa2 } };

// {D44CAC41-F0F3-4C2A-ADB4-0274D862B5DB}
static const GUID IID_IProcEngine =
{ 0xd44cac41, 0xf0f3, 0x4c2a,{ 0xad, 0xb4, 0x2, 0x74, 0xd8, 0x62, 0xb5, 0xdb } };


// {7DB76ACA-46CF-4539-B3AB-F7737FD84ADD}
static const GUID IID_IWndSigned =
{ 0x7db76aca, 0x46cf, 0x4539,{ 0xb3, 0xab, 0xf7, 0x73, 0x7f, 0xd8, 0x4a, 0xdd } };

// {1834B84D-9752-4c82-AE8C-259A647DE846}
static const GUID CLSID_IWndSigned =
{ 0x1834b84d, 0x9752, 0x4c82,{ 0xae, 0x8c, 0x25, 0x9a, 0x64, 0x7d, 0xe8, 0x46 } };


// {F3EBADDA-2200-4d05-9D7F-8B7A14714D76}
static const GUID CLSID_IFileEngine =
{ 0xf3ebadda, 0x2200, 0x4d05,{ 0x9d, 0x7f, 0x8b, 0x7a, 0x14, 0x71, 0x4d, 0x76 } };

// {8B474B04-4940-46c7-9102-8312511EA11B}
static const GUID IID_IFileEngine =
{ 0x8b474b04, 0x4940, 0x46c7,{ 0x91, 0x2, 0x83, 0x12, 0x51, 0x1e, 0xa1, 0x1b } };


// {F8F96468-0E5B-4485-8029-326D88640EDC}
extern "C" const GUID IID_IWndPreview =
{ 0xf8f96468, 0xe5b, 0x4485,{ 0x80, 0x29, 0x32, 0x6d, 0x88, 0x64, 0xe, 0xdc } };

// {A7C6EAF4-6BDF-448f-A8EC-7BADC8A06549}
extern "C" const GUID CLSID_IWndPreview =
{ 0xa7c6eaf4, 0x6bdf, 0x448f,{ 0xa8, 0xec, 0x7b, 0xad, 0xc8, 0xa0, 0x65, 0x49 } };


// {74420AE8-B2C6-4216-9305-A5CA77DC38AA}
extern "C"  const GUID IID_IWndModule =
{ 0x74420ae8, 0xb2c6, 0x4216,{ 0x93, 0x5, 0xa5, 0xca, 0x77, 0xdc, 0x38, 0xaa } };
// {E0E54DE5-A585-4FF5-8839-AE4F4DD42B06}
extern "C"  const GUID CLSID_IWndModule =
{ 0xe0e54de5, 0xa585, 0x4ff5,{ 0x88, 0x39, 0xae, 0x4f, 0x4d, 0xd4, 0x2b, 0x6 } };


// {03E1DA29-14FB-4D92-B37E-293BD6C6C81A}
extern "C" const GUID IID_IWndAttribute =
{ 0x3e1da29, 0x14fb, 0x4d92,{ 0xb3, 0x7e, 0x29, 0x3b, 0xd6, 0xc6, 0xc8, 0x1a } };
// {1A5DD9D7-255B-462C-93A6-7EBA3DA8254A}
extern "C" const GUID CLSID_IWndAttribute =
{ 0x1a5dd9d7, 0x255b, 0x462c,{ 0x93, 0xa6, 0x7e, 0xba, 0x3d, 0xa8, 0x25, 0x4a } };


// {CB260DCF-4553-488d-9E8A-0343C0DC6786}
extern "C"  const GUID CLSID_IWndExtract =
{ 0xcb260dcf, 0x4553, 0x488d,{ 0x9e, 0x8a, 0x3, 0x43, 0xc0, 0xdc, 0x67, 0x86 } };

// {D7AE683E-35FC-434b-B856-B5A8B01D0F2D}
extern "C"  const GUID IID_IWndExtract =
{ 0xd7ae683e, 0x35fc, 0x434b,{ 0xb8, 0x56, 0xb5, 0xa8, 0xb0, 0x1d, 0xf, 0x2d } };


// {0AD39BD6-5108-468f-A6E6-05031EDDEDFA}
extern "C" const GUID IID_IViewWizard =
{ 0xad39bd6, 0x5108, 0x468f,{ 0xa6, 0xe6, 0x5, 0x3, 0x1e, 0xdd, 0xed, 0xfa } };

// {1E938831-FDA7-4a8c-91B8-2D306E75F2E6}
extern "C" const GUID CLSID_IWndWizard =
{ 0x1e938831, 0xfda7, 0x4a8c,{ 0x91, 0xb8, 0x2d, 0x30, 0x6e, 0x75, 0xf2, 0xe6 } };


