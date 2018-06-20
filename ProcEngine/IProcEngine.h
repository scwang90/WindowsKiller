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

	DWORD dwThreadsCount;		//�߳���Ŀ��

	DWORD dwBasePriority;		//��������Ȩ��
	DWORD dwProcessID;			//���̱�ʶ����
	DWORD dwParentProcessId;	//�����̵ı�ʶ����
	DWORD dwHandleCount;		//�����Ŀ��

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

// �����Ǵ� ProcEngine.dll ������
class PROCENGINE_API DllProcEngine {
public:
	// TODO: �ڴ�������ķ�����
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
};

// {F3EBADDA-2200-4d05-9D7F-8B7A14714D76}
extern "C" const GUID CLSID_IProcEngine;
