#pragma once

#include <string>

typedef std::basic_string<TCHAR> tstring;

class COMMON_API String : public tstring {

	using tstring::tstring;

public:
	operator LPCTSTR() const;

public:
	String& format(LPCTSTR format, ...);

};

