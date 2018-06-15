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

WORD 	FileEngine::m_dwVersion = MAKEWORD(1, 2);
LPCTSTR FileEngine::m_lpDescribe = TEXT("FileEngine 1.2 V");

FileEngine::FileEngine(void)
	:m_hFile(INVALID_HANDLE_VALUE)
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
		m_eType[i].e_vType = i;
		m_eType[i].e_nVar = 0;
		m_eType[i].e_lfanew = 0;
	}

	m_lpDepict = NULL;

	lstrcpy(m_szLastError, TEXT("未执行任何操作。"));

	FileEngine::SetDepict(m_lpDescribe);
}

FileEngine::~FileEngine(void)
{
	this->CloseFile();

	if (m_lpDepict != NULL)
	{
		delete[] m_lpDepict;
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
bool	FileEngine::CloseFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		if (this->m_bSave)
		{
			this->SaveFile();
		}
		this->FreeData();
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		return true;
	}
	return false;
}
bool FileEngine::SetDepict(LPCTSTR lpDepict)
{
	int len = lstrlen(lpDepict);

	if (m_lpDepict != NULL)
		delete[] m_lpDepict;

	m_lpDepict = new TCHAR[len + 1];
	m_fHeader.e_ldescribe = WORD(sizeof(TCHAR)*len);
	m_fHeader.e_lfanew = LONG(sizeof(FileHeader) + sizeof(TCHAR)*(len + 1));
	m_tHeader.e_lfanew = LONG(sizeof(TypeHeader) + sizeof(FileHeader) + sizeof(TCHAR)*(len + 1));

	return lstrcpy(m_lpDepict, lpDepict) != NULL;
}
bool FileEngine::GetDepict(LPTSTR lpBuffer, int iSize)
{
	if (lpBuffer != NULL)
	{
		return lstrcpyn(lpBuffer, m_lpDepict, iSize) != NULL;
	}
	return false;
}

WORD FileEngine::GetVersion(BYTE bType)
{
	if (bType == VT_FILE)
	{
		return m_fHeader.e_version;
	}
	return m_dwVersion;
}

bool FileEngine::OpenFile(LPCTSTR lpFile, DWORD dwDesiredAccess)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		this->FreeData();
		CloseHandle(m_hFile);
	}

	m_hFile = CreateFile(lpFile, dwDesiredAccess, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		m_hFile = CreateFile(lpFile, dwDesiredAccess | DA_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)return false;
		if (CreateFileMap() == false)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}
	if (CheckFileMap() == false)
	{
		if (CreateFileMap() == false)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
		return CheckFileMap();
	}
	return true;
}

bool FileEngine::SaveFile()
{
	if (m_hFile == INVALID_HANDLE_VALUE)return false;

	BOOL 	ret = TRUE;
	DWORD	nWriteBytes = 0;
	TypeElerment eType[BVT_ALLTYPE];
	memcpy(eType, m_eType, sizeof(eType));

	FileVariable	cFileVariable;
	LpVarString pVarString = NULL;
	LpVarStruct pVarStruct = NULL;
	//计算文件相关偏移量
	this->MapFile(cFileVariable, eType, pVarString, pVarStruct);

	//将文件头、文件描述、格式头、变量表写入文件
	SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
	ret &= WriteFile(m_hFile, &m_fHeader, sizeof(FileHeader), &nWriteBytes, NULL);
	ret &= WriteFile(m_hFile, m_lpDepict, sizeof(TCHAR) + m_fHeader.e_ldescribe, &nWriteBytes, NULL);
	ret &= WriteFile(m_hFile, &m_tHeader, sizeof(TypeHeader), &nWriteBytes, NULL);
	ret &= WriteFile(m_hFile, eType, sizeof(TypeElerment) * BVT_ALLTYPE, &nWriteBytes, NULL);
	//讲数据按照偏移量写入文件
	this->SaveData(cFileVariable, eType, pVarString, pVarStruct);

	return ret != FALSE;
}

bool FileEngine::IsEmptyRecorde()
{
	if (m_hFile == INVALID_HANDLE_VALUE)return false;

	for (WORD i = 0; i < BVT_ALLTYPE; i++)
		if (m_eType[i].e_nVar != 0)
			return false;
	return true;
}

bool FileEngine::GetVarString(LPCTSTR lpName, LPTSTR lpStr, int iSize)
{
	if (m_eType[BVT_STRING].e_nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(BVT_STRING, lpName);
	if (lpByte != NULL)
	{
		LpVarString pVarString = LpVarString(lpByte);
		lstrcpyn(lpStr, pVarString->e_tValue, iSize);
		return	true;
	}
	return false;
}

bool FileEngine::SetVarValue(WORD vType, LPCTSTR lpName, int dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)return false;

	if (bValueType == TD_VALUE)
		return SetVarValue(vType, lpName, &dwDefaut);

	return SetVarValue(vType, lpName, LPCVOID(dwDefaut));
}

bool FileEngine::SetVarValue(WORD vType, LPCTSTR lpName, LPCVOID pDefaut)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(vType, lpName);
	if (lpByte != NULL)
	{
		if (vType == BVT_STRUCT)
		{
			LpVarStruct pVarStruct = LpVarStruct(lpByte);
			memcpy(pVarStruct->e_tValue, pDefaut, pVarStruct->e_tSize);
		}
		else if (vType == BVT_STRING)
		{
			LpVarString pVarString = LpVarString(lpByte);
			delete[] pVarString->e_tValue;
			pVarString->e_tValue = new TCHAR[1 + lstrlen(LPTSTR(pDefaut))];
			lstrcpy(pVarString->e_tValue, LPTSTR(pDefaut));
		}
		else
		{
			lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

			LPBYTE	lpDefaut = LPBYTE(pDefaut);
			long	tSize = GetTypeSize(vType);

			for (int j = 0; j < tSize; j++)
				lpByte[j] = lpDefaut[j];
		}
		m_bSave = true;
		return true;
	}
	return false;
}

bool FileEngine::GetVarValue(WORD vType, LPCTSTR lpName, PVOID pWrite)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(vType, lpName);
	if (lpByte != NULL)
	{
		if (vType == BVT_STRUCT)
		{
			LpVarStruct pVarStruct = LpVarStruct(lpByte);
			memcpy(pWrite, pVarStruct->e_tValue, pVarStruct->e_tSize);
		}
		else if (vType == BVT_STRING)
		{
			LpVarString pVarString = LpVarString(lpByte);
			lstrcpy(LPTSTR(pWrite), pVarString->e_tValue);
		}
		else
		{
			lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

			LPBYTE	lpWrite = LPBYTE(pWrite);
			long	tSize = GetTypeSize(vType);

			for (int j = 0; j < tSize; j++)
				lpWrite[j] = lpByte[j];
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
		long nType = GetTypeData(wType, lpTypeData, nLength);
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

bool FileEngine::AddVarValue(WORD vType, LPCTSTR lpName, LPCVOID pDefaut)
{
	if (FindVarValue(vType, lpName) != NULL || vType == BVT_STRUCT)
		return false;

	long 	wSize = GetTypeSize(vType);
	long 	vSize = GetTypeVarSize(vType);

	long 	nVar = m_eType[vType].e_nVar;
	PVOID	&pVoid = m_Variable.pAdd[vType];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (AddVarValue(vType, wSize, LPBYTE(pVoid) + nVar * vSize, lpName, pDefaut))
	{
		m_eType[vType].e_nVar++;
		m_bSave = true;
		return true;
	}

	return false;
}

bool FileEngine::AddVarValue(WORD vType, LPCTSTR lpName, int dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)return false;

	if (bValueType == TD_VALUE)
		return AddVarValue(vType, lpName, &dwDefaut);

	return AddVarValue(vType, lpName, LPCVOID(dwDefaut));
}

bool FileEngine::AddVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize)
{
	if (FindVarValue(BVT_STRUCT, lpName) != NULL || pDefaut == NULL || sSize == 0)
		return false;

	long 	vSize = sizeof(VarStruct);
	long 	nStrct = m_eType[BVT_STRUCT].e_nVar;
	PVOID	&pVoid = m_Variable.pAdd[BVT_STRUCT];

	pVoid = MemAllocate(pVoid, (nStrct + 1)*vSize);

	LpVarStruct pVarStruct = LpVarStruct(LPBYTE(pVoid) + nStrct * vSize);

	pVarStruct->e_tSize = sSize;
	pVarStruct->e_tValue = malloc(vSize);
	memcpy(pVarStruct->e_tValue, pDefaut, sSize);
	lstrcpyn(pVarStruct->e_tName, lpName, VAR_NAME_SIZE);

	m_eType[BVT_STRUCT].e_nVar++;
	return m_bSave = true;
}

void FileEngine::OutPutDataInfo()
{
	if (!this->IsEmptyRecorde())
	{
		cout << "文件版本::" << int(LOBYTE(m_fHeader.e_version)) << "." << int(HIBYTE(m_fHeader.e_version)) << endl;
		cout << "文件描述::" << m_lpDepict << endl;
	}
	else
	{
		cout << "文件记录为空！" << endl;
	}

	for (WORD wType = 0; wType < BVT_ALLTYPE; wType++)
	{
		if (m_eType[wType].e_nVar != 0)
		{
			int 	n = m_eType[wType].e_nVar;
			LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[wType]);
			WORD	wSize = (WORD)GetTypeSize(wType);

			LPCTSTR	lpType[] = {
				TEXT("8位变量"),TEXT("16位变量"),TEXT("32位变量"),TEXT("64位变量"),TEXT("字符串变量"),TEXT("结构体变量")
			};
			cout << lpType[wType] << n << "个{\n";

			for (int i = 0; i < n; i++)
			{
				cout << "   " << lpByte << "::";

				lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

				if (wType == BVT_STRUCT)
				{
					LpVarStruct	pStrcut = LpVarStruct(lpByte - sizeof(TCHAR)*VAR_NAME_SIZE);
					LPBYTE	lRead = LPBYTE(pStrcut->e_tValue);

					cout << "{\n      ";
					for (size_t ibyte = 0; ibyte < pStrcut->e_tSize; ibyte++)
					{
						if (ibyte && !((ibyte) % 35))
							cout << "\n      ";
						printf("%02X", lRead[ibyte]);
					}
					printf("\n   }\n");
				}
				else if (wType == BVT_STRING)
				{
					LPCTSTR lpText = LPCTSTR(*LPDWORD(lpByte));
					ULONG	iSize = lstrlen(lpText);

					cout << "{\n      ";
					for (size_t ibyte = 0, in = 0; ibyte < iSize; ibyte++)
					{
						printf("%c，%d", lpText[ibyte], in++);
						if (lpText[ibyte] == '\n')
						{
							in = 0;
							cout << "      ";
						}
						else if (in == 70) {
							in = 0;
							cout << "\n      ";
						}
					}
					cout << "\n   }\n";
					//cout << LPCTSTR(*LPDWORD(lpByte)) << endl;
				}
				else if (wType == BVT_VALUE64)
				{
					double	dValue = 0;
					LPBYTE	lRead = LPBYTE(&dValue);

					for (int ibyte = 0; ibyte < sizeof(double); ibyte++)
						lRead[ibyte] = lpByte[ibyte];

					printf("\r   %#08X::%s = %f\n", *((LPINT)&dValue), (lpByte - sizeof(TCHAR)*VAR_NAME_SIZE), dValue);
				}
				else
				{
					DWORD	dValue = 0;
					LPBYTE	lRead = LPBYTE(&dValue);

					for (int ibyte = 0; ibyte < wSize; ibyte++)
						lRead[ibyte] = lpByte[ibyte];

					printf("\r   %010u::%#08x::%s = %d\n", dValue, dValue, lpByte - sizeof(TCHAR)*VAR_NAME_SIZE, dValue);
				}
				lpByte += wSize;
			}
			cout << "}\n";
		}
	}
}

//protected
void FileEngine::MapFile(FileVariable &cFileVariable, TypeElerment eType[], LpVarString &pVarString, LpVarStruct &pVarStruct)
{
	//数据储存从eType[BVT_ALLTYPE]后面开始
	LONG lLastFanew = m_tHeader.e_lfanew + sizeof(m_eType);

	for (int i = 0; i < BVT_ALLTYPE; i++)
	{
		if (eType[i].e_nVar != 0)
		{
			//数据类型【i】的偏离eType[i].e_lfanew
			eType[i].e_lfanew = lLastFanew;
			//根据本【i】类型尺寸计算下一类型起始偏离
			lLastFanew += eType[i].e_nVar*GetTypeFileVarSize(i);
		}
		else
		{
			eType[i].e_lfanew = 0;
		}
	}
	for (int j = 0; j < BVT_ALLTYPE; j++)
	{
		if (m_eType[j].e_nVar != 0)
		{
			long	lvSize = GetTypeVarSize(j);		//单个类型变量在内存的尺寸
			long	lfSize = GetTypeFileVarSize(j);	//单个类型变量在文件的尺寸

													//根据元素个数m_eType[j].e_nVar申请多个文件类型变量到内存
			cFileVariable.pFAdd[j] = malloc(m_eType[j].e_nVar*lfSize);
			//分别映射多个文件变量和内存变量地址到lpFByte，lpVByte
			LPBYTE	lpFByte = LPBYTE(cFileVariable.pFAdd[j]);
			LPBYTE	lpVByte = LPBYTE(m_Variable.pAdd[j]);

			for (int nVar = 0; nVar < m_eType[j].e_nVar; nVar++)
			{
				//分别映射单个文件变量和内存变量地址到lpFileVar，lpvVar
				LPBYTE lpvVar = lpVByte + nVar * lvSize;
				LPBYTE lpFileVar = lpFByte + nVar * lfSize;
				//映射单个文件变量数据域地址到lpData
				//sizeof(long) 变量前四个字节为变量名字符串偏离地址
				LPBYTE lpData = lpFileVar + sizeof(long);
				//映射单个内存变量数据域地址到lpData
				LPBYTE lpRead = lpvVar + sizeof(TCHAR)*VAR_NAME_SIZE;

				//映射单个文件变量地址（变量的变量名指针域地址）
				LPLONG lpName = LPLONG(lpFileVar);
				//定义当前偏移量给 变量的变量名指针
				lpName[0] = lLastFanew;
				//根据当前变量名的长度确定写一个偏移起始地址
				lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(LPCTSTR(lpvVar))));
				//从内存变量 拷贝 数据 到 本件变量
				for (int ibyte = 0, nByte = GetTypeSize(j); ibyte < nByte; ibyte++)
				{
					lpData[ibyte] = lpRead[ibyte];
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////
	if (m_Variable.pVarString != NULL && eType[BVT_STRING].e_nVar)
	{
		pVarString = new VarString[eType[BVT_STRING].e_nVar];
		memcpy(pVarString, m_Variable.pVarString, sizeof(VarString)*eType[BVT_STRING].e_nVar);
		for (int i = 0; i < eType[BVT_STRING].e_nVar; i++)
		{
			//m_Variable.pVarString[i].e_tAddres = lLastFanew;
			cFileVariable.pFileVarString[i].e_tAddres = lLastFanew;
			lLastFanew += long(sizeof(TCHAR)*(1 + lstrlen(pVarString[i].e_tValue)));
		}
	}
	if (m_Variable.pVarStruct != NULL && eType[BVT_STRUCT].e_nVar)
	{
		pVarStruct = new VarStruct[eType[BVT_STRUCT].e_nVar];
		memcpy(pVarStruct, m_Variable.pVarStruct, sizeof(VarStruct)*eType[BVT_STRUCT].e_nVar);
		for (int i = 0; i < eType[BVT_STRUCT].e_nVar; i++)
		{
			//m_Variable.pVarStruct[i].e_tAddres = lLastFanew;
			cFileVariable.pFileVarStruct[i].e_tAddres = lLastFanew;
			lLastFanew += long(m_Variable.pVarStruct[i].e_tSize);
		}
	}
}

BOOL FileEngine::SaveData(FileVariable &cFileVariable, TypeElerment eType[], LpVarString &pVarString, LpVarStruct &pVarStruct)
{
	BOOL 	ret = TRUE;
	DWORD	nWriteBytes = 0;
	for (int i = 0; i < BVT_ALLTYPE; i++)
	{
		if (eType[i].e_nVar != 0)
		{
			ret &= WriteFile(m_hFile, cFileVariable.pFAdd[i], eType[i].e_nVar*GetTypeFileVarSize(i), &nWriteBytes, NULL);
		}
	}
	for (int j = 0; j < BVT_ALLTYPE; j++)
	{//储存变量名 
		if (eType[j].e_nVar != 0)
		{
			long	lvSize = GetTypeVarSize(j);
			LPBYTE	lpVByte = LPBYTE(m_Variable.pAdd[j]);
			for (int nVar = 0; nVar < m_eType[j].e_nVar; nVar++)
			{
				LPBYTE lpvVar = lpVByte + nVar * lvSize;
				LPLONG lpName = LPLONG(lpvVar);
				ret &= WriteFile(m_hFile, lpvVar, sizeof(TCHAR)*(1 + lstrlen(PTSTR(lpvVar))), &nWriteBytes, NULL);
			}
		}
	}
	if (pVarString != NULL)
	{//储存字符串数据 
		for (int i = 0; i < eType[BVT_STRING].e_nVar; i++)
		{
			ret &= WriteFile(m_hFile, pVarString[i].e_tValue, sizeof(TCHAR)*(1 + lstrlen(pVarString[i].e_tValue)), &nWriteBytes, NULL);
		}
		memcpy(m_Variable.pVarString, pVarString, sizeof(VarString)*eType[BVT_STRING].e_nVar);

		delete[]	pVarString;
	}
	if (pVarStruct != NULL)
	{
		for (int i = 0; i < eType[BVT_STRUCT].e_nVar; i++)
		{
			ret &= WriteFile(m_hFile, pVarStruct[i].e_tValue, DWORD(pVarStruct[i].e_tSize), &nWriteBytes, NULL);
		}
		memcpy(m_Variable.pVarStruct, pVarStruct, sizeof(VarString)*eType[BVT_STRUCT].e_nVar);

		delete[]	pVarStruct;
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
		m_eType[i].e_nVar = 0;
		if (m_Variable.pAdd[i])
		{
			free(m_Variable.pAdd[i]);
			m_Variable.pAdd[i] = NULL;
		}
	}

	if (m_Variable.pVarString != 0)
	{
		for (int i = 0; i < m_eType[BVT_STRING].e_nVar; i++)
			delete[] m_Variable.pVarString[i].e_tValue;

		m_eType[BVT_STRING].e_nVar = 0;
		free(m_Variable.pVarString);
		m_Variable.pVarString = NULL;
	}
	if (m_Variable.pVarStruct != 0)
	{
		for (int i = 0; i < m_eType[BVT_STRUCT].e_nVar; i++)
		{
			//printf("free pVarStruct = %#x\n",DWORD(m_Variable.pVarStruct[i].e_tValue));
			free(m_Variable.pVarStruct[i].e_tValue);
		}

		m_eType[BVT_STRUCT].e_nVar = 0;
		free(m_Variable.pVarStruct);
		m_Variable.pVarStruct = NULL;
	}
}

bool FileEngine::CreateFileMap()
{
	if (m_hFile == INVALID_HANDLE_VALUE)return false;

	int len = lstrlen(m_lpDepict);
	m_fHeader.e_ldescribe = WORD(sizeof(TCHAR)*len);
	m_fHeader.e_lfanew = LONG(sizeof(FileHeader) + sizeof(TCHAR)*(len + 1));
	m_tHeader.e_lfanew = LONG(sizeof(TypeHeader) + sizeof(FileHeader) + sizeof(TCHAR)*(len + 1));

	BOOL 	ret = TRUE;
	DWORD	nWriteBytes = 0;
	SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);

	ret &= WriteFile(m_hFile, &m_fHeader, sizeof(FileHeader), &nWriteBytes, NULL);
	ret &= WriteFile(m_hFile, m_lpDepict, sizeof(TCHAR) + m_fHeader.e_ldescribe, &nWriteBytes, NULL);
	ret &= WriteFile(m_hFile, &m_tHeader, sizeof(TypeHeader), &nWriteBytes, NULL);
	ret &= WriteFile(m_hFile, m_eType, BVT_ALLTYPE * sizeof(TypeElerment), &nWriteBytes, NULL);

	return ret != FALSE;
}
void FileEngine::ReadFileMap(LPVOID lpBase, LpFileHeader pFileHeader, LpTypeHeader pTypeHeader)
{
	m_fHeader = pFileHeader[0];
	m_tHeader = pTypeHeader[0];

	FileEngine::SetDepict(LPCTSTR(LPBYTE(lpBase) + sizeof(FileHeader)));

	if (m_tHeader.e_ntype > BVT_ALLTYPE)m_tHeader.e_ntype = BVT_ALLTYPE;

	memcpy(m_eType, LPBYTE(lpBase) + pTypeHeader->e_lfanew, m_tHeader.e_ntype * sizeof(TypeElerment));

	FileVariable cFileVariable;
	for (WORD wType = 0; wType < m_tHeader.e_ntype; wType++)
	{
		if (m_eType[wType].e_nVar != 0)
		{
			size_t	vsize = GetTypeVarSize(wType), avsize = m_eType[wType].e_nVar*vsize;
			size_t	fsize = GetTypeFileVarSize(wType), afsize = m_eType[wType].e_nVar*fsize;

			LPBYTE	lpVByte = LPBYTE(m_Variable.pAdd[wType] = malloc(avsize));
			LPBYTE	lpFByte = LPBYTE(cFileVariable.pFAdd[wType] = malloc(afsize));


			memset(m_Variable.pAdd[wType], 0, avsize);
			memcpy(cFileVariable.pFAdd[wType], LPBYTE(lpBase) + m_eType[wType].e_lfanew, afsize);


			for (int nVar = 0; nVar < m_eType[wType].e_nVar; nVar++)
			{
				LPBYTE lpvVar = lpVByte + nVar * vsize;
				LPBYTE lpFileVar = lpFByte + nVar * fsize;
				LPBYTE lpRead = lpFileVar + sizeof(long);
				LPBYTE lpData = lpvVar + sizeof(TCHAR)*VAR_NAME_SIZE;
				LPLONG lpName = LPLONG(lpFileVar);

				lstrcpyn(LPTSTR(lpvVar), LPCTSTR(LPBYTE(lpBase) + lpName[0]), VAR_NAME_SIZE);

				for (int ibyte = 0, nByte = GetTypeSize(wType); ibyte < nByte; ibyte++)
				{
					lpData[ibyte] = lpRead[ibyte];
				}
			}
		}
	}
	if (m_Variable.pVarString != 0)
	{
		for (int i = 0; i < m_eType[BVT_STRING].e_nVar; i++)
		{
			LPTSTR lpStr = LPTSTR(LPBYTE(lpBase) + m_Variable.pVarString[i].e_tAddres);
			int 	len = lstrlen(lpStr);
			m_Variable.pVarString[i].e_tValue = new TCHAR[len + 1];
			lstrcpy(m_Variable.pVarString[i].e_tValue, lpStr);
		}
	}
	if (m_Variable.pVarStruct != 0)
	{
		for (int i = 0; i < m_eType[BVT_STRUCT].e_nVar; i++)
		{
			PVOID lpData = LPBYTE(lpBase) + m_Variable.pVarStruct[i].e_tAddres;
			size_t 	size = m_Variable.pVarStruct[i].e_tSize;
			m_Variable.pVarStruct[i].e_tValue = malloc(size);
			memcpy(m_Variable.pVarStruct[i].e_tValue, lpData, size);
		}
	}
}

bool FileEngine::CheckFileMap()
{
	if (m_hFile == INVALID_HANDLE_VALUE)return false;

	HANDLE hMap = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	LPVOID lpBase = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);

	if (lpBase == NULL)  return false;

	bool ret = true;

	LpFileHeader pFileHeader = LpFileHeader(lpBase);
	LpTypeHeader pTypeHeader = LpTypeHeader((PBYTE)lpBase + pFileHeader->e_lfanew);

	ret = ret && pFileHeader->e_magic == IMAGE_SIGNATURE_FILE;
	ret = ret && pTypeHeader->e_magic == IMAGE_SIGNATURE_TYPE;

	if (ret != FALSE)
	{
		this->ReadFileMap(lpBase, pFileHeader, pTypeHeader);
	}

	UnmapViewOfFile(lpBase);
	CloseHandle(hMap);

	//if (ret == FALSE)
	//{
	//	this->FreeData();
	//	CloseHandle(m_hFile);
	//	m_hFile = INVALID_HANDLE_VALUE;
	//}

	return ret;
}

long FileEngine::GetTypeSize(WORD vType)
{
	long size = 1;

	if (vType == BVT_STRING)
		size = sizeof(PTSTR);
	else if (vType == BVT_STRUCT)
		size = sizeof(size_t) + sizeof(PVOID);
	else
		while (vType--)size *= 2;

	return size;
}
long FileEngine::GetTypeVarSize(WORD vType)
{
	return GetTypeSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
}
long FileEngine::GetTypeFileVarSize(WORD vType)
{
	return GetTypeSize(vType) + sizeof(LONG);
}
long FileEngine::GetTypeData(WORD wType, LPTYPEDATA lpTypeData, int nLength)
{
	long nType = 0;

	for (int n = 0; n < nLength; n++)
		if (lpTypeData[n].wType == wType)
			nType++;

	return nType;
}
bool FileEngine::AddVarValue(WORD vType, long wSize, LPBYTE lpByte, LPCTSTR lpName, LPCVOID pDefaut)
{
	lstrcpyn(LPTSTR(lpByte), lpName, VAR_NAME_SIZE);

	if (vType == BVT_STRING)
	{
		LPCTSTR lpCStr = LPCTSTR(pDefaut);
		LpVarString pVarString = LpVarString(lpByte);
		pVarString->e_tValue = new TCHAR[1 + lstrlen(lpCStr)];
		lstrcpy(pVarString->e_tValue, lpCStr);
	}
	else
	{
		LPBYTE lpDefaut = LPBYTE(pDefaut) ? LPBYTE(pDefaut) : LPBYTE(&pDefaut);

		lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

		for (long i = 0; i < wSize; i++)
			lpByte[i] = lpDefaut[i];
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
			lstrcpyn(LPTSTR(lpByte), lpTypeData[n].tName, VAR_NAME_SIZE);

			lpByte += sizeof(TCHAR)*VAR_NAME_SIZE;

			if (lpTypeData[n].bValueType == TD_VALUE)
				lpDefaut = LPBYTE(&lpTypeData[n].dwValue);
			else
				lpDefaut = LPBYTE(lpTypeData[n].dwValue);

			for (int i = 0; i < wSize; i++)
				lpByte[i] = lpDefaut[i];

			nVar++;
			lpByte += wSize;
		}
	}
	return nVar;
}

long FileEngine::AddVarValue(WORD wType, long nType, LPTYPEDATA lpTypeData, int nLength)
{
	long	wSize = GetTypeSize(wType);
	long 	vSize = GetTypeVarSize(wType);

	PVOID&	pVoid = m_Variable.pAdd[wType];
	ULONG	lType = m_eType[wType].e_nVar + WORD(nType);

	pVoid = MemAllocate(pVoid, lType*vSize);
	nType = AddVarValue(LPBYTE(pVoid) + (lType - nType)*vSize, wType, WORD(wSize), lpTypeData, nLength);

	m_eType[wType].e_nVar += WORD(nType);

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
			pVarString->e_tValue = new TCHAR[sizeof(TCHAR)*(1 + lstrlen(PTSTR(lpTypeData[n].dwValue)))];

			lstrcpyn(pVarString->e_tName, lpTypeData[n].tName, VAR_NAME_SIZE);
			lstrcpy(pVarString->e_tValue, LPCTSTR(lpTypeData[n].dwValue));

			nVar++;
			pVarString++;
		}
	}
	return nVar;
}

long FileEngine::AddVarString(long nType, LPTYPEDATA lpTypeData, int nLength)
{
	PVOID&	pVoid = m_Variable.pAdd[BVT_STRING];
	long	lType = m_eType[BVT_STRING].e_nVar + nType;

	pVoid = MemAllocate(pVoid, lType * sizeof(VarString));
	nType = AddVarString(m_Variable.pVarString + (lType - nType), lpTypeData, nLength);

	m_eType[BVT_STRING].e_nVar += WORD(nType);

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
			pVarStruct->e_tSize = lpTypeData[n].wStrcutSize;
			pVarStruct->e_tValue = malloc(lpTypeData[n].wStrcutSize);

			//printf("malloc pVarStruct = %#x\n",DWORD(pVarStruct->e_tValue));

			memcpy(pVarStruct->e_tValue, lpTypeData[n].dwValue, lpTypeData[n].wStrcutSize);
			lstrcpyn(pVarStruct->e_tName, lpTypeData[n].tName, VAR_NAME_SIZE);

			nVar++;
			pVarStruct++;
		}
	}
	return nVar;
}

long FileEngine::AddVarStruct(long nType, LPTYPEDATA lpTypeData, int nLength)
{
	PVOID&	pVoid = m_Variable.pAdd[BVT_STRUCT];
	WORD	lType = m_eType[BVT_STRUCT].e_nVar + WORD(nType);

	pVoid = MemAllocate(pVoid, lType * sizeof(VarStruct));
	nType = AddVarStruct(m_Variable.pVarStruct + (lType - nType), lpTypeData, nLength);

	m_eType[BVT_STRUCT].e_nVar += WORD(nType);

	return nType;
}

LPBYTE FileEngine::FindVarValue(WORD vType, LPCTSTR lpName)
{
	int 	m = 0, n = m_eType[vType].e_nVar;
	LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[vType]);

	long vSize = GetTypeSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
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
	int 	m = 0, n = m_eType[vType].e_nVar;
	LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[vType]);

	long vSize = GetTypeSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	if (index >= 0 && index < n)
	{
		return lpByte + index * vSize;
	}

	return NULL;
}
/**
Name: IFileEngine	1.2 V

Last Change:
*/
bool FileEngine::GetLastError(LPTSTR lpStr, int iSize)
{
	lstrcpyn(lpStr, m_szLastError, min(256, iSize));
	return true;
}

bool FileEngine::GetVarStruct(LPCTSTR lpName, LPVOID pWrite)
{
	return GetVarValue(BVT_STRUCT, lpName, pWrite);
}

bool FileEngine::SetVarStruct(LPCTSTR lpName, LPCVOID pDefaut, size_t sSize)
{
	if (sSize == 0)
	{
		return SetVarValue(BVT_STRUCT, lpName, pDefaut);
	}

	if (m_eType[BVT_STRUCT].e_nVar == 0)
		return false;

	LPBYTE lpByte = FindVarValue(BVT_STRUCT, lpName);
	if (lpByte != NULL)
	{
		LpVarStruct pVarStruct = LpVarStruct(lpByte);

		free(pVarStruct->e_tValue);

		pVarStruct->e_tSize = sSize;
		pVarStruct->e_tValue = MemAllocate(pVarStruct->e_tValue, sSize);

		memcpy(pVarStruct->e_tValue, pDefaut, pVarStruct->e_tSize);

		m_bSave = true;
		return true;
	}
	return false;
}

bool FileEngine::AddVarString(LPCTSTR lpName, LPCTSTR lpStr)
{
	return AddVarValue(BVT_STRING, lpName, lpStr);
}

bool FileEngine::SetVarString(LPCTSTR lpName, LPCTSTR lpStr)
{
	return SetVarValue(BVT_STRING, lpName, lpStr);
}

bool FileEngine::SetVarName(WORD vType, LPCTSTR lpName, LPTSTR lpSetName)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("SetVarName::指定类型不存在或类型元素为空。"));
		return false;
	}
	LPBYTE lpByte = FindVarValue(vType, lpName);
	if (lpByte != NULL)
	{
		lstrcpyn(LPTSTR(lpByte), lpSetName, VAR_NAME_SIZE);
		lstrcpy(m_szLastError, TEXT("SetVarName::操作成功完成。"));
		m_bSave = true;
		return true;
	}
	lstrcpy(m_szLastError, TEXT("SetVarName::找不到指定的变量名称。"));
	return false;
}

bool FileEngine::SetVarName(WORD vType, int index, LPTSTR lpSetName)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("SetVarName::指定类型不存在或类型元素为空。"));
		return false;
	}

	LPBYTE lpByte = FindVarValue(vType, index);
	if (lpByte != NULL)
	{
		lstrcpyn(LPTSTR(lpByte), lpSetName, VAR_NAME_SIZE);
		lstrcpy(m_szLastError, TEXT("SetVarName::操作成功完成。"));
		m_bSave = true;
		return true;
	}
	lstrcpy(m_szLastError, TEXT("SetVarName::找不到指定的序号变量。"));
	return false;
}

bool FileEngine::GetVarName(WORD vType, int index, LPTSTR lpStr, int iSize)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarName::指定类型不存在或类型元素为空。"));
		return false;
	}

	LPBYTE lpByte = FindVarValue(vType, index);
	if (lpByte != NULL)
	{
		lstrcpyn(lpStr, LPCTSTR(lpByte), iSize);
		lstrcpy(m_szLastError, TEXT("GetVarName::操作成功完成。"));
		return true;
	}
	lstrcpy(m_szLastError, TEXT("GetVarName::找不到指定的序号变量。"));
	return false;
}

long FileEngine::GetVarIndex(WORD vType, LPCTSTR lpName)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarIndex::指定类型不存在或类型元素为空。"));
		return -1;
	}

	long 	m = 0, n = m_eType[vType].e_nVar;
	LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[vType]);

	long vSize = GetTypeSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	do
	{
		if (lstrcmp(LPCTSTR(lpByte), lpName) == 0)
		{
			lstrcpy(m_szLastError, TEXT("GetVarIndex::操作成功完成。"));
			return m;
		}
		lpByte += vSize;
	} while (++m < n);

	lstrcpy(m_szLastError, TEXT("GetVarIndex::找不到指定的结构体变量名。"));
	return -1;
}

bool FileEngine::SetVarIndex(WORD vType, LPCTSTR lpName, int indexto, bool bswap)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("SetVarIndex::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (indexto < 0 || indexto >= m_eType[vType].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("SetVarIndex::指定序号越界（参数int indexto）。"));
		return false;
	}

	long 	m = 0, n = m_eType[vType].e_nVar;
	LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[vType]);
	LPBYTE	lpBase = LPBYTE(m_Variable.pAdd[vType]);

	long vSize = GetTypeSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	do
	{
		if (lstrcmp(LPCTSTR(lpByte), lpName) == 0)
		{
			if (lpBase + indexto * vSize == lpByte)
			{
				lstrcpy(m_szLastError, TEXT("SetVarIndex::指定序号不变（参数::int indexto）。"));
				return false;
			}
			char*	pbuffer = new char[vSize];
			LPBYTE	lpBase = LPBYTE(m_Variable.pAdd[vType]);
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
			lstrcpy(m_szLastError, TEXT("SetVarIndex::操作成功完成。"));
			delete[] pbuffer;
			m_bSave = true;
			return true;
		}
		lpByte += vSize;
	} while (++m < n);

	lstrcpy(m_szLastError, TEXT("SetVarIndex::找不到指定的变量名（参数::LPCTSTR lpName）。"));
	return false;
}

bool FileEngine::SetVarIndex(WORD vType, int index, int indexto, bool bswap)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("SetVarIndex::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (indexto < 0 || indexto >= m_eType[vType].e_nVar || index < 0 || index >= m_eType[vType].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("SetVarIndex::指定序号越界（参数::int index, int indexto）。"));
		return false;
	}
	if (indexto == index)
	{
		lstrcpy(m_szLastError, TEXT("SetVarIndex::指定序号不变（参数::int indexto）。"));
		return false;
	}

	long 	vSize = GetTypeSize(vType) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE	lpBase = LPBYTE(m_Variable.pAdd[vType]);


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
	lstrcpy(m_szLastError, TEXT("SetVarIndex::操作成功完成。"));
	delete[] pbuffer;
	m_bSave = true;
	return true;
}

size_t FileEngine::GetVarStringSize(int index)
{
	//	if(m_eType[BVT_STRING].e_nVar == 0)
	//	{
	//		lstrcpy(m_szLastError,TEXT("GetVarStringSize::指定类型不存在或类型元素为空。"));
	//		return false;
	//	} 
	if (index < 0 || index >= m_eType[BVT_STRING].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("GetVarStringSize::指定序号越界（参数::int index）。"));
		return false;
	}

	long 		vSize = GetTypeSize(BVT_STRING) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE		lpByte = LPBYTE(m_Variable.pAdd[BVT_STRING]);

	LpVarString pLpVar = LpVarString(lpByte + index * vSize);

	lstrcpy(m_szLastError, TEXT("GetVarStringSize::操作成功完成。"));
	return lstrlen(pLpVar->e_tValue);
}

size_t FileEngine::GetVarStringSize(LPCTSTR lpName)
{
	if (m_eType[BVT_STRING].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarStringSize::指定类型不存在或类型元素为空。"));
		return false;
	}
	LPBYTE lpByte = FindVarValue(BVT_STRING, lpName);
	if (lpByte != NULL)
	{
		LpVarString pLpVar = LpVarString(lpByte);
		lstrcpy(m_szLastError, TEXT("GetVarStringSize::操作成功完成。"));
		return lstrlen(pLpVar->e_tValue);
	}

	lstrcpy(m_szLastError, TEXT("GetVarStringSize::找不到指定的字符串变量名。"));
	return -1;
}

size_t FileEngine::GetVarStructSize(int index)
{
	if (m_eType[BVT_STRUCT].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarStructSize::指定类型不存在或类型元素为空。"));
		return -1;
	}
	if (index < 0 || index >= m_eType[BVT_STRUCT].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("GetVarStructSize::指定序号越界（参数::int index）。"));
		return -1;
	}

	long 	vSize = GetTypeSize(BVT_STRUCT) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[BVT_STRUCT]);

	LpVarStruct pLpVar = LpVarStruct(lpByte + index * vSize);

	lstrcpy(m_szLastError, TEXT("GetVarStructSize::操作成功完成。"));
	return pLpVar->e_tSize;

}

size_t FileEngine::GetVarStructSize(LPCTSTR lpName)
{
	if (m_eType[BVT_STRUCT].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarStructSize::指定类型不存在或类型元素为空。"));
		return false;
	}
	LPBYTE lpByte = FindVarValue(BVT_STRUCT, lpName);
	if (lpByte != NULL)
	{
		LpVarStruct pLpVar = LpVarStruct(lpByte);
		lstrcpy(m_szLastError, TEXT("GetVarStructSize::操作成功完成。"));
		return pLpVar->e_tSize;
	}
	lstrcpy(m_szLastError, TEXT("GetVarStructSize::找不到指定的结构体变量名。"));
	return -1;
}

bool FileEngine::AddVarString(int index, LPCTSTR lpName, LPCTSTR lpStr)
{
	if (index < 0 || index >= m_eType[BVT_STRING].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("AddVarString::指定序号越界（参数::int index）。"));
		return false;
	}
	if (FindVarValue(BVT_STRING, lpName) != NULL)
	{
		lstrcpy(m_szLastError, TEXT("AddVarString::变量名已经存在。"));
		return false;
	}

	long 	wSize = GetTypeSize(BVT_STRING);
	long 	vSize = GetTypeVarSize(BVT_STRING);

	long 	nVar = m_eType[BVT_STRING].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[BVT_STRING];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + (index + 1)*vSize, LPBYTE(pVoid) + index * vSize, (nVar - index)*vSize);
	}

	LpVarString pVarString = LpVarString(LPBYTE(pVoid) + index * vSize);
	pVarString->e_tValue = new TCHAR[1 + lstrlen(lpStr)];
	lstrcpy(pVarString->e_tValue, lpStr);
	m_eType[BVT_STRING].e_nVar++;

	lstrcpy(m_szLastError, TEXT("AddVarString::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::SetVarString(int index, LPCTSTR lpStr)
{
	if (index < 0 || index >= m_eType[BVT_STRING].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("SetVarString::指定序号越界（参数::int index）。"));
		return false;
	}

	long 		vSize = GetTypeSize(BVT_STRING) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE		lpByte = LPBYTE(m_Variable.pAdd[BVT_STRING]);

	LpVarString pLpVar = LpVarString(lpByte + index * vSize);

	delete[] pLpVar->e_tValue;
	pLpVar->e_tValue = new TCHAR[1 + lstrlen(lpStr)];
	lstrcpy(pLpVar->e_tValue, lpStr);

	lstrcpy(m_szLastError, TEXT("SetVarString::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::GetVarString(int index, LPTSTR lpStr, int iSize)
{
	if (m_eType[BVT_STRING].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarString::指定类型元素为空。"));
		return false;
	}
	long 		vSize = GetTypeSize(BVT_STRING) + sizeof(TCHAR)*VAR_NAME_SIZE;
	LPBYTE		lpByte = LPBYTE(m_Variable.pAdd[BVT_STRING]);

	LpVarString pLpVar = LpVarString(lpByte + index * vSize);

	lstrcpyn(lpStr, pLpVar->e_tValue, iSize);
	lstrcpy(m_szLastError, TEXT("GetVarString::操作成功完成。"));

	return	true;
}

bool FileEngine::AddVarStruct(int index, LPCTSTR lpName, LPCVOID pDefaut, size_t sSize)
{
	if (index < 0 || index >= m_eType[BVT_STRUCT].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("AddVarStruct::指定序号越界（参数::int index）。"));
		return false;
	}
	if (FindVarValue(BVT_STRUCT, lpName) != NULL)
	{
		lstrcpy(m_szLastError, TEXT("AddVarStruct::变量名已经存在（参数::LPCTSTR lpName）。"));
		return false;
	}

	long 	wSize = GetTypeSize(BVT_STRUCT);
	long 	vSize = GetTypeVarSize(BVT_STRUCT);

	long 	nVar = m_eType[BVT_STRUCT].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[BVT_STRUCT];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + (index + 1)*vSize, LPBYTE(pVoid) + index * vSize, (nVar - index)*vSize);
	}

	LpVarStruct pLpVar = LpVarStruct(LPBYTE(pVoid) + index * vSize);

	pLpVar->e_tSize = sSize;
	pLpVar->e_tValue = malloc(sSize);

	memcpy(pLpVar->e_tValue, pDefaut, sSize);
	lstrcpyn(pLpVar->e_tName, lpName, VAR_NAME_SIZE);

	m_eType[BVT_STRUCT].e_nVar++;

	lstrcpy(m_szLastError, TEXT("AddVarString::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::SetVarStruct(int index, LPCVOID pDefaut, size_t sSize)
{
	if (index < 0 || index >= m_eType[BVT_STRUCT].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("SetVarStruct::指定序号越界（参数::int index）。"));
		return false;
	}

	long 	wSize = GetTypeSize(BVT_STRUCT);
	long 	vSize = GetTypeVarSize(BVT_STRUCT);

	long 	nVar = m_eType[BVT_STRUCT].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[BVT_STRUCT];

	LpVarStruct pLpVar = LpVarStruct(LPBYTE(pVoid) + index * vSize);

	if (sSize == 0)
	{
		memcpy(pLpVar->e_tValue, pDefaut, pLpVar->e_tSize);
	}
	else
	{
		memcpy(pLpVar->e_tValue, pDefaut, pLpVar->e_tSize = sSize);
	}

	lstrcpy(m_szLastError, TEXT("SetVarStruct::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::GetVarStruct(int index, LPVOID pWrite)
{
	if (index < 0 || index >= m_eType[BVT_STRUCT].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("GetVarStruct::指定序号越界（参数::int index）。"));
		return false;
	}

	long 	wSize = GetTypeSize(BVT_STRUCT);
	long 	vSize = GetTypeVarSize(BVT_STRUCT);

	long 	nVar = m_eType[BVT_STRUCT].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[BVT_STRUCT];

	LpVarStruct pLpVar = LpVarStruct(LPBYTE(pVoid) + index * vSize);
	memcpy(pWrite, pLpVar->e_tValue, pLpVar->e_tSize);

	lstrcpy(m_szLastError, TEXT("GetVarStruct::操作成功完成。"));
	return true;
}

bool FileEngine::AddVarValue(WORD vType, int index, LPCTSTR lpName, LPCVOID pDefaut)
{
	if (FindVarValue(vType, lpName) != NULL || vType == BVT_STRUCT)
	{
		lstrcpy(m_szLastError, TEXT("AddVarValue::已经存在同类型、同名的变量，或者不支持的类型。"));
		return false;
	}

	if (index < 0 || index > m_eType[vType].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("AddVarValue::指定序号越界（参数::int index）。"));
		return false;
	}

	long 	wSize = GetTypeSize(vType);
	long 	vSize = GetTypeVarSize(vType);

	long 	nVar = m_eType[vType].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[vType];

	pVoid = MemAllocate(pVoid, (nVar + 1)*vSize);

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + (index + 1)*vSize, LPBYTE(pVoid) + index * vSize, (nVar - index)*vSize);
	}

	LPBYTE lpByte = LPBYTE(pVoid) + index * vSize;

	lstrcpyn(LPTSTR(lpByte), lpName, VAR_NAME_SIZE);

	if (vType == BVT_STRING)
	{
		LPCTSTR lpCStr = LPCTSTR(pDefaut);
		LpVarString pVarString = LpVarString(lpByte);
		pVarString->e_tValue = new TCHAR[1 + lstrlen(lpCStr)];
		lstrcpy(pVarString->e_tValue, lpCStr);
	}
	else
	{
		LPBYTE lpDefaut = LPBYTE(pDefaut) ? LPBYTE(pDefaut) : LPBYTE(&pDefaut);
		memcpy(lpByte + sizeof(TCHAR)*VAR_NAME_SIZE, lpDefaut, wSize);
	}
	m_eType[vType].e_nVar++;
	lstrcpy(m_szLastError, TEXT("AddVarValue::操作成功完成。"));
	m_bSave = true;
	return true;
}

bool FileEngine::AddVarValue(WORD vType, int index, LPCTSTR lpName, int dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)
	{
		lstrcpy(m_szLastError, TEXT("AddVarValue::大于32位的变量不能传值。"));
		return false;
	}

	if (bValueType == TD_VALUE)
		return AddVarValue(vType, index, lpName, &dwDefaut);

	return AddVarValue(vType, index, lpName, LPCVOID(dwDefaut));
}

bool FileEngine::SetVarValue(WORD vType, int index, LPCVOID pDefaut)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("SetVarValue::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (index < 0 || index >= m_eType[vType].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("SetVarValue::指定序号越界（参数::int index）。"));
		return false;
	}

	long 	wSize = GetTypeSize(vType);
	long 	vSize = GetTypeVarSize(vType);

	long 	nVar = m_eType[vType].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[vType];

	LPCTSTR lpName = LPCTSTR(LPBYTE(m_Variable.pAdd[vType]) + index * vSize);

	lstrcpy(m_szLastError, TEXT("SetVarValue::操作成功完成。"));
	return SetVarValue(vType, lpName, pDefaut);

}

bool FileEngine::SetVarValue(WORD vType, int index, int dwDefaut, BYTE bValueType)
{
	if (vType > BVT_VALUE32 && bValueType == TD_VALUE)
	{
		lstrcpy(m_szLastError, TEXT("SetVarValue::大于32位的变量不能传值。"));
		return false;
	}

	if (bValueType == TD_VALUE)
		return SetVarValue(vType, index, &dwDefaut);

	return SetVarValue(vType, index, LPCVOID(dwDefaut));
}

bool FileEngine::GetVarValue(WORD vType, int index, LPVOID pWrite)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("GetVarValue::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (index < 0 || index >= m_eType[vType].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("GetVarValue::指定序号越界（参数::int index）。"));
		return false;
	}

	long 	wSize = GetTypeSize(vType);
	long 	vSize = GetTypeVarSize(vType);

	long 	nVar = m_eType[vType].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[vType];

	LPCTSTR lpName = LPCTSTR(LPBYTE(m_Variable.pAdd[vType]) + index * vSize);

	lstrcpy(m_szLastError, TEXT("GetVarValue::操作成功完成。"));
	return GetVarValue(vType, lpName, pWrite);
}

bool FileEngine::DelVarValue(WORD vType, int index)
{
	if (vType > BVT_ALLTYPE - 1 || m_eType[vType].e_nVar == 0)
	{
		lstrcpy(m_szLastError, TEXT("DelVarValue::指定类型不存在或类型元素为空。"));
		return false;
	}

	if (index < 0 || index >= m_eType[vType].e_nVar)
	{
		lstrcpy(m_szLastError, TEXT("DelVarValue::指定序号越界（参数::int index）。"));
		return false;
	}
	long 	wSize = GetTypeSize(vType);
	long 	vSize = GetTypeVarSize(vType);

	long 	nVar = m_eType[vType].e_nVar;
	PVOID&	pVoid = m_Variable.pAdd[vType];

	LPBYTE	lpByte = LPBYTE(m_Variable.pAdd[vType]) + index * vSize;

	if (vType == BVT_STRUCT)
	{
		LpVarStruct lpVar = LpVarStruct(lpByte);
		free(lpVar->e_tValue);
	}
	else if (vType == BVT_STRING)
	{
		LpVarString lpVar = LpVarString(lpByte);
		delete[] lpVar->e_tValue;
	}

	if (index != nVar)
	{
		memmove(LPBYTE(pVoid) + index * vSize, LPBYTE(pVoid) + (index + 1)*vSize, (nVar - index)*vSize);
	}
	m_eType[vType].e_nVar--;
	lstrcpy(m_szLastError, TEXT("GetVarValue::操作成功完成。"));
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
		lstrcpy(m_szLastError, TEXT("DelAllValue::不支持的类型。"));
		return false;
	}
	else
	{
		long 	wSize = GetTypeSize(vType);
		long 	vSize = GetTypeVarSize(vType);

		long 	nVar = m_eType[vType].e_nVar;
		PVOID&	pVoid = m_Variable.pAdd[vType];

		if (vType == BVT_STRUCT)
		{
			for (int i = 0; i < nVar; i++)
			{
				LpVarStruct lpVar = LpVarStruct(LPBYTE(pVoid) + i * vSize);
				free(lpVar->e_tValue);
			}
		}
		else if (vType == BVT_STRING)
		{
			for (int i = 0; i < nVar; i++)
			{
				LpVarString lpVar = LpVarString(LPBYTE(pVoid) + i * vSize);
				delete[] lpVar->e_tValue;
			}
		}

		free(m_Variable.pAdd[vType]);
		m_Variable.pAdd[vType] = NULL;
		m_eType[vType].e_nVar = 0;

		lstrcpy(m_szLastError, TEXT("DelAllValue::操作成功完成。"));
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
		lstrcpy(m_szLastError, TEXT("GetVarNumber::不支持的类型。"));
		return -1;
	}
	else
	{
		lstrcpy(m_szLastError, TEXT("GetVarNumber::操作成功完成。"));
		return 	m_eType[vType].e_nVar;
	}
}