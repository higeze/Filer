#pragma once
#include "TextCaret.h"
#include "FileStatus.h"
#include "JsonSerializer.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "encoding_type.h"

#include "ShellFile.h"

class CTextDoc: public CShellFile
{
public:
	std::shared_ptr<int> Dummy;
	reactive_wstring_ptr Path;	
	reactive_wstring_ptr Text;
	reactive_property_ptr<encoding_type> Encoding;
	reactive_property_ptr<FileStatus> Status;

	CTextDoc(const std::wstring& path = std::wstring(), const encoding_type& encoding = encoding_type::UNKNOWN);
	~CTextDoc() = default;
	//CTextDoc(const CTextDoc&) = delete;
	//CTextDoc& operator=(const CTextDoc&) = delete;
	//CTextDoc(CTextDoc&&) noexcept = default;
	//CTextDoc& operator=(CTextDoc&&) noexcept = default;
	//auto operator<=>(const CToDoDoc&) const = default;
	bool operator == (const CTextDoc & doc) const
	{
		return *Path == *doc.Path;
	};

	void Open(const std::wstring& path, const encoding_type& encoding_type = encoding_type::UNKNOWN);
	void Close();
	void Save(const std::wstring& path, const encoding_type& encoding_type);

	friend void to_json(json& j, const CTextDoc& o)
	{
		j["Path"] = o.Path;
	}
	friend void from_json(const json& j, CTextDoc& o)
	{
		get_to(j, "Path", o.Path);
		o.Open(*o.Path);
	}
};

