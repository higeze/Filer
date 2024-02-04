#pragma once
#include "MyFriendSerializer.h"
#include "JsonSerializer.h"

struct FileSizeArgs
{
	bool NoFolderSize = false;
	bool NoFolderSizeOnNetwork = true;
	bool TimeLimitFolderSize = true;
	int TimeLimitMs = 300;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("NoFolderSize", NoFolderSize);
		ar("NoFolderSizeOnNetwork", NoFolderSizeOnNetwork);
		ar("TimeLimitFolderSize", TimeLimitFolderSize);
		ar("TimeLimitMs", TimeLimitMs);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(FileSizeArgs,
		NoFolderSize,
		NoFolderSizeOnNetwork,
		TimeLimitFolderSize,
		TimeLimitMs)

};