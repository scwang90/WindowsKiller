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
	va_list ap;				//����һ��va_list����
	va_start(ap, format);	//��ʼ�����ڶ�������Ϊ���һ��ȷ�����β�
	int len = lstrlen(format);
	TCHAR* szTemp = new TCHAR[len * 2];

#pragma warning (push)
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
	StringVPrintfWorker(szTemp, len * 2, NULL, format, ap);
#pragma warning (pop)

	assign(szTemp);
	delete[]szTemp;
	va_end(ap);				//������ 
	return *this;
}
