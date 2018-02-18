#pragma once

#define FILE_LINE_FUNC_TRACE std::cout<<boost::format("File:%1%, Line:%2%, Func:%3%") % ::PathFindFileNameA(__FILE__) % __LINE__ % __FUNCTION__<<std::endl
