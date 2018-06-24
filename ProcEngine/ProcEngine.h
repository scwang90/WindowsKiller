#pragma once

#include "IProcEngine.h"

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
	LPCTSTR GetProcessUserName(HANDLE hProcess);
	LPCTSTR GetProcessUserName(DWORD ProcessId);
	LPCTSTR GetProcessFilePath(HANDLE hProcess);
	LPCTSTR GetProcessFilePath(DWORD ProcessId);

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IBackstage member function	
	virtual long __stdcall UpdateProcessInfo(void);
	void FillProcessFileInfo(const LPPI &lpcProcess, const LPCTSTR &path, CFileVersion &file);
	virtual bool __stdcall InjectDllToProcess(DWORD dwProcessId, LPCTSTR lpDllPath);
	virtual UINT __stdcall GetCurrentProcessNumber();
	virtual PCPI __stdcall GetProcessInfoById(DWORD dwProcessId);
	virtual PCPI __stdcall GetProcessInfoByIndex(int index);
	virtual long __stdcall GetProcessModulesNumber(DWORD dwProcessId);
	virtual long __stdcall GetProcessModulesInfos(DWORD dwProcessId, LPMI lpmi, UINT nMax);
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

