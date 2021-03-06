// FileEngine.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "FileEngine.h"

#include <iostream>
using namespace std;
// class IFileEngine implementation

ULONG    g_LockNumber = 0;
ULONG    g_IFileEngineNumber = 0;
HANDLE	 g_hModule;

WORD 	FileEngine::msDwVersion = MAKEWORD(1, 3);
LPCTSTR FileEngine::msLpStrDescribe = TEXT("FileEngine 1.3 V");

FileEngine::FileEngine(void)
	:mFileHandle(INVALID_HANDLE_VALUE)
	, m_pData(NULL)
	, m_bSave(false)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IFileEngineNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IFileEngine
	for (WORD i = 0; i < BVT_ALLTYPE; i++)
	{
		mTypeElements[i].vType = i;
		mTypeElements[i].nVar = 0;
		mTypeElements[i].ptrTypeVariable = 0;
	}

	mLpStrDepict = NULL;

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("未执行任何操作。"));

	FileEngine::SetDepict(msLpStrDescribe);
}

FileEngine::~FileEngine(void)
{
	this->CloseFile();

	if (mLpStrDepict != NULL)
	{
		delete[] mLpStrDepict;
	}
}

HRESULT DllFileEngine::DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	if (clsid == CLSID_IFileEngine) {

		FileEngine *pFactory = new FileEngine;

		if (pFactory == NULL) {
			return E_OUTOFMEMORY;
		}

		HRESULT result = pFactory->QueryInterface(iid, ppv);

		if (E_NOINTERFACE == result)
		{
			delete pFactory;
		}

		return result;
	}
	else {
		return CLASS_E_CLASSNOTAVAILABLE;
	}
}

HRESULT  FileEngine::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IFileEngine)
	{
		*ppv = (IFileEngine *)this;
		((IFileEngine *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	 FileEngine::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 FileEngine::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IFileEngineNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}

//public
bool FileEngine::CloseFile()
{
	if (mFileHandle != INVALID_HANDLE_VALUE)
	{
		if (this->m_bSave)
		{
			this->SaveFile();
		}
		this->FreeData();
		CloseHandle(mFileHandle);
		mFileHandle = INVALID_HANDLE_VALUE;
		return true;
	}
	return false;
}
bool FileEngine::SetDepict(LPCTSTR lpDepict)
{
	int len = 1 + lstrlen(lpDepict);

	if (mLpStrDepict != NULL)
		delete[] mLpStrDepict;

	mLpStrDepict = new TCHAR[len];
	mFileHeader.sizeOfDescribe = WORD(sizeof(TCHAR)*(len));
	mFileHeader.ptrTypeHeader = LONG(sizeof(FileHeader) + sizeof(TCHAR)*(len));
	mTypeHeader.ptrTypeElement = LONG(sizeof(TypeHeader) + sizeof(FileHeader) + sizeof(TCHAR)*(len));

	return StringCchCopy(mLpStrDepict, len, lpDepict) != NULL;
}
bool FileEngine::GetDepict(LPTSTR lpBuffer, int iSize)
{
	if (lpBuffer != NULL)
	{
		return StringCchCopy(lpBuffer, iSize, mLpStrDepict) != NULL;
	}
	return false;
}

WORD FileEngine::GetVersion(BYTE bType)
{
	if (bType == VT_FILE)
	{
		return mFileHeader.version;
	}
	return msDwVersion;
}

bool FileEngine::OpenFile(LPCTSTR lpFile, DWORD dwDesiredAccess)
{
	if (mFileHandle != INVALID_HANDLE_VALUE)
	{
		this->FreeData();
		CloseHandle(mFileHandle);
	}

	mFileHandle = CreateFile(lpFile, dwDesiredAccess, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (mFileHandle == INVALID_HANDLE_VALUE)
	{
		mFileHandle = CreateFile(lpFile, dwDesiredAccess | DA_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (mFileHandle == INVALID_HANDLE_VALUE)return false;
		if (CreateFileMap() == false)
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}
	}
	if (CheckFileMap() == false)
	{
		if (CreateFileMap() == false)
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}
		return CheckFileMap();
	}
	return true;
}

bool FileEngine::SaveFile()
{
	if (mFileHandle == INVALID_HANDLE_VALUE)return false;

	BOOL 	ret = TRUE;
	DWORD	nWriteBytes = 0;
	TypeElement eType[BVT_ALLTYPE];
	memcpy(eType, mTypeElements, sizeof(eType));

	FileVariable cFileVariable;
	LpVarString pVarString = NULL;
	LpVarStruct pVarStruct = NULL;
	//计算文件相关偏移量
	this->MapFile(cFileVariable, eType, pVarString, pVarStruct);

	//将文件头、文件描述、格式头、变量表写入文件
	SetFilePointer(mFileHandle, 0, NULL, FILE_BEGIN);
	ret &= WriteFile(mFileHandle, &mFileHeader, sizeof(FileHeader), &nWriteBytes, NULL);
	ret &= WriteFile(mFileHandle, mLpStrDepict, mFileHeader.sizeOfDescribe, &nWriteBytes, NULL);
	ret &= WriteFile(mFileHandle, &mTypeHeader, sizeof(TypeHeader), &nWriteBytes, NULL);
	ret &= WriteFile(mFileHandle, eType, sizeof(TypeElement) * BVT_ALLTYPE, &nWriteBytes, NULL);
	//讲数据按照偏移量写入文件
	this->SaveData(cFileVariable, eType, pVarString, pVarStruct);

	return ret != FALSE;
}

bool FileEngine::IsEmptyRecorde()
{
	if (mFileHandle == INVALID_HANDLE_VALUE)return false;

	for (WORD i = 0; i < BVT_ALLTYPE; i++)
		if (mTypeElements[i].nVar != 0)
			return false;
	return true;
}

bool FileEngine::GetVarString(LPCTSTR lpName, LPTSTR lpStr, int iSize)
{
	if (mTypeElements[BVT_STRING].nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(BVT_STRING, lpName);
	if (lpByte != NULL)
	{
		LpVarString pVarString = LpVarString(lpByte);
		StringCchCopy(lpStr, iSize, pVarString->tValue);
		return	true;
	}
	return false;
}

bool FileEngine::SetVarValue(WORD vType, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)return false;

	if (bValueType == TD_VALUE)
		return SetVarValue(vType, lpName, &dwDefaut);

	return SetVarValue(vType, lpName, LPCVOID(dwDefaut));
}

bool FileEngine::SetVarValue(WORD vType, LPCTSTR lpName, LPCVOID lpDefaut, size_t size)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(vType, lpName);
	if (lpByte != NULL)
	{
		if (vType == BVT_STRUCT)
		{
			LpVarStruct pVarStruct = LpVarStruct(lpByte);
			memcpy(pVarStruct->tValue, lpDefaut, pVarStruct->tSize);
		}
		else if (vType == BVT_STRING)
		{
			LpVarString pVarString = LpVarString(lpByte);
			delete[] pVarString->tValue;
			size_t len = 1 + lstrlen(LPTSTR(lpDefaut));
			size = (size == 0) ? len : min(size, len);
			pVarString->tValue = new TCHAR[size];
			StringCbCopy(pVarString->tValue, size, LPTSTR(lpDefaut));
		}
		else if (vType == BVT_VALUE8)
		{
			LpVar8 pVar = LpVar8(lpByte);
			memcpy(&pVar->tValue, lpDefaut, sizeof(pVar->tValue));
		}
		else if (vType == BVT_VALUE16)
		{
			LpVar16 pVar = LpVar16(lpByte);
			memcpy(&pVar->tValue, lpDefaut, sizeof(pVar->tValue));
		}
		else if (vType == BVT_VALUE32)
		{
			LpVar32 pVar = LpVar32(lpByte);
			memcpy(&pVar->tValue, lpDefaut, sizeof(pVar->tValue));
		}
		else if (vType == BVT_VALUE64)
		{
			LpVar64 pVar = LpVar64(lpByte);
			memcpy(&pVar->tValue, lpDefaut, sizeof(pVar->tValue));
		}
		else
		{
			throw exception("不支持的类型::bool FileEngine::SetVarValue(WORD vType, LPCTSTR lpName, LPCVOID lpDefaut)");
			//lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

			//LPBYTE	lpDefaut = LPBYTE(lpDefaut);
			//long	tSize = GetTypeValueSize(vType);

			//for (int j = 0; j < tSize; j++)
			//	lpByte[j] = lpDefaut[j];
		}
		m_bSave = true;
		return true;
	}
	return false;
}

bool FileEngine::GetVarValue(WORD vType, LPCTSTR lpName, PVOID pWrite)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(vType, lpName);
	if (lpByte != NULL)
	{
		if (vType == BVT_STRUCT)
		{
			LpVarStruct pVarStruct = LpVarStruct(lpByte);
			memcpy(pWrite, pVarStruct->tValue, pVarStruct->tSize);
		}
		else if (vType == BVT_STRING)
		{
			LpVarString pVarString = LpVarString(lpByte);
			lstrcpy(LPTSTR(pWrite), pVarString->tValue);
		}
		else if (vType == BVT_VALUE8)
		{
			LpVar8 pVar = LpVar8(lpByte);
			memcpy(pWrite, &pVar->tValue, sizeof(pVar->tValue));
		}
		else if (vType == BVT_VALUE16)
		{
			LpVar16 pVar = LpVar16(lpByte);
			memcpy(pWrite, &pVar->tValue, sizeof(pVar->tValue));
		}
		else if (vType == BVT_VALUE32)
		{
			LpVar32 pVar = LpVar32(lpByte);
			memcpy(pWrite, &pVar->tValue, sizeof(pVar->tValue));
		}
		else if (vType == BVT_VALUE64)
		{
			LpVar64 pVar = LpVar64(lpByte);
			memcpy(pWrite, &pVar->tValue, sizeof(pVar->tValue));
		}
		else
		{
			throw exception("不支持的类型::bool FileEngine::GetVarValue(WORD vType, LPCTSTR lpName, PVOID pWrite)");
			//lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

			//LPBYTE	lpWrite = LPBYTE(pWrite);
			//long	tSize = GetTypeValueSize(vType);

			//for (int j = 0; j < tSize; j++)
			//	lpWrite[j] = lpByte[j];
		}
		return true;
	}
	return false;
}

long FileEngine::AddVarValue(LPTYPEDATA lpTypeData, int nLength)
{
	long 	laddValue = 0;
	for (int i = 0; i < nLength; i++)
	{
		if ((lpTypeData[i].wType > BVT_VALUE32
			&& lpTypeData[i].bValueType == TD_VALUE)
			|| FindVarValue(lpTypeData[i].wType, lpTypeData[i].tName))
			lpTypeData[i].wType = BVT_INVALID;
	}
	for (WORD wType = 0; wType < BVT_ALLTYPE; wType++)
	{
		long nType = CountTypeInTypeData(wType, lpTypeData, nLength);
		if (nType != 0)
		{
			if (wType == BVT_STRING)
				laddValue += AddVarString(nType, lpTypeData, nLength);
			else if (wType == BVT_STRUCT)
				laddValue += AddVarStruct(nType, lpTypeData, nLength);
			else
				laddValue += AddVarValue(wType, nType, lpTypeData, nLength);
		}
	}
	m_bSave = laddValue != 0;
	return laddValue;
}

bool FileEngine::AddVarValue(WORD vType, LPCTSTR lpName, LPCVOID lpDefaut, size_t size)
{
	if (FindVarValue(vType, lpName) != NULL || vType == BVT_STRUCT)
		return false;

	long 	wSize = -1;// GetTypeValueSize(vType);
	long 	vSize = GetTypeMemVarSize(vType);

	long 	nVar = mTypeElements[vType].nVar;
	PVOID	&pVoid = mMemVariable.pAdd[vType];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (AddVarValue(vType, wSize, LPBYTE(pVoid) + nVar * vSize, lpName, lpDefaut))
	{
		mTypeElements[vType].nVar++;
		m_bSave = true;
		return true;
	}

	return false;
}

bool FileEngine::AddVarValue(WORD vType, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)return false;

	if (bValueType == TD_VALUE)
		return AddVarValue(vType, lpName, &dwDefaut);

	return AddVarValue(vType, lpName, LPCVOID(dwDefaut));
}

bool FileEngine::AddVarStruct(LPCTSTR lpName, LPCVOID lpDefaut, size_t sSize)
{
	if (FindVarValue(BVT_STRUCT, lpName) != NULL || lpDefaut == NULL || sSize == 0)
		return false;

	long 	vSize = sizeof(VarStruct);
	long 	nStrct = mTypeElements[BVT_STRUCT].nVar;
	PVOID	&pVoid = mMemVariable.pAdd[BVT_STRUCT];

	pVoid = MemAllocate(pVoid, (nStrct + 1)*vSize);

	LpVarStruct pVarStruct = LpVarStruct(LPBYTE(pVoid) + nStrct * vSize);

	pVarStruct->tSize = sSize;
	pVarStruct->tValue = malloc(vSize);
	memcpy(pVarStruct->tValue, lpDefaut, sSize);
	StringCchCopy(pVarStruct->tName, VAR_NAME_SIZE, lpName);
	mTypeElements[BVT_STRUCT].nVar++;
	return m_bSave = true;
}

void FileEngine::OutPutDataInfo()
{
	if (!this->IsEmptyRecorde())
	{
		cout << "文件版本::" << int(LOBYTE(mFileHeader.version)) << "." << int(HIBYTE(mFileHeader.version)) << endl;
		cout << "文件描述::" << mLpStrDepict << endl;
	}
	else
	{
		cout << "文件记录为空！" << endl;
	}

	for (WORD wType = 0; wType < BVT_ALLTYPE; wType++)
	{
		if (mTypeElements[wType].nVar != 0)
		{
			int 	n = mTypeElements[wType].nVar;
			LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[wType]);
			WORD	wSize = (WORD)GetTypeValueSize(wType);

			LPCTSTR	lpType[] = {
				TEXT("8位变量"),TEXT("16位变量"),TEXT("32位变量"),TEXT("64位变量"),TEXT("字符串变量"),TEXT("结构体变量")
			};
			cout << lpType[wType] << n << "个{\n";

			//for (int i = 0; i < n; i++)
			//{
			//	cout << "   " << lpByte << "::";

			//	lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

			//	if (wType == BVT_STRUCT)
			//	{
			//		LpVarStruct	pStrcut = LpVarStruct(lpByte - sizeof(TCHAR)*VAR_NAME_SIZE);
			//		LPBYTE	lRead = LPBYTE(pStrcut->tValue);

			//		cout << "{\n      ";
			//		for (size_t ibyte = 0; ibyte < pStrcut->tSize; ibyte++)
			//		{
			//			if (ibyte && !((ibyte) % 35))
			//				cout << "\n      ";
			//			printf("%02X", lRead[ibyte]);
			//		}
			//		printf("\n   }\n");
			//	}
			//	else if (wType == BVT_STRING)
			//	{
			//		LPCTSTR lpText = LPCTSTR(*LPDWORD(lpByte));
			//		ULONG	iSize = lstrlen(lpText);

			//		cout << "{\n      ";
			//		for (size_t ibyte = 0, in = 0; ibyte < iSize; ibyte++)
			//		{
			//			printf("%c，%d", lpText[ibyte], in++);
			//			if (lpText[ibyte] == '\n')
			//			{
			//				in = 0;
			//				cout << "      ";
			//			}
			//			else if (in == 70) {
			//				in = 0;
			//				cout << "\n      ";
			//			}
			//		}
			//		cout << "\n   }\n";
			//		//cout << LPCTSTR(*LPDWORD(lpByte)) << endl;
			//	}
			//	else if (wType == BVT_VALUE64)
			//	{
			//		double	dValue = 0;
			//		LPBYTE	lRead = LPBYTE(&dValue);

			//		for (int ibyte = 0; ibyte < sizeof(double); ibyte++)
			//			lRead[ibyte] = lpByte[ibyte];

			//		printf("\r   %#08X::%s = %f\n", *((LPINT)&dValue), (lpByte - sizeof(TCHAR)*VAR_NAME_SIZE), dValue);
			//	}
			//	else
			//	{
			//		DWORD	dValue = 0;
			//		LPBYTE	lRead = LPBYTE(&dValue);

			//		for (int ibyte = 0; ibyte < wSize; ibyte++)
			//			lRead[ibyte] = lpByte[ibyte];

			//		printf("\r   %010u::%#08x::%s = %d\n", dValue, dValue, lpByte - sizeof(TCHAR)*VAR_NAME_SIZE, dValue);
			//	}
			//	lpByte += wSize;
			//}
			cout << "}\n";
		}
	}
}

//protected
void FileEngine::MapFile(FileVariable &cFileVariable, TypeElement eType[], LpVarString &pVarString, LpVarStruct &pVarStruct)
{
	//数据储存从eType[BVT_ALLTYPE]后面开始
	LONG lLastFanew = mTypeHeader.ptrTypeElement + sizeof(mTypeElements);

	for (int i = 0; i < BVT_ALLTYPE; i++)
	{
		if (eType[i].nVar != 0)
		{
			//数据类型【i】的偏离eType[i].ptrTypeVariable
			eType[i].ptrTypeVariable = lLastFanew;
			//根据本【i】类型尺寸计算下一类型起始偏离
			lLastFanew += eType[i].nVar*GetTypeFileVarSize(i);
		}
		else
		{
			eType[i].ptrTypeVariable = 0;
		}
	}
	for (int j = 0; j < BVT_ALLTYPE; j++)
	{
		if (mTypeElements[j].nVar != 0)
		{
			//long lvSize = GetTypeMemVarSize(j);		//单个类型变量在内存的尺寸
			//long lfSize = GetTypeFileVarSize(j);	//单个类型变量在文件的尺寸

			////根据元素个数mTypeElements[j].nVar申请多个文件类型变量到内存
			//tFileVariable.pFileAdd[j] = malloc(mTypeElements[j].nVar*lfSize);
			////分别映射多个文件变量和内存变量地址到lpFileByte，lpMemVByte
			//LPBYTE	lpMemVByte = LPBYTE(mMemVariable.pAdd[j]);
			//LPBYTE	lpFileByte = LPBYTE(tFileVariable.pFileAdd[j]);

			//for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++)
			//{
			//	//分别映射单个文件变量和内存变量地址到lpFileVar，lpvMemVar
			//	LPBYTE lpvMemVar = lpMemVByte + nVar * lvSize;
			//	LPBYTE lpFileVar = lpFileByte + nVar * lfSize;
			//	//映射单个文件变量数据域地址到lpData
			//	//sizeof(long) 变量前四个字节为变量名字符串偏离地址
			//	LPBYTE lpData = lpFileVar + sizeof(long);
			//	//映射单个内存变量数据域地址到lpData
			//	LPBYTE lpRead = lpvMemVar + sizeof(TCHAR)*VAR_NAME_SIZE;

			//	//映射单个文件变量地址（变量的变量名指针域地址）
			//	LPLONG lpName = LPLONG(lpFileVar);
			//	//定义当前偏移量给 变量的变量名指针
			//	lpName[0] = lLastFanew;
			//	//根据当前变量名的长度确定写一个偏移起始地址
			//	lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(LPCTSTR(lpvMemVar))));
			//	//从内存变量 拷贝 数据 到 本件变量
			//	for (int ibyte = 0, nByte = GetTypeValueSize(j); ibyte < nByte; ibyte++)
			//	{
			//		lpData[ibyte] = lpRead[ibyte];
			//	}
			//}

			switch (j)
			{
			case BVT_VALUE8:
				cFileVariable.pFileVar8 = new FileVar8[mTypeElements[j].nVar];
				for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++) {
					cFileVariable.pFileVar8[nVar].tName = lLastFanew;
					cFileVariable.pFileVar8[nVar].tValue = mMemVariable.pVar8[nVar].tValue;
					//根据当前变量名的长度确定写一个偏移起始地址
					lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(mMemVariable.pVar8[nVar].tName)));
				}
				break;
			case BVT_VALUE16:
				cFileVariable.pFileVar16 = new FileVar16[mTypeElements[j].nVar];
				for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++) {
					cFileVariable.pFileVar16[nVar].tName = lLastFanew;
					cFileVariable.pFileVar16[nVar].tValue = mMemVariable.pVar16[nVar].tValue;
					//根据当前变量名的长度确定写一个偏移起始地址
					lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(mMemVariable.pVar16[nVar].tName)));
				}
				break;
			case BVT_VALUE32:
				cFileVariable.pFileVar32 = new FileVar32[mTypeElements[j].nVar];
				for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++) {
					cFileVariable.pFileVar32[nVar].tName = lLastFanew;
					cFileVariable.pFileVar32[nVar].tValue = mMemVariable.pVar32[nVar].tValue;
					//根据当前变量名的长度确定写一个偏移起始地址
					lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(mMemVariable.pVar32[nVar].tName)));
				}
				break;
			case BVT_VALUE64:
				cFileVariable.pFileVar64 = new FileVar64[mTypeElements[j].nVar];
				for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++) {
					cFileVariable.pFileVar64[nVar].tName = lLastFanew;
					cFileVariable.pFileVar64[nVar].tValue = mMemVariable.pVar64[nVar].tValue;
					//根据当前变量名的长度确定写一个偏移起始地址
					lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(mMemVariable.pVar64[nVar].tName)));
				}
				break;
			case BVT_STRING:
				cFileVariable.pFileVarString = new FileVarString[mTypeElements[j].nVar];
				for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++) {
					cFileVariable.pFileVarString[nVar].tName = lLastFanew;
					cFileVariable.pFileVarString[nVar].tValue = mMemVariable.pVarString[nVar].tValue;
					//根据当前变量名的长度确定写一个偏移起始地址
					lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(mMemVariable.pVarString[nVar].tName)));
				}
				break;
			case BVT_STRUCT:
				cFileVariable.pFileVarStruct = new FileVarStruct[mTypeElements[j].nVar];
				for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++) {
					cFileVariable.pFileVarStruct[nVar].tName = lLastFanew;
					cFileVariable.pFileVarStruct[nVar].tSize = mMemVariable.pVarStruct[nVar].tSize;
					cFileVariable.pFileVarStruct[nVar].tValue = mMemVariable.pVarStruct[nVar].tValue;
					//根据当前变量名的长度确定写一个偏移起始地址
					lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(mMemVariable.pVarStruct[nVar].tName)));
				}
				break;
			default:
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////
	if (mMemVariable.pVarString != NULL && eType[BVT_STRING].nVar > 0)
	{
		pVarString = new VarString[eType[BVT_STRING].nVar];
		memcpy(pVarString, mMemVariable.pVarString, sizeof(VarString)*eType[BVT_STRING].nVar);
		for (int i = 0; i < eType[BVT_STRING].nVar; i++)
		{
			//mMemVariable.pVarString[i].tAddres = lLastFanew;
			cFileVariable.pFileVarString[i].tAddres = lLastFanew;
			lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(pVarString[i].tValue)));
		}
	}
	if (mMemVariable.pVarStruct != NULL && eType[BVT_STRUCT].nVar > 0)
	{
		pVarStruct = new VarStruct[eType[BVT_STRUCT].nVar];
		memcpy(pVarStruct, mMemVariable.pVarStruct, sizeof(VarStruct)*eType[BVT_STRUCT].nVar);
		for (int i = 0; i < eType[BVT_STRUCT].nVar; i++)
		{
			//mMemVariable.pVarStruct[i].tAddres = lLastFanew;
			cFileVariable.pFileVarStruct[i].tAddres = lLastFanew;
			lLastFanew += long(mMemVariable.pVarStruct[i].tSize);
		}
	}
}

BOOL FileEngine::SaveData(FileVariable &cFileVariable, TypeElement eType[], LpVarString &pVarString, LpVarStruct &pVarStruct)
{
	BOOL 	ret = TRUE;
	DWORD	nWriteBytes = 0;
	for (int i = 0; i < BVT_ALLTYPE; i++)
	{
		if (eType[i].nVar != 0)
		{
			ret &= WriteFile(mFileHandle, cFileVariable.pFileAdd[i], eType[i].nVar*GetTypeFileVarSize(i), &nWriteBytes, NULL);
		}
	}
	for (int j = 0; j < BVT_ALLTYPE; j++)
	{//储存变量名 
		if (eType[j].nVar != 0)
		{
			long	lvSize = GetTypeMemVarSize(j);
			LPBYTE	lpVByte = LPBYTE(mMemVariable.pAdd[j]);
			for (int nVar = 0; nVar < mTypeElements[j].nVar; nVar++)
			{
				LPBYTE lpvVar = lpVByte + nVar * lvSize;
				LPLONG lpName = LPLONG(lpvVar);
				ret &= WriteFile(mFileHandle, lpvVar, sizeof(TCHAR)*(1 + lstrlen(PTSTR(lpvVar))), &nWriteBytes, NULL);
			}
		}
	}
	if (pVarString != NULL)
	{//储存字符串数据 
		for (int i = 0; i < eType[BVT_STRING].nVar; i++)
		{
			ret &= WriteFile(mFileHandle, pVarString[i].tValue, sizeof(TCHAR)*(1 + lstrlen(pVarString[i].tValue)), &nWriteBytes, NULL);
		}
		memcpy(mMemVariable.pVarString, pVarString, sizeof(VarString)*eType[BVT_STRING].nVar);

		delete[] pVarString;
	}
	if (pVarStruct != NULL)
	{
		for (int i = 0; i < eType[BVT_STRUCT].nVar; i++)
		{
			ret &= WriteFile(mFileHandle, pVarStruct[i].tValue, DWORD(pVarStruct[i].tSize), &nWriteBytes, NULL);
		}
		memcpy(mMemVariable.pVarStruct, pVarStruct, sizeof(VarStruct)*eType[BVT_STRUCT].nVar);

		delete[] pVarStruct;
	}
	return ret;
}

void* FileEngine::MemAllocate(void* address, size_t newsize)
{
	if (address == NULL)
		address = malloc(newsize);
	else
		address = realloc(address, newsize);
	return address;
}

void FileEngine::FreeData()
{
	for (int i = 0; i < BVT_STRING; i++)
	{
		mTypeElements[i].nVar = 0;
		if (mMemVariable.pAdd[i])
		{
			free(mMemVariable.pAdd[i]);
			mMemVariable.pAdd[i] = NULL;
		}
	}

	if (mMemVariable.pVarString != 0)
	{
		for (int i = 0; i < mTypeElements[BVT_STRING].nVar; i++)
			delete[] mMemVariable.pVarString[i].tValue;

		mTypeElements[BVT_STRING].nVar = 0;
		free(mMemVariable.pVarString);
		mMemVariable.pVarString = NULL;
	}
	if (mMemVariable.pVarStruct != 0)
	{
		for (int i = 0; i < mTypeElements[BVT_STRUCT].nVar; i++)
		{
			//printf("free pVarStruct = %#x\n",DWORD(mMemVariable.pVarStruct[i].tValue));
			free(mMemVariable.pVarStruct[i].tValue);
		}

		mTypeElements[BVT_STRUCT].nVar = 0;
		free(mMemVariable.pVarStruct);
		mMemVariable.pVarStruct = NULL;
	}
}

bool FileEngine::CreateFileMap()
{
	if (mFileHandle == INVALID_HANDLE_VALUE)return false;

	int len = 1 + lstrlen(mLpStrDepict);
	mFileHeader.sizeOfDescribe = WORD(sizeof(TCHAR)*(len));
	mFileHeader.ptrTypeHeader = LONG(sizeof(FileHeader) + sizeof(TCHAR)*(len));
	mTypeHeader.ptrTypeElement = LONG(sizeof(TypeHeader) + sizeof(FileHeader) + sizeof(TCHAR)*(len));

	BOOL 	ret = TRUE;
	DWORD	nWriteBytes = 0;
	SetFilePointer(mFileHandle, 0, NULL, FILE_BEGIN);

	ret &= WriteFile(mFileHandle, &mFileHeader, sizeof(FileHeader), &nWriteBytes, NULL);
	ret &= WriteFile(mFileHandle, mLpStrDepict, mFileHeader.sizeOfDescribe, &nWriteBytes, NULL);
	ret &= WriteFile(mFileHandle, &mTypeHeader, sizeof(TypeHeader), &nWriteBytes, NULL);
	ret &= WriteFile(mFileHandle, mTypeElements, BVT_ALLTYPE * sizeof(TypeElement), &nWriteBytes, NULL);

	return ret != FALSE;
}
void FileEngine::ReadFileMap(LPVOID lpBase, LpFileHeader pFileHeader, LpTypeHeader pTypeHeader)
{
	mFileHeader = pFileHeader[0];
	mTypeHeader = pTypeHeader[0];

	FileEngine::SetDepict(LPCTSTR(LPBYTE(lpBase) + sizeof(FileHeader)));

	if (mTypeHeader.ntype > BVT_ALLTYPE) mTypeHeader.ntype = BVT_ALLTYPE;

	memcpy(mTypeElements, LPBYTE(lpBase) + pTypeHeader->ptrTypeElement, mTypeHeader.ntype * sizeof(TypeElement));

	FileVariable tFileVariable;
	for (WORD wType = 0; wType < mTypeHeader.ntype; wType++)
	{
		int nVar = mTypeElements[wType].nVar;
		if (nVar > 0)
		{
			switch (wType)
			{
			case BVT_VALUE8:
				mMemVariable.pVar8 = new Var8[nVar];
				tFileVariable.pFileVar8 = new FileVar8[nVar];
				memcpy(tFileVariable.pFileVar8, LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, sizeof(FileVar8)*nVar);
				for (int n = 0; n < nVar; n++) {
					mMemVariable.pVar8[n].tValue = tFileVariable.pFileVar8[n].tValue;
					//lstrcpyn(LPTSTR(mMemVariable.pVar8[n].tName), LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar8[n].tName), VAR_NAME_SIZE);
					StringCchCopy(mMemVariable.pVar8[n].tName, VAR_NAME_SIZE, LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar8[n].tName));
				}
				break;
			case BVT_VALUE16:
				mMemVariable.pVar16 = new Var16[nVar];
				tFileVariable.pFileVar16 = new FileVar16[nVar];
				memcpy(tFileVariable.pFileVar16, LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, sizeof(FileVar16)*nVar);
				for (int n = 0; n < nVar; n++) {
					mMemVariable.pVar16[n].tValue = tFileVariable.pFileVar16[n].tValue;
					//lstrcpyn(LPTSTR(mMemVariable.pVar16[n].tName), LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar16[n].tName), VAR_NAME_SIZE);
					StringCchCopy(mMemVariable.pVar16[n].tName, VAR_NAME_SIZE, LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar16[n].tName));
				}
				break;
			case BVT_VALUE32:
				mMemVariable.pVar32 = new Var32[nVar];
				tFileVariable.pFileVar32 = new FileVar32[nVar];
				memcpy(tFileVariable.pFileVar32, LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, sizeof(FileVar32)*nVar);
				for (int n = 0; n < nVar; n++) {
					mMemVariable.pVar32[n].tValue = tFileVariable.pFileVar32[n].tValue;
					//lstrcpyn(LPTSTR(mMemVariable.pVar32[n].tName), LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar32[n].tName), VAR_NAME_SIZE);
					StringCchCopy(mMemVariable.pVar32[n].tName, VAR_NAME_SIZE, LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar32[n].tName));
				}
				break;
			case BVT_VALUE64:
				mMemVariable.pVar64 = new Var64[nVar];
				tFileVariable.pFileVar64 = new FileVar64[nVar];
				memcpy(tFileVariable.pFileVar64, LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, sizeof(FileVar64)*nVar);
				for (int n = 0; n < nVar; n++) {
					mMemVariable.pVar64[n].tValue = tFileVariable.pFileVar64[n].tValue;
					//lstrcpyn(LPTSTR(mMemVariable.pVar64[n].tName), LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar64[n].tName), VAR_NAME_SIZE);
					StringCchCopy(mMemVariable.pVar64[n].tName, VAR_NAME_SIZE, LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVar64[n].tName));
				}
				break;
			case BVT_STRING:
				mMemVariable.pVarString = new VarString[nVar];
				tFileVariable.pFileVarString = new FileVarString[nVar];
				memcpy(tFileVariable.pFileVarString, LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, sizeof(FileVarString)*nVar);
				for (int n = 0; n < nVar; n++) {
					mMemVariable.pVarString[n].tAddres = tFileVariable.pFileVarString[n].tAddres;
					//lstrcpyn(LPTSTR(mMemVariable.pVarString[n].tName), LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVarString[n].tName), VAR_NAME_SIZE);
					StringCchCopy(mMemVariable.pVarString[n].tName, VAR_NAME_SIZE, LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVarString[n].tName));
				}
				break;
			case BVT_STRUCT:
				mMemVariable.pVarStruct = new VarStruct[nVar];
				tFileVariable.pFileVarStruct = new FileVarStruct[nVar];
				memcpy(tFileVariable.pFileVarStruct, LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, sizeof(FileVarStruct)*nVar);
				for (int n = 0; n < nVar; n++) {
					mMemVariable.pVarStruct[n].tSize = tFileVariable.pFileVarStruct[n].tSize;
					mMemVariable.pVarStruct[n].tAddres = tFileVariable.pFileVarStruct[n].tAddres;
					//lstrcpyn(LPTSTR(mMemVariable.pVarStruct[n].tName), LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVarStruct[n].tName), VAR_NAME_SIZE);
					StringCchCopy(mMemVariable.pVarStruct[n].tName, VAR_NAME_SIZE, LPCTSTR(LPBYTE(lpBase) + tFileVariable.pFileVarStruct[n].tName));
				}
				break;
			default:
				break;
			}

			//size_t	vsize = GetTypeMemVarSize(wType), avsize = mTypeElements[wType].nVar*vsize;
			//size_t	fsize = GetTypeFileVarSize(wType), afsize = mTypeElements[wType].nVar*fsize;

			//LPBYTE	lpVByte = LPBYTE(mMemVariable.pAdd[wType] = malloc(avsize));
			//LPBYTE	lpFByte = LPBYTE(tFileVariable.pFileAdd[wType] = malloc(afsize));

			//memset(mMemVariable.pAdd[wType], 0, avsize);
			//memcpy(tFileVariable.pFileAdd[wType], LPBYTE(lpBase) + mTypeElements[wType].ptrTypeVariable, afsize);

			//for (int nVar = 0; nVar < mTypeElements[wType].nVar; nVar++)
			//{
			//	LPBYTE lpvVar = lpVByte + nVar * vsize;
			//	LPBYTE lpFileVar = lpFByte + nVar * fsize;
			//	LPBYTE lpRead = lpFileVar + sizeof(long);
			//	LPBYTE lpData = lpvVar + sizeof(TCHAR)*VAR_NAME_SIZE;
			//	LPLONG lpName = LPLONG(lpFileVar);

			//	lstrcpyn(LPTSTR(lpvVar), LPCTSTR(LPBYTE(lpBase) + lpName[0]), VAR_NAME_SIZE);

			//	for (int ibyte = 0, nByte = GetTypeValueSize(wType); ibyte < nByte; ibyte++)
			//	{
			//		lpData[ibyte] = lpRead[ibyte];
			//	}
			//}
		}
	}
	if (mMemVariable.pVarString != 0)
	{
		for (int i = 0; i < mTypeElements[BVT_STRING].nVar; i++)
		{
			LPTSTR lpStr = LPTSTR(LPBYTE(lpBase) + mMemVariable.pVarString[i].tAddres);
			int len = lstrlen(lpStr) + 1;
			mMemVariable.pVarString[i].tValue = new TCHAR[len];
			//lstrcpyn(mMemVariable.pVarString[i].tValue, lpStr, len);
			StringCchCopy(mMemVariable.pVarString[i].tValue, len, lpStr);
		}
	}
	if (mMemVariable.pVarStruct != 0)
	{
		for (int i = 0; i < mTypeElements[BVT_STRUCT].nVar; i++)
		{
			PVOID lpData = LPBYTE(lpBase) + mMemVariable.pVarStruct[i].tAddres;
			size_t 	size = mMemVariable.pVarStruct[i].tSize;
			mMemVariable.pVarStruct[i].tValue = malloc(size);
			memcpy(mMemVariable.pVarStruct[i].tValue, lpData, size);
		}
	}
}

bool FileEngine::CheckFileMap()
{
	if (mFileHandle == INVALID_HANDLE_VALUE)return false;

	HANDLE hMap = CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
	LPVOID lpBase = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);

	if (lpBase == NULL)  return false;

	bool ret = true;

	LpFileHeader pFileHeader = LpFileHeader(lpBase);
	LpTypeHeader pTypeHeader = LpTypeHeader((PBYTE)lpBase + pFileHeader->ptrTypeHeader);

	FileHeader header;
	ret = ret && pFileHeader->size == sizeof(FileHeader);
	ret = ret && pFileHeader->magic == IMAGE_SIGNATURE_FILE;
	ret = ret && pFileHeader->platform == header.platform;
	ret = ret && pTypeHeader->size == sizeof(TypeHeader);
	ret = ret && pTypeHeader->magic == IMAGE_SIGNATURE_TYPE;

	if (ret != FALSE)
	{
		this->ReadFileMap(lpBase, pFileHeader, pTypeHeader);
	}

	UnmapViewOfFile(lpBase);
	CloseHandle(hMap);

	//if (ret == FALSE)
	//{
	//	this->FreeData();
	//	CloseHandle(mFileHandle);
	//	mFileHandle = INVALID_HANDLE_VALUE;
	//}

	return ret;
}

long FileEngine::GetTypeValueSize(WORD vType)
{
	//long sizes[] = {
	//	sizeof(FileVar8) - sizeof(LONG),
	//	sizeof(FileVar16) - sizeof(LONG),
	//	sizeof(FileVar32) - sizeof(LONG),
	//	sizeof(FileVar64) - sizeof(LONG),
	//	sizeof(FileVarString) - sizeof(LONG),
	//	sizeof(FileVarStruct) - sizeof(LONG),
	//};
	//return sizes[vType];
	long size = 1;

	if (vType == BVT_STRING)
		size = sizeof(PTSTR);
	else if (vType == BVT_STRUCT)
		size = sizeof(size_t) + sizeof(PVOID);
	else
		while (vType--)size *= 2;

	return size;
}
long FileEngine::GetTypeMemVarSize(WORD vType)
{
	long sizes[] = {
		sizeof(Var8),
		sizeof(Var16),
		sizeof(Var32),
		sizeof(Var64),
		sizeof(VarString),
		sizeof(VarStruct),
	};
//#ifdef _DEBUG
//	string names[] = {
//		typeid(Var8).name(),
//		typeid(Var16).name(),
//		typeid(Var32).name(),
//		typeid(Var64).name(),
//		typeid(VarString).name(),
//		typeid(VarStruct).name(),
//	};
//	string log = "";
//	char szlog[MAX_PATH];
//	for (int i = 0; i < sizeof(sizes) / sizeof(long); i++) {
//		sprintf_s(szlog, MAX_PATH,"%s\t:sizes->%d\ttypes->%d\n", names[i].c_str(), sizes[i], GetTypeValueSize(i) + sizeof(TCHAR)*VAR_NAME_SIZE);
//		log += szlog;
//	}
//	MessageBoxA(NULL, log.c_str(), "MemVarSize", MB_OK);
//#endif // DEBUG
	return sizes[vType];
	//return GetTypeValueSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
}
long FileEngine::GetTypeFileVarSize(WORD vType)
{
	long sizes[] = {
		sizeof(FileVar8),
		sizeof(FileVar16),
		sizeof(FileVar32),
		sizeof(FileVar64),
		sizeof(FileVarString),
		sizeof(FileVarStruct),
	};
//#ifdef _DEBUG
//	string names[] = {
//		typeid(FileVar8).name(),
//		typeid(FileVar16).name(),
//		typeid(FileVar32).name(),
//		typeid(FileVar64).name(),
//		typeid(FileVarString).name(),
//		typeid(FileVarStruct).name(),
//	};
//	string log = "";
//	char szlog[MAX_PATH];
//	for (int i = 0; i < sizeof(sizes) / sizeof(long); i++) {
//		sprintf_s(szlog, MAX_PATH, "%s\t:sizes->%d\ttypes->%d\n", names[i].c_str(), sizes[i], GetTypeValueSize(i) + sizeof(LONG));
//		log += szlog;
//	}
//	MessageBoxA(NULL, log.c_str(), "FileVarSize", MB_OK);
//#endif // DEBUG
	return sizes[vType];
	//return GetTypeValueSize(vType) + sizeof(LONG);
}
/*
 * 统计数据数组中 指定数据类型 的个数
 */
long FileEngine::CountTypeInTypeData(WORD wType, LPTYPEDATA lpTypeData, int nLength)
{
	long nType = 0;

	for (int n = 0; n < nLength; n++)
		if (lpTypeData[n].wType == wType)
			nType++;

	return nType;
}
bool FileEngine::AddVarValue(WORD vType, long wSize, LPBYTE lpByte, LPCTSTR lpName, LPCVOID lpDefaut, size_t size)
{
	//StringCchCopy(LPTSTR(lpByte), VAR_NAME_SIZE, lpName);

	if (vType == BVT_STRING) {
		LPCTSTR lpCStr = LPCTSTR(lpDefaut);
		LpVarString pVarString = LpVarString(lpByte);
		size_t len = 1 + lstrlen(lpCStr);
		size = (size == 0) ? len : min(size, len);
		pVarString->tValue = new TCHAR[size];
		StringCchCopy(pVarString->tValue, size, LPTSTR(lpDefaut));
	} else if(vType == BVT_VALUE8) {
		LpVar8 lpVar = LpVar8(lpByte);
		int sizeName = sizeof(lpVar->tName);
		int sizeValue = sizeof(lpVar->tValue);
		StringCbCopy(lpVar->tName, sizeName, lpName);
		memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
	} else if (vType == BVT_VALUE16) {
		LpVar16 lpVar = LpVar16(lpByte);
		int sizeName = sizeof(lpVar->tName);
		int sizeValue = sizeof(lpVar->tValue);
		StringCbCopy(lpVar->tName, sizeName, lpName);
		memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
	} else if (vType == BVT_VALUE32) {
		LpVar32 lpVar = LpVar32(lpByte);
		int sizeName = sizeof(lpVar->tName);
		int sizeValue = sizeof(lpVar->tValue);
		StringCbCopy(lpVar->tName, sizeName, lpName);
		memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
	} else if (vType == BVT_VALUE64) {
		LpVar64 lpVar = LpVar64(lpByte);
		int sizeName = sizeof(lpVar->tName);
		int sizeValue = sizeof(lpVar->tValue);
		StringCbCopy(lpVar->tName, sizeName, lpName);
		memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
	} else {
		throw exception("不支持的类型::bool FileEngine::AddVarValue(WORD vType, long wSize, LPBYTE lpByte, LPCTSTR lpName, LPCVOID lpDefaut)");
 		//LPBYTE lpDefaut = LPBYTE(lpDefaut) ? LPBYTE(lpDefaut) : LPBYTE(&lpDefaut);

		//lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

		//for (long i = 0; i < wSize; i++)
		//	lpByte[i] = lpDefaut[i];
	}
	return true;
}

long FileEngine::AddVarValue(LPBYTE lpByte, WORD wType, WORD wSize, LPTYPEDATA lpTypeData, int nLength)
{
	long	nVar = 0;
	for (int n = 0; n < nLength; n++)
	{
		LPBYTE lpDefaut = NULL;
		if (lpTypeData[n].wType == wType)
		{
			if (lpTypeData[n].bValueType == TD_VALUE)
				lpDefaut = LPBYTE(&lpTypeData[n].dwValue);
			else
				lpDefaut = LPBYTE(lpTypeData[n].dwValue);
			switch (wType)
			{
			case BVT_VALUE8: {
				LpVar8 lpVar = LpVar8(lpByte);
				int sizeName = sizeof(lpVar->tName);
				int sizeValue = sizeof(lpVar->tValue);
				//lstrcpyn(lpVar->tName, lpTypeData[n].tName, sizeName);
				StringCbCopy(lpVar->tName, sizeName, lpTypeData[n].tName);
				memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
				lpByte = LPBYTE(lpVar + 1); nVar++;
				break;
			}
			case BVT_VALUE16: {
				LpVar16 lpVar = LpVar16(lpByte);
				int sizeName = sizeof(lpVar->tName);
				int sizeValue = sizeof(lpVar->tValue);
				//lstrcpyn(lpVar->tName, lpTypeData[n].tName, sizeName);
				StringCbCopy(lpVar->tName, sizeName, lpTypeData[n].tName);
				memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
				lpByte = LPBYTE(lpVar + 1); nVar++;
				break;
			}
			case BVT_VALUE32: {
				LpVar32 lpVar = LpVar32(lpByte);
				int sizeName = sizeof(lpVar->tName);
				int sizeValue = sizeof(lpVar->tValue);
				//lstrcpyn(lpVar->tName, lpTypeData[n].tName, sizeName);
				StringCbCopy(lpVar->tName, sizeName, lpTypeData[n].tName);
				memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
				lpByte = LPBYTE(lpVar + 1); nVar++;
				break;
			}
			case BVT_VALUE64: {
				LpVar64 lpVar = LpVar64(lpByte);
				int sizeName = sizeof(lpVar->tName);
				int sizeValue = sizeof(lpVar->tValue);
				//lstrcpyn(lpVar->tName, lpTypeData[n].tName, sizeName);
				StringCbCopy(lpVar->tName, sizeName, lpTypeData[n].tName);
				memcpy_s(&lpVar->tValue, sizeValue, lpDefaut, sizeValue);
				lpByte = LPBYTE(lpVar + 1); nVar++;
				break;
			}
			default: {
				throw exception("不支持的类型::long FileEngine::AddVarValue(LPBYTE lpByte, WORD wType, WORD wSize, LPTYPEDATA lpTypeData, int nLength)");
				//lstrcpyn(LPTSTR(lpByte), lpTypeData[n].tName, VAR_NAME_SIZE);

				//lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

				//for (int i = 0; i < wSize; i++)
				//	lpByte[i] = lpDefaut[i];

				//nVar++;
				//lpByte += wSize;
				break;
			}
			}
		}
	}
	return nVar;
}

long FileEngine::AddVarValue(WORD wType, long nType, LPTYPEDATA lpTypeData, int nLength)
{
	long	wSize = GetTypeValueSize(wType);
	long 	vSize = GetTypeMemVarSize(wType);

	PVOID&	pVoid = mMemVariable.pAdd[wType];
	ULONG	lType = mTypeElements[wType].nVar + WORD(nType);

	pVoid = MemAllocate(pVoid, lType*vSize);
	nType = AddVarValue(LPBYTE(pVoid) + (lType - nType)*vSize, wType, WORD(wSize), lpTypeData, nLength);

	mTypeElements[wType].nVar += WORD(nType);

	return	nType;
}

long FileEngine::AddVarString(LpVarString pVarString, LPTYPEDATA lpTypeData, int nLength)
{
	long	nVar = 0;
	for (int n = 0; n < nLength; n++)
	{
		LPBYTE lpDefaut = NULL;
		if (lpTypeData[n].wType == BVT_STRING)
		{
			pVarString->tValue = new TCHAR[sizeof(TCHAR)*(1 + lstrlen(PTSTR(lpTypeData[n].dwValue)))];

			//lstrcpyn(pVarString->tName, lpTypeData[n].tName, VAR_NAME_SIZE);
			StringCchCopy(pVarString->tName, VAR_NAME_SIZE, lpTypeData[n].tName);
			lstrcpy(pVarString->tValue, LPCTSTR(lpTypeData[n].dwValue));

			nVar++;
			pVarString++;
		}
	}
	return nVar;
}

long FileEngine::AddVarString(long nType, LPTYPEDATA lpTypeData, int nLength)
{
	PVOID&	pVoid = mMemVariable.pAdd[BVT_STRING];
	long	lType = mTypeElements[BVT_STRING].nVar + nType;

	pVoid = MemAllocate(pVoid, lType * sizeof(VarString));
	nType = AddVarString(mMemVariable.pVarString + (lType - nType), lpTypeData, nLength);

	mTypeElements[BVT_STRING].nVar += WORD(nType);

	return nType;
}

long FileEngine::AddVarStruct(LpVarStruct pVarStruct, LPTYPEDATA lpTypeData, int nLength)
{
	long	nVar = 0;
	for (int n = 0; n < nLength; n++)
	{
		LPBYTE lpDefaut = NULL;
		if (lpTypeData[n].wType == BVT_STRUCT)
		{
			pVarStruct->tSize = lpTypeData[n].wStrcutSize;
			pVarStruct->tValue = malloc(lpTypeData[n].wStrcutSize);

			//printf("malloc pVarStruct = %#x\n",DWORD(pVarStruct->tValue));

			memcpy(pVarStruct->tValue, lpTypeData[n].dwValue, lpTypeData[n].wStrcutSize);
			//lstrcpyn(pVarStruct->tName, lpTypeData[n].tName, VAR_NAME_SIZE);
			StringCchCopy(pVarStruct->tName, VAR_NAME_SIZE, lpTypeData[n].tName);

			nVar++;
			pVarStruct++;
		}
	}
	return nVar;
}

long FileEngine::AddVarStruct(long nType, LPTYPEDATA lpTypeData, int nLength)
{
	PVOID&	pVoid = mMemVariable.pAdd[BVT_STRUCT];
	WORD	lType = mTypeElements[BVT_STRUCT].nVar + WORD(nType);

	pVoid = MemAllocate(pVoid, lType * sizeof(VarStruct));
	nType = AddVarStruct(mMemVariable.pVarStruct + (lType - nType), lpTypeData, nLength);

	mTypeElements[BVT_STRUCT].nVar += WORD(nType);

	return nType;
}

LPBYTE FileEngine::FindVarValue(WORD vType, LPCTSTR lpName)
{
	int 	m = 0, n = mTypeElements[vType].nVar;
	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[vType]);

	long vSize = GetTypeMemVarSize(vType);//GetTypeValueSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	do
	{
		if (lstrcmp(LPCTSTR(lpByte), lpName) == 0)
			return lpByte;
		lpByte += vSize;
	} while (++m < n);

	return NULL;
}
/**
Name: IFileEngine	1.2 V

Last Change:
*/
LPBYTE FileEngine::FindVarValue(WORD vType, int index)
{
	int 	m = 0, n = mTypeElements[vType].nVar;
	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[vType]);

	long vSize = GetTypeMemVarSize(vType);//GetTypeValueSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	if (index >= 0 && index < n)
	{
		return lpByte + index * vSize;
	}

	return NULL;
}
auto FileEngine::MapBaseValueType(size_t size) -> BaseValueType
{
	static BaseValueType maps[] = {
		BVT_VALUE8,
		BVT_VALUE8,
		BVT_VALUE16,
		BVT_VALUE16,
		BVT_VALUE32,
		BVT_VALUE32,
		BVT_VALUE32,
		BVT_VALUE32,
		BVT_VALUE64,
	};
	if (size >= sizeof(maps)/sizeof(BaseValueType)) {
		return BVT_STRUCT;
	}
	return maps[size];
}
/**
Name: IFileEngine	1.2 V

Last Change:
*/
bool FileEngine::GetLastError(LPTSTR lpStr, int iSize)
{
	//lstrcpyn(lpStr, mSzLastError, min(256, iSize));
	StringCchCopy(lpStr, min(iSize, sizeof(mSzLastError) / sizeof(TCHAR)), mSzLastError);
	return true;
}

bool FileEngine::GetVarStruct(LPCTSTR lpName, LPVOID pWrite)
{
	return GetVarValue(BVT_STRUCT, lpName, pWrite);
}

bool FileEngine::SetVarStruct(LPCTSTR lpName, LPCVOID lpDefaut, size_t sSize)
{
	if (sSize == 0)
	{
		return SetVarValue(BVT_STRUCT, lpName, lpDefaut);
	}

	if (mTypeElements[BVT_STRUCT].nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(BVT_STRUCT, lpName);
	if (lpByte != NULL)
	{
		LpVarStruct pVarStruct = LpVarStruct(lpByte);

		free(pVarStruct->tValue);

		pVarStruct->tSize = sSize;
		pVarStruct->tValue = MemAllocate(pVarStruct->tValue, sSize);

		memcpy(pVarStruct->tValue, lpDefaut, pVarStruct->tSize);

		m_bSave = true;
		return true;
	}
	return false;
}

bool FileEngine::AddVarString(LPCTSTR lpName, LPCTSTR lpStr, size_t size)
{
	return AddVarValue(BVT_STRING, lpName, lpStr, size);
}

bool FileEngine::SetVarString(LPCTSTR lpName, LPCTSTR lpStr, size_t size)
{
	return SetVarValue(BVT_STRING, lpName, lpStr, size);
}

bool FileEngine::SetVarName(WORD vType, LPCTSTR lpName, LPTSTR lpSetName)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarName::指定类型不存在或类型元素为空。"));
		return false;
	}
	LPBYTE lpByte = FindVarValue(vType, lpName);
	if (lpByte != NULL)
	{
		StringCchCopy(LPTSTR(lpByte), VAR_NAME_SIZE, lpSetName);
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarName::操作成功完成。"));
		m_bSave = true;
		return true;
	}
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarName::找不到指定的变量名称。"));
	return false;
}

bool FileEngine::SetVarName(WORD vType, int index, LPTSTR lpSetName)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarName::指定类型不存在或类型元素为空。"));
		return false;
	}

	LPBYTE lpByte = FindVarValue(vType, index);
	if (lpByte != NULL)
	{
		//lstrcpyn(LPTSTR(lpByte), lpSetName, VAR_NAME_SIZE);
		StringCchCopy(LPTSTR(lpByte), VAR_NAME_SIZE, lpSetName);
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarName::操作成功完成。"));
		m_bSave = true;
		return true;
	}
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarName::找不到指定的序号变量。"));
	return false;
}

bool FileEngine::GetVarName(WORD vType, int index, LPTSTR lpStr, int nMax)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarName::指定类型不存在或类型元素为空。"));
		return false;
	}

	LPBYTE lpByte = FindVarValue(vType, index);
	if (lpByte != NULL)
	{
		//lstrcpyn(lpStr, LPCTSTR(lpByte), iSize);
		StringCchCopy(lpStr, nMax, LPCTSTR(lpByte));
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarName::操作成功完成。"));
		return true;
	}
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarName::找不到指定的序号变量。"));
	return false;
}

long FileEngine::GetVarIndex(WORD vType, LPCTSTR lpName)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarIndex::指定类型不存在或类型元素为空。"));
		return -1;
	}

	long 	m = 0, n = mTypeElements[vType].nVar;
	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[vType]);

	long vSize = GetTypeMemVarSize(vType);//GetTypeValueSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	do
	{
		if (lstrcmp(LPCTSTR(lpByte), lpName) == 0)
		{
			StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarIndex::操作成功完成。"));
			return m;
		}
		lpByte += vSize;
	} while (++m < n);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarIndex::找不到指定的结构体变量名。"));
	return -1;
}

bool FileEngine::SetVarIndex(WORD vType, LPCTSTR lpName, int indexto, bool bswap)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (indexto < 0 || indexto >= mTypeElements[vType].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::指定序号越界（参数int indexto）。"));
		return false;
	}

	long 	m = 0, n = mTypeElements[vType].nVar;
	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[vType]);
	LPBYTE	lpBase = LPBYTE(mMemVariable.pAdd[vType]);

	long vSize = GetTypeMemVarSize(vType);//GetTypeValueSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	do
	{
		if (lstrcmp(LPCTSTR(lpByte), lpName) == 0)
		{
			if (lpBase + indexto * vSize == lpByte)
			{
				StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::指定序号不变（参数::int indexto）。"));
				return false;
			}
			char*	pbuffer = new char[vSize];
			LPBYTE	lpBase = LPBYTE(mMemVariable.pAdd[vType]);
			memcpy(pbuffer, lpByte, vSize);

			if (bswap == true)
			{
				memcpy(lpByte, lpBase + indexto * vSize, vSize);
			}
			else
			{
				if (lpBase + indexto * vSize < lpByte)
					memmove(lpBase + (indexto + 1)*vSize, lpBase + indexto * vSize, lpByte - (lpBase + indexto * vSize));
				else
					memmove(lpByte, lpByte + 1, lpBase + indexto * vSize - lpByte);
			}
			memcpy(lpBase + indexto * vSize, pbuffer, vSize);
			StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::操作成功完成。"));
			delete[] pbuffer;
			m_bSave = true;
			return true;
		}
		lpByte += vSize;
	} while (++m < n);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::找不到指定的变量名（参数::LPCTSTR lpName）。"));
	return false;
}

bool FileEngine::SetVarIndex(WORD vType, int index, int indexto, bool bswap)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (indexto < 0 || indexto >= mTypeElements[vType].nVar || index < 0 || index >= mTypeElements[vType].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::指定序号越界（参数::int index, int indexto）。"));
		return false;
	}
	if (indexto == index)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::指定序号不变（参数::int indexto）。"));
		return false;
	}

	long 	vSize = GetTypeMemVarSize(vType);//GetTypeValueSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;;
	LPBYTE	lpBase = LPBYTE(mMemVariable.pAdd[vType]);


	char*	pbuffer = new char[vSize];

	memcpy(pbuffer, lpBase + index * vSize, vSize);

	if (bswap == true)
	{
		memcpy(lpBase + index * vSize, lpBase + indexto * vSize, vSize);
	}
	else
	{
		if (indexto < index)
			memmove(lpBase + (indexto + 1)*vSize, lpBase + indexto * vSize, (index - indexto)*vSize);
		else
			memmove(lpBase + indexto * vSize, lpBase + (indexto + 1)*vSize, (indexto - index)*vSize);
	}
	memcpy(lpBase + indexto * vSize, pbuffer, vSize);
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarIndex::操作成功完成。"));
	delete[] pbuffer;
	m_bSave = true;
	return true;
}

long FileEngine::GetVarStringSize(int index)
{
	//	if(mTypeElements[BVT_STRING].nVar == 0)
	//	{
	//		lstrcpy(m_szLastError,TEXT("GetVarStringSize::指定类型不存在或类型元素为空。"));
	//		return false;
	//	} 
	if (index < 0 || index >= mTypeElements[BVT_STRING].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStringSize::指定序号越界（参数::int index）。"));
		return false;
	}

	long 		vSize = GetTypeMemVarSize(BVT_STRING);// GetTypeValueSize(BVT_STRING) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE		lpByte = LPBYTE(mMemVariable.pAdd[BVT_STRING]);

	LpVarString pLpVar = LpVarString(lpByte + index * vSize);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStringSize::操作成功完成。"));
	return lstrlen(pLpVar->tValue);
}

long FileEngine::GetVarStringSize(LPCTSTR lpName)
{
	if (mTypeElements[BVT_STRING].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStringSize::指定类型不存在或类型元素为空。"));
		return false;
	}
	LPBYTE lpByte = FindVarValue(BVT_STRING, lpName);
	if (lpByte != NULL)
	{
		LpVarString pLpVar = LpVarString(lpByte);
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStringSize::操作成功完成。"));
		return lstrlen(pLpVar->tValue);
	}

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStringSize::找不到指定的字符串变量名。"));
	return -1;
}

long FileEngine::GetVarStructSize(int index)
{
	if (mTypeElements[BVT_STRUCT].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStructSize::指定类型不存在或类型元素为空。"));
		return -1;
	}
	if (index < 0 || index >= mTypeElements[BVT_STRUCT].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStructSize::指定序号越界（参数::int index）。"));
		return -1;
	}

	long 	vSize = GetTypeMemVarSize(BVT_STRUCT);// GetTypeValueSize(BVT_STRUCT) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[BVT_STRUCT]);

	LpVarStruct pLpVar = LpVarStruct(lpByte + index * vSize);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStructSize::操作成功完成。"));
	return long(pLpVar->tSize);

}

long FileEngine::GetVarStructSize(LPCTSTR lpName)
{
	if (mTypeElements[BVT_STRUCT].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStructSize::指定类型不存在或类型元素为空。"));
		return false;
	}
	LPBYTE lpByte = FindVarValue(BVT_STRUCT, lpName);
	if (lpByte != NULL)
	{
		LpVarStruct pLpVar = LpVarStruct(lpByte);
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStructSize::操作成功完成。"));
		return long(pLpVar->tSize);
	}
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStructSize::找不到指定的结构体变量名。"));
	return -1;
}

bool FileEngine::AddVarString(int index, LPCTSTR lpName, LPCTSTR lpStr)
{
	if (index < 0 || index >= mTypeElements[BVT_STRING].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarString::指定序号越界（参数::int index）。"));
		return false;
	}
	if (FindVarValue(BVT_STRING, lpName) != NULL)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarString::变量名已经存在。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(BVT_STRING);
	long 	vSize = GetTypeMemVarSize(BVT_STRING);

	long 	nVar = mTypeElements[BVT_STRING].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[BVT_STRING];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + (index + 1)*vSize, LPBYTE(pVoid) + index * vSize, (nVar - index)*vSize);
	}

	LpVarString pVarString = LpVarString(LPBYTE(pVoid) + index * vSize);
	pVarString->tValue = new TCHAR[1 + lstrlen(lpStr)];
	lstrcpy(pVarString->tValue, lpStr);
	mTypeElements[BVT_STRING].nVar++;

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarString::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::SetVarString(int index, LPCTSTR lpStr)
{
	if (index < 0 || index >= mTypeElements[BVT_STRING].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarString::指定序号越界（参数::int index）。"));
		return false;
	}

	long	vSize = GetTypeMemVarSize(BVT_STRING);// GetTypeValueSize(BVT_STRING) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[BVT_STRING]);

	LpVarString pLpVar = LpVarString(lpByte + index * vSize);

	delete[] pLpVar->tValue;
	int len = 1 + lstrlen(lpStr);
	pLpVar->tValue = new TCHAR[len];
	StringCchCopy(pLpVar->tValue, len, lpStr);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarString::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::GetVarString(int index, LPTSTR lpStr, int iSize)
{
	if (mTypeElements[BVT_STRING].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarString::指定类型元素为空。"));
		return false;
	}
	long 		vSize = GetTypeMemVarSize(BVT_STRING);// GetTypeValueSize(BVT_STRING) + sizeof(TCHAR)*VAR_NAME_SIZE;;
	LPBYTE		lpByte = LPBYTE(mMemVariable.pAdd[BVT_STRING]);

	LpVarString pLpVar = LpVarString(lpByte + index * vSize);

	StringCchCopy(lpStr, iSize, pLpVar->tValue);
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarString::操作成功完成。"));

	return	true;
}

bool FileEngine::AddVarStruct(int index, LPCTSTR lpName, LPCVOID lpDefaut, size_t sSize)
{
	if (index < 0 || index >= mTypeElements[BVT_STRUCT].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarStruct::指定序号越界（参数::int index）。"));
		return false;
	}
	if (FindVarValue(BVT_STRUCT, lpName) != NULL)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarStruct::变量名已经存在（参数::LPCTSTR lpName）。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(BVT_STRUCT);
	long 	vSize = GetTypeMemVarSize(BVT_STRUCT);

	long 	nVar = mTypeElements[BVT_STRUCT].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[BVT_STRUCT];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + (index + 1)*vSize, LPBYTE(pVoid) + index * vSize, (nVar - index)*vSize);
	}

	LpVarStruct pLpVar = LpVarStruct(LPBYTE(pVoid) + index * vSize);

	pLpVar->tSize = sSize;
	pLpVar->tValue = malloc(sSize);

	memcpy(pLpVar->tValue, lpDefaut, sSize);
	StringCchCopy(pLpVar->tName, VAR_NAME_SIZE, lpName);

	mTypeElements[BVT_STRUCT].nVar++;

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarString::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::SetVarStruct(int index, LPCVOID lpDefaut, size_t sSize)
{
	if (index < 0 || index >= mTypeElements[BVT_STRUCT].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarStruct::指定序号越界（参数::int index）。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(BVT_STRUCT);
	long 	vSize = GetTypeMemVarSize(BVT_STRUCT);

	long 	nVar = mTypeElements[BVT_STRUCT].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[BVT_STRUCT];

	LpVarStruct pLpVar = LpVarStruct(LPBYTE(pVoid) + index * vSize);

	if (sSize == 0)
	{
		memcpy(pLpVar->tValue, lpDefaut, pLpVar->tSize);
	}
	else
	{
		memcpy(pLpVar->tValue, lpDefaut, pLpVar->tSize = sSize);
	}

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarStruct::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::GetVarStruct(int index, LPVOID pWrite)
{
	if (index < 0 || index >= mTypeElements[BVT_STRUCT].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStruct::指定序号越界（参数::int index）。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(BVT_STRUCT);
	long 	vSize = GetTypeMemVarSize(BVT_STRUCT);

	long 	nVar = mTypeElements[BVT_STRUCT].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[BVT_STRUCT];

	LpVarStruct pLpVar = LpVarStruct(LPBYTE(pVoid) + index * vSize);
	memcpy(pWrite, pLpVar->tValue, pLpVar->tSize);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarStruct::操作成功完成。"));
	return true;
}

bool FileEngine::AddVarValue(WORD vType, int index, LPCTSTR lpName, LPCVOID lpDefaut)
{
	if (FindVarValue(vType, lpName) != NULL || vType == BVT_STRUCT)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarValue::已经存在同类型、同名的变量，或者不支持的类型。"));
		return false;
	}

	if (index < 0 || index > mTypeElements[vType].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarValue::指定序号越界（参数::int index）。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(vType);
	long 	vSize = GetTypeMemVarSize(vType);

	long 	nVar = mTypeElements[vType].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[vType];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + (index + 1)*vSize, LPBYTE(pVoid) + index * vSize, (nVar - index)*vSize);
	}

	LPBYTE lpByte = LPBYTE(pVoid) + index * vSize;

	StringCchCopy(LPTSTR(lpByte), VAR_NAME_SIZE, lpName);

	if (vType == BVT_STRING)
	{
		LPCTSTR lpCStr = LPCTSTR(lpDefaut);
		LpVarString pVar = LpVarString(lpByte);
		int len = 1 + lstrlen(lpCStr);
		pVar->tValue = new TCHAR[len];
		StringCchCopy(pVar->tValue, len, lpCStr);
	}
	//else if (vType == BVT_STRUCT)
	//{
	//	LPCTSTR lpCStr = LPCTSTR(lpDefaut);
	//	LpVarStruct pVar = LpVarStruct(lpByte);
	//	pVar->tValue = new TCHAR[1 + lstrlen(lpCStr)];
	//	lstrcpy(pVar->tValue, lpCStr);
	//}
	else if (vType == BVT_VALUE8)
	{
		LpVar8 pVar = LpVar8(lpByte);
		memcpy_s(&pVar->tValue, sizeof(pVar->tValue), lpDefaut, sizeof(pVar->tValue));
	}
	else if (vType == BVT_VALUE16)
	{
		LpVar16 pVar = LpVar16(lpByte);
		memcpy_s(&pVar->tValue, sizeof(pVar->tValue), lpDefaut, sizeof(pVar->tValue));
	}
	else if (vType == BVT_VALUE32)
	{
		LpVar32 pVar = LpVar32(lpByte);
		memcpy_s(&pVar->tValue, sizeof(pVar->tValue), lpDefaut, sizeof(pVar->tValue));
	}
	else if (vType == BVT_VALUE64)
	{
		LpVar64 pVar = LpVar64(lpByte);
		memcpy_s(&pVar->tValue, sizeof(pVar->tValue), lpDefaut, sizeof(pVar->tValue));
	}
	else
	{
		throw exception("不支持的类型::bool FileEngine::AddVarValue(WORD vType, int index, LPCTSTR lpName, LPCVOID lpDefaut)");
		//LPBYTE lpDefaut = LPBYTE(lpDefaut) ? LPBYTE(lpDefaut) : LPBYTE(&lpDefaut);
		//memcpy(lpByte + sizeof(TCHAR)*VAR_NAME_SIZE, lpDefaut, wSize);
	}
	mTypeElements[vType].nVar++;
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarValue::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::AddVarValue(WORD vType, int index, LPCTSTR lpName, UINT_PTR dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("AddVarValue::大于32位的变量不能传值。"));
		return false;
	}

	if (bValueType == TD_VALUE)
		return AddVarValue(vType, index, lpName, &dwDefaut);

	return AddVarValue(vType, index, lpName, LPCVOID(dwDefaut));
}

bool FileEngine::SetVarValue(WORD vType, int index, LPCVOID lpDefaut)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarValue::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (index < 0 || index >= mTypeElements[vType].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarValue::指定序号越界（参数::int index）。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(vType);
	long 	vSize = GetTypeMemVarSize(vType);

	long 	nVar = mTypeElements[vType].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[vType];

	LPCTSTR lpName = LPCTSTR(LPBYTE(mMemVariable.pAdd[vType]) + index * vSize);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarValue::操作成功完成。"));
	return SetVarValue(vType, lpName, lpDefaut);

}

bool FileEngine::SetVarValue(WORD vType, int index, UINT_PTR dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("SetVarValue::大于32位的变量不能传值。"));
		return false;
	}

	if (bValueType == TD_VALUE)
		return SetVarValue(vType, index, &dwDefaut);

	return SetVarValue(vType, index, LPCVOID(dwDefaut));
}

bool FileEngine::GetVarValue(WORD vType, int index, LPVOID pWrite)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarValue::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (index < 0 || index >= mTypeElements[vType].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarValue::指定序号越界（参数::int index）。"));
		return false;
	}

	//long 	wSize = GetTypeValueSize(vType);
	long 	vSize = GetTypeMemVarSize(vType);

	long 	nVar = mTypeElements[vType].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[vType];

	LPCTSTR lpName = LPCTSTR(LPBYTE(mMemVariable.pAdd[vType]) + index * vSize);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarValue::操作成功完成。"));
	return GetVarValue(vType, lpName, pWrite);
}

bool FileEngine::DelVarValue(WORD vType, int index)
{
	if (vType > BVT_ALLTYPE - 1 || mTypeElements[vType].nVar == 0)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("DelVarValue::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (index < 0 || index >= mTypeElements[vType].nVar)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("DelVarValue::指定序号越界（参数::int index）。"));
		return false;
	}
	//long 	wSize = GetTypeValueSize(vType);
	long 	vSize = GetTypeMemVarSize(vType);

	long 	nVar = mTypeElements[vType].nVar;
	PVOID&	pVoid = mMemVariable.pAdd[vType];

	LPBYTE	lpByte = LPBYTE(mMemVariable.pAdd[vType]) + index * vSize;

	if (vType == BVT_STRUCT)
	{
		LpVarStruct lpVar = LpVarStruct(lpByte);
		free(lpVar->tValue);
	}
	else if (vType == BVT_STRING)
	{
		LpVarString lpVar = LpVarString(lpByte);
		delete[] lpVar->tValue;
	}

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + index * vSize, LPBYTE(pVoid) + (index + 1)*vSize, (nVar - index)*vSize);
	}
	mTypeElements[vType].nVar--;
	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarValue::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::DelVarValue(WORD vType, LPCTSTR lpName)
{
	int index = GetVarIndex(vType, lpName);

	if (index != -1)
	{
		return DelVarValue(vType, index);
	}

	return false;
}

bool FileEngine::DelAllValue(WORD vType)
{
	if (vType == BVT_ALLTYPE)
	{
		bool bRelut = false;
		for (WORD i = 0; i < BVT_ALLTYPE; i++)
			bRelut |= DelAllValue(i);
		return bRelut;
	}
	else if (vType < 0 || vType > BVT_ALLTYPE)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("DelAllValue::不支持的类型。"));
		return false;
	}
	else
	{
		//long 	wSize = GetTypeValueSize(vType);
		long 	vSize = GetTypeMemVarSize(vType);

		long 	nVar = mTypeElements[vType].nVar;
		PVOID&	pVoid = mMemVariable.pAdd[vType];

		if (vType == BVT_STRUCT)
		{
			for (int i = 0; i < nVar; i++)
			{
				LpVarStruct lpVar = LpVarStruct(LPBYTE(pVoid) + i * vSize);
				free(lpVar->tValue);
			}
		}
		else if (vType == BVT_STRING)
		{
			for (int i = 0; i < nVar; i++)
			{
				LpVarString lpVar = LpVarString(LPBYTE(pVoid) + i * vSize);
				delete[] lpVar->tValue;
			}
		}

		free(mMemVariable.pAdd[vType]);
		mMemVariable.pAdd[vType] = NULL;
		mTypeElements[vType].nVar = 0;

		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("DelAllValue::操作成功完成。"));
		m_bSave = true;
		return true;
	}
}

long FileEngine::GetVarNumber(WORD vType)
{
	if (vType == BVT_ALLTYPE)
	{
		long bRelut = false, tn;
		for (WORD i = 0; i < BVT_ALLTYPE; i++)
		{
			tn = GetVarNumber(i);
			if (tn != -1)
			{
				bRelut += tn;
			}
		}
		return bRelut;
	}
	else if (vType < 0 || vType > BVT_ALLTYPE)
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarNumber::不支持的类型。"));
		return -1;
	}
	else
	{
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("GetVarNumber::操作成功完成。"));
		return 	mTypeElements[vType].nVar;
	}
}

bool FileEngine::IsHasVarName(WORD vType, LPCTSTR lpName)
{
	if (vType >= BVT_ALLTYPE) {
		StringCbPrintf(mSzLastError, sizeof(mSzLastError), TEXT("IsHasVarName::未识别的类型 ValueType{%d}。"), vType);
		return false;
	}

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("操作成功完成。"));
	return mTypeElements[vType].nVar > 0 && FindVarValue(vType, lpName);
}

template<typename T>
bool FileEngine::PutVarTemplate(LPCTSTR lpName, const T & value)
{
	WORD vType = this->MapBaseValueType(sizeof(value));
	return IsHasVarName(vType, lpName) ? SetVarValue(vType, lpName, &value) : AddVarValue(vType, lpName, &value);
}

bool FileEngine::PutVarString(LPCTSTR lpName, LPCTSTR lpctstr, size_t size)
{
	return IsHasVarName(VT_STRING, lpName) ? SetVarString(lpName, lpctstr, size) : AddVarString(lpName, lpctstr, size);
}

bool FileEngine::PutVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t size)
{
	return IsHasVarName(VT_STRUCT, lpName) ? SetVarStruct(lpName, pDefaut, size) : AddVarStruct(lpName, pDefaut, size);
}

bool FileEngine::PutVar(LPCTSTR lpName, const bool &value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const char & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const BYTE & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const short & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const WORD & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const int & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const DWORD & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const long & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const INT64 & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const UINT64 & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const float & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, const double & value)
{
	return PutVarTemplate(lpName, value);
}

bool FileEngine::PutVar(LPCTSTR lpName, LPCTSTR lpStr)
{
	return PutVarString(lpName, lpStr);
}

LPCTSTR FileEngine::GetLastError()
{
	return LPCTSTR(mSzLastError);
}

