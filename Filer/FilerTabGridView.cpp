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

#include "TextBox.h"


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
		auto pData = std::static_pointer_cast<FilerTabData>(ItemsSource->at(*SelectedIndex));
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
				if (p->Doc.Path->empty()) {
					return L"No file";
				} else {
					return std::wstring(*p->Doc.Status == FileStatus::Dirty ? L"*" : L"") + ::PathFindFileName(p->Doc.Path->c_str());
				}
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				return std::wstring(*p->Status == FileStatus::Dirty?L"*":L"") + ::PathFindFileName(p->Path->c_str());
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				if (p->Doc->Path->empty()) {
					return L"No file";
				} else {
					return std::wstring(*p->Doc->IsDirty? L"*" : L"") + ::PathFindFileName(p->Doc->Path->c_str());
				}
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(ImageTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<ImageTabData>(pTabData)) {
				if (p->Image->GetPath().empty()) {
					return L"No file";
				} else {
					return std::wstring(::PathFindFileName(p->Image->GetPath().c_str()));
				}
			} else {
				return L"nullptr";
			}
		});

	m_itemsHeaderTemplate.emplace(typeid(PreviewTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<PreviewTabData>(pTabData)) {
				if (p->Path->empty()) {
					return L"No file";
				} else {
					return std::wstring(::PathFindFileName(p->Path->c_str()));
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
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(*p->Doc.Path);
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(TextTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(*p->Path);
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(PdfTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(*p->Doc->Path);
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	m_itemsHeaderIconTemplate.emplace(typeid(ImageTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<ImageTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(p->Image->GetPath());
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetDispExt(), spFile->GetAttributes(), updated);
			} else {
				//return GetWndPtr()->GetDirectPtr()->GetIconCachePtr()->GetDefaultIconBitmap();
			}
		});

	m_itemsHeaderIconTemplate.emplace(typeid(PreviewTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<PreviewTabData>(pTabData)) {
				auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(*p->Path);
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

	m_itemsControlTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();

		spViewModel->OpenCommand.binding(spView->OpenCommand);
		spViewModel->SaveCommand.binding(spView->SaveCommand);
		spViewModel->Doc.Path.binding(spView->Path);
		spViewModel->Doc.Tasks.binding(spView->ItemsSource);

		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();

		//Path
		spViewModel->Path.binding(spView->Path);
		spView->Path.subscribe([this](auto)
		{
			UpdateHeaderRects();
		}, shared_from_this());
		//Status
		spViewModel->Status.subscribe([this](const auto&) { UpdateHeaderRects(); }, shared_from_this());
		//Text
		spViewModel->Text.binding(spView->GetTextBoxPtr()->Text);
		//Encoding
		spViewModel->Encoding.binding(spView->Encoding);		
		//Carets
		spViewModel->Caret.get_unconst()->Old.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Old);
		spViewModel->Caret.get_unconst()->Current.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Current);
		spViewModel->Caret.get_unconst()->Anchor.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Anchor);
		spViewModel->Caret.get_unconst()->SelectedBegin.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->SelectedBegin);
		spViewModel->Caret.get_unconst()->SelectedEnd.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->SelectedEnd);
		spViewModel->Caret.get_unconst()->Point.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Point);
		//Command
		spViewModel->OpenCommand.binding(spView->OpenCommand);
		spViewModel->OpenAsCommand.binding(spView->OpenAsCommand);
		spViewModel->SaveCommand.binding(spView->SaveCommand);
		spViewModel->SaveAsCommand.binding(spView->SaveAsCommand);

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		if (*spViewModel->Status == FileStatus::None) {
			spViewModel->Open(*spViewModel->Path, *spViewModel->Encoding);
		}

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PdfTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PdfTabData>(pTabData);
		auto spView = GetPdfViewPtr();

		//Scale
		spViewModel->Scale.binding(spView->GetPDFViewPtr()->Scale);
		//Scroll
		spViewModel->VScroll.binding(spView->GetPDFViewPtr()->GetVScrollPtr()->Position);
		spViewModel->HScroll.binding(spView->GetPDFViewPtr()->GetHScrollPtr()->Position);

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		spView->GetPDFViewPtr()->Reset(spViewModel->Doc);

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(ImageTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<ImageTabData>(pTabData);
		auto spView = GetImageViewPtr();

		//Path
		spViewModel->Image.binding(spView->GetImageViewPtr()->Image);
		spView->GetImageViewPtr()->Image.subscribe([this](auto) { UpdateHeaderRects(); }, shared_from_this());
		//Scale
		spViewModel->Scale.binding(spView->GetImageViewPtr()->Scale);
		//Scroll
		spViewModel->VScroll.binding(spView->GetImageViewPtr()->GetVScrollPtr()->Position);
		spViewModel->HScroll.binding(spView->GetImageViewPtr()->GetHScrollPtr()->Position);

		spView->GetImageViewPtr()->Open(spViewModel->Image->GetPath());
		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PreviewTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PreviewTabData>(pTabData);
		auto spView = GetPreviewControlPtr();

		//Path
		spViewModel->Path.binding(spView->Path);
		spView->Path.subscribe([this](auto) { UpdateHeaderRects(); }, shared_from_this());

		spView->Open(*spViewModel->Path);
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

	GetFilerGridViewPtr()->IsEnabled.set(false);
	GetToDoGridViewPtr()->IsEnabled.set(false);
	GetTextViewPtr()->IsEnabled.set(false);
	GetPdfViewPtr()->IsEnabled.set(false);
	GetImageViewPtr()->IsEnabled.set(false);
	GetPreviewControlPtr()->IsEnabled.set(false);

	//ItemsSource
	if (ItemsSource->empty()) {
		ItemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
		ItemsSource.push_back(std::make_shared<ToDoTabData>(L""));
		ItemsSource.push_back(std::make_shared<TextTabData>(L""));
		SelectedIndex.force_notify_set(0);
	} else {
		if (*SelectedIndex < 0) {
			SelectedIndex.set(0);
		} else {
			SelectedIndex.force_notify_set(*SelectedIndex);
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
	ItemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewToDoTab(const CommandEvent& e)
{
	ItemsSource.push_back(std::make_shared<ToDoTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewTextTab(const CommandEvent& e)
{
	ItemsSource.push_back(std::make_shared<TextTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewPdfTab(const CommandEvent& e)
{
	ItemsSource.push_back(std::make_shared<PdfTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewImageTab(const CommandEvent& e)
{
	ItemsSource.push_back(std::make_shared<ImageTabData>(GetWndPtr()->GetDirectPtr(), L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewPreviewTab(const CommandEvent& e)
{
	ItemsSource.push_back(std::make_shared<PreviewTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}


void CFilerTabGridView::OnCommandAddToFavorite(const CommandEvent& e)
{
	//TODOLOW Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())){
		p->GetFavoritesPropPtr()->Favorites.push_back(std::make_shared<CFavorite>(std::static_pointer_cast<FilerTabData>(ItemsSource->at(*SelectedIndex))->Path, L""));
		p->GetLeftFavoritesView()->SubmitUpdate();
		p->GetRightFavoritesView()->SubmitUpdate();
	}
}

void CFilerTabGridView::OnCommandOpenSameAsOther(const CommandEvent& e)
{
	//TODO Bad connection between FilerTabGridView and FavoritesView
	if (auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())) {
			
		std::shared_ptr<CFilerTabGridView> otherView = (this == p->GetLeftWnd().get())? p->GetRightWnd(): p->GetLeftWnd();
		
		if (ItemsSource->at(*SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
			ItemsSource.replace(ItemsSource.get_unconst()->begin() + *SelectedIndex, otherView->ItemsSource->at(*otherView->SelectedIndex));
		} else {
			ItemsSource.push_back(otherView->ItemsSource->at(*otherView->SelectedIndex));
		}
	}
}




