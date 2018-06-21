#include "stdafx.h"
#include "String.h"
#include <strsafe.h>

String::operator LPCTSTR() const { 
	return c_str(); 
}

String & String::format(LPCTSTR format, ...) {
	va_list ap;				//声明一个va_list变量
	va_start(ap, format);	//初始化，第二个参数为最后一个确定的形参
	int len = lstrlen(format);
	TCHAR* szTemp = new TCHAR[len * 2];

#pragma warning (push)
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#ifdef UNICODE
	StringVPrintfWorkerW(szTemp, len * 2, NULL, format, ap);
#else
	StringVPrintfWorkerA(szTemp, len * 2, NULL, format, ap);
#endif // UNICODE

#pragma warning (pop)

	*this = szTemp;
	delete[]szTemp;
	va_end(ap);				//清理工作 
	return *this;
}
