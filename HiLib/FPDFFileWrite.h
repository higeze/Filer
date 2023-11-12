#pragma once
#include "FPDF.h"
#include <string>
#include <Windows.h>


class CFPDFFileWrite : public FPDF_FILEWRITE
{
private:
	FILE* m_pFile = nullptr;
public:
	CFPDFFileWrite(const std::string& path)
	{
		fopen_s(&m_pFile, path.c_str(), "w+b");

		version = 1;
		WriteBlock = [](FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)->int
		{
			return fwrite(pData, 1, size, static_cast<CFPDFFileWrite*>(pThis)->m_pFile);
		};
	}
	CFPDFFileWrite(const std::wstring& path)
	{
		_wfopen_s(&m_pFile, path.c_str(), L"w+b");

		version = 1;
		WriteBlock = [](FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)->int
		{
			return fwrite(pData, 1, size, static_cast<CFPDFFileWrite*>(pThis)->m_pFile);
		};
	}

	virtual ~CFPDFFileWrite()
	{
		fclose(m_pFile);
	}
};

