#pragma once
#include "MyFriendSerializer.h"

struct FileTimeArgs
{
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
};
