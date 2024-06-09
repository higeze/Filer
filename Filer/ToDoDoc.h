#pragma once
#include "Task.h"
#include "FileStatus.h"
#include "ShellFile.h"

#include "JsonSerializer.h"
#include "reactive_property.h"
#include "reactive_vector.h"

class CToDoDoc: public CShellFile
{
public:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<std::wstring> Path;
	reactive_vector_ptr<MainTask> Tasks;
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
public:
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(CEREAL_NVP(Path));
	}
	template<class Archive>
	void load(Archive& archive)
	{
		archive(CEREAL_NVP(Path));
		Open(*Path);
	}

	friend void to_json(json& j, const CToDoDoc& o)
	{
		j["Path"] = o.Path;
	}
	friend void from_json(const json& j, CToDoDoc& o)
	{
		get_to(j, "Path", o.Path);
		o.Open(*o.Path);
	}
};