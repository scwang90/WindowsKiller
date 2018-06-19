#pragma once

#include "IProcEngine.h"

typedef struct _THREAD_INFO
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

}THREADINFO, *PTHREADINFO;

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaxLength;
	PWSTR Buffer;

} UNICODE_STRING;

typedef struct _PROCESS_INFO
{

	DWORD dwOffset;				//���ɽṹ���е�ƫ������
	DWORD dwThreadsCount;		//�߳���Ŀ��
	DWORD dwUnused1[6];

	LARGE_INTEGER CreateTime;	//����ʱ�䣻
	LARGE_INTEGER UserTime;		//�û�ģʽ(Ring 3)��CPUʱ�䣻
	LARGE_INTEGER KernelTime;	//�ں�ģʽ(Ring 0)��CPUʱ�䣻
	UNICODE_STRING ProcessName;	//�������ƣ�

	DWORD dwBasePriority;		//��������Ȩ��
	DWORD dwProcessID;			//���̱�ʶ����
	DWORD dwParentProcessId;	//�����̵ı�ʶ����
	DWORD dwHandleCount;		//�����Ŀ��
	DWORD dwUnused3[2];
	DWORD dwVirtualBytesPeak;	//����洢��ֵ��С��
	DWORD dwVirtualBytes;		//����洢��С��	
	ULONG dwPageFaults;			//ҳ������Ŀ��
	DWORD dwWorkingSetPeak;		//��������ֵ��С��
	DWORD dwWorkingSet;			//��������С��

	DWORD dwQuotaPeakPagedPoolUsage;	//��ҳ��ʹ������ֵ��
	DWORD dwQuotaPagedPoolUsage;		//��ҳ��ʹ����

	DWORD dwQuotaPeakNonPagedPoolUsage;	//�Ƿ�ҳ��ʹ������ֵ��
	DWORD dwQuotaNonPagedPoolUsage;		//�Ƿ�ҳ��ʹ����

	DWORD dwPageFileUsage;			//ҳ�ļ�ʹ�������
	DWORD dwPageFileUsagePeak;		//ҳ�ļ�ʹ�÷�ֵ��
	DWORD dCommitCharge;
	THREADINFO ThreadSysInfo[1];	//��������̵߳Ľṹ����

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

