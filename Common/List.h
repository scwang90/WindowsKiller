#pragma once

#include <vector>

template<class T>
class List : public std::vector<T> {

	using std::vector<T>::vector;

public:
	__declspec(property(get = size)) size_t length;

public:
	List & operator << (const T& t) {
		__super::push_back(t);
		return *this;
	}

//private:
//	static BYTE CPP;
};
