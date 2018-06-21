#pragma once

#include "IProcEngine.h"
//
//typedef struct _THREAD_INFO
//{
//	LARGE_INTEGER CreateTime;
//	DWORD dwUnknown1;
//	DWORD dwStartAddress;		//�߳̿�ʼ�������ַ��
//	DWORD StartEIP;
//	DWORD dwOwnerPID;
//	DWORD dwThreadId;			//�̱߳�ʶ����
//	DWORD dwCurrentPriority;	//�߳����ȼ���
//	DWORD dwBasePriority;		//�������ȼ���
//	DWORD dwContextSwitches;	//�����л���Ŀ��
//	DWORD Unknown;
//	DWORD WaitReason;			//�ȴ�ԭ��
//
//}THREADINFO, *PTHREADINFO;
//
//typedef struct _UNICODE_STRING
//{
//	USHORT Length;
//	USHORT MaxLength;
//#ifdef _WIN64
//	LPTSTR Buffer;
//#else
//	PWSTR Buffer;
//#endif // DEBUG
//
//
//} UNICODE_STRING;
//
//typedef struct _PROCESS_INFO
//{
//
//	DWORD dwOffset;				//���ɽṹ���е�ƫ������
//	DWORD dwThreadsCount;		//�߳���Ŀ��
//	DWORD dwUnused1[6];
//
//	LARGE_INTEGER CreateTime;	//����ʱ�䣻
//	LARGE_INTEGER UserTime;		//�û�ģʽ(Ring 3)��CPUʱ�䣻
//	LARGE_INTEGER KernelTime;	//�ں�ģʽ(Ring 0)��CPUʱ�䣻
//	UNICODE_STRING ProcessName;	//�������ƣ�
//
//	DWORD dwBasePriority;		//��������Ȩ��
//	DWORD dwProcessID;			//���̱�ʶ����
//	DWORD dwParentProcessId;	//�����̵ı�ʶ����
//	DWORD dwHandleCount;		//�����Ŀ��
//	DWORD dwUnused3[2];
//	DWORD_PTR dwVirtualBytesPeak;	//����洢��ֵ��С��
//	DWORD_PTR dwVirtualBytes;		//����洢��С��	
//	DWORD dwPageFaults;				//ҳ������Ŀ��
//	DWORD_PTR dwWorkingSetPeak;		//��������ֵ��С��
//	DWORD_PTR dwWorkingSet;			//��������С��
//
//	DWORD_PTR dwQuotaPeakPagedPoolUsage;	//��ҳ��ʹ������ֵ��
//	DWORD_PTR dwQuotaPagedPoolUsage;		//��ҳ��ʹ����
//
//	DWORD_PTR dwQuotaPeakNonPagedPoolUsage;	//�Ƿ�ҳ��ʹ������ֵ��
//	DWORD_PTR dwQuotaNonPagedPoolUsage;		//�Ƿ�ҳ��ʹ����
//
//	DWORD_PTR dwPageFileUsage;			//ҳ�ļ�ʹ�������
//	DWORD_PTR dwPageFileUsagePeak;		//ҳ�ļ�ʹ�÷�ֵ��
//	DWORD_PTR dCommitCharge;
//	THREADINFO ThreadSysInfo[1];	//��������̵߳Ľṹ����
//
//} PROCESSINFO, *PPROCESSINFO;


class ProcEngine :public IProcEngine
{
public:
	ProcEngine(void);
	~ProcEngine(void);

//protected:
//	PPROCESSINFO FindProcFormPid(DWORD dwPid = 0xFFFFFFFF);
//	void CopyProcConfigData(CONFIG *pConfig, PPROCESSINFO pProc);

protected:
	bool FormatLastError(LPCTSTR lpError = NULL);
	bool AnalyzeProcessCpuUsage();

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IBackstage member function	
	virtual long __stdcall UpdateProcessInfo(void);
	virtual bool __stdcall InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath);
	virtual UINT __stdcall GetCurrentProcessNumber();
	virtual PCPI __stdcall GetProcessInfoById(DWORD dwProcessId);
	virtual PCPI __stdcall GetProcessInfoByIndex(int index);
	//virtual LPCCONFIG __stdcall GetProcessConfig(DWORD dwProcessId = 0xFFFFFFFF);
	virtual LPCTSTR GetLastError();

private:
	// IUnknown member data	
	int		m_Ref;

protected:
	// IBackstage member function

//#define	InfoSize 0x200000
//
//	CONFIG			m_Config;
//	BYTE			pProcInfo[InfoSize];
//	ULONG			ulProcCount;
//	__int64 		TotalCPUUsage;
//	__int64			LastTotalCPU;
//	PPROCESSINFO	pProcessInfo;

	TCHAR mSzLastError[MAX_PATH];

	UINT64			mU64CpuUsageLast;
	UINT64			mU64CpuUsageTotal;
	List<PROCESS_INFO> mMltProcessInfo;
};

