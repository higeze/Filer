#pragma once
#include <windows.h>
#include <iostream>
#include <exception>
#include "Debug.h"

class se_exception:public std::exception
{
	private:
		unsigned int m_exceptionCode;
		EXCEPTION_POINTERS* m_exceptionPointers;
		std::string m_what;
	public:
		se_exception(
			unsigned int exceptionCode,
			EXCEPTION_POINTERS* exceptionPointers,
			std::string& what)
			:std::exception(what.c_str()),
			m_exceptionCode(exceptionCode ),
			m_exceptionPointers(exceptionPointers),
			m_what(what){}

	virtual ~se_exception(){}
		//virtual const char* what()const override;
	static void TransferSEHtoCEH ( unsigned int exceptionCode, PEXCEPTION_POINTERS exceptionPointers );
};

class scoped_se_translator
{
private:
    const _se_translator_function old_SE_translator;
public:
	scoped_se_translator() noexcept
		: old_SE_translator(_set_se_translator(se_exception::TransferSEHtoCEH)){}

    scoped_se_translator( _se_translator_function new_SE_translator ) noexcept
        : old_SE_translator{ _set_se_translator( new_SE_translator ) } {}

    ~scoped_se_translator() noexcept { _set_se_translator( old_SE_translator ); }
};


