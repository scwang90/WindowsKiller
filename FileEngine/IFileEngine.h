#pragma once

#include <Unknwn.h>

// {F3EBADDA-2200-4d05-9D7F-8B7A14714D76}
extern "C" const GUID IID_IFileEngine;

#define	VAR_NAME_SIZE	32
#define	TD_ADDRESS	0
#define	TD_VALUE	1

typedef struct tagTYPEDATA {
	WORD	wType;                    // Type
	TCHAR	tName[VAR_NAME_SIZE];	  // Variable Name
	LPCVOID	dwValue;				  // Variable Value
	union {
		int		wStrcutSize;		  // Size of Strcut
		int		bValueType;           // TD_ADDRESS or TD_VALUE
	};
}TYPEDATA, *PTYPEDATA, *LPTYPEDATA;


interface IFileEngine : public IUnknown
{
	enum VersionType {
		VT_FILE,
		VT_ENGINE
	};
	enum DesiredAccess {
		DA_READ = GENERIC_READ,
		DA_WRITE = GENERIC_WRITE
	};
	enum BaseValueType {
		BVT_VALUE8,
		BVT_VALUE16,
		BVT_VALUE32,
		BVT_VALUE64,
		BVT_STRING,
		BVT_STRUCT,
		BVT_ALLTYPE,
		BVT_INVALID = BVT_ALLTYPE
	};
	enum ValueType {
		VT_CHAR = BVT_VALUE8,
		VT_BYTE = BVT_VALUE8,
		VT_BOOL = BVT_VALUE8,
		VT_SHORT = BVT_VALUE16,
		VT_WORD = BVT_VALUE16,
		VT_INT = BVT_VALUE32,
		VT_LONG = BVT_VALUE32,
		VT_FLOAT = BVT_VALUE32,
		VT_DOUBLE = BVT_VALUE64,
		VT_STRING = BVT_STRING,
		VT_STRUCT = BVT_STRUCT,
	};
	//IFileEngine	1.0 V
	virtual bool CloseFile() = 0;
	virtual bool SaveFile() = 0;
	virtual bool IsEmptyRecorde() = 0;
	virtual bool OpenFile(LPCTSTR lpFile, DWORD dwDesiredAccess = DA_READ | DA_WRITE) = 0;

	virtual bool GetVarString(LPCTSTR lpName, LPTSTR lpStr, int nMax) = 0;
	virtual bool SetVarValue(WORD vType, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType = TD_VALUE) = 0;
	virtual bool SetVarValue(WORD vType, LPCTSTR lpName, LPCVOID pDefaut, size_t size = 0) = 0;
	virtual bool GetVarValue(WORD vType, LPCTSTR lpName, PVOID pWrite) = 0;
	virtual long AddVarValue(LPTYPEDATA lpTypeData, int nLength) = 0;

	virtual bool AddVarValue(WORD vType, LPCTSTR lpName, LPCVOID pDefaut, size_t size = 0) = 0;
	virtual bool AddVarValue(WORD vType, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType = TD_VALUE) = 0;
	virtual bool AddVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize) = 0;
	virtual void OutPutDataInfo() = 0;
	//IFileEngine	1.1 V
	virtual WORD GetVersion(BYTE bType = VT_FILE) = 0;
	virtual bool SetDepict(LPCTSTR lpDepict) = 0;
	virtual bool GetDepict(LPTSTR lpBuffer, int nMax) = 0;
	//IFileEngine	1.2 V
	virtual bool GetLastError(LPTSTR lpStr, int nMax) = 0;

	virtual bool GetVarStruct(LPCTSTR lpName, LPVOID pDefaut) = 0;
	virtual bool SetVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize = 0) = 0;
	virtual bool AddVarString(LPCTSTR lpName, LPCTSTR lpStr, size_t sSize = 0) = 0;
	virtual bool SetVarString(LPCTSTR lpName, LPCTSTR lpStr, size_t sSize = 0) = 0;

	virtual bool GetVarName(WORD vType, int index, LPTSTR lpStr, int nMax) = 0;
	virtual bool SetVarName(WORD vType, int index, LPTSTR lpSetName) = 0;
	virtual bool SetVarName(WORD vType, LPCTSTR lpName, LPTSTR lpSetName) = 0;
	virtual long GetVarIndex(WORD vType, LPCTSTR lpName) = 0;
	virtual bool SetVarIndex(WORD vType, LPCTSTR lpName, int indexto, bool bswap = true) = 0;
	virtual bool SetVarIndex(WORD vType, int index, int indexto, bool bswap = true) = 0;

	virtual long GetVarStringSize(int index) = 0;
	virtual long GetVarStringSize(LPCTSTR lpName) = 0;
	virtual long GetVarStructSize(int index) = 0;
	virtual long GetVarStructSize(LPCTSTR lpName) = 0;

	virtual bool AddVarString(int index, LPCTSTR lpName, LPCTSTR lpStr) = 0;
	virtual bool SetVarString(int index, LPCTSTR lpStr) = 0;
	virtual bool GetVarString(int index, LPTSTR lpStr, int nMax) = 0;
	virtual bool AddVarStruct(int index, LPCTSTR lpName, LPCVOID pDefaut, size_t sSize) = 0;
	virtual bool SetVarStruct(int index, LPCVOID pDefaut, size_t sSize = 0) = 0;
	virtual bool GetVarStruct(int index, LPVOID pWrite) = 0;
	virtual bool AddVarValue(WORD vType, int index, LPCTSTR lpName, LPCVOID pDefaut) = 0;
	virtual bool AddVarValue(WORD vType, int index, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType = TD_VALUE) = 0;
	virtual bool SetVarValue(WORD vType, int index, LPCVOID pDefaut) = 0;
	virtual bool SetVarValue(WORD vType, int index, UINT_PTR dwDefaut, BYTE bValueType = TD_VALUE) = 0;
	virtual bool GetVarValue(WORD vType, int index, LPVOID pWrite) = 0;
	virtual bool DelVarValue(WORD vType, int index) = 0;
	virtual bool DelVarValue(WORD vType, LPCTSTR lpName) = 0;

	virtual bool DelAllValue(WORD vType = BVT_ALLTYPE) = 0;
	virtual long GetVarNumber(WORD vType = BVT_ALLTYPE) = 0;

	//IFileEngine	1.3 V
	virtual bool IsHasVarName(WORD vType, LPCTSTR lpName = nullptr) = 0;
	virtual bool PutVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t size) = 0;
	virtual bool PutVarString(LPCTSTR lpName, LPCTSTR pDefaut, size_t size = 0) = 0;
	virtual bool PutVar(LPCTSTR lpName, const bool& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const char& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const BYTE& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const short& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const WORD& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const int& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const DWORD& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const long& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const INT64& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const UINT64& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const float& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, const double& value) = 0;
	virtual bool PutVar(LPCTSTR lpName, LPCTSTR lpStr) = 0;

};


#ifdef FILEENGINE_EXPORTS
#define FILEENGINE_API __declspec(dllexport)
#else
#define FILEENGINE_API __declspec(dllimport)
#endif

// 此类是从 FileEngine.dll 导出的
class FILEENGINE_API DllFileEngine {
public:
	// TODO: 在此添加您的方法。
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
};

// {8AA0D665-3C3A-4A7E-8FAE-F0A83AB474A2}
extern "C" const GUID CLSID_IFileEngine;
