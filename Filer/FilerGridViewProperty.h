#pragma once

#include "GridViewProperty.h"
#include "FileSizeArgs.h"

struct FilerGridViewProperty
{
	std::shared_ptr<FileSizeArgs> FileSizeArgsPtr = std::make_shared<FileSizeArgs>();

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("FileSizeArgsProperty", FileSizeArgsPtr);
	}
};
