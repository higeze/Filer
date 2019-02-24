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
		SizeCellPropPtr->FontAndColor->Font.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING;
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
};
