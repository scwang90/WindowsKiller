#include "stdafx.h"
#include "String.h"
#include <strsafe.h>

String::operator LPCTSTR() const { 
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
#ifdef UNICODE
	StringVPrintfWorkerW(szTemp, len * 2, NULL, format, ap);
#else
	StringVPrintfWorkerA(szTemp, len * 2, NULL, format, ap);
#endif // UNICODE

#pragma warning (pop)

	*this = szTemp;
	delete[]szTemp;
	va_end(ap);				//������ 
	return *this;
}
