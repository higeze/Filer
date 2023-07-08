#pragma once
#include "Task.h"
#include "ReactiveProperty.h"
#include "FileStatus.h"

#include "reactive_property.h"
#include "reactive_vector.h"



class CToDoDoc
{
private:
	std::vector<sigslot::connection> m_connections;
public:
	reactive_property_ptr<std::wstring> Path;
	reactive_vector_ptr<std::tuple<MainTask>> Tasks;
	reactive_property_ptr<FileStatus> Status;
public:
	CToDoDoc();
	~CToDoDoc();
	//auto operator<=>(const CToDoDoc&) const = default;
	bool operator == (const CToDoDoc & doc) const
	{
		return Path.get() == doc.Path.get();
	};

	void Open(const std::wstring& path);
	void Save(const std::wstring& path);

};