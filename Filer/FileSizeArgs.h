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

	friend void to_json(json& j, const FileSizeArgs& o);
    friend void from_json(const json& j, FileSizeArgs& o);
};

void to_json(json& j, const FileSizeArgs& o)
{
	j = json{
		{"NoFolderSize", o.NoFolderSize},
		{"NoFolderSizeOnNetwork", o.NoFolderSizeOnNetwork},
		{"TimeLimitFolderSize", o.TimeLimitFolderSize},
		{"TimeLimitMs", o.TimeLimitMs}
	};
}
void from_json(const json& j, FileSizeArgs& o)
{
	j.at("NoFolderSize").get_to(o.NoFolderSize);
	j.at("NoFolderSizeOnNetwork").get_to(o.NoFolderSizeOnNetwork);
	j.at("TimeLimitFolderSize").get_to(o.TimeLimitFolderSize);
	j.at("TimeLimitMs").get_to(o.TimeLimitMs);
}
