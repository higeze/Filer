#include "FileExtCell.h"
#include "Sheet.h"
#include "GridView.h"
#include "ShellFile.h"
#include "FileRow.h"

std::wstring CFileExtCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetExt();
}

void CFileExtCell::SetStringCore(const std::wstring& str)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	pFileRow->GetFilePointer()->SetExt(str, m_pSheet->GetGridPtr()->m_hWnd);
}
