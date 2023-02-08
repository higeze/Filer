#include "se_exception.h"
#include "MyString.h"
#include <format>

void se_exception::TransferSEHtoCEH ( unsigned int exceptionCode, PEXCEPTION_POINTERS exceptionPointers )
{
	PEXCEPTION_RECORD p = exceptionPointers->ExceptionRecord;
	std::string what = std::format(
		"SEH exception\n"
		"Exception Code:    {:08X}\n"
		"Exception Flags:   {:08X}\n"
		"Exception Address: {:08X}\n",
		p->ExceptionCode, p->ExceptionFlags, reinterpret_cast<long>(p->ExceptionAddress));
	throw se_exception(exceptionCode, exceptionPointers, what);
}

//const char* CSEException::what()const
//{
//	return m_what.c_str();
//}
