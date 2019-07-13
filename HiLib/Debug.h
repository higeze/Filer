#pragma once

#define FILE_LINE_FUNC (boost::format("File:%1%, Line:%2%, Func:%3%") % ::PathFindFileNameA(__FILE__) % __LINE__ % __FUNCTION__).str().c_str()

#define FILE_LINE_FUNC_TRACE spdlog::info((boost::format("File:%1%, Line:%2%, Func:%3%") % ::PathFindFileNameA(__FILE__) % __LINE__ % __FUNCTION__).str());
