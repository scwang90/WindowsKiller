#pragma once

#include <list>

template<class T>
class MutableList : public std::list<T> {

	using std::list<T>::list;
public:
	__declspec(property(get = size)) UINT length;
public:
	MutableList& operator << (const T& t){
		__super::push_back(t);
		return *this;
	}

//private:
//	static BYTE CPP;
};
