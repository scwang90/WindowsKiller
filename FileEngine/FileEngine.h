#pragma once


#include "IFileEngine.h"

#define		IMAGE_SIGNATURE_FILE	0x4653		//SF
#define		IMAGE_SIGNATURE_TYPE	0x4854		//TH

class FileEngine :public IFileEngine
{
public:
	FileEngine(void);
public:
	~FileEngine(void);

protected:
	typedef struct FileHeader {
		FileHeader()
			:e_magic(WORD(IMAGE_SIGNATURE_FILE))
			, e_version(m_dwVersion)
			, e_ldescribe(WORD(sizeof(TCHAR)*lstrlen(m_lpDescribe)))
			, e_lfanew(LONG(sizeof(FileHeader) + sizeof(TCHAR)*lstrlen(m_lpDescribe) + sizeof(TCHAR))) {}
		WORD	e_magic;                    // Magic number
		WORD	e_version;					// version Code
		WORD	e_ldescribe;				// length of describe
		LONG	e_lfanew;                   // File address of new type header
	}*LpFileHeader;
	typedef	struct TypeHeader {
		TypeHeader()
			:e_magic(WORD(IMAGE_SIGNATURE_TYPE))
			, e_ntype(WORD(BVT_ALLTYPE))
			, e_lfanew(LONG(sizeof(TypeHeader) + sizeof(FileHeader) + sizeof(TCHAR)*lstrlen(m_lpDescribe) + sizeof(TCHAR))) {}
		WORD	e_magic;                    // Magic number
		WORD	e_ntype;					// type number
		LONG	e_lfanew;                   // File address of new type Elerment
	}*LpTypeHeader;
	typedef	struct TypeElerment {
		WORD	e_vType;                    // Type
		WORD	e_nVar;						// Variable number
		LONG	e_lfanew;					// File address of new type Variable
	}*LpTypeElerment;
	typedef	struct Var8 {
		TCHAR	e_tName[VAR_NAME_SIZE];		// Variable Name
		BYTE	e_tValue;                   // Value
	}*LpVar8;
	typedef	struct FVar8 {
		LONG	e_tName;					// address of Variable Name
		BYTE	e_tValue;                   // Value
	}*LpFVar8;
	typedef	struct Var16 {
		TCHAR	e_tName[VAR_NAME_SIZE];		// Variable Name
		WORD	e_tValue;                   // Value
	}*LpVar16;
	typedef	struct FVar16 {
		LONG	e_tName;					// address of Variable Name
		WORD	e_tValue;                   // Value
	}*LpFVar16;
	typedef	struct Var32 {
		TCHAR	e_tName[VAR_NAME_SIZE];		// Variable Name
		DWORD	e_tValue;                   // Value
	}*LpVar32;
	typedef	struct FVar32 {
		LONG	e_tName;					// address of Variable Name
		DWORD	e_tValue;                   // Value
	}*LpFVar32;
	typedef	struct Var64 {
		TCHAR	e_tName[VAR_NAME_SIZE];		// Variable Name
		double	e_tValue;                   // Value
	}*LpVar64;
	typedef	struct FVar64 {
		LONG	e_tName;					// address of Variable Name
		double	e_tValue;                   // Value
	}*LpFVar64;
	typedef	struct VarString {
		TCHAR	e_tName[VAR_NAME_SIZE];		// Variable Name
		union {
			PTSTR	e_tValue;               // Value
			LONG	e_tAddres;
		};
	}*LpVarString;
	typedef	struct FVarString {
		LONG	e_tName;					// address of Variable Name
		union {
			PTSTR	e_tValue;               // Value
			LONG	e_tAddres;
		};
	}*LpFVarString;
	typedef	struct VarStruct {
		TCHAR	e_tName[VAR_NAME_SIZE];		// Strcut Name
		size_t	e_tSize;					// Strcut Size
		union {
			PVOID	e_tValue;               // Value
			LONG	e_tAddres;
		};
	}*LpVarStruct;
	typedef	struct FVarStruct {
		LONG	e_tName;					// address of Variable Name
		size_t	e_tSize;					// Strcut Size
		union {
			PVOID	e_tValue;               // Value
			LONG	e_tAddres;
		};
	}*LpFVarStruct;

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
	typedef	struct FVariable {
		FVariable()
			:pFVar8(NULL)
			, pFVar16(NULL)
			, pFVar32(NULL)
			, pFVar64(NULL)
			, pFVarString(NULL)
			, pFVarStruct(NULL) {}

		~FVariable()
		{
			for (int i = 0; i < 6; i++)
			{
				if (pFAdd[i])
				{
					free(pFAdd[i]);
					pFAdd[i] = NULL;
				}
			}
		}
		union {
			struct {
				LpFVar8			pFVar8;
				LpFVar16		pFVar16;
				LpFVar32		pFVar32;
				LpFVar64		pFVar64;
				LpFVarString	pFVarString;
				LpFVarStruct	pFVarStruct;
			};
			struct { PVOID pFAdd[6]; };
		};
	}*LpFVariable;

protected:
	void FreeData();
	bool CreateFileMap();
	bool CheckFileMap();
	long GetTypeSize(WORD vType);
	long GetTypeVarSize(WORD vType);
	long GetTypeFVarSize(WORD vType);
	void ReadFileMap(LPVOID lpBase, LpFileHeader pFileHeader, LpTypeHeader pTypeHeader);
	bool AddVarValue(WORD vType, long wSize, LPBYTE lpByte, LPCTSTR lpName, LPCVOID pDefaut);
	long AddVarValue(LPBYTE lpByte, WORD wType, WORD wSize, LPTYPEDATA lpTypeData, int nLength);
	long AddVarValue(WORD wType, long nType, LPTYPEDATA lpTypeData, int nLength);
	long AddVarString(LpVarString pVarString, LPTYPEDATA lpTypeData, int nLength);
	long AddVarString(long nType, LPTYPEDATA lpTypeData, int nLength);
	long AddVarStruct(LpVarStruct pVarStruct, LPTYPEDATA lpTypeData, int nLength);
	long AddVarStruct(long nType, LPTYPEDATA lpTypeData, int nLength);
	void* MemAllocate(void* address, size_t newsize);
	long GetTypeData(WORD wType, LPTYPEDATA lpTypeData, int nLength);
	void MapFile(FVariable &cFVariable, TypeElerment eType[], LpVarString &pVarString, LpVarStruct &pVarStruct);
	BOOL SaveData(FVariable &cFVariable, TypeElerment eType[], LpVarString &pVarString, LpVarStruct &pVarStruct);
	LPBYTE FindVarValue(WORD vType, LPCTSTR lpName);
	//IFileEngine	1.2 V
	LPBYTE FindVarValue(WORD vType, int index);

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
	virtual bool GetVarString(LPCTSTR lpName, LPTSTR lpStr, int iSize);
	virtual bool SetVarValue(WORD vType, LPCTSTR lpName, int dwDefaut, BYTE bValueType);
	virtual bool SetVarValue(WORD vType, LPCTSTR lpName, LPCVOID pWrite);
	virtual bool GetVarValue(WORD vType, LPCTSTR lpName, PVOID pWrite);
	virtual long AddVarValue(LPTYPEDATA lpTypeData, int nLength);

	virtual bool AddVarValue(WORD vType, LPCTSTR lpName, LPCVOID pDefaut);
	virtual bool AddVarValue(WORD vType, LPCTSTR lpName, int dwDefaut, BYTE bValueType);
	virtual bool AddVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize);
	virtual void OutPutDataInfo();
	//CSupperFile	1.1 V
	virtual WORD GetVersion(BYTE bType = VT_FILE);
	virtual bool SetDepict(LPCTSTR lpDepict);
	virtual bool GetDepict(LPTSTR lpBuffer, int iSize);

	//IFileEngine	1.2 V
	virtual bool GetLastError(LPTSTR lpStr, int iSize);

	virtual bool GetVarStruct(LPCTSTR lpName, LPVOID pWrite);
	virtual bool SetVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize = 0);
	virtual bool AddVarString(LPCTSTR lpName, LPCTSTR lpStr);
	virtual bool SetVarString(LPCTSTR lpName, LPCTSTR lpStr);

	virtual bool GetVarName(WORD vType, int index, LPTSTR lpStr, int iSize);
	virtual bool SetVarName(WORD vType, int index, LPTSTR lpSetName);
	virtual bool SetVarName(WORD vType, LPCTSTR lpName, LPTSTR lpSetName);
	virtual long GetVarIndex(WORD vType, LPCTSTR lpName);
	virtual bool SetVarIndex(WORD vType, LPCTSTR lpName, int indexto, bool bswap = true);
	virtual bool SetVarIndex(WORD vType, int index, int indexto, bool bswap = true);

	virtual size_t GetVarStringSize(int index);
	virtual size_t GetVarStringSize(LPCTSTR lpName);
	virtual size_t GetVarStructSize(int index);
	virtual size_t GetVarStructSize(LPCTSTR lpName);

	virtual bool AddVarString(int index, LPCTSTR lpName, LPCTSTR lpStr);
	virtual bool SetVarString(int index, LPCTSTR lpStr);
	virtual bool GetVarString(int index, LPTSTR lpStr, int iSize);
	virtual bool AddVarStruct(int index, LPCTSTR lpName, LPCVOID pDefaut, size_t sSize);
	virtual bool SetVarStruct(int index, LPCVOID pDefaut, size_t sSize = 0);
	virtual bool GetVarStruct(int index, LPVOID pWrite);
	virtual bool AddVarValue(WORD vType, int index, LPCTSTR lpName, LPCVOID pDefaut);
	virtual bool AddVarValue(WORD vType, int index, LPCTSTR lpName, int dwDefaut, BYTE bValueType);
	virtual bool SetVarValue(WORD vType, int index, LPCVOID pDefaut);
	virtual bool SetVarValue(WORD vType, int index, int dwDefaut, BYTE bValueType);
	virtual bool GetVarValue(WORD vType, int index, LPVOID pWrite);
	virtual bool DelVarValue(WORD vType, int index);
	virtual bool DelVarValue(WORD vType, LPCTSTR lpName);

	virtual bool DelAllValue(WORD vType = BVT_ALLTYPE);
	virtual long GetVarNumber(WORD vType = BVT_ALLTYPE);

	//virtual WORD GetFileVersion();
private:
	// IUnknown member data	
	int		m_Ref;

protected:
	// IBackstage member function
	bool			m_bSave;
	HANDLE			m_hFile;
	LpTypeElerment	m_pData;

	FileHeader		m_fHeader;
	TypeHeader		m_tHeader;
	TypeElerment 	m_eType[BVT_ALLTYPE];
	Variable		m_Variable;

	LPTSTR			m_lpDepict;

	//IFileEngine	1.2 V
	TCHAR			m_szLastError[256];
public:
	static	LPCTSTR	m_lpDescribe;
	static	WORD	m_dwVersion;
};
