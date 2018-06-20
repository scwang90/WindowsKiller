#pragma once


#include "IFileEngine.h"

#define	IMAGE_SIGNATURE_FILE	0x4653		//SF
#define	IMAGE_SIGNATURE_TYPE	0x4854		//TH
#define	PLATFORM_X64		0b0001
#define	PLATFORM_UNICODE	0b0010

class FileEngine :public IFileEngine
{
public:
	FileEngine(void);
public:
	~FileEngine(void);

protected:
	typedef struct FileHeader {
		FileHeader()
			:magic(WORD(IMAGE_SIGNATURE_FILE))
			, size(sizeof(FileHeader))
			, version(msDwVersion)
			, platform(0)
			, sizeOfDescribe(WORD(sizeof(TCHAR)*(lstrlen(msLpStrDescribe)+1)))
			, ptrTypeHeader(LONG(sizeof(FileHeader) + sizeof(TCHAR)*(lstrlen(msLpStrDescribe)+1))) {
#ifdef _WIN64
			platform = platform | PLATFORM_X64;
#endif // _x64
#ifdef _UNICODE
			platform = platform | PLATFORM_UNICODE;
#endif // _UNICODE
		}
		WORD	magic;                    // Magic number
		WORD	size;                     // size number FileHeader 块大小	V1.3
		WORD	version;				  // version Code
		WORD	platform;				  // platform 平台兼容性 V1.3
		WORD	sizeOfDescribe;			  // size of describe
		LONG	ptrTypeHeader;            // File address of new type header
	}*LpFileHeader;
	typedef	struct TypeHeader {
		TypeHeader()
			:magic(WORD(IMAGE_SIGNATURE_TYPE))
			, size(sizeof(TypeHeader))
			, ntype(WORD(BVT_ALLTYPE))
			, ptrTypeElement(LONG(sizeof(TypeHeader) + sizeof(FileHeader) + sizeof(TCHAR)*(lstrlen(msLpStrDescribe)+1))) {}
		WORD	magic;                    // Magic number
		WORD	size;                     // size number TypeHeader 块大小 V1.3
		WORD	ntype;					  // type number
		LONG	ptrTypeElement;           // File address of new type Element
	}*LpTypeHeader;
	typedef	struct TypeElement {
		WORD	vType;                    // Type
		WORD	nVar;						// Variable number
		LONG	ptrTypeVariable;			// File address of new type Variable
	}*LpTypeElement;
	typedef	struct Var8 {
		TCHAR	tName[VAR_NAME_SIZE];		// Variable Name
		BYTE	tValue;                     // Value
	}*LpVar8;
	typedef	struct FileVar8 {
		LONG	tName;					// address of Variable Name
		BYTE	tValue;                   // Value
	}*LpFileVar8;
	typedef	struct Var16 {
		TCHAR	tName[VAR_NAME_SIZE];		// Variable Name
		WORD	tValue;                   // Value
	}*LpVar16;
	typedef	struct FileVar16 {
		LONG	tName;					// address of Variable Name
		WORD	tValue;                   // Value
	}*LpFileVar16;
	typedef	struct Var32 {
		TCHAR	tName[VAR_NAME_SIZE];		// Variable Name
		DWORD	tValue;                   // Value
	}*LpVar32;
	typedef	struct FileVar32 {
		LONG	tName;					// address of Variable Name
		DWORD	tValue;                   // Value
	}*LpFileVar32;
	typedef	struct Var64 {
		TCHAR	tName[VAR_NAME_SIZE];		// Variable Name
		__int64	tValue;                   // Value
	}*LpVar64;
	typedef	struct FileVar64 {
		LONG	tName;					// address of Variable Name
		__int64	tValue;                   // Value
	}*LpFileVar64;
	typedef	struct VarString {
		TCHAR	tName[VAR_NAME_SIZE];		// Variable Name
		union {
			LPTSTR	tValue;               // Value
			LONG	tAddres;
		};
	}*LpVarString;
	typedef	struct FileVarString {
		LONG	tName;					// address of Variable Name
		union {
			LPTSTR	tValue;               // Value
			LONG	tAddres;
		};
	}*LpFileVarString;
	typedef	struct VarStruct {
		TCHAR	tName[VAR_NAME_SIZE];		// Strcut Name
		size_t	tSize;					// Strcut Size
		union {
			LPVOID	tValue;               // Value
			LONG	tAddres;
		};
	}*LpVarStruct;
	typedef	struct FileVarStruct {
		LONG	tName;					// address of Variable Name
		size_t	tSize;					// Strcut Size
		union {
			LPVOID	tValue;               // Value
			LONG	tAddres;
		};
	}*LpFileVarStruct;

	typedef	struct Variable {
		Variable()
			:pVar8(NULL)
			, pVar16(NULL)
			, pVar32(NULL)
			, pVar64(NULL)
			, pVarString(NULL)
			, pVarStruct(NULL) {}
		union {
			struct {
				LpVar8		pVar8;
				LpVar16		pVar16;
				LpVar32		pVar32;
				LpVar64		pVar64;
				LpVarString	pVarString;
				LpVarStruct	pVarStruct;
			};
			struct { PVOID pAdd[6]; };
		};
	}*LpVariable;
	/*
	 * 再文件中的变量
	 */
	typedef	struct FileVariable {
		FileVariable()
			: pFileVar8(NULL)
			, pFileVar16(NULL)
			, pFileVar32(NULL)
			, pFileVar64(NULL)
			, pFileVarString(NULL)
			, pFileVarStruct(NULL) {}

		~FileVariable()
		{
			for (int i = 0; i < 6; i++)
			{
				if (pFileAdd[i])
				{
					free(pFileAdd[i]);
					pFileAdd[i] = NULL;
				}
			}
		}
		union {
			struct {
				LpFileVar8		pFileVar8;
				LpFileVar16		pFileVar16;
				LpFileVar32		pFileVar32;
				LpFileVar64		pFileVar64;
				LpFileVarString	pFileVarString;
				LpFileVarStruct	pFileVarStruct;
			};
			struct { PVOID pFileAdd[6]; };
		};
	}*LpFileVariable;

protected:
	void FreeData();
	bool CreateFileMap();
	bool CheckFileMap();
	long GetTypeValueSize(WORD vType);
	long GetTypeMemVarSize(WORD vType);
	long GetTypeFileVarSize(WORD vType);
	void ReadFileMap(LPVOID lpBase, LpFileHeader pFileHeader, LpTypeHeader pTypeHeader);
	bool AddVarValue(WORD vType, long wSize, LPBYTE lpByte, LPCTSTR lpName, LPCVOID pDefaut, size_t size = 0);
	long AddVarValue(LPBYTE lpByte, WORD wType, WORD wSize, LPTYPEDATA lpTypeData, int nLength);
	long AddVarValue(WORD wType, long nType, LPTYPEDATA lpTypeData, int nLength);
	long AddVarString(LpVarString pVarString, LPTYPEDATA lpTypeData, int nLength);
	long AddVarString(long nType, LPTYPEDATA lpTypeData, int nLength);
	long AddVarStruct(LpVarStruct pVarStruct, LPTYPEDATA lpTypeData, int nLength);
	long AddVarStruct(long nType, LPTYPEDATA lpTypeData, int nLength);
	void* MemAllocate(void* address, size_t newsize);
	long CountTypeInTypeData(WORD wType, LPTYPEDATA lpTypeData, int nLength);
	void MapFile(FileVariable &cFileVariable, TypeElement eType[], LpVarString &pVarString, LpVarStruct &pVarStruct);
	BOOL SaveData(FileVariable &cFileVariable, TypeElement eType[], LpVarString &pVarString, LpVarStruct &pVarStruct);
	LPBYTE FindVarValue(WORD vType, LPCTSTR lpName);
	
	//IFileEngine 1.2 V
	LPBYTE FindVarValue(WORD vType, int index);
	
	//IFileEngine 1.3 V
	BaseValueType MapBaseValueType(size_t size);

	template<typename T>
	bool PutVarTemplate(LPCTSTR lpName, const T& value);

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IBackstage member function	
	virtual bool CloseFile();
	virtual bool SaveFile();
	virtual bool IsEmptyRecorde();
	virtual bool OpenFile(LPCTSTR lpFile, DWORD dwDesiredAccess = DA_READ | DA_WRITE);
	
	//CSupperFile	1.0 V
	virtual bool GetVarString(LPCTSTR lpName, LPTSTR lpStr, int nMax);
	virtual bool SetVarValue(WORD vType, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType);
	virtual bool SetVarValue(WORD vType, LPCTSTR lpName, LPCVOID pWrite, size_t size = 0);
	virtual bool GetVarValue(WORD vType, LPCTSTR lpName, PVOID pWrite);
	virtual long AddVarValue(LPTYPEDATA lpTypeData, int nLength);

	virtual bool AddVarValue(WORD vType, LPCTSTR lpName, LPCVOID pDefaut, size_t size = 0);
	virtual bool AddVarValue(WORD vType, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType);
	virtual bool AddVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize);
	virtual void OutPutDataInfo();
	//CSupperFile	1.1 V
	virtual WORD GetVersion(BYTE bType = VT_FILE);
	virtual bool SetDepict(LPCTSTR lpDepict);
	virtual bool GetDepict(LPTSTR lpBuffer, int nMax);

	//IFileEngine	1.2 V
	virtual bool GetLastError(LPTSTR lpStr, int nMax);

	virtual bool GetVarStruct(LPCTSTR lpName, LPVOID pWrite);
	virtual bool SetVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize = 0);
	virtual bool AddVarString(LPCTSTR lpName, LPCTSTR lpStr, size_t sSize = 0);
	virtual bool SetVarString(LPCTSTR lpName, LPCTSTR lpStr, size_t sSize = 0);

	virtual bool GetVarName(WORD vType, int index, LPTSTR lpStr, int nMax);
	virtual bool SetVarName(WORD vType, int index, LPTSTR lpSetName);
	virtual bool SetVarName(WORD vType, LPCTSTR lpName, LPTSTR lpSetName);
	virtual bool SetVarIndex(WORD vType, LPCTSTR lpName, int indexto, bool bswap = true);
	virtual bool SetVarIndex(WORD vType, int index, int indexto, bool bswap = true);
	virtual long GetVarIndex(WORD vType, LPCTSTR lpName);

	virtual long GetVarStringSize(int index);
	virtual long GetVarStringSize(LPCTSTR lpName);
	virtual long GetVarStructSize(int index);
	virtual long GetVarStructSize(LPCTSTR lpName);

	virtual bool AddVarString(int index, LPCTSTR lpName, LPCTSTR lpStr);
	virtual bool SetVarString(int index, LPCTSTR lpStr);
	virtual bool GetVarString(int index, LPTSTR lpStr, int nMax);
	virtual bool AddVarStruct(int index, LPCTSTR lpName, LPCVOID pDefaut, size_t sSize);
	virtual bool SetVarStruct(int index, LPCVOID pDefaut, size_t sSize = 0);
	virtual bool GetVarStruct(int index, LPVOID pWrite);
	virtual bool AddVarValue(WORD vType, int index, LPCTSTR lpName, LPCVOID pDefaut);
	virtual bool AddVarValue(WORD vType, int index, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType);
	virtual bool SetVarValue(WORD vType, int index, LPCVOID pDefaut);
	virtual bool SetVarValue(WORD vType, int index, UINT_PTR dwDefaut, BYTE bValueType);
	virtual bool GetVarValue(WORD vType, int index, LPVOID pWrite);
	virtual bool DelVarValue(WORD vType, int index);
	virtual bool DelVarValue(WORD vType, LPCTSTR lpName);

	virtual bool DelAllValue(WORD vType = BVT_ALLTYPE);
	virtual long GetVarNumber(WORD vType = BVT_ALLTYPE);

	//IFileEngine	1.3 V
	virtual bool IsHasVarName(WORD vType, LPCTSTR lpName = nullptr);
	virtual bool PutVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t size);
	virtual bool PutVarString(LPCTSTR lpName, LPCTSTR pDefaut, size_t size = 0);

	virtual bool PutVar(LPCTSTR lpName, const bool& value);
	virtual bool PutVar(LPCTSTR lpName, const char& value);
	virtual bool PutVar(LPCTSTR lpName, const BYTE& value);
	virtual bool PutVar(LPCTSTR lpName, const short& value);
	virtual bool PutVar(LPCTSTR lpName, const WORD& value);
	virtual bool PutVar(LPCTSTR lpName, const int& value);
	virtual bool PutVar(LPCTSTR lpName, const DWORD& value);
	virtual bool PutVar(LPCTSTR lpName, const long& value);
	virtual bool PutVar(LPCTSTR lpName, const INT64& value);
	virtual bool PutVar(LPCTSTR lpName, const UINT64& value);
	virtual bool PutVar(LPCTSTR lpName, const float& value);
	virtual bool PutVar(LPCTSTR lpName, const double& value);
	virtual bool PutVar(LPCTSTR lpName, LPCTSTR lpStr);

	virtual LPCTSTR GetLastError();

private:
	// IUnknown member data	
	int		m_Ref;

protected:
	// IBackstage member function
	bool			m_bSave;
	HANDLE			mFileHandle;
	LpTypeElement	m_pData;

	FileHeader		mFileHeader;
	TypeHeader		mTypeHeader;
	TypeElement 	mTypeElements[BVT_ALLTYPE];
	Variable		mMemVariable;

	LPTSTR			mLpStrDepict;

	//IFileEngine	1.2 V
	TCHAR			mSzLastError[MAX_PATH];

public:
	static	LPCTSTR	msLpStrDescribe;
	static	WORD	msDwVersion;
};
