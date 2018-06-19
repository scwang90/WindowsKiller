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
	PWSTR Buffer;

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
	DWORD dCommitCharge;
	THREADINFO ThreadSysInfo[1];	//进程相关线程的结构数组

} PROCESSINFO, *PPROCESSINFO;


class ProcEngine :public IProcEngine
{
public:
	ProcEngine(void);
	~ProcEngine(void);

protected:
	PPROCESSINFO FindProcFormPid(DWORD dwPid = 0xFFFFFFFF);

protected:
	void	CopyProcConfigData(Config *pConfig, PPROCESSINFO pProc);

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IBackstage member function	
	virtual long	__stdcall UpdateProcessInfo(void);
	virtual Config*	__stdcall GetProcessConfig(DWORD dwPid = 0xFFFFFFFF);

private:
	// IUnknown member data	
	int		m_Ref;

protected:
	// IBackstage member function
	Config			m_Config;
	LPBYTE			pProcInfo;
	ULONG			ulProcCount;
	__int64 		TotalCPUUsage;
	__int64			LastTotalCPU;
	PPROCESSINFO	pProcessInfo;
};

