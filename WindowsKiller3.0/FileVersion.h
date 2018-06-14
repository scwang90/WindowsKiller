#pragma once

// CFileVersion 命令目标

class CFileVersion : public CObject
{
public:
	//初始化函数,初始化文件路径
	CFileVersion(LPCTSTR szPath) {
		m_lpData = 0;
		m_szPath = szPath;
	}
	CFileVersion() {
		m_lpData = 0;
		m_szPath = "";
	}
	~CFileVersion();
	BOOL SetFilePath(LPCTSTR szPath);
	//取得所有的文件版本信息，各信息间以\n分隔
	CString GetFullVersion();

	//取得备注
	CString GetComments() {
		return GetVer(TEXT("Comments"));
	}
	//取得内部名称
	CString GetInternalName() {
		return GetVer(TEXT("InternalName"));
	}
	//取得产品名称
	CString GetProductName() {
		return GetVer(TEXT("ProductName"));
	}
	//取得公司名称
	CString GetCompanyName() {
		return GetVer(TEXT("CompanyName"));
	}
	//取得版权
	CString GetLegalCopyright() {
		return GetVer(TEXT("LegalCopyright"));
	}
	//取得产品版本
	CString GetProductVersion() {
		return GetVer(TEXT("ProductVersion"));
	}
	//取得文件描述
	CString GetFileDescription() {
		return GetVer(TEXT("FileDescription"));
	}
	//取得合法商标
	CString GetLegalTrademarks() {
		return GetVer(TEXT("LegalTrademarks"));
	}
	//取得个人用内部版本说明
	CString GetPrivateBuild() {
		return GetVer(TEXT("PrivateBuild"));
	}
	//取得文件版本
	CString GetFileVersion() {
		return GetVer(TEXT("FileVersion"));
	}
	//取得源文件名
	CString GetOriginalFilename() {
		return GetVer(TEXT("OriginalFilename"));
	}
	//取得特殊内部版本说明
	CString GetSpecialBuild() {
		return GetVer(TEXT("SpecialBuild"));
	}

	//获得文件各种信息
	LPCTSTR GetVer(LPCTSTR szSubBlock);

private:

	struct LANGANDCODEPAGE {
		WORD wLanguage; //语言代码,简体中文是2052
		WORD wCodePage;
	}*m_lpTranslate;
	//保存路径
	CString m_szPath;

	DWORD m_dwInfoSize;
	DWORD m_dwHandle;
	char* m_lpData;
};


