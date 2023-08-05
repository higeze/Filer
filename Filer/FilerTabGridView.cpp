#include "FilerTabGridView.h"
#include "FilerGridView.h"
#include "FilerGridViewProperty.h"
#include "ToDoGridView.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "FilerWnd.h"
#include "BindRow.h"
#include "KnownFolder.h"
#include "ShellFileFactory.h"
#include "Debug.h"

//#include "BindRow.h"
//#include "BindTextColumn.h"
//#include "BindTextCell.h"
//#include "BindCheckBoxColumn.h"
//#include "BindCheckBoxCell.h"
//#include "BindSheetCellColumn.h"
//#include "BindItemsSheetCell.h"

#include "D2DWWindow.h"
#include "Dispatcher.h"
#include "CellTextBox.h"
#include "Editor.h"
#include "EditorTextBox.h"
#include "EditorProperty.h"
#include "MyFile.h"
#include "D2DFileIconDrawer.h"
#include "ShellFileFactory.h"

#include "PDFEditor.h"
#include "PDFView.h"
#include "PDFEditorProperty.h"

#include "ImageEditor.h"
#include "ImageView.h"
#include "D2DImage.h"
#include "ImageEditorProperty.h"

#include "PreviewControl.h"
#include "PreviewControlProperty.h"

#include "StatusBar.h"

#include "ResourceIDFactory.h"


#include "TextFileDialog.h"
#include "TextEnDecoder.h"

#include "reactive_binding.h"


/****************/
/* FilerTabData */
/****************/

FilerTabData::FilerTabData(const std::wstring& path)
	:TabData(), Path(path)
{
	if (!Path.empty()) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
		if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			FolderPtr = sp;
		} else {
			FolderPtr = CKnownFolderManager::GetInstance()->GetDesktopFolder();
			Path = FolderPtr->GetPath();
		}
	}
}

/**************/
/* PdfTabData */
/**************/
bool PdfTabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!TabData::AcceptClosing(pWnd, isWndClosing)) {
		return false;
	} else {
		if (Doc->get_const().IsDirty->get_const()) {
			int ync = pWnd->MessageBox(
				fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(Doc->get_const().Path->get_const().c_str())).c_str(),
				L"Save?",
				MB_YESNOCANCEL);
			switch (ync) {
				case IDYES:
					Doc->get_unconst().Save();
					return true;
				case IDNO:
					return true;
				case IDCANCEL:
					return false;
				default:
					return true;
			}
		} else {
			return true;
		}
	}
}

/***************/
/* ToDoTabData */
/***************/
bool ToDoTabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!TabData::AcceptClosing(pWnd, isWndClosing)) {
		return false;
	} else {
		if (Doc.Status->get_const() == FileStatus::Dirty) {
			int ync = pWnd->MessageBox(
				fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(Doc.Path->get_const().c_str())).c_str(),
				L"Save?",
				MB_YESNOCANCEL);
			switch (ync) {
				case IDYES:
					Doc.Save(Doc.Path->get_const().c_str());
					return true;
				case IDNO:
					return true;
				case IDCANCEL:
					return false;
				default:
					return true;
			}
		} else {
			return true;
		}
	}
}

template<>
struct adl_vector_item<std::tuple<MainTask>>
{
	static std::tuple<MainTask> clone(const std::tuple<MainTask>& item) 
	{
		return std::make_tuple(std::get<MainTask>(item).Clone());
	}

	static void bind(std::tuple<MainTask>& src, std::tuple<MainTask>& dst)
	{
		reactive_binding(std::get<MainTask>(src).State, std::get<MainTask>(dst).State);
		reactive_binding(std::get<MainTask>(src).Name, std::get<MainTask>(dst).Name);
		reactive_binding(std::get<MainTask>(src).Memo, std::get<MainTask>(dst).Memo);
		reactive_binding(std::get<MainTask>(src).YearMonthDay, std::get<MainTask>(dst).YearMonthDay);
	}
};

/***************/
/* TextTabData */
/***************/

void TextTabData::Open(HWND hWnd)
{
	CTextFileOpenDialog dlg;

	dlg.SetEncodingTypes(
		{
			encoding_type::UNKNOWN,
			encoding_type::UTF16BE,
			encoding_type::UTF16LE,
			encoding_type::UTF16LEN,
			encoding_type::UTF8,
			encoding_type::UTF8N,
			encoding_type::SJIS,
			encoding_type::ASCII,
			encoding_type::JIS,
			encoding_type::EUC
		}
	);

	dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });
	
	dlg.Show(hWnd);

	if (!dlg.GetPath().empty()) {
		Open(dlg.GetPath(), dlg.GetSelectedEncodingType());
	} else {
		return;
	}
}

void TextTabData::OpenAs(HWND hWnd)
{
	if (::PathFileExists(Path->c_str())) {
		CTextFileOpenDialog dlg;

		dlg.SetEncodingTypes(
			{
				encoding_type::UNKNOWN,
				encoding_type::UTF16BE,
				encoding_type::UTF16LE,
				encoding_type::UTF16LEN,
				encoding_type::UTF8,
				encoding_type::UTF8N,
				encoding_type::SJIS,
				encoding_type::ASCII,
				encoding_type::JIS,
				encoding_type::EUC
			}
		);
		dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });

		dlg.SetFolder(::PathFindDirectory(Path->get_const()));
		dlg.SetFileName(::PathFindFileNameW(Path->c_str()));
		dlg.SetSelectedEncodingType(Encoding);

		dlg.Show(hWnd);

		if (!dlg.GetPath().empty()) {
			Open(dlg.GetPath(), dlg.GetSelectedEncodingType());
		} else {
			return;
		}
	} else {
		Open(hWnd);
	}
}


//{
//	std::wstring path;
//	OPENFILENAME ofn = { 0 };
//	ofn.lStructSize = sizeof(OPENFILENAME);
//	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
//	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
//	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
//	ofn.nMaxFile = MAX_PATH;
//	ofn.lpstrTitle = L"Open";
//	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
//	//ofn.lpstrDefExt = L"txt";
//
//	if (!GetOpenFileName(&ofn)) {
//		DWORD errCode = CommDlgExtendedError();
//		if (errCode) {
//			throw std::exception(FILE_LINE_FUNC);
//		}
//	} else {
//		::ReleaseBuffer(path);
//		Open(path);
//	}
//}


void TextTabData::Open(const std::wstring& path, const encoding_type& enc)
{
	if (::PathFileExists(path.c_str())) {
		Path->set(path);

		std::vector<byte> bytes = CFile::ReadAllBytes(path);
		encoding_type enc = CTextEnDecoder::GetInstance()->DetectEncoding(bytes);
		std::wstring wstr = CTextEnDecoder::GetInstance()->Decode(bytes, enc);
		auto iter = std::remove_if(wstr.begin(), wstr.end(), [](const auto& c)->bool { return c == L'\r'; });
		wstr.erase(iter, wstr.end());

		//std::ifstream ifs(path);
		//std::string str = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		//encoding_type enc = CTextEnDecoder::GetInstance()->DetectEncoding(str);
		//std::wstring wstr = CTextEnDecoder::GetInstance()->Decode(str, enc);

		Encoding.set(enc);
		Text->assign(wstr);
		Status.force_notify_set(FileStatus::Saved);
		Carets.set(0, 0, 0, 0, 0);
		CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	} else {
		Path->set(L"");
		Status.force_notify_set(FileStatus::Saved);
		Carets.set(0, 0, 0, 0, 0);
		CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	}
	//if (::PathFileExists(path.c_str())) {
	//	Path.set(path);
	//	Text.assign(str2wstr(CFile::ReadAllString<char>(path)));
	//	Status.force_notify_set(TextStatus::Saved);
	//	Carets.set(0, 0, 0, 0, 0);
	//	CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	//} else {
	//	Path.set(L"");
	//	Status.force_notify_set(TextStatus::Saved);
	//	Carets.set(0, 0, 0, 0, 0);
	//	CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	//}
}

void TextTabData::Save(HWND hWnd)
{
	if (!::PathFileExistsW(Path->c_str())) {
		CTextFileSaveDialog dlg;

		dlg.SetEncodingTypes(
			{
				encoding_type::UNKNOWN,
				encoding_type::UTF16BE,
				encoding_type::UTF16LE,
				encoding_type::UTF16LEN,
				encoding_type::UTF8,
				encoding_type::UTF8N,
				encoding_type::SJIS,
				encoding_type::ASCII,
				encoding_type::JIS,
				encoding_type::EUC
			}
		);

		dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });

		dlg.Show(hWnd);

		if (!dlg.GetPath().empty()) {
			Save(dlg.GetPath(), dlg.GetSelectedEncodingType());
		} else {
			return;
		}
	} else {
		Save(Path->get_const(), Encoding.get());
	}
}

void TextTabData::SaveAs(HWND hWnd)
{
	if (::PathFileExistsW(Path->c_str())) {
		CTextFileSaveDialog dlg;

		dlg.SetEncodingTypes(
			{
				encoding_type::UNKNOWN,
				encoding_type::UTF16BE,
				encoding_type::UTF16LE,
				encoding_type::UTF16LEN,
				encoding_type::UTF8,
				encoding_type::UTF8N,
				encoding_type::SJIS,
				encoding_type::ASCII,
				encoding_type::JIS,
				encoding_type::EUC
			}
		);
		dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });

		dlg.SetFolder(::PathFindDirectory(Path->get_const()));
		dlg.SetFileName(::PathFindFileNameW(Path->c_str()));
		dlg.SetSelectedEncodingType(Encoding);

		dlg.Show(hWnd);
		
		if (!dlg.GetPath().empty()) {
			Save(dlg.GetPath(), dlg.GetSelectedEncodingType());
		} else {
			return;
		}
	} else {
		Save(hWnd);
	}
}

	//if (!::PathFileExistsW(Path.c_str())) {
	//	std::wstring path;
	//	OPENFILENAME ofn = { 0 };
	//	ofn.lStructSize = sizeof(OPENFILENAME);
	//	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//	ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	//	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	//	ofn.nMaxFile = MAX_PATH;
	//	ofn.lpstrTitle = L"Save as";
	//	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	//	ofn.lpstrDefExt = L"txt";

	//	if (!GetSaveFileName(&ofn)) {
	//		DWORD errCode = CommDlgExtendedError();
	//		if (errCode) {
	//			//throw std::exception(FILE_LINE_FUNC);
	//		}
	//	} else {
	//		::ReleaseBuffer(path);
	//	}
	//	Save(path);
	//} else {
	//	Save(Path);
	//}
//}

void TextTabData::Save(const std::wstring& path, const encoding_type& enc)
{
	Path->set(path);
	Encoding.set(enc);
	Status.force_notify_set(FileStatus::Saved);
	std::ofstream ofs(path);
	std::vector<byte> bytes = CTextEnDecoder::GetInstance()->Encode(Text->get_const(), enc);
	CFile::WriteAllBytes(Path->get_const(), bytes);
}

bool TextTabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!TabData::AcceptClosing(pWnd, isWndClosing)) {
		return false;
	} else {
		if (Status.get() == FileStatus::Dirty) {
			int ync = pWnd->MessageBox(
				fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(Path->c_str())).c_str(),
				L"Save?",
				MB_YESNOCANCEL);
			switch (ync) {
				case IDYES:
					Save(pWnd->m_hWnd);
					return true;
				case IDNO:
					return true;
				case IDCANCEL:
					return false;
				default:
					return true;
			}
		} else {
			return true;
		}
	}
}

/********************/
/* FilerTabGridView */
/********************/

CFilerTabGridView::CFilerTabGridView(CD2DWControl* pParentControl, 
	const std::shared_ptr<TabControlProperty>& spTabProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp, 
	const std::shared_ptr<EditorProperty>& spEditorProp,
	const std::shared_ptr<PDFEditorProperty>& spPdfEditorProp,
	const std::shared_ptr<ImageEditorProperty>& spImageEditorProp,
	const std::shared_ptr<PreviewControlProperty>& spPreviewControlProp)
	:CTabControl(pParentControl, spTabProp), 
	m_spFilerGridViewProp(spFilerGridViewProp),
	m_spEditorProp(spEditorProp),
	m_spPdfEditorProp(spPdfEditorProp),
	m_spImageEditorProp(spImageEditorProp),
	m_spPreviewControlProp(spPreviewControlProp),
	m_spFilerGridView(std::make_shared<CFilerGridView>(this, spFilerGridViewProp)),
	m_spTextView(std::make_shared<CEditor>(this, spEditorProp)),
	m_spPdfView(std::make_shared<CPDFEditor>(this, spPdfEditorProp)),
	m_spImageView(std::make_shared<CImageEditor>(this, spImageEditorProp)),
	m_spToDoGridView(std::make_shared<CToDoGridView>(this, std::static_pointer_cast<GridViewProperty>(spFilerGridViewProp))),
	m_spPreviewControl(std::make_shared<CPreviewControl>(this, spPreviewControlProp))

{
	//Command
	m_commandMap.emplace(IDM_NEWFILERTAB, std::bind(&CFilerTabGridView::OnCommandNewFilerTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWTODOTAB, std::bind(&CFilerTabGridView::OnCommandNewToDoTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWTEXTTAB, std::bind(&CFilerTabGridView::OnCommandNewTextTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWPDFTAB, std::bind(&CFilerTabGridView::OnCommandNewPdfTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWIMAGETAB, std::bind(&CFilerTabGridView::OnCommandNewImageTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWPREVIEWTAB, std::bind(&CFilerTabGridView::OnCommandNewPreviewTab, this, phs::_1));

	m_commandMap.emplace(IDM_LOCKTAB, std::bind(&CFilerTabGridView::OnCommandLockTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLONETAB, std::bind(&CFilerTabGridView::OnCommandCloneTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLOSETAB, std::bind(&CFilerTabGridView::OnCommandCloseTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLOSEALLBUTTHISTAB, std::bind(&CFilerTabGridView::OnCommandCloseAllButThisTab, this, phs::_1));
	m_commandMap.emplace(IDM_ADDTOFAVORITE, std::bind(&CFilerTabGridView::OnCommandAddToFavorite, this, phs::_1));
	m_commandMap.emplace(IDM_OPENSAMEASOTHER, std::bind(&CFilerTabGridView::OnCommandOpenSameAsOther, this, phs::_1));


	//FilerGridView
	m_spFilerGridView->FolderChanged = [&](std::shared_ptr<CShellFolder>& pFolder)
	{
		auto pData = std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex.get()]);
		pData->FolderPtr = pFolder;
		pData->Path = pFolder->GetPath();
		UpdateHeaderRects();
		m_spFilerGridView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
	};

	//ToDoGridView Closure
	////Columns
	//m_spToDoGridView->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(m_spToDoGridView.get()));
	//m_spToDoGridView->PushColumns(
	//	m_spToDoGridView->GetHeaderColumnPtr(),
	//	std::make_shared<CBindCheckBoxColumn<MainTask>>(
	//		m_spToDoGridView.get(),
	//		L"State",
	//		[](const std::tuple<MainTask>& tk)->CheckBoxState {return std::get<MainTask>(tk).State; },
	//		[](std::tuple<MainTask>& tk, const CheckBoxState& state)->void {std::get<MainTask>(tk).State = state; }),
	//	std::make_shared<CBindTextColumn<MainTask>>(
	//		m_spToDoGridView.get(),
	//		L"Name",
	//		[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Name; },
	//		[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Name = str; }),
	//	std::make_shared<CBindTextColumn<MainTask>>(
	//		m_spToDoGridView.get(),
	//		L"Memo",
	//		[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Memo; },
	//		[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Memo = str; }),
	//	std::make_shared<CDateColumn>(
	//		m_spToDoGridView.get(),
	//		L"Due date")//,
	//	//std::make_shared<CBindSheetCellColumn< MainTask, SubTask>>(
	//	//	m_spToDoGridView.get(),
	//	//	L"Sub Task",
	//	//	[](std::tuple<MainTask>& tk)->ReactiveVectorProperty<std::tuple<SubTask>>& {return std::get<MainTask>(tk).SubTasks; },
	//	//	[](CBindItemsSheetCell<MainTask, SubTask>* pCell)->void {
	//	//		pCell->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(pCell));
	//	//		pCell->PushColumns(
	//	//			pCell->GetHeaderColumnPtr(),
	//	//			std::make_shared<CBindCheckBoxColumn<SubTask>>(
	//	//				pCell,
	//	//				L"Done",
	//	//				[](const std::tuple<SubTask>& tk)->CheckBoxState {return std::get<SubTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
	//	//				[](std::tuple<SubTask>& tk, const CheckBoxState& state)->void {std::get<SubTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
	//	//			std::make_shared<CBindTextColumn<SubTask>>(
	//	//				pCell,
	//	//				L"Name",
	//	//				[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Name; },
	//	//				[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Name = str; }),
	//	//			std::make_shared<CBindTextColumn<SubTask>>(
	//	//				pCell,
	//	//				L"Memo",
	//	//				[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Memo; },
	//	//				[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Memo = str; })
	//	//		);
	//	//		pCell->SetFrozenCount<ColTag>(1);

	//	//		pCell->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(pCell));
	//	//		pCell->InsertRow(0, pCell->GetNameHeaderRowPtr());
	//	//		pCell->SetFrozenCount<RowTag>(1);
	//	//	},
	//	//	arg<"maxwidth"_s>() = FLT_MAX)
	//);
	//m_spToDoGridView->SetFrozenCount<ColTag>(1);

	////Rows
	//m_spToDoGridView->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(m_spToDoGridView.get()));
	//m_spToDoGridView->SetFilterRowPtr(std::make_shared<CRow>(m_spToDoGridView.get()));

	//m_spToDoGridView->PushRows(
	//	m_spToDoGridView->GetNameHeaderRowPtr(),
	//	m_spToDoGridView->GetFilterRowPtr());

	//m_spToDoGridView->SetFrozenCount<RowTag>(2);

	//Path Changed
	//m_spToDoGridView->Doc.get().Path.Subscribe([&](const auto& notify) {
	//	auto pData = std::static_pointer_cast<ToDoTabData>(m_itemsSource[m_selectedIndex.get()]);
	//	pData->Doc.get().Path = m_spToDoGridView->Doc.get().Path;
	//	m_spToDoGridView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
	//});
}

CFilerTabGridView::~CFilerTabGridView() = default;

void CFilerTabGridView::OnCreate(const CreateEvt& e)
{
	/*****************/
	/* ItemsTemplate */
	/*****************/
	//ItemsHeader
	m_itemsHeaderTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{

			if (auto p = std::dynamic_pointer_cast<FilerTabData>(pTabData)) {
				return p->FolderPtr->GetFileNameWithoutExt().c_str();
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				if (p->Doc.Path->get_const().empty()) {
					return L"No file";
				} else {
					return std::wstring(p->Doc.Status->get_const() == FileStatus::Dirty ? L"*" : L"") + ::PathFindFileName(p->Doc.Path->get_const().c_str());
				}
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				return std::wstring(p->Status.get() == FileStatus::Dirty?L"*":L"") + ::PathFindFileName(p->Path->c_str());
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				if (p->Doc->get_const().Path->get_const().empty()) {
					return L"No file";
				} else {
					return std::wstring(p->Doc->get_const().IsDirty->get_const()? L"*" : L"") + ::PathFindFileName(p->Doc->get_const().Path->get_const().c_str());
				}
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(ImageTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<ImageTabData>(pTabData)) {
				if (p->Image.get().GetPath().empty()) {
					return L"No file";
				} else {
					return std::wstring(::PathFindFileName(p->Image.get().GetPath().c_str()));
				}
			} else {
				return L"nullptr";
			}
		});

	m_itemsHeaderTemplate.emplace(typeid(PreviewTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<PreviewTabData>(pTabData)) {
				if (p->Path.empty()) {
					return L"No file";
				} else {
					return std::wstring(::PathFindFileName(p->Path.c_str()));
				}
			} else {
				return L"nullptr";
			}
		});

	//ItemsHeaderIcon
	auto updated = [this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]()->void
		{
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		});
	};

	m_itemsHeaderIconTemplate.emplace(typeid(FilerTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{

			if (auto p = std::dynamic_pointer_cast<FilerTabData>(pTabData)) {
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->FolderPtr->GetAbsoluteIdl(), p->FolderPtr->GetPath(), p->FolderPtr->GetDispExt(), p->FolderPtr->GetAttributes(), updated);
			} else {
				/*return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();*/
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(ToDoTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Doc.Path->get_const());
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(TextTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Path->get_const());
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(PdfTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Doc->get_const().Path->get_const());
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	m_itemsHeaderIconTemplate.emplace(typeid(ImageTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<ImageTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Image.get().GetPath());
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	m_itemsHeaderIconTemplate.emplace(typeid(PreviewTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<PreviewTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Path.get());
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	//ItemsTemplate
	m_itemsControlTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto pData = std::static_pointer_cast<FilerTabData>(pTabData);
		auto &spView = GetFilerGridViewPtr();
		spView->OpenFolder(pData->FolderPtr);
		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	//rxcpp::composite_subscription subs;
	//rxcpp::subjects::behavior<int> test(19);
	//subs.add(test.get_observable().subscribe([](auto) {OutputDebugString(L"A"); }));
	//test.get_observable().subscribe([](auto) {OutputDebugString(L"B"); });
	//rxcpp::subjects::behavior<int> test2 = test;
	//test.get_subscriber().on_next(10);
	//subs.add(test.get_observable().subscribe([](auto) {OutputDebugString(L"C"); }));
	//subs.add(test.get_observable().subscribe([](auto) {OutputDebugString(L"D"); }));
	//test2.get_subscriber().on_next(3);
	//test.get_subscriber().on_next(5);
	//subs.clear();
	//test.get_subscriber().on_next(99);
	//test2.get_subscriber().on_next(98);



	//ReactiveProperty<int> test(19);
	//test.Subscribe([](auto) {OutputDebugString(L"A"); });
	//test.Subscribe([](auto) {OutputDebugString(L"B"); });

	//ReactiveProperty<int> test2;
	//test2 = test;

	//test.set(10);

	//test.Subscribe([](auto) {OutputDebugString(L"C"); });
	//test.Subscribe([](auto) {OutputDebugString(L"D"); });
	//test2.set(3);

	//test.set(5);

	m_itemsControlTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();

		reactive_command_binding(spViewModel->OpenCommand, spView->OpenCommand);
		reactive_command_binding(spViewModel->SaveCommand, spView->SaveCommand);
		reactive_binding(spViewModel->Doc.Path, spView->Path);
		
		m_todoTasksConnections.first.disconnect();
		m_todoTasksConnections.second.disconnect();
		m_todoTasksConnections = reactive_vector_binding(spViewModel->Doc.Tasks, spView->ItemsSource);

		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();

		//Path
		reactive_string_binding(spViewModel->Path, spView->Path);
		m_pTextPathConnection = std::make_unique<sigslot::scoped_connection>(spView->Path->subscribe([this](const auto&) { UpdateHeaderRects(); }));
		//Status
		m_pStatusConnection = std::make_unique<sigslot::scoped_connection>(spViewModel->Status.Subscribe([this](const auto&) { UpdateHeaderRects(); }));
		//Text
		reactive_string_binding(spViewModel->Text, spView->GetTextBoxPtr()->Text);
		//Encoding
		m_textEncodingBinding.Attach(spViewModel->Encoding, spView->GetEncoding());		
		//Carets
		m_caretsBinding.Attach(spViewModel->Carets, spView->GetTextBoxPtr()->GetCarets());
		//CaretPos
		m_caretPosBinding.Attach(spViewModel->CaretPos, spView->GetTextBoxPtr()->GetCaretPos());
		//Open
		m_openBinding.Attach(spViewModel->OpenCommand, spView->GetOpenCommand());
		m_openAsBinding.Attach(spViewModel->OpenAsCommand, spView->GetOpenAsCommand());
		//Save()
		m_saveBinding.Attach(spViewModel->SaveCommand, spView->GetSaveCommand());
		m_saveAsBinding.Attach(spViewModel->SaveAsCommand, spView->GetSaveAsCommand());

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		if (spViewModel->Status.get() == FileStatus::None) {
			spViewModel->Open(spViewModel->Path->get_const(), spViewModel->Encoding);
		}

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PdfTabData>(pTabData);
		auto spView = GetPdfViewPtr();

		//Scale
		reactive_binding(spViewModel->Scale, spView->GetPDFViewPtr()->Scale);
		//Scroll
		reactive_binding(spViewModel->VScroll, spView->GetPDFViewPtr()->GetVScrollPtr()->Position);
		reactive_binding(spViewModel->HScroll, spView->GetPDFViewPtr()->GetHScrollPtr()->Position);

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		spView->GetPDFViewPtr()->Reset(spViewModel->Doc);

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(ImageTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<ImageTabData>(pTabData);
		auto spView = GetImageViewPtr();

		//Path
		m_imageBinding.Attach(spViewModel->Image, spView->GetImageViewPtr()->PropImage());
		m_pImageConnection = std::make_unique<sigslot::scoped_connection>(spView->GetImageViewPtr()->PropImage().Subscribe([this](const auto&) { UpdateHeaderRects(); }));
		//Scale
		reactive_binding(spViewModel->Scale, spView->GetImageViewPtr()->Scale);
		//Scroll
		reactive_binding(spViewModel->VScroll, spView->GetImageViewPtr()->GetVScrollPtr()->Position);
		reactive_binding(spViewModel->HScroll, spView->GetImageViewPtr()->GetHScrollPtr()->Position);

		spView->GetImageViewPtr()->Open(spViewModel->Image.get().GetPath());
		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PreviewTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PreviewTabData>(pTabData);
		auto spView = GetPreviewControlPtr();

		//Path
		m_previewPathBinding.Attach(spViewModel->Path, spView->Path);
		m_pPreviewPathConnection = std::make_unique<sigslot::scoped_connection>(spView->Path.Subscribe([this](const auto&) { UpdateHeaderRects(); }));

		spView->Open(spViewModel->Path.get());
		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));

		return spView;
	});

	/**********/
	/* Create */
	/**********/
	CTabControl::OnCreate(e);
	GetFilerGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetToDoGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetTextViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetPdfViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetImageViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetPreviewControlPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));

	GetFilerGridViewPtr()->GetIsEnabled().set(false);
	GetToDoGridViewPtr()->GetIsEnabled().set(false);
	GetTextViewPtr()->GetIsEnabled().set(false);
	GetPdfViewPtr()->GetIsEnabled().set(false);
	GetImageViewPtr()->GetIsEnabled().set(false);
	GetPreviewControlPtr()->GetIsEnabled().set(false);

	//ItemsSource
	if (m_itemsSource.empty()) {
		m_itemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
		m_itemsSource.push_back(std::make_shared<ToDoTabData>(L""));
		m_itemsSource.push_back(std::make_shared<TextTabData>(L""));
		m_selectedIndex.set(0);
	} else {
		if (m_selectedIndex.get() < 0) {
			m_selectedIndex.force_notify_set(0); 
		} else {
			m_selectedIndex.force_notify_set(m_selectedIndex.get());
		}
	}
}

void CFilerTabGridView::OnContextMenu(const ContextMenuEvent& e)
{
	CTabControl::OnContextMenu(e);

	if (m_contextIndex) {
		CMenu menu;
		menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));
		GetWndPtr()->SetForegroundWindow();
		menu.TrackPopupMenu(0, e.PointInScreen.x, e.PointInScreen.y, GetWndPtr()->m_hWnd);
	} else if (m_spCurControl->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))){
		m_spCurControl->OnContextMenu(e);
	}
}

void CFilerTabGridView::OnCommandNewFilerTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewToDoTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<ToDoTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewTextTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<TextTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewPdfTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<PdfTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewImageTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<ImageTabData>(GetWndPtr()->GetDirectPtr(), L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}

void CFilerTabGridView::OnCommandNewPreviewTab(const CommandEvent& e)
{
	m_itemsSource.push_back(std::make_shared<PreviewTabData>(L""));
	m_selectedIndex.set(m_itemsSource.size() - 1);
}


void CFilerTabGridView::OnCommandAddToFavorite(const CommandEvent& e)
{
	//TODOLOW Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())){
		p->GetFavoritesPropPtr()->GetFavorites().push_back(std::make_shared<CFavorite>(std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex])->Path, L""));
		p->GetLeftFavoritesView()->SubmitUpdate();
		p->GetRightFavoritesView()->SubmitUpdate();
	}
}

void CFilerTabGridView::OnCommandOpenSameAsOther(const CommandEvent& e)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if (auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())) {
			
		std::shared_ptr<CFilerTabGridView> otherView = (this == p->GetLeftWnd().get())? p->GetRightWnd(): p->GetLeftWnd();
		
		if (m_itemsSource[m_selectedIndex.get()]->AcceptClosing(GetWndPtr(), false)) {
			m_itemsSource.replace(m_itemsSource.begin() + m_selectedIndex.get(), otherView->GetItemsSource()[otherView->GetSelectedIndex()]);
		} else {
			m_itemsSource.push_back(otherView->GetItemsSource()[otherView->GetSelectedIndex()]);
		}
	}
}




