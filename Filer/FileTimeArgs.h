#pragma once
#include "MyFriendSerializer.h"
#include "JsonSerializer.h"

struct FileTimeArgs
{
	FileTimeArgs(){}

	bool TimeLimitFolderLastWrite = true;
	int TimeLimitMs = 100;
	bool IgnoreFolderTime = false;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("TimeLimitLastWrite", TimeLimitFolderLastWrite);
		ar("TimeLimitMs", TimeLimitMs);
		ar("IgnoreFolderTime", IgnoreFolderTime);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(FileTimeArgs,
		TimeLimitFolderLastWrite,
		TimeLimitMs,
		IgnoreFolderTime)
};

