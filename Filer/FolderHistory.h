//#pragma once
//#include <deque>
//#include <sigslot/signal.hpp>
//
//#include "reactive_property.h"
//
//class CFolderHistory
//{
//private:
//	static const int kMaxsize = 50;
//	std::deque<std::wstring> m_history;
//public:
//	reactive_property_ptr<bool> CanPrevious;
//	reactive_property_ptr<bool> CanNext;
//public:
//	CFolderHistory():CanPrevious(false),CanNext(false){}
//	virtual ~CFolderHistory() = default;
//
//	void Current(const std::wstring& path);
//	void Next();
//	void Previousi();
//	void Clear();
//};
//
