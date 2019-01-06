#include "FileNameCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "GridView.h"

CFileNameCell::CFileNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CParameterCell(pSheet, pRow, pColumn, spProperty){}

std::wstring CFileNameCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetFileNameWithoutExt();
}

void CFileNameCell::SetStringCore(const std::basic_string<TCHAR>& str)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	pFileRow->GetFilePointer()->SetFileNameWithoutExt(str);

	//CIDLPtr pIdlNew;
	//HRESULT hRes = spFile->GetParentShellFolderPtr()->SetNameOf(
	//	m_pSheet->GetGridPtr()->m_hWnd,
	//	spFile->GetAbsolutePidl().FindLastID(),
	//	str.c_str(),
	//	SHGDN_FOREDITING | SHGDN_INFOLDER,
	//	&pIdlNew );
	//if(SUCCEEDED(hRes)){
	//	pFileRow->SetFilePointer(std::make_shared<CShellFile>(spFile->GetParentShellFolderPtr(), spFile->GetAbsolutePidl().GetPreviousIDLPtr() + pIdlNew));
	//}
}