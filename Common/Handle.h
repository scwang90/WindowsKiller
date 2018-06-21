#pragma once

class Handle {

protected:
	HANDLE mHandle;

public:
	Handle() = default;
	Handle(HANDLE hHandle) :mHandle(hHandle) {}
	virtual ~Handle() {
		if (mHandle) {
			CloseHandle(mHandle);
			mHandle = nullptr;
		}
	}
public:
	__declspec(property(get = GetHandle)) HANDLE handle;

public:
	HANDLE GetHandle() {
		return mHandle;
	}
	operator HANDLE() {
		return mHandle;
	}
	operator bool() {
		return mHandle != nullptr;
	}
};
