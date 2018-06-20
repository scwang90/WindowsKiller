#pragma once

#include "IProcEngine.h"

typedef struct _THREAD_INFO
{
	LARGE_INTEGER CreateTime;
	DWORD dwUnknown1;
	DWORD dwStartAddress;		//线程开始的虚拟地址；
	DWORD StartEIP;
	DWORD dwOwnerPID;
	DWORD dwThreadId;			//线程标识符；
	DWORD dwCurrentPriority;	//线程优先级；
	DWORD dwBasePriority;		//基本优先级；
	DWORD dwContextSwitches;	//环境切换数目；
	DWORD Unknown;
	DWORD WaitReason;			//等待原因；

}THREADINFO, *PTHREADINFO;

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaxLength;
#ifdef _WIN64
	LPTSTR Buffer;
#else
	PWSTR Buffer;
#endif // DEBUG


} UNICODE_STRING;

typedef struct _PROCESS_INFO
{

	DWORD dwOffset;				//构成结构序列的偏移量；
	DWORD dwThreadsCount;		//线程数目；
	DWORD dwUnused1[6];

	LARGE_INTEGER CreateTime;	//创建时间；
	LARGE_INTEGER UserTime;		//用户模式(Ring 3)的CPU时间；
	LARGE_INTEGER KernelTime;	//内核模式(Ring 0)的CPU时间；
	UNICODE_STRING ProcessName;	//进程名称；

	DWORD dwBasePriority;		//进程优先权；
	DWORD dwProcessID;			//进程标识符；
	DWORD dwParentProcessId;	//父进程的标识符；
	DWORD dwHandleCount;		//句柄数目；
	DWORD dwUnused3[2];
	DWORD_PTR dwVirtualBytesPeak;	//虚拟存储峰值大小；
	DWORD_PTR dwVirtualBytes;		//虚拟存储大小；	
	DWORD dwPageFaults;			//页故障数目；
	DWORD_PTR dwWorkingSetPeak;		//工作集峰值大小；
	DWORD_PTR dwWorkingSet;			//工作集大小；

	DWORD_PTR dwQuotaPeakPagedPoolUsage;	//分页池使用配额峰值；
	DWORD_PTR dwQuotaPagedPoolUsage;		//分页池使用配额；

	DWORD_PTR dwQuotaPeakNonPagedPoolUsage;	//非分页池使用配额峰值；
	DWORD_PTR dwQuotaNonPagedPoolUsage;		//非分页池使用配额；

	DWORD_PTR dwPageFileUsage;			//页文件使用情况；
	DWORD_PTR dwPageFileUsagePeak;		//页文件使用峰值；
	DWORD_PTR dCommitCharge;
	THREADINFO ThreadSysInfo[1];	//进程相关线程的结构数组

} PROCESSINFO, *PPROCESSINFO;

class Thread {
protected:
	HANDLE m_hThread;

public:
	Thread(HANDLE hThread) :m_hThread(hThread) {}
	~Thread() {
		if (m_hThread) {
			CloseHandle(m_hThread);
		}
	}
public:
	__declspec(property(get = GetHandle)) HANDLE mHdlThread;

public:
	HANDLE GetHandle() {
		return m_hThread;
	}
	operator HANDLE() {
		return m_hThread;
	}
};
class Process {

protected:
	HANDLE m_hProcess;

public:
	Process(HANDLE hProcess) :m_hProcess(hProcess) {}
	~Process() {
		if (m_hProcess) {
			CloseHandle(m_hProcess);
		}
	}
public:
	__declspec(property(get = GetHandle)) HANDLE mHdlProcess;

public:
	HANDLE GetHandle() {
		return m_hProcess;
	}
	operator HANDLE() {
		return m_hProcess;
	}
	LPVOID VirtualAllocEx(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
		return ::VirtualAllocEx(m_hProcess, lpAddress, dwSize, flAllocationType, flProtect);
	}
	BOOL WriteProcessMemory(LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) {
		return ::WriteProcessMemory(m_hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
	}
	BOOL VirtualFreeEx(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
		return ::VirtualFreeEx(m_hProcess, lpAddress, dwSize, dwFreeType);
	}
	HANDLE CreateRemoteThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
		return ::CreateRemoteThread(m_hProcess, lpThreadAttributes, dwStackSize,lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	}
};

class ProcEngine :public IProcEngine
{
public:
	ProcEngine(void);
	~ProcEngine(void);

protected:
	PPROCESSINFO FindProcFormPid(DWORD dwPid = 0xFFFFFFFF);

protected:
	void CopyProcConfigData(CONFIG *pConfig, PPROCESSINFO pProc);
	bool FormatLastError(LPCTSTR lpError = NULL);

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IBackstage member function	
	virtual long __stdcall UpdateProcessInfo(void);
	virtual bool __stdcall InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath);
	virtual LPCCONFIG __stdcall GetProcessConfig(DWORD dwProcessId = 0xFFFFFFFF);
	virtual LPCTSTR GetLastError();

private:
	// IUnknown member data	
	int		m_Ref;

protected:
	// IBackstage member function

#define	InfoSize 0x200000

	CONFIG			m_Config;
	BYTE			pProcInfo[InfoSize];
	ULONG			ulProcCount;
	__int64 		TotalCPUUsage;
	__int64			LastTotalCPU;
	PPROCESSINFO	pProcessInfo;

	TCHAR			mSzLastError[MAX_PATH];
};

