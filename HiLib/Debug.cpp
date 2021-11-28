#include "Debug.h"

std::string exception_msg_to_string(const std::exception& e, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n"
		"MSG:{:04X}\r\n"
		"WPARAM:{:04X}\r\n"
		"LPARAM:{:04X}",
		e.what() ,GetLastErrorString());
}

std::string exception_to_string(const std::exception& e)
{
	return fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());
}

std::string msg_to_string(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return	fmt::format(
		"Last Error:{}\r\n"
		"MSG:{:04X}\r\n"
		"WPARAM:{:04X}\r\n"
		"LPARAM:{:04X}",
		msg, GetLastErrorString(), wParam, lParam);
}