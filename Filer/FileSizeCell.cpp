#include "FileSizeCell.h"
#include "FileSizeColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "FileRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"

CFileSizeCell::CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty)
{}

CFileSizeCell::~CFileSizeCell()
{
	m_conDelayUpdateAction.disconnect();
}

std::shared_ptr<CShellFile> CFileSizeCell::GetShellFile()
{
	if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
		return pFileRow->GetFilePointer();
	} else {
		return nullptr;
	}
}

d2dw::CSizeF CFileSizeCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
//	d2dw::CRectF rcContent;
	std::basic_string<TCHAR> str;
	auto spFile = GetShellFile();
	auto size = spFile->ReadSize();
	switch (size.second) {
	case FileSizeStatus::Available:
		str = Size2String(size.first.QuadPart);
		break;
	default:
		str =  L"00,000,000";
		break;
	}
	if (str.empty()) { str = _T("a"); }
	return direct.CalcTextSize(*(m_spProperty->Format), str);
	//return rcContent.Size();
}


std::wstring CFileSizeCell::GetString()
{
	try {
		auto spFile = GetShellFile();
		std::weak_ptr<CFileSizeCell> wp(shared_from_this());
		auto changed = [wp]()->void {
			if (auto sp = wp.lock()) {
				auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
					[wp]()->void {
					if (auto sp = wp.lock()) {
						sp->OnPropertyChanged(L"value");
					}
				});
				sp->m_conDelayUpdateAction = con;
				sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
			}
		};
		auto size = spFile->GetSize(static_cast<CFileSizeColumn*>(m_pColumn)->GetSizeArgsPtr(), changed);
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

std::wstring CFileSizeCell::GetSortString()
{	
	try {
		auto spFile = GetShellFile();
		std::weak_ptr<CFileSizeCell> wp(shared_from_this());
		auto changed = [wp]()->void {
			if (auto sp = wp.lock()) {
				auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
					[wp]()->void {
					if (auto sp = wp.lock()) {
						sp->OnPropertyChanged(L"value");
					}
				});
				sp->m_conDelayUpdateAction = con;
				sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
			}
		};
		auto size = spFile->GetSize(static_cast<CFileSizeColumn*>(m_pColumn)->GetSizeArgsPtr(), changed);
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
