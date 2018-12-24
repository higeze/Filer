#include "FileSizeCell.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "FileRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include "FileSizeColumn.h"

CFileSizeCell::CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty)
{}

CFileSizeCell::~CFileSizeCell()
{
	m_conDelayUpdateAction.disconnect();
	m_conFileSizeChanged.disconnect();
}

std::shared_ptr<CShellFile> CFileSizeCell::GetShellFile()
{
	if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
		if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(pFileRow->GetFilePointer())) {
			if (!m_conFileSizeChanged.connected()) {
				std::weak_ptr<CFileSizeCell> wp(shared_from_this());
				m_conFileSizeChanged = spFolder->SignalFileSizeChanged.connect(
					[wp](CShellFile* pFile)->void {
					if (auto sp = wp.lock()) {
						auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
							[wp]()->void {
							if (auto sp = wp.lock()) {
								sp->GetSheetPtr()->CellValueChanged(CellEventArgs(sp.get()));
							}
						});
						sp->m_conDelayUpdateAction = con;
						sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
					}
				});
			}
			return spFolder;
		} else {
			return pFileRow->GetFilePointer();
		}
	} else {
		return nullptr;
	}
}

CSize CFileSizeCell::MeasureContentSize(CDC* pDC)
{
	//Calc Content Rect
	HFONT hFont = (HFONT)pDC->SelectFont(*m_spProperty->GetFontPtr());
	CRect rcContent;
	std::basic_string<TCHAR> str;
	auto spFile = GetShellFile();
	auto size = spFile->ReadSize();
	switch (size.second) {
	case FileSizeStatus::Available:
		str = Size2String(size.first.QuadPart);
	default:
		str =  L"00,000,000";
	}
	if (str.empty()) { str = _T("a"); }
	pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()), str.size(), rcContent,
		DT_CALCRECT | GetFormat()&~DT_WORDBREAK, NULL);
	pDC->SelectFont(hFont);
	return rcContent.Size();
}


CCell::string_type CFileSizeCell::GetString()
{
	try {
		auto spFile = GetShellFile();
		auto size = spFile->GetSize(static_cast<CFileSizeColumn*>(m_pColumn)->GetSizeArgsPtr());
		switch (size.second) {
		case FileSizeStatus::None:
			return L"none";
		case FileSizeStatus::Available:
			return Size2String(size.first.QuadPart);
		case FileSizeStatus::Calculating:
			return L"...";
		case FileSizeStatus::Unavailable:
			return L"-";
		default:
			return L"";
		}
	} catch (...) {
		return L"err";
	}
}

CCell::string_type CFileSizeCell::GetSortString()
{	
	try {
		auto spFile = GetShellFile();
		auto size = spFile->GetSize(static_cast<CFileSizeColumn*>(m_pColumn)->GetSizeArgsPtr());
		switch (size.second) {
		case FileSizeStatus::None:
			return L"none";
		case FileSizeStatus::Available:
			return (boost::wformat(L"%12d") % size.first.QuadPart).str();
		case FileSizeStatus::Calculating:
			return L"...";
		case FileSizeStatus::Unavailable:
			return L"-";
		default:
			return L"";
		}
	} catch (...) {
		return L"err";
	}
}
