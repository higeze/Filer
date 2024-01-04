#include "FileSizeCell.h"

CSizeF CFileSizeCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
//	CRectF rcContent;
	std::basic_string<TCHAR> str;
	auto spFile = GetShellFile();
	auto size = spFile->ReadSize();
	switch (size.second) {
		case FileSizeStatus::Available:
			str = shell::Size2String(size.first.QuadPart);
			break;
		default:
			str = L"00,000,000";
			break;
	}
	if (str.empty()) { str = _T("a"); }
	return pDirect->CalcTextSize(*(m_spCellProperty->Format), str);
	//return rcContent.Size();
}

std::wstring CFileSizeCell::GetString()
{
	try {
		auto spFile = GetShellFile();
		std::weak_ptr<CFileSizeCell> wp(std::dynamic_pointer_cast<CFileSizeCell>(shared_from_this()));
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
		auto size = spFile->GetSize(static_cast<const CFileSizeColumn*>(m_pColumn)->GetSizeArgsPtr(), changed);
		switch (size.second) {
			case FileSizeStatus::None:
				return L"none";
			case FileSizeStatus::Available:
				return shell::Size2String(size.first.QuadPart);
			case FileSizeStatus::Calculating:
				return L"...";
			case FileSizeStatus::Unavailable:
				return L"-";
			default:
				return L"";
		}
	}
	catch (...) {
		return L"err";
	}
}

std::wstring CFileSizeCell::GetSortString()
{
	try {
		auto spFile = GetShellFile();
		auto changed = [wp = std::weak_ptr(std::dynamic_pointer_cast<CFileSizeCell>(shared_from_this()))]()->void {
			if (auto sp = wp.lock()) {
				sp->m_conDelayUpdateAction = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
					[wp]()->void {
						if (auto sp = wp.lock()) {
							sp->OnPropertyChanged(L"value");
						}
					});
				sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
			}
		};
		auto size = spFile->GetSize(static_cast<const CFileSizeColumn*>(m_pColumn)->GetSizeArgsPtr(), changed);
		switch (size.second) {
			case FileSizeStatus::None:
				return L"none";
			case FileSizeStatus::Available:
				return fmt::format(L"{:12d}", size.first.QuadPart);
			case FileSizeStatus::Calculating:
				return L"...";
			case FileSizeStatus::Unavailable:
				return L"-";
			default:
				return L"";
		}
	}
	catch (...) {
		return L"err";
	}
}


std::shared_ptr<CShellFile> CFileSizeCell::GetShellFile()
{
	if (auto pBindRow = dynamic_cast<CBindRow*>(m_pRow)) {
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>();
	} else {
		return nullptr;
	}
}
