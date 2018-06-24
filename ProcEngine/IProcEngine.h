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
	DWORD dwStartAddress;		//�߳̿�ʼ�������ַ��
	DWORD StartEIP;
	DWORD dwOwnerPID;
	DWORD dwThreadId;			//�̱߳�ʶ����
	DWORD dwCurrentPriority;	//�߳����ȼ���
	DWORD dwBasePriority;		//�������ȼ���
	DWORD dwContextSwitches;	//�����л���Ŀ��
	DWORD Unknown;
	DWORD WaitReason;			//�ȴ�ԭ��

}THREAD_INFO, *PTHREAD_INFO, *LPTHREAD_INFO;

typedef const THREAD_INFO *PCTHREAD_INFO, *LPCTHREAD_INFO;

typedef struct __MEMORY_INFO
{
	//DWORD_PTR dwVirtualBytesPeak;			//����洢��ֵ��С��
	//DWORD_PTR dwVirtualBytes;				//����洢��С��	
	DWORD_PTR dwWorkingSetPeak;				//��������ֵ��С��
	DWORD_PTR dwWorkingSet;					//��������С��

	DWORD_PTR dwQuotaPeakPagedPoolUsage;	//��ҳ��ʹ������ֵ��
	DWORD_PTR dwQuotaPagedPoolUsage;		//��ҳ��ʹ����

	DWORD_PTR dwQuotaPeakNonPagedPoolUsage;	//�Ƿ�ҳ��ʹ������ֵ��
	DWORD_PTR dwQuotaNonPagedPoolUsage;		//�Ƿ�ҳ��ʹ����

	DWORD_PTR dwPageFileUsage;				//ҳ�ļ�ʹ�������
	DWORD_PTR dwPageFileUsagePeak;			//ҳ�ļ�ʹ�÷�ֵ��

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

	DWORD dwThreadsCount;		//�߳���Ŀ��

	DWORD dwBasePriority;		//��������Ȩ��
	DWORD dwProcessID;			//���̱�ʶ����
	DWORD dwParentProcessId;	//�����̵ı�ʶ����
	DWORD dwHandleCount;		//�����Ŀ��

	DWORD dwPageFaults;			//ҳ������Ŀ��

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

// �����Ǵ� ProcEngine.dll ������
class PROCENGINE_API DllProcEngine {
public:
	// TODO: �ڴ�������ķ�����
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
};

// {F3EBADDA-2200-4d05-9D7F-8B7A14714D76}
extern "C" const GUID CLSID_IProcEngine;
