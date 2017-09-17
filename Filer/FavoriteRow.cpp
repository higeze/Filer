#include "FavoriteRow.h"
#include "ShellFile.h"


CFavoriteRow::CFavoriteRow(CGridView* pGrid, std::shared_ptr<CShellFile> spFile, std::wstring& name)
	:CFileRow(pGrid, spFile), m_name(name){}
