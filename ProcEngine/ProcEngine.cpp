// ProcEngine.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ProcEngine.h"
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <TlHelp32.h>

#include <string>
typedef std::basic_string<TCHAR> tstring;

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
	static ProcCPU	sProcCPU[400];
};


#ifdef _DEBUG
tstring	zsDebug;// , szTemp;
#endif
// class IProcEngine implementation

ULONG    g_LockNumber = 0;
ULONG    g_IProcEngineNumber = 0;
HANDLE	 g_hModule = GetModuleHandle(TEXT("ProcEngine.dll"));
//IsWow64Process
class SystemInfo : public SYSTEM_INFO {
public :
	SystemInfo() {
		GetNativeSystemInfo(this);
	}
	bool IsWin64System() {
		return wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64
			|| wProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64;
	}
	bool isWin32Process(HANDLE hProcess) {
		if (IsWin64System()) {
			BOOL Wow64Process = false;
			IsWow64Process(hProcess, &Wow64Process);
			return Wow64Process;
		}
		return true;
	}
} g_SystemInfo;

ProcEngine::ProcEngine(void)
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
	//pProcInfo = new BYTE[InfoSize];
	lstrcpyn(mSzLastError, TEXT("未执行任何操作。"), sizeof(mSzLastError));
}


ProcEngine::~ProcEngine(void)
{
	//delete[] pProcInfo;
}

HRESULT  DllProcEngine::DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	if (clsid == CLSID_IProcEngine) {

		ProcEngine *pFactory = new ProcEngine;

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

HRESULT  ProcEngine::QueryInterface(const IID& iid, void **ppv)
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

ULONG	 ProcEngine::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 ProcEngine::Release()
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

long ProcEngine::UpdateProcessInfo(void)
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
	ret = -1;// NtQuerySystemInformation(5, pProcInfo, InfoSize, &ReturnLength);

	if (ret)
	{
		_tprintf(TEXT("NtQuerySystemInformation-Fail::%s"), GetLastErrorInfo());

		HANDLE PHANDLE = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (PHANDLE != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof(pe32);
			pe32.dwFlags = sizeof(pe32);
			BOOL success = Process32First(PHANDLE, &pe32);
			LPBYTE lpbyte = pProcInfo;
			PPROCESSINFO lastProcessInfo = PPROCESSINFO(lpbyte);
			ZeroMemory(pProcInfo, sizeof(pProcInfo));
			while (success)
			{
				PPROCESSINFO pProcessInfo = PPROCESSINFO(lpbyte);
				pProcessInfo->dwOffset = sizeof(PROCESSINFO);
				pProcessInfo->dwProcessID = pe32.th32ProcessID;
				pProcessInfo->dwParentProcessId = pe32.th32ParentProcessID;
				pProcessInfo->dwThreadsCount = pe32.cntThreads;
				pProcessInfo->dwUnused1[0] = 0;

				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
				if (NULL != hProcess)
				{
					pProcessInfo->dwUnused1[0] = g_SystemInfo.isWin32Process(hProcess);
					GetProcessHandleCount(hProcess, &pProcessInfo->dwHandleCount);

					//HMODULE hMod;
					//DWORD cbNeeded;

					LPTSTR lpProcessName = LPTSTR(lpbyte + pProcessInfo->dwOffset);
					lstrcpyn(lpProcessName, pe32.szExeFile, sizeof(pe32.szExeFile) / sizeof(TCHAR));
					//lstrcpyn(lpProcessName, TEXT("<unknown>"), MAX_PATH);
					//if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
					//{
					//	GetModuleBaseName(hProcess, hMod, lpProcessName, MAX_PATH);
					//}
					//else
					//{
					//	lstrcpyn(lpProcessName, pe32.szExeFile, sizeof(pe32.szExeFile) / sizeof(TCHAR));
					//	swprintf_s(lpProcessName + lstrlen(lpProcessName), MAX_PATH,TEXT(" 枚举失败：%s"), GetLastErrorInfo());
					//}
					pProcessInfo->ProcessName.Length = lstrlen(lpProcessName);
					pProcessInfo->ProcessName.MaxLength = pProcessInfo->ProcessName.Length + 1;
					pProcessInfo->ProcessName.Buffer = lpProcessName;
					pProcessInfo->dwOffset += (pProcessInfo->ProcessName.MaxLength) * sizeof(TCHAR);

					FILETIME CreationTime = { 0 };
					FILETIME ExitTime = { 0 };
					FILETIME KernelTime = { 0 };
					FILETIME UserTime = { 0 };
					GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
					pProcessInfo->CreateTime.LowPart = CreationTime.dwLowDateTime;
					pProcessInfo->CreateTime.HighPart = CreationTime.dwHighDateTime;
					pProcessInfo->KernelTime.LowPart = KernelTime.dwLowDateTime;
					pProcessInfo->KernelTime.HighPart = KernelTime.dwHighDateTime;
					pProcessInfo->UserTime.LowPart = UserTime.dwLowDateTime;
					pProcessInfo->UserTime.HighPart = UserTime.dwHighDateTime;

					PROCESS_MEMORY_COUNTERS psmemCounters = { 0 };
					GetProcessMemoryInfo(hProcess, &psmemCounters, sizeof(PROCESS_MEMORY_COUNTERS));
					pProcessInfo->dwPageFaults = psmemCounters.PageFaultCount;
					pProcessInfo->dwPageFileUsage = psmemCounters.PagefileUsage;
					pProcessInfo->dwPageFileUsagePeak = psmemCounters.PeakPagefileUsage;
					pProcessInfo->dwQuotaPagedPoolUsage = psmemCounters.QuotaPagedPoolUsage;
					pProcessInfo->dwQuotaNonPagedPoolUsage = psmemCounters.QuotaNonPagedPoolUsage;
					pProcessInfo->dwQuotaPeakPagedPoolUsage = psmemCounters.QuotaPeakPagedPoolUsage;
					pProcessInfo->dwQuotaPeakNonPagedPoolUsage = psmemCounters.QuotaPeakNonPagedPoolUsage;
					pProcessInfo->dwWorkingSet = psmemCounters.WorkingSetSize;
					pProcessInfo->dwWorkingSetPeak = psmemCounters.PeakWorkingSetSize;

				}
				else
				{
					pProcessInfo->dwUnused1[0] = 0;
					LPTSTR lpProcessName = LPTSTR(lpbyte + pProcessInfo->dwOffset);
					lstrcpyn(lpProcessName, pe32.szExeFile, sizeof(pe32.szExeFile)/sizeof(TCHAR));
					//lstrcpyn(lpProcessName, TEXT("<unknown>"), MAX_PATH);
					//FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(0, 1), lpProcessName, MAX_PATH, NULL);
					pProcessInfo->ProcessName.Length = lstrlen(lpProcessName);
					pProcessInfo->ProcessName.MaxLength = pProcessInfo->ProcessName.Length + 1;
					pProcessInfo->ProcessName.Buffer = lpProcessName;
					pProcessInfo->dwOffset += (pProcessInfo->ProcessName.MaxLength) * sizeof(TCHAR);
				}

				lpbyte = lpbyte + pProcessInfo->dwOffset;
				ReturnLength += pProcessInfo->dwOffset;
				lastProcessInfo = pProcessInfo;

				if (sizeof(this->pProcInfo) < ReturnLength) {
					_tprintf(TEXT("统计溢出！！\n"));
				}

				CloseHandle(hProcess);
				success = Process32Next(PHANDLE, &pe32);

			}
			lastProcessInfo->dwOffset = 0;
		}

		//DWORD dwProcessIds[1024], cbNeeded;
		//if (EnumProcesses(dwProcessIds, sizeof(dwProcessIds), &cbNeeded))
		//{
		//	int cProcesses = cbNeeded / sizeof(DWORD);
		//	LPBYTE lpbyte = pProcInfo;
		//	PPROCESSINFO lastProcessInfo = PPROCESSINFO(lpbyte);
		//	ReturnLength = 0;
		//	for (int i = 0; i < cProcesses; i++)
		//	{
		//		if (dwProcessIds[i] != 0)
		//		{
		//		}
		//	}
		//	lastProcessInfo->dwOffset = 0;
		//}
		else
		{
			return 0;
		}
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
PPROCESSINFO ProcEngine::FindProcFormPid(DWORD dwPid)
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
void ProcEngine::CopyProcConfigData(CONFIG *pConfig, PPROCESSINFO pProc)
{
	pConfig->blIsWin32 = pProcessInfo->dwUnused1[0] != 0;
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
auto ProcEngine::GetProcessConfig(DWORD dwPid) -> LPCCONFIG
{
	if (pProcessInfo = FindProcFormPid(dwPid))
	{
		CopyProcConfigData(&m_Config, pProcessInfo);

		if (pProcessInfo->dwProcessID == 0)
			lstrcpy(m_Config.szProcessName, TEXT("[System Process]"));
		else
		{
			lstrcpy(m_Config.szProcessName, PTSTR(pProcessInfo->ProcessName.Buffer));
//#ifdef UNICODE
//			lstrcpy(m_Config.szProcessName, PTSTR(pProcessInfo->ProcessName.Buffer));
//#else
//			WideCharToMultiByte(CP_ACP, 0, pProcessInfo->ProcessName.Buffer, -1, m_Config.szProcessName, MAX_PATH, NULL, NULL);
//#endif // UNICODE
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

LPCTSTR ProcEngine::GetLastError()
{
	return LPCTSTR(mSzLastError);
}

bool ProcEngine::FormatLastError(LPCTSTR lpError)
{
	if (lpError != nullptr) {
		TCHAR szTemp[MAX_PATH];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, ::GetLastError(), MAKELANGID(0, 1), szTemp, MAX_PATH, NULL);
		StringCbPrintf(mSzLastError, sizeof(mSzLastError), TEXT("%s::%s"), lpError, szTemp);
	} else {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, ::GetLastError(), MAKELANGID(0, 1), mSzLastError, sizeof(mSzLastError), NULL);
	}
	return false;
}

bool ProcEngine::InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath)
{
	// 打开目标进程  
	if (dwProcessId == ::GetCurrentProcessId()) {
		lstrcpyn(mSzLastError, TEXT("不能对本进程注入！"), sizeof(mSzLastError));
		return false;
	}
	Process tProcess(OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId));
	if (tProcess.mHdlProcess == nullptr) {
		return FormatLastError(TEXT("打开进程失败"));
	}
	SIZE_T dwSize = (lstrlen(lpDllPath) + 1) * sizeof(TCHAR);
	LPVOID lpBuf = tProcess.VirtualAllocEx(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == lpBuf) {
		return FormatLastError(TEXT("远程开辟空间失败"));
	}
	SIZE_T dwWritten;
	if (!tProcess.WriteProcessMemory(lpBuf, LPVOID(lpDllPath), dwSize, &dwWritten)) {
		return FormatLastError(TEXT("远程写入数据失败"));
	}
	if (dwWritten != dwSize) {
		tProcess.VirtualFreeEx(lpBuf, dwWritten, MEM_DECOMMIT);
		return FormatLastError(TEXT("远程写入数据不符"));
	}
	DWORD dwThreadId;
	Thread tThread(tProcess.CreateRemoteThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, lpBuf, 0, &dwThreadId));
	if (tThread.mHdlThread == nullptr) {
		tProcess.VirtualFreeEx(lpBuf, dwWritten, MEM_DECOMMIT);
		return FormatLastError(TEXT("远程注入代码失败"));
	}
	// 等待LoadLibrary加载完毕  
	WaitForSingleObject(tThread, INFINITE);
	// 释放目标进程中申请的空间  
	tProcess.VirtualFreeEx(lpBuf, dwWritten, MEM_DECOMMIT);
	return true;
}


#define MAX_CPU 400

ULONG	ProcCPU::nProc = 0;
ProcCPU	ProcCPU::sProcCPU[MAX_CPU];

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
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("ProcCPU::UpdateList()\n"));
	zsDebug = szTemp;
#endif
	do
	{
#ifdef _DEBUG
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("    %-25s%15I64dk%15I64du\n"), pProcInfo->ProcessName.Buffer, pProcInfo->KernelTime.QuadPart, pProcInfo->UserTime.QuadPart);
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
					StringCbPrintf(szTemp, sizeof(szTemp), TEXT("        dwCPUTime = %I64d =  %I64d -  %I64d\n"), sProcCPU[i].dwCPUTime, sProcCPU[i].CurrProcCPU, sProcCPU[i].LastProcCPU);
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

	if (MAX_CPU < index) {
		_tprintf(TEXT("归档溢出！！\n"));
	}
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
	else
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(0, 1), szAccName, MAX_PATH, NULL);
		return szAccName;
	}
	return (szAccName[0] = 0, szAccName);
}
