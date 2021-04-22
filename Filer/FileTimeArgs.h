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

	friend void to_json(json& j, const FileTimeArgs& o);
    friend void from_json(const json& j, FileTimeArgs& o);
};

void to_json(json& j, const FileTimeArgs& o)
{
	j = json{
		{"TimeLimitLastWrite", o.TimeLimitFolderLastWrite},
		{"TimeLimitMs", o.TimeLimitMs},
		{"IgnoreFolderTime", o.IgnoreFolderTime}
	};
}
void from_json(const json& j, FileTimeArgs& o)
{
	j.at("TimeLimitLastWrite").get_to(o.TimeLimitFolderLastWrite);
	j.at("TimeLimitMs").get_to(o.TimeLimitMs);
	j.at("IgnoreFolderTime").get_to(o.IgnoreFolderTime);
}

