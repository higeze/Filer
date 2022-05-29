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

#include "BindRow.h"
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "BindCheckBoxColumn.h"
#include "BindCheckBoxCell.h"
#include "BindSheetCellColumn.h"
#include "BindItemsSheetCell.h"

#include "D2DWWindow.h"
#include "Dispatcher.h"
#include "CellTextBox.h"
#include "Editor.h"
#include "EditorTextBox.h"
#include "EditorProperty.h"
#include "MyFile.h"
#include "FileIconCache.h"
#include "ShellFileFactory.h"
#include "PDFEditor.h"
#include "PDFView.h"
#include "PDFEditorProperty.h"
#include "StatusBar.h"

#include "ResourceIDFactory.h"


#include "TextFileDialog.h"
#include "TextEnDecoder.h"


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
	if (::PathFileExists(Path.c_str())) {
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

		dlg.SetFolder(::PathFindDirectory(Path));
		dlg.SetFileName(::PathFindFileNameW(Path.c_str()));
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
		Path.set(path);

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
		Text.assign(wstr);
		Status.force_notify_set(TextStatus::Saved);
		Carets.set(0, 0, 0, 0, 0);
		CaretPos.set(CPointF(0, 10 * 0.5f));//TODOLOW
	} else {
		Path.set(L"");
		Status.force_notify_set(TextStatus::Saved);
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
	if (!::PathFileExistsW(Path.c_str())) {
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
		Save(Path.get(), Encoding.get());
	}
}

void TextTabData::SaveAs(HWND hWnd)
{
	if (::PathFileExistsW(Path.c_str())) {
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

		dlg.SetFolder(::PathFindDirectory(Path));
		dlg.SetFileName(::PathFindFileNameW(Path.c_str()));
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
	Path.set(path);
	Encoding.set(enc);
	Status.force_notify_set(TextStatus::Saved);
	std::ofstream ofs(path);
	std::vector<byte> bytes = CTextEnDecoder::GetInstance()->Encode(Text.get(), enc);
	CFile::WriteAllBytes(Path.get(), bytes);
}

bool TextTabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!TabData::AcceptClosing(pWnd, isWndClosing)) {
		return false;
	} else {
		if (Status.get() == TextStatus::Dirty) {
			int ync = pWnd->MessageBox(
				fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(Path.c_str())).c_str(),
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
	const std::shared_ptr<PDFEditorProperty>& spPdfViewProp)
	:CTabControl(pParentControl, spTabProp), 
	m_spFilerGridViewProp(spFilerGridViewProp),
	m_spEditorProp(spEditorProp),
	m_spPdfEditorProp(spPdfViewProp)
{
	//Command
	m_commandMap.emplace(IDM_NEWFILERTAB, std::bind(&CFilerTabGridView::OnCommandNewFilerTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWTODOTAB, std::bind(&CFilerTabGridView::OnCommandNewToDoTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWTEXTTAB, std::bind(&CFilerTabGridView::OnCommandNewTextTab, this, phs::_1));
	m_commandMap.emplace(IDM_NEWPDFTAB, std::bind(&CFilerTabGridView::OnCommandNewPdfTab, this, phs::_1));

	m_commandMap.emplace(IDM_LOCKTAB, std::bind(&CFilerTabGridView::OnCommandLockTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLONETAB, std::bind(&CFilerTabGridView::OnCommandCloneTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLOSETAB, std::bind(&CFilerTabGridView::OnCommandCloseTab, this, phs::_1));
	m_commandMap.emplace(IDM_CLOSEALLBUTTHISTAB, std::bind(&CFilerTabGridView::OnCommandCloseAllButThisTab, this, phs::_1));
	m_commandMap.emplace(IDM_ADDTOFAVORITE, std::bind(&CFilerTabGridView::OnCommandAddToFavorite, this, phs::_1));
	m_commandMap.emplace(IDM_OPENSAMEASOTHER, std::bind(&CFilerTabGridView::OnCommandOpenSameAsOther, this, phs::_1));


	//FilerGridView Closure
	FilerGridViewPtr(
		[&val = FilerGridViewPtr.value, this]()->std::shared_ptr<CFilerGridView>{
			if (!val) {
				val = std::make_shared<CFilerGridView>(this, m_spFilerGridViewProp);
				//Folder Changed
				val->FolderChanged = [&](std::shared_ptr<CShellFolder>& pFolder) {
					auto pData = std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex.get()]);
					pData->FolderPtr = pFolder;
					pData->Path = pFolder->GetPath();
					UpdateHeaderRects();
					val->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
				};
			}
			return val;
		},

		[&val = FilerGridViewPtr.value, this](const std::shared_ptr<CFilerGridView>& value)->void{
			if (val != value) {
				val = value;
				val->FolderChanged = [&](std::shared_ptr<CShellFolder>& pFolder) {
					auto pData = std::static_pointer_cast<FilerTabData>(m_itemsSource[m_selectedIndex.get()]);
					pData->FolderPtr = pFolder;
					pData->Path = pFolder->GetPath();
					UpdateHeaderRects();
					val->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
				};
			}
		}
		);

	//ToDoGridView Closure
	GetToDoGridViewPtr = [spToDoView = std::make_shared<CToDoGridView>(this, std::static_pointer_cast<GridViewProperty>(m_spFilerGridViewProp)), isInitialized = false, this]()mutable->std::shared_ptr<CToDoGridView>& {

		if (!isInitialized && GetWndPtr()->IsWindow()) {
			isInitialized = true;

			//Columns
			spToDoView->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(spToDoView.get()));
			spToDoView->PushColumns(
				spToDoView->GetHeaderColumnPtr(),
				std::make_shared<CBindCheckBoxColumn<MainTask>>(
					spToDoView.get(),
					L"Done",
					[](const std::tuple<MainTask>& tk)->CheckBoxState {return std::get<MainTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
					[](std::tuple<MainTask>& tk, const CheckBoxState& state)->void {std::get<MainTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
				std::make_shared<CBindTextColumn<MainTask>>(
					spToDoView.get(),
					L"Name",
					[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Name; },
					[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Name = str; }),
				std::make_shared<CBindTextColumn<MainTask>>(
					spToDoView.get(),
					L"Memo",
					[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Memo; },
					[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Memo = str; }),
				std::make_shared<CBindSheetCellColumn< MainTask, SubTask>>(
					spToDoView.get(),
					L"Sub Task",
					[](std::tuple<MainTask>& tk)->ReactiveVectorProperty<std::tuple<SubTask>>& {return std::get<MainTask>(tk).SubTasks; },
					[](CBindItemsSheetCell<MainTask, SubTask>* pCell)->void {
						pCell->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(pCell));
						pCell->PushColumns(
							pCell->GetHeaderColumnPtr(),
							std::make_shared<CBindCheckBoxColumn<SubTask>>(
								pCell,
								L"Done",
								[](const std::tuple<SubTask>& tk)->CheckBoxState {return std::get<SubTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
								[](std::tuple<SubTask>& tk, const CheckBoxState& state)->void {std::get<SubTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
							std::make_shared<CBindTextColumn<SubTask>>(
								pCell,
								L"Name",
								[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Name; },
								[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Name = str; }),
							std::make_shared<CBindTextColumn<SubTask>>(
								pCell,
								L"Memo",
								[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Memo; },
								[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Memo = str; })
						);
						pCell->SetFrozenCount<ColTag>(1);

						pCell->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(pCell));
						pCell->InsertRow(0, pCell->GetNameHeaderRowPtr());
						pCell->SetFrozenCount<RowTag>(1);
					},
					arg<"maxwidth"_s>() = FLT_MAX)
			);
			spToDoView->SetFrozenCount<ColTag>(1);

			//Rows
			spToDoView->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(spToDoView.get()));
			spToDoView->SetFilterRowPtr(std::make_shared<CRow>(spToDoView.get()));

			spToDoView->PushRows(
				spToDoView->GetNameHeaderRowPtr(),
				spToDoView->GetFilterRowPtr());

			spToDoView->SetFrozenCount<RowTag>(2);

			//Path Changed
			spToDoView->GetPath().Subscribe([&](const auto& notify) {
				auto pData = std::static_pointer_cast<ToDoTabData>(m_itemsSource[m_selectedIndex.get()]);
				pData->Path = spToDoView->GetPath().get();
				spToDoView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
			});
		}
		return spToDoView;
	};

	//TextView Closure
	GetTextViewPtr = [spTextView = std::make_shared<CEditor>(this, m_spEditorProp), isInitialized = false, this]()mutable->std::shared_ptr<CEditor>& {
		return spTextView;
	};

	//PdfView Closure
	GetPdfViewPtr = [spPdfView = std::make_shared<CPDFEditor>(this, m_spPdfEditorProp), isInitialized = false, this]()mutable->std::shared_ptr<CPDFEditor>& {
		return spPdfView;
	};
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
				return ::PathFindFileName(p->Path.c_str());
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				return std::wstring(p->Status.get() == TextStatus::Dirty?L"*":L"") + ::PathFindFileName(p->Path.c_str());
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
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
	std::function<void()> updated = [this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]()->void
		{
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		});
	};

	m_itemsHeaderIconTemplate.emplace(typeid(FilerTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{

			if (auto p = std::dynamic_pointer_cast<FilerTabData>(pTabData)) {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetFileIconBitmap(
					p->FolderPtr->GetAbsoluteIdl(), p->FolderPtr->GetPath(), p->FolderPtr->GetDispExt(), updated);
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(ToDoTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(TextTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Path);
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetFileIconBitmap(
					spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated);
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(PdfTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData)->CComPtr<ID2D1Bitmap>
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Path);
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetFileIconBitmap(
					spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), updated);
			} else {
				return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	//ItemsTemplate
	m_itemsControlTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto pData = std::static_pointer_cast<FilerTabData>(pTabData);
		auto spView = FilerGridViewPtr();
		spView->OpenFolder(pData->FolderPtr);
		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto pData = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();
		spView->Open(pData->Path);
		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();

		//Path
		m_pTextPathBinding.reset(nullptr);//Need to dispose first to disconnect
		m_pTextPathBinding.reset(new CBinding(spViewModel->Path, spView->GetPath()));
		m_pTextPathConnection = std::make_unique<sigslot::scoped_connection>(spView->GetPath().Subscribe([this](const auto&) { UpdateHeaderRects(); }));
		//Status
		m_pStatusBinding.reset(nullptr);
		m_pStatusConnection = std::make_unique<sigslot::scoped_connection>(spViewModel->Status.Subscribe([this](const auto&) { UpdateHeaderRects(); }));
		//Text
		m_pTextBinding.reset(nullptr);
		m_pTextBinding.reset(new CBinding(spViewModel->Text, spView->GetTextBoxPtr()->GetText()));
		//Encoding
		m_pTextEncodingBinding.reset(nullptr);
		m_pTextEncodingBinding.reset(new CBinding(spViewModel->Encoding, spView->GetEncoding()));		
		//Carets
		m_pCaretsBinding.reset(nullptr);
		m_pCaretsBinding.reset(new CBinding(spViewModel->Carets, spView->GetTextBoxPtr()->GetCarets()));
		//CaretPos
		m_pCaretPosBinding.reset(nullptr);
		m_pCaretPosBinding.reset(new CBinding(spViewModel->CaretPos, spView->GetTextBoxPtr()->GetCaretPos()));
		//Open
		m_pOpenBinding.reset(nullptr);
		m_pOpenBinding.reset(new CBinding(spViewModel->OpenCommand, spView->GetOpenCommand()));
		m_pOpenAsBinding.reset(nullptr);
		m_pOpenAsBinding.reset(new CBinding(spViewModel->OpenAsCommand, spView->GetOpenAsCommand()));
		//Save()
		m_pSaveBinding.reset(nullptr);
		m_pSaveBinding.reset(new CBinding(spViewModel->SaveCommand, spView->GetSaveCommand()));
		m_pSaveAsBinding.reset(nullptr);
		m_pSaveAsBinding.reset(new CBinding(spViewModel->SaveAsCommand, spView->GetSaveAsCommand()));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		if (spViewModel->Status.get() == TextStatus::None) {
			spViewModel->Open(spViewModel->Path, spViewModel->Encoding);
		}

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PdfTabData>(pTabData);
		auto spView = GetPdfViewPtr();

		//Path
		m_pPdfPathBinding.reset(nullptr);//Need to dispose first to disconnect
		m_pPdfPathBinding.reset(new CBinding(spViewModel->Path, spView->GetPDFViewPtr()->GetPath()));
		m_pPdfPathConnection = std::make_unique<sigslot::scoped_connection>(spView->GetPDFViewPtr()->GetPath().Subscribe([this](const auto&) { UpdateHeaderRects(); }));
		//Scale
		m_pPdfScaleBinding.reset(nullptr);//Need to dispose first to disconnect
		m_pPdfScaleBinding.reset(new CBinding(spViewModel->Scale, spView->GetPDFViewPtr()->GetScale()));

		//Open
		//m_pOpenBinding.reset(nullptr);
		//m_pOpenBinding.reset(new CBinding<void>(spViewModel->OpenCommand, spView->GetOpenCommand()));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		//spView->GetPDFViewPtr()->Open(spViewModel->Path);//TODOHIGH

		return spView;
	});


	/**********/
	/* Create */
	/**********/
	CTabControl::OnCreate(e);
	FilerGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetToDoGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetTextViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetPdfViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));

	FilerGridViewPtr()->GetIsEnabled().set(false);
	GetToDoGridViewPtr()->GetIsEnabled().set(false);
	GetTextViewPtr()->GetIsEnabled().set(false);
	GetPdfViewPtr()->GetIsEnabled().set(false);

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




