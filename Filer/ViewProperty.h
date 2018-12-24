#pragma once
#include "MyFriendSerializer.h"
#include "FileSizeArgs.h"

struct ViewProperty
{
	std::shared_ptr<FileSizeArgs> FileSizeArgsPtr = std::make_shared<FileSizeArgs>();

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("FileSizeArgsProperty", FileSizeArgsPtr);
	}

};

