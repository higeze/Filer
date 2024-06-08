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

std::vector<std::wstring> CFilerTabGridView::imageExts = { L".bmp", L".gif", L".ico", L".jpg", L".jpeg", L".png",L".tiff" };
std::vector<std::wstring> CFilerTabGridView::previewExts = {L".docx", L".doc", L".xlsx", L".xls", L".ppt", L".pptx"};


CFilerTabGridView::CFilerTabGridView(CD2DWControl* pParentControl)
	:CTabControl(pParentControl), 
	m_spFilerView(std::make_shared<CFilerView>(this)),
	m_spTextView(std::make_shared<CEditor>(this)),
	m_spPdfView(std::make_shared<CPDFEditor>(this)),
	m_spImageView(std::make_shared<CImageEditor>(this)),
	m_spToDoGridView(std::make_shared<CToDoGridView>(this)),
	m_spPreviewControl(std::make_shared<CPreviewControl>(this)){}

CFilerTabGridView::~CFilerTabGridView() = default;

void CFilerTabGridView::Measure(const CSizeF& availableSize)
{
	m_spCurControl->Measure(availableSize);
	m_size = m_spCurControl->DesiredSize();
}


void CFilerTabGridView::OnCreate(const CreateEvt& e)
{
	/*****************/
	/* ItemsTemplate */
	/*****************/
	//ItemsHeader
	m_itemsHeaderTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{

			if (auto p = std::dynamic_pointer_cast<FilerTabData>(pTabData)) {
				return p->Folder->GetDispNameWithoutExt().c_str();
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(ToDoTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				if (p->Doc->Path->empty()) {
					return L"No file";
				} else {
					return std::wstring(*p->Doc->Status == FileStatus::Dirty ? L"*" : L"") + ::PathFindFileName(p->Doc->Path->c_str());
				}
			} else {
				return L"nullptr";
			}
		});
	m_itemsHeaderTemplate.emplace(typeid(TextTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::wstring
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				return std::wstring(*p->Doc->Status == FileStatus::Dirty?L"*":L"") + ::PathFindFileName(p->Doc->Path->c_str());
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
				if (p->Doc->GetPath().empty()) {
					return L"No file";
				} else {
					return std::wstring(::PathFindFileName(p->Doc->GetPath().c_str()));
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
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(
					GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->Folder.get_shared_unconst().get(), updated);
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(ToDoTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<ToDoTabData>(pTabData)) {
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(
					GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->Doc.operator->(), updated);
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(TextTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<TextTabData>(pTabData)) {
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(
					GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->Doc.operator->(), updated);
			}
		});
	m_itemsHeaderIconTemplate.emplace(typeid(PdfTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<PdfTabData>(pTabData)) {
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(
					GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->Doc.operator->(), updated);
			}
		});

	m_itemsHeaderIconTemplate.emplace(typeid(ImageTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<ImageTabData>(pTabData)) {
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(
					GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->Image.operator->(), updated);
			}
		});

	m_itemsHeaderIconTemplate.emplace(typeid(PreviewTabData).name(), [this, updated](const std::shared_ptr<TabData>& pTabData, const CRectF& dstRect)->void
		{
			if (auto p = std::dynamic_pointer_cast<PreviewTabData>(pTabData)) {
				GetWndPtr()->GetDirectPtr()->GetFileIconDrawerPtr()->DrawFileIconBitmap(
					GetWndPtr()->GetDirectPtr(), dstRect.LeftTop(), p->Doc.operator->(), updated);
			}
		});

	//ItemsTemplate
	auto disconnect = [](sigslot::connection& con) { return con.disconnect(); };
	auto disconnect2 = [](std::pair<sigslot::connection, sigslot::connection>& paircon) { return paircon.first.disconnect() && paircon.second.disconnect(); };

	m_itemsControlTemplate.emplace(typeid(FilerTabData).name(), [this](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<FilerTabData>(pTabData);

		m_filerConnections.clear();
		
		m_filerConnections.push_back(
			GetFilerViewPtr()->GetFileGridPtr()->Folder.subscribe([&](auto value) { UpdateHeaderRects(); }, shared_from_this()),
			spViewModel->Folder.binding(GetFilerViewPtr()->GetFileGridPtr()->Folder)
		);
		//spView->GetFileGridPtr()->OpenFolder(pData->FolderPtr);

		GetFilerViewPtr()->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		GetFilerViewPtr()->GetFileGridPtr()->PostUpdate(Updates::All);
		GetFilerViewPtr()->GetFileGridPtr()->SubmitUpdate();

		return GetFilerViewPtr();
	});

	m_itemsControlTemplate.emplace(typeid(ToDoTabData).name(), [this, disconnect, disconnect2](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<ToDoTabData>(pTabData);
		auto spView = GetToDoGridViewPtr();

		m_todoConnections.clear();

		m_todoConnections.push_back(
			spViewModel->OpenCommand.binding(spView->OpenCommand),
			spViewModel->SaveCommand.binding(spView->SaveCommand),
			spViewModel->Doc.get_unconst()->Path.binding(spView->Path),
			spViewModel->Doc.get_unconst()->Tasks.binding(spView->ItemsSource));

		spView->OnRectWoSubmit(RectEvent(GetWndPtr(), GetControlRect()));
		spView->PostUpdate(Updates::All);
		spView->SubmitUpdate();

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(TextTabData).name(), [this, disconnect, disconnect2](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<TextTabData>(pTabData);
		auto spView = GetTextViewPtr();

		m_textConnections.clear();

		m_textConnections.push_back(
			//Doc
			spViewModel->Doc.get_unconst()->Path.binding(spView->Path),
			spViewModel->Doc.get_unconst()->Encoding.binding(spView->Encoding),
			spViewModel->Doc.get_unconst()->Status.binding(spView->Status),
			spViewModel->Doc.get_unconst()->Text.binding(spView->GetTextBoxPtr()->Text),
			//Subscribe
			spViewModel->Doc.get_unconst()->Path.subscribe([this](const auto&) { UpdateHeaderRects(); }, shared_from_this()),
			spViewModel->Doc.get_unconst()->Status.subscribe([this](const auto&) { UpdateHeaderRects(); }, shared_from_this()),
			//Command
			spViewModel->OpenCommand.binding(spView->OpenCommand),
			spViewModel->OpenAsCommand.binding(spView->OpenAsCommand),
			spViewModel->SaveCommand.binding(spView->SaveCommand),
			spViewModel->SaveAsCommand.binding(spView->SaveAsCommand));
		//In functio argument, order is not defined. Therefore it is necessary to make sure calling caret last.
		m_textConnections.push_back(
			//Carets
			spViewModel->Caret.get_unconst()->Old.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Old),
			spViewModel->Caret.get_unconst()->Current.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Current),
			spViewModel->Caret.get_unconst()->Anchor.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Anchor),
			spViewModel->Caret.get_unconst()->SelectedBegin.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->SelectedBegin),
			spViewModel->Caret.get_unconst()->SelectedEnd.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->SelectedEnd),
			spViewModel->Caret.get_unconst()->Point.binding(spView->GetTextBoxPtr()->Caret.get_unconst()->Point));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		if (*spViewModel->Doc->Status == FileStatus::None) {
			spViewModel->Doc.get_unconst()->Open(*spViewModel->Doc->Path, *spViewModel->Doc->Encoding);
		}

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PdfTabData).name(), [this, disconnect, disconnect2](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PdfTabData>(pTabData);
		auto spView = GetPdfViewPtr();

		m_pdfConnections.clear();

		//Scale
		m_pdfConnections.push_back(
			spViewModel->Scale.binding(spView->GetPDFViewPtr()->Scale),
			spViewModel->VScroll.binding(spView->GetPDFViewPtr()->GetVScrollPtr()->Position),
			spViewModel->HScroll.binding(spView->GetPDFViewPtr()->GetHScrollPtr()->Position));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));
		//spView->GetPDFViewPtr()->Reset(spViewModel->Doc);
		m_pdfConnections.push_back(
			spViewModel->Doc.binding(spView->GetPDFViewPtr()->PDF));

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(ImageTabData).name(), [this, disconnect, disconnect2](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<ImageTabData>(pTabData);
		auto spView = GetImageViewPtr();

		m_imageConnections.clear();

		//Path
		m_imageConnections.push_back(
			spView->GetImageViewPtr()->Image.subscribe([this](auto) { UpdateHeaderRects(); }, shared_from_this()),
			spViewModel->Scale.binding(spView->GetImageViewPtr()->Scale),
			spViewModel->VScroll.binding(spView->GetImageViewPtr()->GetVScrollPtr()->Position),
			spViewModel->HScroll.binding(spView->GetImageViewPtr()->GetHScrollPtr()->Position));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));

		m_imageConnections.push_back(spViewModel->Image.binding(spView->GetImageViewPtr()->Image));

		//spView->GetImageViewPtr()->Open(spViewModel->Image->GetPath());

		return spView;
	});

	m_itemsControlTemplate.emplace(typeid(PreviewTabData).name(), [this, disconnect, disconnect2](const std::shared_ptr<TabData>& pTabData)->std::shared_ptr<CD2DWControl> {
		auto spViewModel = std::static_pointer_cast<PreviewTabData>(pTabData);
		auto spView = GetPreviewControlPtr();

		m_prevConnections.clear();

		//Path
		m_prevConnections.push_back(spViewModel->Doc.binding(spView->Doc),
			spView->Doc.subscribe([this](auto) { UpdateHeaderRects(); }, shared_from_this()));

		spView->OnRect(RectEvent(GetWndPtr(), GetControlRect()));

		return spView;
	});

	/**********/
	/* Create */
	/**********/
	CTabControl::OnCreate(e);
	GetFilerViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetToDoGridViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetTextViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetPdfViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetImageViewPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));
	GetPreviewControlPtr()->OnCreate(CreateEvt(GetWndPtr(), this, GetControlRect()));

	GetFilerViewPtr()->IsEnabled.set(false);
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

	GetFilerViewPtr()->GetFileGridPtr()->SelectedItem.subscribe([this](const std::shared_ptr<CShellFile>& spFile) {

		if (auto spOther = m_wpOther.lock()) {
			if (!spOther->m_isPreview) return;

			std::shared_ptr<TabData> spObsData = spOther->ItemsSource.get_unconst()->at(*spOther->SelectedIndex);

			//Text
			if (auto spTxtData = std::dynamic_pointer_cast<TextTabData>(spObsData);
				spTxtData && boost::iequals(spFile->GetPathExt(), L".txt")) {
				//TODO
			//PDF
			} else if (auto spPdfData = std::dynamic_pointer_cast<PdfTabData>(spObsData);
				spPdfData && boost::iequals(spFile->GetPathExt(), L".pdf")) {
				std::shared_ptr<CPDFDoc> newDoc;
				newDoc->Open(spFile->GetPath());
				spPdfData->Scale.set(-1);
				spPdfData->VScroll.set(0.f);
				spPdfData->HScroll.set(0.f);
				spPdfData->Doc.set(newDoc);
				//Image
			} else if (auto spImgData = std::dynamic_pointer_cast<ImageTabData>(spObsData);
				spImgData && std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
				CD2DImage newDoc(spFile->GetPath());
				spImgData->Scale.set(-1);
				spImgData->VScroll.set(0.f);
				spImgData->HScroll.set(0.f);
				spImgData->Image.set(newDoc);
				//Preview
			} else if (auto spPrvData = std::dynamic_pointer_cast<PreviewTabData>(spObsData);
				spPrvData && std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
				CShellFile newDoc(spFile->GetPath());
				//spPrvData->Scale.set(-1);
				//spPrvData->VScroll.set(0.f);
				//spPrvData->HScroll.set(0.f);
				spPrvData->Doc.set(newDoc);
			}
		}

	}, shared_from_this());

}

void CFilerTabGridView::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char)
	{
	case 'Q':
		//Replace or PushBack
		if(::IsKeyDown(VK_CONTROL))
		if(auto spOther = m_wpOther.lock()){
			if (auto spCurView = std::dynamic_pointer_cast<CFilerView>(GetCurrentControlPtr())) {
				std::shared_ptr<TabData> spNewData;
				if (boost::iequals(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt(), L".txt")) {
					spNewData = std::make_shared<TextTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
				} else if (boost::iequals(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt(), L".pdf")) {
					spNewData = std::make_shared<PdfTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
				} else if (std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
					spNewData = std::make_shared<ImageTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
				} else if (std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spCurView->GetFileGridPtr()->GetFocusedFile()->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
					spNewData = std::make_shared<PreviewTabData>(spCurView->GetFileGridPtr()->GetFocusedFile()->GetPath());
				}

				if (spNewData) {
					//Replace
					if (::IsKeyDown(VK_SHIFT) && spOther->ItemsSource.get_unconst()->at(*spOther->SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
						spOther->ItemsSource.replace(spOther->ItemsSource.get_unconst()->begin() + *spOther->SelectedIndex, spNewData);
					//Push back	
					} else {
						spOther->ItemsSource.push_back(spNewData);
					}
					*(e.HandledPtr) = TRUE;
				}
			}
		}
		break;

	case VK_F5:
		if(auto spOther = m_wpOther.lock()){
			if (auto spFilerView = std::dynamic_pointer_cast<CFilerView>(GetCurrentControlPtr())) {
				if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerView>(spOther->GetCurrentControlPtr())) {
					spFilerView->GetFileGridPtr()->CopySelectedFilesTo(spOtherFilerGrid->GetFileGridPtr()->Folder->GetAbsoluteIdl());
					*(e.HandledPtr) = TRUE;
				}
			}
		}
		break;
	case VK_F6:
		if (auto spOther = m_wpOther.lock()) {
			if (auto spFilerView = std::dynamic_pointer_cast<CFilerView>(GetCurrentControlPtr())) {
				if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerView>(spOther->GetCurrentControlPtr())) {
					spFilerView->GetFileGridPtr()->MoveSelectedFilesTo(spOtherFilerGrid->GetFileGridPtr()->Folder->GetAbsoluteIdl());
					*(e.HandledPtr) = TRUE;
				}
			}
		}
		break;
	case VK_F9:
		if (auto spOther = m_wpOther.lock()) {
			if (auto spFilerView = std::dynamic_pointer_cast<CFilerView>(GetCurrentControlPtr())) {
				if (auto spOtherFilerGrid = std::dynamic_pointer_cast<CFilerView>(spOther->GetCurrentControlPtr())) {
					spFilerView->GetFileGridPtr()->CopyIncrementalSelectedFilesTo(spOtherFilerGrid->GetFileGridPtr()->Folder->GetAbsoluteIdl());
					*(e.HandledPtr) = TRUE;
				}
			}
		}
		break;
	default:
		CD2DWControl::OnKeyDown(e);
		break;
	}
}

void CFilerTabGridView::OnContextMenu(const ContextMenuEvent& e)
{
	CTabControl::OnContextMenu(e);
	
	auto me = std::dynamic_pointer_cast<CFilerTabGridView>(shared_from_this());
	CContextMenu2 menu;
	menu.Add(
		std::make_unique<CMenuItem2>(L"New Filer tab\tCtrl+T", &CFilerTabGridView::OnCommandNewFilerTab, me),
		std::make_unique<CMenuItem2>(L"New ToDo tab", &CFilerTabGridView::OnCommandNewToDoTab, me),
		std::make_unique<CMenuItem2>(L"New Text tab", &CFilerTabGridView::OnCommandNewTextTab, me),
		std::make_unique<CMenuItem2>(L"New Pdf tab", &CFilerTabGridView::OnCommandNewPdfTab, me),
		std::make_unique<CMenuItem2>(L"New Image tab", &CFilerTabGridView::OnCommandNewImageTab, me),
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Clone", &CFilerTabGridView::OnCommandCloneTab, me),
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Open same as other", &CFilerTabGridView::OnCommandOpenSameAsOther, me),
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Lock", &CFilerTabGridView::OnCommandLockTab, me),
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Close\tCtrl+W", &CFilerTabGridView::OnCommandCloseTab, me),
		std::make_unique<CMenuItem2>(L"Close all but this", &CFilerTabGridView::OnCommandCloseAllButThisTab, me),
		std::make_unique<CMenuSeparator2>(),
		std::make_unique<CMenuItem2>(L"Add to Favorite", &CFilerTabGridView::OnCommandAddToFavorite, me)
	);
	menu.Popup(GetWndPtr()->m_hWnd, CPointU(e.PointInScreen.x, e.PointInScreen.y));
	//CTabControl::OnContextMenu(e);

	//if (m_contextIndex) {
	//	CMenu menu;
	//	menu.Attach(::GetSubMenu(::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXTMENU_TAB)), 0));
	//	GetWndPtr()->SetForegroundWindow();
	//	menu.TrackPopupMenu(0, e.PointInScreen.x, e.PointInScreen.y, GetWndPtr()->m_hWnd);
	//} else if (m_spCurControl->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))){
	//	m_spCurControl->OnContextMenu(e);
	//}
}

void CFilerTabGridView::OnCommandNewFilerTab()
{
	ItemsSource.push_back(std::make_shared<FilerTabData>(std::static_pointer_cast<CShellFolder>(CKnownFolderManager::GetInstance()->GetDesktopFolder())));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewToDoTab()
{
	ItemsSource.push_back(std::make_shared<ToDoTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewTextTab()
{
	ItemsSource.push_back(std::make_shared<TextTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewPdfTab()
{
	ItemsSource.push_back(std::make_shared<PdfTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewImageTab()
{
	ItemsSource.push_back(std::make_shared<ImageTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}

void CFilerTabGridView::OnCommandNewPreviewTab()
{
	ItemsSource.push_back(std::make_shared<PreviewTabData>(L""));
	SelectedIndex.set(ItemsSource->size() - 1);
}


void CFilerTabGridView::OnCommandAddToFavorite()
{
	//TODOLOW Bad connection between FilerTabGridView and FavoritesView
	if(auto p = dynamic_cast<CFilerWnd*>(GetWndPtr())){
		p->Favorites.push_back(CFavorite(std::static_pointer_cast<FilerTabData>(ItemsSource->at(*SelectedIndex))->Folder->GetPath(), L""));//TODOTODO
	}
}

void CFilerTabGridView::OnCommandOpenSameAsOther()
{
	
	if (auto spOther = m_wpOther.lock()) {

		if (ItemsSource->at(*SelectedIndex)->AcceptClosing(GetWndPtr(), false)) {
			ItemsSource.replace(ItemsSource.get_unconst()->begin() + *SelectedIndex, spOther->ItemsSource->at(*spOther->SelectedIndex)->ClonePtr());
		} else {
			ItemsSource.push_back(spOther->ItemsSource->at(*spOther->SelectedIndex));
		}
	}
}




