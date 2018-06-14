// FileVersion.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "FileVersion.h"


#pragma comment(lib, "Version.lib")

//取得所有的文件版本信息，各信息间以\n分隔
CFileVersion::~CFileVersion()
{
	if (m_lpData != NULL)
	{
		delete[] m_lpData;
	}
}
CString CFileVersion::GetFullVersion()
{
	CString szRet;

	if (m_lpData != NULL)
	{
		szRet = CString(TEXT("文件版本: ")) + GetFileVersion() + TEXT("\n");
		szRet += CString(TEXT("描述: ")) + GetFileDescription() + TEXT("\n");
		szRet += CString(TEXT("版权: ")) + GetLegalCopyright() + TEXT("\n");
		szRet += CString(TEXT("备注: ")) + GetComments() + TEXT("\n");
		szRet += CString(TEXT("产品版本: ")) + GetProductVersion() + TEXT("\n");
		szRet += CString(TEXT("产品名称: ")) + GetProductName() + TEXT("\n");
		szRet += CString(TEXT("个人用内部版本说明: ")) + GetPrivateBuild() + TEXT("\n");
		szRet += CString(TEXT("公司名称: ")) + GetCompanyName() + TEXT("\n");
		szRet += CString(TEXT("合法商标: ")) + GetLegalTrademarks() + TEXT("\n");
		szRet += CString(TEXT("内部名称: ")) + GetInternalName() + TEXT("\n");
		szRet += CString(TEXT("特殊内部版本说明: ")) + GetSpecialBuild() + TEXT("\n");
		szRet += CString(TEXT("源文件名: ")) + GetOriginalFilename() + TEXT("\n");
	}
	/*
	szRet = "FileVersion: " + GetFileVersion() + TEXT("\n");
	szRet += "FileDescription: " + GetFileDescription() +"\n";
	szRet += "CopyRight: " + GetLegalCopyright() +"\n";
	szRet += "Comments: " + GetComments() + TEXT("\n");
	szRet += "ProductVersion: " + GetProductVersion() +"\n";
	szRet += "ProductName: " + GetProductName() +"\n";
	szRet += "PrivateBuild: " + GetPrivateBuild() +"\n";
	szRet += "CompanyName: " + GetCompanyName() +"\n";
	szRet += "TradeMarks: " + GetLegalTrademarks() +"\n";
	szRet += "InternalName: " + GetInternalName() +"\n";
	szRet += "SpecialBuild: " + GetSpecialBuild() +"\n";
	szRet += "OriginalFileName: " + GetOriginalFilename() +"\n";*/
	return szRet;
}
BOOL CFileVersion::SetFilePath(LPCTSTR szPath)
{
	if (m_lpData != NULL)
	{
		delete[] m_lpData;
		m_lpData = NULL;
	}

	m_szPath = szPath;
	m_dwInfoSize = GetFileVersionInfoSize(m_szPath, &m_dwHandle);
	if (m_dwInfoSize == 0)return FALSE;

	m_lpData = new char[m_dwInfoSize];

	if (GetFileVersionInfo(m_szPath, 0, m_dwInfoSize, m_lpData) == FALSE)
	{
		delete[] m_lpData;
		m_lpData = NULL;
		return FALSE;
	}

	UINT cbTranslate = 0;
	BOOL ret = VerQueryValue(m_lpData, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&m_lpTranslate, &cbTranslate);

#ifdef _DEBUG
	if (ret == 0)
		_tprintf(::GetLastErrorInfo());
#endif

	return ret;
}
LPCTSTR CFileVersion::GetVer(LPCTSTR szSubBlock)
{
	CString SubBlock;
	LPCTSTR	lpBuffer;
	UINT	dwBytes = 0;

	if (m_lpData != NULL)
	{
		SubBlock.Format(TEXT("\\StringFileInfo\\%04x%04x\\%s"),
			m_lpTranslate->wLanguage,
			m_lpTranslate->wCodePage, szSubBlock);

		if (VerQueryValue(m_lpData, LPTSTR(LPCTSTR(SubBlock)), (LPVOID*)&lpBuffer, &dwBytes))
			return lpBuffer;
	}
	return TEXT("");
}
