#pragma once

#include <Unknwn.h>

// {D44CAC41-F0F3-4C2A-ADB4-0274D862B5DB}
extern "C" const GUID IID_IProcEngine;

typedef struct tagPROCCONFIG
{
	ULONG ulCPUPage;
	TCHAR szUserName[MAX_PATH];
	TCHAR szProcessName[MAX_PATH];
	bool  blIsWin32;

	DWORD dwThreadsCount;		//线程数目；

	DWORD dwBasePriority;		//进程优先权；
	DWORD dwProcessID;			//进程标识符；
	DWORD dwParentProcessId;	//父进程的标识符；
	DWORD dwHandleCount;		//句柄数目；

	DWORD dwVirtualBytesPeak;	//虚拟存储峰值大小；
	DWORD dwVirtualBytes;		//虚拟存储大小；	
	ULONG dwPageFaults;			//页故障数目；
	DWORD dwWorkingSetPeak;		//工作集峰值大小；
	DWORD dwWorkingSet;			//工作集大小；

	DWORD dwQuotaPeakPagedPoolUsage;	//分页池使用配额峰值；
	DWORD dwQuotaPagedPoolUsage;		//分页池使用配额；

	DWORD dwQuotaPeakNonPagedPoolUsage;	//非分页池使用配额峰值；
	DWORD dwQuotaNonPagedPoolUsage;		//非分页池使用配额；

	DWORD dwPageFileUsage;			//页文件使用情况；
	DWORD dwPageFileUsagePeak;		//页文件使用峰值；

}PROCCONFIG, *PPROCCONFIG, *LPPROCCONFIG;

interface IProcEngine : public IUnknown
{
	typedef	PROCCONFIG	CONFIG,*PCONFIG, const near *PCCONFIG, const far *LPCCONFIG;

	virtual long __stdcall UpdateProcessInfo(void) = 0;
	virtual bool __stdcall InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath) = 0;
	virtual LPCCONFIG __stdcall GetProcessConfig(DWORD dwPid = 0xFFFFFFFF) = 0;
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
