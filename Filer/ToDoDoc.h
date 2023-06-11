#pragma once
#include "Task.h"
#include "ReactiveProperty.h"
#include "FileStatus.h"

#include "reactive_property.h"
#include "reactive_vector.h"



class CToDoDoc
{
public:
	ReactiveWStringProperty Path;
	reactive_vector<std::tuple<MainTask>> Tasks;
	reactive_property<FileStatus> Status = FileStatus::None;
public:
	CToDoDoc() {};
	~CToDoDoc() = default;
	auto operator<=>(const CToDoDoc&) const = default;
	bool operator == (const CToDoDoc & doc) const
	{
		return Path.get() == doc.Path.get();
	};
	bool operator != (const CToDoDoc & doc) const
	{
		return Path.get() != doc.Path.get();
	};


	void Open(const std::wstring& path);
	void Save(const std::wstring& path);

};