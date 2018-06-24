#pragma once

#include <string>
#include <strsafe.h>

typedef std::basic_string<TCHAR> tstring;

class COMMON_API String : public tstring {

	using tstring::tstring;

public:
	//virtual ~String();
	operator LPCTSTR() const;
	//String& operator=(String& str);
	//String& operator=(LPCTSTR str);

public:
	String & format(LPCTSTR format, ...);
//	{
//		va_list ap;				//声明一个va_list变量
//		va_start(ap, format);	//初始化，第二个参数为最后一个确定的形参
//		int len = lstrlen(format);
//		TCHAR* szTemp = new TCHAR[len * 2];
//
//#pragma warning (push)
//#pragma warning(disable : 4995)
//#pragma warning(disable : 4996)
//		StringVPrintfWorker(szTemp, len * 2, NULL, format, ap);
//#pragma warning (pop)
//
//		*this = szTemp;
//		delete[]szTemp;
//		va_end(ap);				//清理工作 
//		return *this;
//	}



};


