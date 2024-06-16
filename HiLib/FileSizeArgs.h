#pragma once
#include "MyFriendSerializer.h"
#include "JsonSerializer.h"

struct FileSizeArgs
{
	bool NoFolderSize = false;
	bool NoFolderSizeOnNetwork = true;
	bool TimeLimitFolderSize = true;
	int TimeLimitMs = 300;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(FileSizeArgs,
		NoFolderSize,
		NoFolderSizeOnNetwork,
		TimeLimitFolderSize,
		TimeLimitMs)

};