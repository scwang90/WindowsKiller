#pragma once

#include <Unknwn.h>
#include "../Common/Common.h"
// {D44CAC41-F0F3-4C2A-ADB4-0274D862B5DB}
extern "C" const GUID IID_IProcEngine;

typedef struct _MODULE_INFO
{
	HMODULE hModule;
	String strFilePath;
	String strCompany;
	String strDescription;
}MODULE_INFO, *PMODULE_INFO, *LPMODULE_INFO, *PMI, *LPMI;

typedef const MODULE_INFO *PCMODULE_INFO, *LPCMODULE_INFO, *PCMI, *LCPMI;

typedef struct __THREAD_INFO
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

}THREAD_INFO, *PTHREAD_INFO, *LPTHREAD_INFO;

typedef const THREAD_INFO *PCTHREAD_INFO, *LPCTHREAD_INFO;

typedef struct __MEMORY_INFO
{
	//DWORD_PTR dwVirtualBytesPeak;			//虚拟存储峰值大小；
	//DWORD_PTR dwVirtualBytes;				//虚拟存储大小；	
	DWORD_PTR dwWorkingSetPeak;				//工作集峰值大小；
	DWORD_PTR dwWorkingSet;					//工作集大小；

	DWORD_PTR dwQuotaPeakPagedPoolUsage;	//分页池使用配额峰值；
	DWORD_PTR dwQuotaPagedPoolUsage;		//分页池使用配额；

	DWORD_PTR dwQuotaPeakNonPagedPoolUsage;	//非分页池使用配额峰值；
	DWORD_PTR dwQuotaNonPagedPoolUsage;		//非分页池使用配额；

	DWORD_PTR dwPageFileUsage;				//页文件使用情况；
	DWORD_PTR dwPageFileUsagePeak;			//页文件使用峰值；

}MEMORY_INFO, *PMEMORY_INFO, *LPMEMORY_INFO;

typedef const MEMORY_INFO *PCMEMORY_INFO, *LPCMEMORY_INFO;

typedef struct __CPUTIME_INFO
{
	union {
		FILETIME ExitTime;
		LONGLONG ExitTimeQuad;
	};
	union {
		FILETIME UserTime;
		LONGLONG UserTimeQuad;
	};
	union {
		FILETIME KernelTime;
		LONGLONG KernelTimeQuad;
	};
	union {
		FILETIME CreationTime;
		LONGLONG CreationTimeQuad;
	};
	UINT64 LastTime;
	UINT64 LastUserTime;
	UINT64 LastKernelTime;
	UINT64 ThisTime;
	UINT64 ThisUserTime;
	UINT64 ThisKernelTime;
}CPUTIME_INFO, *PCPUTIME_INFO, *LPCPUTIME_INFO;

typedef const CPUTIME_INFO *PCCPUTIME_INFO, *LPCCPUTIME_INFO;

typedef struct __PROCESS_INFO {

	bool  blIsWin32;

	ULONG ulCpuPage;
	String strUserName;
	String strProcessName;
	String strFilePath;
	String strCompanyName;
	String strDescription;

	DWORD dwThreadsCount;		//线程数目；

	DWORD dwBasePriority;		//进程优先权；
	DWORD dwProcessID;			//进程标识符；
	DWORD dwParentProcessId;	//父进程的标识符；
	DWORD dwHandleCount;		//句柄数目；

	DWORD dwPageFaults;			//页故障数目；

	MEMORY_INFO			miMemoryInfo;
	CPUTIME_INFO		tiCpuTimeInfo;
	List<THREAD_INFO>	ltThreadInfo;

}PROCESS_INFO,*PPROCESS_INFO,*LPPROCESS_INFO,*PPI,*LPPI;

typedef const PROCESS_INFO *PCPROCESS_INFO, *LPCPROCESS_INFO,*PCPI,*LPCPI;

interface IProcEngine : public IUnknown
{
	virtual long __stdcall UpdateProcessInfo(void) = 0;
	virtual bool __stdcall InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath) = 0;
	virtual UINT __stdcall GetCurrentProcessNumber() = 0;
	virtual PCPI __stdcall GetProcessInfoById(DWORD dwProcessId) = 0;
	virtual PCPI __stdcall GetProcessInfoByIndex(int index) = 0;
	virtual long __stdcall GetProcessModulesNumber(DWORD dwProcessId) = 0;
	virtual long __stdcall GetProcessModulesInfos(DWORD dwProcessId, LPMI lpmi, UINT nMax) = 0;
	virtual LPCTSTR GetLastError() = 0;
};

#ifdef PROCENGINE_EXPORTS
#define PROCENGINE_API __declspec(dllexport)
#else
#define PROCENGINE_API __declspec(dllimport)
#endif

// 此类是从 ProcEngine.dll 导出的
class PROCENGINE_API DllProcEngine {
public:
	// TODO: 在此添加您的方法。
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
};

// {F3EBADDA-2200-4d05-9D7F-8B7A14714D76}
extern "C" const GUID CLSID_IProcEngine;
