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

	DWORD_PTR dwVirtualBytesPeak;	//����洢��ֵ��С��
	DWORD_PTR dwVirtualBytes;		//����洢��С��	
	ULONG dwPageFaults;			//ҳ������Ŀ��
	DWORD_PTR dwWorkingSetPeak;		//��������ֵ��С��
	DWORD_PTR dwWorkingSet;			//��������С��

	DWORD_PTR dwQuotaPeakPagedPoolUsage;	//��ҳ��ʹ������ֵ��
	DWORD_PTR dwQuotaPagedPoolUsage;		//��ҳ��ʹ����

	DWORD_PTR dwQuotaPeakNonPagedPoolUsage;	//�Ƿ�ҳ��ʹ������ֵ��
	DWORD_PTR dwQuotaNonPagedPoolUsage;		//�Ƿ�ҳ��ʹ����

	DWORD_PTR dwPageFileUsage;			//ҳ�ļ�ʹ�������
	DWORD_PTR dwPageFileUsagePeak;		//ҳ�ļ�ʹ�÷�ֵ��

}PROCCONFIG, *PPROCCONFIG, *LPPROCCONFIG;

interface IProcEngine : public IUnknown
{
	typedef	PROCCONFIG	CONFIG, *PCONFIG, *LPCONFIG;
	typedef const PCONFIG PCCONFIG, LPCCONFIG;

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
