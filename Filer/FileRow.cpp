#include "FileRow.h"
#include "ShellFile.h"


CFileRow::CFileRow(CGridView* pGrid, std::shared_ptr<CShellFile> spFile)
	:CParentRow(pGrid), m_spFile(spFile){}
