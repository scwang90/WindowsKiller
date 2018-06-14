// ProcEngine.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ProcEngine.h"
#include <stdio.h>
#include <tchar.h>

#include <string>
typedef std::basic_string<TCHAR> tstring;

#define	InfoSize 0x20000

typedef	long(__stdcall*NQSI)(DWORD, _Inout_ PVOID, _In_ ULONG, _Out_opt_ PULONG);

NQSI NtQuerySystemInformation = (NQSI)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), ("NtQuerySystemInformation"));

struct ProcCPU
{
	ProcCPU() :LastProcCPU(0), CurrProcCPU(0) {}
	__int64 CurrProcCPU;
	__int64 LastProcCPU;
	__int64	dwCPUTime;
	DWORD	dwProcessId;
	TCHAR	szUserName[64];

	static ProcCPU*	GetProcCPU(DWORD dwProcessId);
	static long		UpdateList(PPROCESSINFO pProcInfo, __int64 &TotalCPUUsage);
	static LPCTSTR	GetProcessUserName(DWORD ProcessID);

	static ULONG	nProc;
	static ProcCPU	sProcCPU[200];
};


#ifdef _DEBUG
tstring	zsDebug;// , szTemp;
#endif
// class IProcEngine implementation

ULONG    g_LockNumber = 0;
ULONG    g_IProcEngineNumber = 0;
HANDLE	 g_hModule = GetModuleHandle(TEXT("ProcEngine.dll"));

CProcessConfig::CProcessConfig(void)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IProcEngineNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IFileEngine
	TotalCPUUsage = 0;
	LastTotalCPU = 0;
	ulProcCount = 0;
	pProcessInfo = NULL;
	pProcInfo = new BYTE[InfoSize];
}


CProcessConfig::~CProcessConfig(void)
{
	delete[] pProcInfo;
}

HRESULT  DllProcEngine::DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	if (clsid == CLSID_IProcEngine) {

		CProcessConfig *pFactory = new CProcessConfig;

		if (pFactory == NULL) {
			return E_OUTOFMEMORY;
		}

		HRESULT result = pFactory->QueryInterface(iid, ppv);

		return result;
	}
	else {
		return CLASS_E_CLASSNOTAVAILABLE;
	}
}

HRESULT  CProcessConfig::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IProcEngine)
	{
		*ppv = (IProcEngine *)this;
		((IProcEngine *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	 CProcessConfig::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 CProcessConfig::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IProcEngineNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}

//public

long CProcessConfig::UpdateProcessInfo(void)
{
	//static int i = 0;

	//if(i == 0)
	//{
	//	i = 1;
	//	MessageBox(0,"-CProcessConfig::GetProcessConfig(DWORD dwPid)","",0);
	//}

	__int64 CurTotalCPUUsage = 0;

	long ret = 0;
	ULONG ReturnLength = -1;
	ret = NtQuerySystemInformation(5, pProcInfo, InfoSize, &ReturnLength);

	if (ret)
	{
		_tprintf(TEXT("%s"), GetLastErrorInfo());
		return 0;
	}

	ulProcCount = ProcCPU::UpdateList(pProcessInfo = PPROCESSINFO(pProcInfo), CurTotalCPUUsage);

	TotalCPUUsage = CurTotalCPUUsage - LastTotalCPU;

	if (TotalCPUUsage < 0)
	{
#ifdef _DEBUG
		_tprintf(TEXT("CProcessConfig::UpdateProcessInfo::TotalCPUUsage < 0\n"));
		_tprintf(TEXT("	TotalCPUUsage = %I64d = %I64d - %I64d\n"), TotalCPUUsage, CurTotalCPUUsage, LastTotalCPU);
		_tprintf(LPCTSTR(zsDebug.c_str()));
		TotalCPUUsage += 0X7FFFFFFFFFFFFFFF;
#endif

	}

	LastTotalCPU = CurTotalCPUUsage;

	return long(ulProcCount);
}
PPROCESSINFO CProcessConfig::FindProcFormPid(DWORD dwPid)
{
	if (dwPid != 0xFFFFFFFF)
	{
		pProcessInfo = PPROCESSINFO(pProcInfo);
		do
		{
			if (pProcessInfo->dwProcessID == dwPid)
			{
				return pProcessInfo;
			}
			pProcessInfo = PPROCESSINFO(LPBYTE(pProcessInfo) + pProcessInfo->dwOffset);
		} while (pProcessInfo->dwOffset);

		return NULL;
	}
	return pProcessInfo;
}
void CProcessConfig::CopyProcConfigData(Config *pConfig, PPROCESSINFO pProc)
{
	pConfig->dwThreadsCount = pProcessInfo->dwThreadsCount;//线程数目；

	pConfig->dwBasePriority = pProcessInfo->dwBasePriority;//进程优先权；
	pConfig->dwProcessID = pProcessInfo->dwProcessID;//进程标识符；
	pConfig->dwParentProcessId = pProcessInfo->dwParentProcessId;//父进程的标识符；
	pConfig->dwHandleCount = pProcessInfo->dwHandleCount;//句柄数目；

	pConfig->dwVirtualBytesPeak = pProcessInfo->dwVirtualBytesPeak;//虚拟存储峰值大小；
	pConfig->dwVirtualBytes = pProcessInfo->dwVirtualBytes;//虚拟存储大小；	
	pConfig->dwPageFaults = pProcessInfo->dwPageFaults;//页故障数目；
	pConfig->dwWorkingSetPeak = pProcessInfo->dwWorkingSetPeak;//工作集峰值大小；
	pConfig->dwWorkingSet = pProcessInfo->dwWorkingSet;//工作集大小；

	pConfig->dwQuotaPeakPagedPoolUsage = pProcessInfo->dwQuotaPeakPagedPoolUsage;//分页池使用配额峰值；
	pConfig->dwQuotaPagedPoolUsage = pProcessInfo->dwQuotaPagedPoolUsage;//分页池使用配额；

	pConfig->dwQuotaPeakNonPagedPoolUsage = pProcessInfo->dwQuotaPeakNonPagedPoolUsage;//非分页池使用配额峰值；
	pConfig->dwQuotaNonPagedPoolUsage = pProcessInfo->dwQuotaNonPagedPoolUsage;//非分页池使用配额；

	pConfig->dwPageFileUsage = pProcessInfo->dwPageFileUsage;//页文件使用情况；
	pConfig->dwPageFileUsagePeak = pProcessInfo->dwPageFileUsagePeak;//页文件使用峰值；
}
CProcessConfig::Config* CProcessConfig::GetProcessConfig(DWORD dwPid)
{

	if (pProcessInfo = FindProcFormPid(dwPid))
	{
		CopyProcConfigData(&m_Config, pProcessInfo);

		if (pProcessInfo->dwProcessID == 0)
			lstrcpy(m_Config.szProcessName, TEXT("[System Process]"));
		else
		{
#ifdef UNICODE
			lstrcpy(m_Config.szProcessName, PTSTR(pProcessInfo->ProcessName.Buffer));
#else
			WideCharToMultiByte(CP_ACP, 0, pProcessInfo->ProcessName.Buffer, -1, m_Config.szProcessName, MAX_PATH, NULL, NULL);
#endif // UNICODE
		}

		ProcCPU* pProcCPU = ProcCPU::GetProcCPU(pProcessInfo->dwProcessID);
		if (pProcCPU != NULL)
		{
			if (TotalCPUUsage != 0)
			{
				m_Config.ulCPUPage = ULONG(99 * pProcCPU->dwCPUTime / TotalCPUUsage);

				if (m_Config.ulCPUPage == 0 && pProcCPU->dwCPUTime != 0)
					m_Config.ulCPUPage = 1;
				//else if(m_Config.ulCPUPage == 100 && pProcCPU->dwCPUTime != TotalCPUUsage)
				//	m_Config.ulCPUPage = 99;

#ifdef _DEBUG
				if (m_Config.ulCPUPage == 100 && pProcCPU->dwCPUTime != TotalCPUUsage)
				{
					_tprintf(TEXT("CProcessConfig::GetProcessConfig::%s.ulCPUPage = 100\n"), pProcessInfo->ProcessName.Buffer);
				}
#endif
			}
			else m_Config.ulCPUPage = 0;

#ifdef _DEBUG
			if (int(m_Config.ulCPUPage) < 0)
			{
				//system("cls");
				_tprintf(TEXT("%s = %d ::dwCPUTime = %I64d::TotalCPUUsage = %I64d\n"), m_Config.szProcessName, m_Config.ulCPUPage, pProcCPU->dwCPUTime, TotalCPUUsage);
				_tprintf(zsDebug.c_str());
			}
			else if (pProcessInfo->dwProcessID == 0)system("cls"), _tprintf(zsDebug.c_str());
#endif
			lstrcpy(m_Config.szUserName, pProcCPU->szUserName);
		}
		else
		{
			m_Config.ulCPUPage = 0;
			m_Config.szUserName[0] = 0;
		}


		pProcessInfo = pProcessInfo->dwOffset ? PPROCESSINFO(LPBYTE(pProcessInfo) + pProcessInfo->dwOffset) : NULL;

		return &m_Config;
	}

	return NULL;
}



ULONG	ProcCPU::nProc = 0;
ProcCPU	ProcCPU::sProcCPU[200];

ProcCPU* ProcCPU::GetProcCPU(DWORD dwProcessId)
{
	for (ULONG i = 0; i < nProc; i++)
	{
		if (sProcCPU[i].dwProcessId == dwProcessId)
		{
			return &sProcCPU[i];
		}
	}
	return NULL;
}

long ProcCPU::UpdateList(PPROCESSINFO pProcInfo, __int64 &TotalCPUUsage)
{
	ULONG i, index = 0;
#ifdef _DEBUG
	TCHAR szTemp[MAX_PATH];
	wsprintf(szTemp, TEXT("ProcCPU::UpdateList()\n"));
	zsDebug = szTemp;
#endif
	do
	{
#ifdef _DEBUG
		wsprintf(szTemp, TEXT("    %-25s%15I64dk%15I64du\n"), pProcInfo->ProcessName.Buffer, pProcInfo->KernelTime.QuadPart, pProcInfo->UserTime.QuadPart);
		zsDebug += szTemp;
#endif
		for (i = index; i < nProc; i++)
		{
			if (sProcCPU[i].dwProcessId == pProcInfo->dwProcessID)
			{
				sProcCPU[i].LastProcCPU = sProcCPU[i].CurrProcCPU;
				sProcCPU[i].CurrProcCPU = pProcInfo->UserTime.QuadPart + pProcInfo->KernelTime.QuadPart;
				sProcCPU[i].dwCPUTime = sProcCPU[i].CurrProcCPU - sProcCPU[i].LastProcCPU;
#ifdef _DEBUG
				if (sProcCPU[i].dwCPUTime < 0)
				{
					wsprintf(szTemp, TEXT("        dwCPUTime = %I64d =  %I64d -  %I64d\n"), sProcCPU[i].dwCPUTime, sProcCPU[i].CurrProcCPU, sProcCPU[i].LastProcCPU);
					zsDebug += szTemp;
				}
#endif
				TotalCPUUsage += sProcCPU[i].CurrProcCPU;

				if (i != index)
				{
					sProcCPU[nProc] = sProcCPU[i];
					sProcCPU[i] = sProcCPU[index];
					sProcCPU[index] = sProcCPU[nProc];
				}
				break;
			}
		}
		if (i == nProc)
		{
			sProcCPU[nProc++] = sProcCPU[index];
			sProcCPU[index].dwProcessId = pProcInfo->dwProcessID;
			sProcCPU[index].LastProcCPU = 0;
			sProcCPU[index].CurrProcCPU = pProcInfo->UserTime.QuadPart + pProcInfo->KernelTime.QuadPart;
			lstrcpy(sProcCPU[index].szUserName, GetProcessUserName(pProcInfo->dwProcessID));
		}
		index++;
		pProcInfo = PPROCESSINFO(LPBYTE(pProcInfo) + pProcInfo->dwOffset);
	} while (pProcInfo->dwOffset);
#ifdef _DEBUG
	//wprintf(LPCWSTR(zsDebug));
#endif
	return nProc = index;
}
LPCTSTR ProcCPU::GetProcessUserName(DWORD ProcessID)
{
	static TCHAR szAccName[MAX_PATH] = { 0 };
	static TCHAR szDomainName[MAX_PATH] = { 0 };

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, ProcessID);
	if (hProcess != NULL)
	{
		HANDLE hToken = NULL;
		if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken) != FALSE)
		{
			DWORD dwTokenUser = 0;
			GetTokenInformation(hToken, TokenUser, NULL, 0L, &dwTokenUser);

			if (dwTokenUser > 0)
			{
				PTOKEN_USER pToken_User = (PTOKEN_USER)GlobalAlloc(GPTR, dwTokenUser);
				GetTokenInformation(hToken, TokenUser, pToken_User, dwTokenUser, &dwTokenUser);

				DWORD dwAccName = 0L;
				DWORD dwDomainName = 0L;
				SID_NAME_USE eUse = SidTypeUnknown;


				LookupAccountSid(NULL, pToken_User->User.Sid, NULL, &dwAccName, NULL, &dwDomainName, &eUse);
				LookupAccountSid(NULL, pToken_User->User.Sid, szAccName, &dwAccName, szDomainName, &dwDomainName, &eUse);

				::GlobalFree(pToken_User);
				::CloseHandle(hToken);
				::CloseHandle(hProcess);
				return szAccName;
			}
			::CloseHandle(hToken);
		}
		::CloseHandle(hProcess);
	}
	return (szAccName[0] = 0, szAccName);
}
