#include "stdafx.h"
#include "String.h"
#include <strsafe.h>

//String::~String()
//{
//	basic_string::~basic_string();
//}

//String & String::operator=(LPCTSTR str)
//{
//	//String temp(str);
//	//*this = String(str);
//	assign(str);
//	//*this = temp;
//	return *this;
//}

//String& String::operator=(String& str) {
//	*this = str.c_str();
//	return *this;
//}
String::operator LPCTSTR () const {
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
	StringVPrintfWorker(szTemp, len * 2, NULL, format, ap);
#pragma warning (pop)

	assign(szTemp);
	delete[]szTemp;
	va_end(ap);				//清理工作 
	return *this;
}
