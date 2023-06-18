#pragma once
#include "Task.h"
#include "ReactiveProperty.h"
#include "FileStatus.h"

#include "reactive_property.h"
#include "reactive_vector.h"



class CToDoDoc
{
public:
	reactive_property<std::wstring> Path;
	reactive_vector<std::tuple<MainTask>> Tasks;
	reactive_property<FileStatus> Status;
public:
	CToDoDoc():Status(FileStatus::None) {};
	~CToDoDoc() = default;
	//auto operator<=>(const CToDoDoc&) const = default;
	bool operator == (const CToDoDoc & doc) const
	{
		return Path.get() == doc.Path.get();
	};


	void Open(const std::wstring& path);
	void Save(const std::wstring& path);

};