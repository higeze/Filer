#pragma once
#include "MyFriendSerializer.h"
#include "FileSizeArgs.h"

struct ViewProperty
{
	std::shared_ptr<FileSizeArgs> FileSizeArgsPtr = std::make_shared<FileSizeArgs>();
};

