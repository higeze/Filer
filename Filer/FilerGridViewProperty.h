#pragma once

#include "GridViewProperty.h"
#include "FileSizeArgs.h"

struct FilerGridViewProperty
{
	std::shared_ptr<FileSizeArgs> FileSizeArgsPtr = std::make_shared<FileSizeArgs>();
	std::shared_ptr<FileTimeArgs> FileTimeArgsPtr = std::make_shared<FileTimeArgs>();

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("FileSizeArgsProperty", FileSizeArgsPtr);
		ar("LastWriteArgsProperty", FileTimeArgsPtr);
	}
};
