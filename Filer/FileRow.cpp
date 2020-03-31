#include "FileRow.h"
#include "ShellFile.h"


CFileRow::CFileRow(CSheet* pSheet, std::shared_ptr<CShellFile> spFile)
	:CRow(pSheet), m_spFile(spFile){}
