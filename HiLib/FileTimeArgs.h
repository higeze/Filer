#pragma once
#include "MyFriendSerializer.h"
#include "JsonSerializer.h"

struct FileTimeArgs
{
	FileTimeArgs(){}

	bool TimeLimitFolderLastWrite = true;
	int TimeLimitMs = 100;
	bool IgnoreFolderTime = false;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(FileTimeArgs,
		TimeLimitFolderLastWrite,
		TimeLimitMs,
		IgnoreFolderTime)
};

