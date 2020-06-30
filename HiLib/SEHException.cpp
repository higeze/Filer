#include "SEHException.h"
#include "MyString.h"
#include <fmt/format.h>

void CSEHException::TransferSEHtoCEH ( unsigned int exceptionCode, PEXCEPTION_POINTERS exceptionPointers )
{
	PEXCEPTION_RECORD p = exceptionPointers->ExceptionRecord;
	std::string what = fmt::format(
		"SEH exception"
		"Exception Code:    {:08X}\n"
		"Exception Flags:   {:08X}\n"
		"Exception Address: {:08X}\n",
		p->ExceptionCode, p->ExceptionFlags, p->ExceptionAddress);
	//sprintf_s(
	//	GetBuffer(what,256), 256,
	//	"SEH exception"
	//	"Exception Code:    0x%08x\n"
	//	"Exception Flags:   0x%08x\n"
	//	"Exception Address: 0x%p\n",
	//	p->ExceptionCode, p->ExceptionFlags, p->ExceptionAddress, p->ExceptionInformation);
	//ReleaseBuffer(what);
	throw CSEHException(exceptionCode, exceptionPointers, what);
}

//const char* CSEHException::what()const
//{
//	return m_what.c_str();
//}
