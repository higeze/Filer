#include "FileSizeCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"

CFileSizeCell::CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty),
	m_delayUpdateAction([this]()->void {
		m_bFitMeasureValid = false;
		m_bActMeasureValid = false;
		m_pSheet->PostUpdate(Updates::Sort);
		m_pSheet->PostUpdate(Updates::Column);
		m_pSheet->CellValueChanged(CellEventArgs(this));
}) {}

std::shared_ptr<CShellFile> CFileSizeCell::GetShellFile()const
{
	if(auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)){
		//It is impossible to plymorphism in constructor, assign signal here.
		auto spFile = pFileRow->GetFilePointer();
		if (spFile->SignalFileSizeChanged.empty()) {
			auto spCell = shared_from_this();
			std::weak_ptr<const CFileSizeCell> wpCell(spCell);
			spFile->SignalFileSizeChanged.connect(
				[wpCell](std::weak_ptr<CShellFile> wpFile)->void {
				if (auto sp = wpCell.lock()) {
					sp->GetSheetPtr()->GetGridPtr()->PushDelayUpdateAction(sp->GetDelayUpdateAction());
					sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
				}
			});
		}
		return spFile;
	} else {
		return nullptr;
	}
}


CFileSizeCell::~CFileSizeCell()
{
	try {
		auto pGrid = m_pSheet->GetGridPtr();
		pGrid->EraseDelayUpdateAction(m_delayUpdateAction);
	} catch (...) {

	}
}

//bool CFileSizeCell::operator<(const CFileSizeCell& rhs)const
//{
//	auto pLhsRow = static_cast<CFileRow*>(m_pRow);
//	auto pRhsRow = static_cast<CFileRow*>(rhs.GetRowPtr());
//	return pLhsRow->GetFilePointer()->GetSize().first.QuadPart < pRhsRow->GetFilePointer()->GetSize().first.QuadPart;
//}
//
//bool CFileSizeCell::operator>(const CFileSizeCell& rhs)const
//{
//	auto pLhsRow = static_cast<CFileRow*>(m_pRow);
//	auto pRhsRow = static_cast<CFileRow*>(rhs.GetRowPtr());
//	return pLhsRow->GetFilePointer()->GetSize().first.QuadPart > pRhsRow->GetFilePointer()->GetSize().first.QuadPart;
//}

CCell::string_type CFileSizeCell::GetString()const
{
	try {
		auto spFile = GetShellFile();
		auto size = spFile->GetSize();
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

CCell::string_type CFileSizeCell::GetSortString()const
{	
	try {
		auto spFile = GetShellFile();
		auto size = spFile->GetSize();
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
