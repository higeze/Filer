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
		m_pSheet->PostUpdate(Updates::Column);
		m_pSheet->CellValueChanged(CellEventArgs(this)); }) 
{
	auto spFile = GetShellFile();
	spFile->SignalFileSizeChanged.connect(
		[this](CShellFile* pFile)->void {
			m_pSheet->GetGridPtr()->PushDelayUpdateAction(m_delayUpdateAction);
			m_pSheet->GetGridPtr()->DelayUpdate();
	});
}

CFileSizeCell::~CFileSizeCell()
{
	m_pSheet->GetGridPtr()->EraseDelayUpdateAction(m_delayUpdateAction);
}

bool CFileSizeCell::operator<(const CCell& rhs)const
{
	auto pLhsRow = static_cast<CFileRow*>(m_pRow);
	auto pRhsRow = static_cast<CFileRow*>(rhs.GetRowPtr());
	return pLhsRow->GetFilePointer()->GetSize().first.QuadPart < pRhsRow->GetFilePointer()->GetSize().first.QuadPart;
}

bool CFileSizeCell::operator>(const CCell& rhs)const
{
	auto pLhsRow = static_cast<CFileRow*>(m_pRow);
	auto pRhsRow = static_cast<CFileRow*>(rhs.GetRowPtr());
	return pLhsRow->GetFilePointer()->GetSize().first.QuadPart > pRhsRow->GetFilePointer()->GetSize().first.QuadPart;
}

CCell::string_type CFileSizeCell::GetString()const
{
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
}

CCell::string_type CFileSizeCell::GetSortString()const
{		
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
}

std::shared_ptr<CShellFile> CFileSizeCell::GetShellFile()const
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer();
}
