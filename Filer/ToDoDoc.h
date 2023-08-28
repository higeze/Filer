#pragma once
#include "Task.h"
#include "FileStatus.h"

#include "reactive_property.h"
#include "reactive_vector.h"

class CToDoDoc
{
public:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<std::wstring> Path;
	reactive_vector_ptr<std::tuple<MainTask>> Tasks;
	reactive_property_ptr<FileStatus> Status;
public:
	CToDoDoc();
	~CToDoDoc() = default;
	//auto operator<=>(const CToDoDoc&) const = default;
	bool operator == (const CToDoDoc & doc) const
	{
		return *Path == *doc.Path;
	};

	void Open(const std::wstring& path);
	void Save(const std::wstring& path);

};