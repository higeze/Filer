#pragma once
#include <cereal/cereal.hpp>
#include "MyFriendSerializer.h"
#include "JsonSerializer.h"

struct FileSizeArgs
{
	bool NoFolderSize = false;
	bool NoFolderSizeOnNetwork = true;
	bool TimeLimitFolderSize = true;
	int TimeLimitMs = 300;

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(NoFolderSize),
			CEREAL_NVP(NoFolderSizeOnNetwork),
			CEREAL_NVP(TimeLimitFolderSize),
			CEREAL_NVP(TimeLimitMs));
	}
	template <class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(NoFolderSize),
			CEREAL_NVP(NoFolderSizeOnNetwork),
			CEREAL_NVP(TimeLimitFolderSize),
			CEREAL_NVP(TimeLimitMs));
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(FileSizeArgs,
		NoFolderSize,
		NoFolderSizeOnNetwork,
		TimeLimitFolderSize,
		TimeLimitMs)

};