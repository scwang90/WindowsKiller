// ProcEngine.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ProcEngine.h"
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <TlHelp32.h>


#ifdef _DEBUG
String	zsDebug;// , szTemp;
#endif

ULONG    g_LockNumber = 0;
ULONG    g_IProcEngineNumber = 0;
HANDLE	 g_hModule = GetModuleHandle(TEXT("ProcEngine.dll"));

class SystemInfo : public SYSTEM_INFO {
public :
	SystemInfo() {
		GetNativeSystemInfo(this);
		SetPrivilege(SE_DEBUG_NAME, TRUE);
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
	BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege){
		HANDLE hToken;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
		    _tprintf(TEXT("OpenProcessToken"));
		    return FALSE;
		}
		LUID luid;
		if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
		{
			_tprintf(TEXT("LookupPrivilegeValue"));
			return FALSE;
		}
		TOKEN_PRIVILEGES tkp;
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Luid = luid;
		tkp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : FALSE;
		if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
			_tprintf(TEXT("AdjustTokenPrivileges"));
			return FALSE;
		}
	    return TRUE;
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
	mMltProcessInfo.reserve(400);

	StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("未执行任何操作。"));
}


ProcEngine::~ProcEngine(void)
{

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
	HANDLE PHANDLE = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (PHANDLE != INVALID_HANDLE_VALUE)
	{
		UINT index = 0;
		CFileVersion file;
		PROCESSENTRY32 pe32 = { sizeof(pe32) ,0 };
		BOOL success = Process32First(PHANDLE, &pe32);
		PROCESS_INFO piProcessInfo;// = { 0 };
		while (success)
		{
			LPPI lpcProcess = nullptr;
			for (int i = index; i < mMltProcessInfo.length; i++) {
				if (mMltProcessInfo[i].dwProcessID == pe32.th32ProcessID) {
					if (i != index) {
						piProcessInfo = mMltProcessInfo[i];
						mMltProcessInfo[i] = mMltProcessInfo[index];
						mMltProcessInfo[index] = piProcessInfo;
					}
					lpcProcess = &mMltProcessInfo[index];
				}
			}

			Process tProcess(Process::Query, pe32.th32ProcessID);
			if (lpcProcess == nullptr) {
				if (mMltProcessInfo.length > index) {
					mMltProcessInfo << mMltProcessInfo[index];
				} else {
					mMltProcessInfo << piProcessInfo;
				}
				lpcProcess = &mMltProcessInfo[index];
				ZeroMemory(lpcProcess, sizeof(*lpcProcess));

				lpcProcess->dwProcessID = pe32.th32ProcessID;
				lpcProcess->dwParentProcessId = pe32.th32ParentProcessID;
				lpcProcess->strProcessName = pe32.szExeFile;

				if (tProcess) {
					lpcProcess->blIsWin32 = g_SystemInfo.isWin32Process(tProcess);
					lpcProcess->strUserName = GetProcessUserName(tProcess);
					FillProcessFileInfo(lpcProcess, GetProcessFilePath(tProcess), file);
				} else {
					lpcProcess->strUserName = GetLastErrorInfo();
					lpcProcess->strFilePath = LPCTSTR(lpcProcess->strUserName);
					lpcProcess->strDescription = LPCTSTR(lpcProcess->strUserName);
					lpcProcess->strCompanyName = LPCTSTR(lpcProcess->strUserName);

					if (lpcProcess->strProcessName == TEXT("Memory Compression")) {
						lpcProcess->strUserName = TEXT("SYSTEM");
					}
					else if (lpcProcess->strProcessName == TEXT("System") || lpcProcess->strProcessName == TEXT("Registry")) {
						lpcProcess->strUserName = TEXT("SYSTEM");
						FillProcessFileInfo(lpcProcess, TEXT("C:\\Windows\\System32\\ntoskrnl.exe"), file);
					}
					else {
						static String systems[] = {
							TEXT("C:\\Windows\\System32\\wininit.exe"),
							TEXT("C:\\Windows\\System32\\csrss.exe"),
							TEXT("C:\\Windows\\System32\\services.exe"),
							TEXT("C:\\Windows\\System32\\smss.exe"),
							TEXT("C:\\Windows\\System32\\SgrmBroker.exe"),
							TEXT("C:\\Windows\\System32\\SecurityHealthService.exe"),
						};
						for (auto& sys : systems) {
							if (sys.find(lpcProcess->strProcessName, 0) != String::npos) {
								lpcProcess->strUserName = TEXT("SYSTEM");
								FillProcessFileInfo(lpcProcess, sys, file);
								break;
							}
						}
					}
				}
			}
			lpcProcess->dwThreadsCount = pe32.cntThreads;

			if (tProcess) {
				lpcProcess->dwHandleCount = tProcess.GetProcessHandleCount();

				LPCPUTIME_INFO lpTime = &lpcProcess->tiCpuTimeInfo;
				GetProcessTimes(tProcess, &lpTime->CreationTime, &lpTime->ExitTime, &lpTime->KernelTime, &lpTime->UserTime);

				PROCESS_MEMORY_COUNTERS psmemCounters = { 0 };
				GetProcessMemoryInfo(tProcess, &psmemCounters, sizeof(PROCESS_MEMORY_COUNTERS));
				LPMEMORY_INFO lpMemory = &lpcProcess->miMemoryInfo;
				lpcProcess->dwPageFaults = psmemCounters.PageFaultCount;
				lpMemory->dwPageFileUsage = psmemCounters.PagefileUsage;
				lpMemory->dwPageFileUsagePeak = psmemCounters.PeakPagefileUsage;
				lpMemory->dwQuotaPagedPoolUsage = psmemCounters.QuotaPagedPoolUsage;
				lpMemory->dwQuotaNonPagedPoolUsage = psmemCounters.QuotaNonPagedPoolUsage;
				lpMemory->dwQuotaPeakPagedPoolUsage = psmemCounters.QuotaPeakPagedPoolUsage;
				lpMemory->dwQuotaPeakNonPagedPoolUsage = psmemCounters.QuotaPeakNonPagedPoolUsage;
				lpMemory->dwWorkingSet = psmemCounters.WorkingSetSize;
				lpMemory->dwWorkingSetPeak = psmemCounters.PeakWorkingSetSize;
			}
			else if (pe32.th32ProcessID == 0) {
				LPCPUTIME_INFO lpTime = &lpcProcess->tiCpuTimeInfo;
				GetSystemTimes(&lpTime->ExitTime, &lpTime->KernelTime, &lpTime->UserTime);
				lpTime->UserTimeQuad = 0;
				lpTime->KernelTimeQuad = lpTime->ExitTimeQuad;
				LPMEMORY_INFO lpMemory = &lpcProcess->miMemoryInfo;
				lpMemory->dwWorkingSet = 8049;
				lpcProcess->strFilePath = TEXT("");
				lpcProcess->strUserName = TEXT("SYSTEM");
				lpcProcess->strDescription = TEXT("处理器空闲时间百分比");
				lpcProcess->strCompanyName = TEXT("Microsoft Corporation");
			} 
			

			success = Process32Next(PHANDLE, &pe32);
			index++;
		}

		while (mMltProcessInfo.length > index)
		{
			mMltProcessInfo.pop_back();
		}

		this->AnalyzeProcessCpuUsage();
		return long(mMltProcessInfo.length);
	}
	else
	{
		return 0;
	}
}
void ProcEngine::FillProcessFileInfo(const LPPI &lpcProcess, const LPCTSTR &path, CFileVersion &file)
{
	lpcProcess->strFilePath = path;
	if (file.SetFilePath(path)) {
		lpcProcess->strCompanyName = file.GetCompanyName();
		lpcProcess->strDescription = file.GetFileDescription();
	}
	else {
		lpcProcess->strDescription = GetLastErrorInfo();
		lpcProcess->strCompanyName = lpcProcess->strDescription;
	}
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

bool ProcEngine::AnalyzeProcessCpuUsage()
{
	if (mMltProcessInfo.size() > 0) {
		UINT64 u64CpuUsageTotal = 0;

		for (auto& info : mMltProcessInfo) {
			auto lpCpuTime = &info.tiCpuTimeInfo;
			auto totalTome = lpCpuTime->KernelTimeQuad + lpCpuTime->UserTimeQuad;
			u64CpuUsageTotal += totalTome;
		}

		if (u64CpuUsageTotal <= mU64CpuUsageTotal) {
			return false;
		}

		mU64CpuUsageLast = mU64CpuUsageTotal;
		mU64CpuUsageTotal = u64CpuUsageTotal;

		auto u64CpuUsage = mU64CpuUsageTotal - mU64CpuUsageLast;
		for (auto& info : mMltProcessInfo) {
			auto lpCpuTime = &info.tiCpuTimeInfo;
			auto totalTome = lpCpuTime->KernelTimeQuad + lpCpuTime->UserTimeQuad;
			lpCpuTime->ThisTime = totalTome - lpCpuTime->LastTime;
			lpCpuTime->ThisUserTime = lpCpuTime->UserTimeQuad - lpCpuTime->LastUserTime;
			lpCpuTime->ThisKernelTime = lpCpuTime->KernelTimeQuad - lpCpuTime->LastKernelTime;
			lpCpuTime->LastTime = totalTome;
			lpCpuTime->LastUserTime = lpCpuTime->UserTimeQuad;
			lpCpuTime->ThisKernelTime = lpCpuTime->KernelTimeQuad;
			info.ulCpuPage = ULONG(100 * lpCpuTime->ThisTime / u64CpuUsage);
		}
		return true;
	}
	return false;
}

LPCTSTR ProcEngine::GetProcessUserName(HANDLE hProcess)
{
	static TCHAR szAccName[MAX_PATH] = { 0 };
	static TCHAR szDomainName[MAX_PATH] = { 0 };
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
				return szAccName;
			}
			::CloseHandle(hToken);
		}
	}
	return (szAccName[0] = 0, szAccName);
}

LPCTSTR ProcEngine::GetProcessUserName(DWORD ProcessId)
{
	static TCHAR szAccName[MAX_PATH] = { 0 };
	Process tProcess;
	if (tProcess.OpenWithQuery(ProcessId)) {
		return GetProcessUserName(tProcess);
	} else {
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, ::GetLastError(), MAKELANGID(0, 1), szAccName, MAX_PATH, NULL);
		return szAccName;
	}
	return (szAccName[0] = 0, szAccName);
}

LPCTSTR ProcEngine::GetProcessFilePath(HANDLE hProcess)
{
	static TCHAR szProcessPath[MAX_PATH] = { 0 };
	if (GetModuleFileNameEx(hProcess, NULL, szProcessPath, MAX_PATH) == 0) {
		return ::GetLastErrorInfo();
	}
	return szProcessPath;
}

LPCTSTR ProcEngine::GetProcessFilePath(DWORD dwProcessId)
{
	::OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, dwProcessId);
	Process tProcess(Process::Query, dwProcessId);
	if (tProcess) {
		return GetProcessUserName(tProcess);
	} else {
		return ::GetLastErrorInfo();
	}
}

bool ProcEngine::InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath)
{
	// 打开目标进程  
	if (dwProcessId == ::GetCurrentProcessId()) {
		//lstrcpyn(mSzLastError, TEXT("不能对本进程注入！"), sizeof(mSzLastError));
		StringCbCopy(mSzLastError, sizeof(mSzLastError), TEXT("不能对本进程注入！"));
		return false;
	}
	Process tProcess;// (OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId));
	if (!tProcess.Open(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId)) {
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
	if (!tThread) {
		tProcess.VirtualFreeEx(lpBuf, dwWritten, MEM_DECOMMIT);
		return FormatLastError(TEXT("远程注入代码失败"));
	}
	// 等待LoadLibrary加载完毕  
	WaitForSingleObject(tThread, INFINITE);
	// 释放目标进程中申请的空间  
	tProcess.VirtualFreeEx(lpBuf, dwWritten, MEM_DECOMMIT);
	return true;
}

UINT ProcEngine::GetCurrentProcessNumber()
{
	return UINT(mMltProcessInfo.length);
}

PCPI ProcEngine::GetProcessInfoById(DWORD dwProcessId)
{
	for (auto& info : mMltProcessInfo) {
		if (info.dwProcessID == dwProcessId) {
			return &info;
		}
	}
	return nullptr;
}

PCPI ProcEngine::GetProcessInfoByIndex(int index)
{
	if (index < mMltProcessInfo.length)
	{
		return &mMltProcessInfo[index];
	}
	return nullptr;
}

long ProcEngine::GetProcessModulesNumber(DWORD dwProcessId)
{
	DWORD number = 0;
	Process process(Process::Privilege::Read, dwProcessId);
	if (process) {
		if (EnumProcessModulesEx(process, NULL, 0, &number, LIST_MODULES_ALL)) {
			number = number / sizeof(HMODULE);
		}
	}
	return number;
}

long ProcEngine::GetProcessModulesInfos(DWORD dwProcessId, LPMI lpmi, UINT nMax)
{
	Process process(Process::Privilege::Read,dwProcessId);
	if (process) {
		HMODULE* lpModule = new HMODULE[nMax];
		if (EnumProcessModulesEx(process, lpModule, nMax * sizeof(HMODULE), &dwProcessId, LIST_MODULES_ALL))
		{
			CFileVersion file;
			TCHAR szTemp[MAX_PATH] = TEXT("[NULL]");
			for (UINT i = 0; i < nMax; i++)
			{
				lpmi[i].hModule = lpModule[i];
				if (GetModuleFileNameEx(process, lpModule[i], szTemp, MAX_PATH))
				{
					file.SetFilePath(szTemp);
					lpmi[i].strFilePath = szTemp;
					lpmi[i].strCompany = file.GetCompanyName();
					lpmi[i].strDescription = file.GetFileDescription();
				}
			}
			delete[] lpModule;
			return nMax;
		}
		delete[] lpModule;
	}
	return 0;
}
