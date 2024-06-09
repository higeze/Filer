#pragma once
#include "MyFriendSerializer.h"
#include "FileSizeArgs.h"

struct ViewProperty
{
	std::shared_ptr<FileSizeArgs> FileSizeArgsPtr = std::make_shared<FileSizeArgs>();

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::make_nvp("FileSizeArgsProperty", FileSizeArgsPtr));
	}
	template <class Archive>
	void load(Archive& archive)
	{
		archive(cereal::make_nvp("FileSizeArgsProperty", FileSizeArgsPtr));
	}
};

