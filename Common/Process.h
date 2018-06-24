#pragma once

#include "Handle.h"
#include "Thread.h"

class Process : public Handle {

	using Handle::Handle;

public:
	enum Privilege {
		Query,Read
	};
public:
	Process() = default;
	Process(Privilege privilege, DWORD dwProcessId) {
		if (privilege == Query) {
			OpenWithQuery(dwProcessId);
		} else {
			OpenWithRead(dwProcessId);
		}
	}

public:
	Process & Open(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) {
		mHandle = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
		return *this;
	}
	Process & OpenWithQuery(DWORD dwProcessId) {
		return Open(PROCESS_QUERY_INFORMATION , TRUE, dwProcessId);
	}
	Process & OpenWithRead(DWORD dwProcessId) {
		return Open(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
	}
	LPVOID VirtualAllocEx(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
		return ::VirtualAllocEx(mHandle, lpAddress, dwSize, flAllocationType, flProtect);
	}
	BOOL WriteProcessMemory(LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) {
		return ::WriteProcessMemory(mHandle, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
	}
	BOOL VirtualFreeEx(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
		return ::VirtualFreeEx(mHandle, lpAddress, dwSize, dwFreeType);
	}
	HANDLE CreateRemoteThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
		return ::CreateRemoteThread(mHandle, lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	}
	DWORD GetProcessHandleCount() {
		DWORD dwCount = 0;
		return ::GetProcessHandleCount(mHandle, &dwCount);
	}
};
