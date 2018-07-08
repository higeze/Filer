#include "FileSizeCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"

CFileSizeCell::CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty)
{
	//Be careful : It is impossible to plymorphism in constructor, assign signal here.
	//if (auto pFileRow = dynamic_cast<CFileRow*>(pRow)) {
	//	auto spFile = pFileRow->GetFilePointer();
	//	std::weak_ptr<CFileSizeCell> wp(shared_from_this());
	//	m_conFileSizeChanged = spFile->SignalFileSizeChanged.connect(
	//		[wp](std::weak_ptr<CShellFile> wpFile)->void {
	//		if (auto sp = wp.lock()) {
	//			auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
	//				[wp]()->void {
	//					if (auto sp = wp.lock()) {
	//						sp->SetFitMeasureValid(false);
	//						sp->SetActMeasureValid(false);
	//						sp->GetSheetPtr()->PostUpdate(Updates::Sort);
	//						sp->GetSheetPtr()->PostUpdate(Updates::Column);
	//						sp->GetSheetPtr()->CellValueChanged(CellEventArgs(sp.get()));
	//					}
	//				});
	//			sp->SetDelayUpdateConnection(con);
	//			sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
	//		}
	//	});
	//}
}

CFileSizeCell::~CFileSizeCell()
{
	m_conDelayUpdateAction.disconnect();
	m_conFileSizeChanged.disconnect();
}

std::shared_ptr<CShellFile> CFileSizeCell::GetShellFile()
{
	if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
		auto spFile = pFileRow->GetFilePointer();
		if (!m_conFileSizeChanged.connected()) {
			std::weak_ptr<CFileSizeCell> wp(shared_from_this());
			m_conFileSizeChanged = spFile->SignalFileSizeChanged.connect(
				[wp](std::weak_ptr<CShellFile> wpFile)->void {
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
		return spFile;
	} else {
		return nullptr;
	}
}

CCell::string_type CFileSizeCell::GetString()
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

CCell::string_type CFileSizeCell::GetSortString()
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
