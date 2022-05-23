#pragma once
#include "GridViewProperty.h"
#include "FileSizeArgs.h"
#include "FileTimeArgs.h"

struct FilerGridViewProperty:public GridViewProperty
{
	std::shared_ptr<CellProperty> SizeCellPropPtr = std::make_shared<CellProperty>();
	std::shared_ptr<FileSizeArgs> FileSizeArgsPtr = std::make_shared<FileSizeArgs>();
	std::shared_ptr<FileTimeArgs> FileTimeArgsPtr = std::make_shared<FileTimeArgs>();

	FilerGridViewProperty():GridViewProperty()
	{
		*SizeCellPropPtr = *CellPropPtr;
		SizeCellPropPtr->Format = std::make_shared<FormatF>();
		*(SizeCellPropPtr->Format) = *CellPropPtr->Format;
		SizeCellPropPtr->Format->Alignment.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING;
	}
	virtual ~FilerGridViewProperty(){}

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		GridViewProperty::serialize(ar);

		ar("FileSizeArgsProperty", FileSizeArgsPtr);
		ar("LastWriteArgsProperty", FileTimeArgsPtr);
	}

	friend void to_json(json& j, const FilerGridViewProperty& o)
	{
		to_json(j, static_cast<const GridViewProperty&>(o));
		j["FileSizeArgsProperty"] = o.FileSizeArgsPtr;
		j["FileTimeArgsProperty"] = o.FileTimeArgsPtr;
	}
	friend void from_json(const json& j, FilerGridViewProperty& o)
	{
		from_json(j, static_cast<GridViewProperty&>(o));
		*o.SizeCellPropPtr = *o.CellPropPtr;
		o.SizeCellPropPtr->Format = std::make_shared<FormatF>();
		*(o.SizeCellPropPtr->Format) = *o.CellPropPtr->Format;
		o.SizeCellPropPtr->Format->Alignment.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING;

		j.at("FileSizeArgsProperty").get_to(o.FileSizeArgsPtr);
		j.at("FileTimeArgsProperty").get_to(o.FileTimeArgsPtr);
	}
};
