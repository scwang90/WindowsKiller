#pragma once

#include <Unknwn.h>


// {7DB76ACA-46CF-4539-B3AB-F7737FD84ADD}
extern "C" const GUID IID_IWndSigned;

interface IWndSigned : public IUnknown
{
	virtual bool SetSignColor(DWORD color) = 0;
	virtual LONG GetSignColor() = 0;
	virtual bool SignRect(LPRECT lpRect, int iTime = 0) = 0;
	virtual bool SignHide() = 0;
	virtual bool IsSignHide() = 0;
};


#ifdef WNDSIGNED_EXPORTS
#define WNDSIGNED_API __declspec(dllexport)
#else
#define WNDSIGNED_API __declspec(dllimport)
#endif

// 此类是从 WndSigned.dll 导出的
class WNDSIGNED_API DllWndSigned {
public:
	// TODO: 在此添加您的方法。
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
};

// {1834B84D-9752-4c82-AE8C-259A647DE846}
extern "C" const GUID CLSID_IWndSigned;
