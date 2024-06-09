#pragma once
#include "MyFriendSerializer.h"
#include "JsonSerializer.h"

struct FileTimeArgs
{
	FileTimeArgs(){}

	bool TimeLimitFolderLastWrite = true;
	int TimeLimitMs = 100;
	bool IgnoreFolderTime = false;

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(TimeLimitFolderLastWrite),
			CEREAL_NVP(TimeLimitMs),
			CEREAL_NVP(IgnoreFolderTime));
	}
	template <class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(TimeLimitFolderLastWrite),
			CEREAL_NVP(TimeLimitMs),
			CEREAL_NVP(IgnoreFolderTime));
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(FileTimeArgs,
		TimeLimitFolderLastWrite,
		TimeLimitMs,
		IgnoreFolderTime)
};

