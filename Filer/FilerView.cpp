#include "FilerView.h"
#include "FilerGridView.h"
#include "FavoritesGridView.h"
#include "TextBox.h"
#include "Button.h"
#include "ShellFileFactory.h"

/*************************/
/* CRecentFolderGridView */
/*************************/
CRecentFolderGridView::CRecentFolderGridView(CD2DWControl* pParentControl)
	:CFilerBindGridView(
	pParentControl,
	//arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr, spFilerGridViewProp->HeaderPropPtr),
	//arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>>>(nullptr, L"Name"),
	arg<"frzcolcnt"_s>() = 0,
	arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
		std::make_shared<CFileIconPathColumn<std::shared_ptr<CShellFile>>>(this, L"Path"),
		std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(nullptr)},
	arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr),
	arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr),
	arg<"frzrowcnt"_s>() = 2)
{
	auto spKnownFolder = CKnownFolderManager::GetInstance()->GetKnownFolderById(FOLDERID_Recent);
	CComPtr<IEnumIDList> pEnumIdl;
	if (SUCCEEDED(spKnownFolder->GetShellFolderPtr()->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnumIdl)) && pEnumIdl) {

		//Enumerate Links
		std::vector<std::shared_ptr<CShellFile>> links;
		CIDL nextIdl;
		while (SUCCEEDED(pEnumIdl->Next(1, nextIdl.ptrptr(), NULL)) && nextIdl) {
			links.push_back(spKnownFolder->CreateShExFileFolder(nextIdl));
			nextIdl.Clear();
		}
		//Sort
		std::ranges::sort(links, [](const std::shared_ptr<CShellFile>& left, const std::shared_ptr<CShellFile>& right) {
			auto leftTime = left->GetFileTimes().value().LastWriteTime;
			auto rightTime = right->GetFileTimes().value().LastWriteTime;
			return ::CompareFileTime(&leftTime, &rightTime) > 0;
		});
		//ItemsSource
		for (const std::shared_ptr<CShellFile>& spFile : links) {
			CComPtr<IShellLink> pShellLink;
			FAILED_CONTINUE(pShellLink.CoCreateInstance(CLSID_ShellLink));
			CComPtr<IPersistFile> pPersistFile;
			FAILED_CONTINUE(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile));
			FAILED_CONTINUE(pPersistFile->Load(spFile->GetPath().c_str(), STGM_READ));
			FAILED_CONTINUE(pShellLink->Resolve(NULL, SLR_UPDATE | SLR_NO_UI));
			CIDL absoluteIdl;
			FAILED_CONTINUE(pShellLink->GetIDList(absoluteIdl.ptrptr()));
			CIDL parentIdl = absoluteIdl.CloneParentIDL();
			CComPtr<IShellFolder> pParentFolder = shell::DesktopBindToShellFolder(parentIdl);

			if (auto p = std::dynamic_pointer_cast<CShellFolder>(CShellFileFactory::GetInstance()->CreateShellFilePtr(pParentFolder, parentIdl, absoluteIdl.CloneLastID()));
				p && p->GetIsExist()) {
				GetItemsSource().push_back(p);
			}
		}
	}
}

void CRecentFolderGridView::OnPaint(const PaintEvent& e)
{
	CFilerBindGridView::OnPaint(e);
	e.WndPtr->GetDirectPtr()->DrawSolidRectangle(GetNormalBorder(), GetRectInWnd());
}

/**************/
/* CFilerView */
/**************/

CFilerView::CFilerView(CD2DWControl* pParentControl)
	:m_spFileGrid(std::make_shared<CFilerGridView>(this)),
	m_spFavoriteGrid(std::make_shared<CFavoritesGridView>(this)),
	m_spTextBox(std::make_shared<CTextBox>(this, L"")),
	m_spRecentButton(std::make_shared<CButton>(this))
{
	
	m_spTextBox->SetIsEnterText(true);
	m_spRecentButton->Content.set(L"R");
	//Favorite-File
	m_spFavoriteGrid->FileChosen = [this](const std::shared_ptr<CShellFile>& spFile)->void {
		if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {//Open Filer
			m_spFileGrid->Folder.set(spFolder);
		}
	};

	//File-Text Binding
	m_spTextBox->EnterText.subscribe([this](auto notify) {
		m_spFileGrid->SetPath(*m_spTextBox->EnterText);
		m_spFileGrid->SubmitUpdate();
	}, m_spFileGrid->Folder.life());
	m_spFileGrid->Folder.subscribe([this](auto value) {
		m_spTextBox->Text.set(value->GetPath());
	}, m_spTextBox);

	//RecentButton-RecentGrid
	m_spRecentButton->Command.subscribe([this]()->void {

		if (auto iter = std::ranges::find_if(m_childControls, [](const std::shared_ptr<CD2DWControl>& spControl)->bool {return typeid(*spControl) == typeid(CRecentFolderGridView); });
			iter != m_childControls.cend()) {
			(*iter)->OnClose(CloseEvent(GetWndPtr(), NULL, NULL));
		} else {
			auto recentGridView = std::make_shared<CRecentFolderGridView>(this);
			recentGridView->SelectedItem.subscribe([this, recentGridView](const std::shared_ptr<CShellFile>& spFile) {
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					m_spFileGrid->Folder.set(spFolder);
					recentGridView->OnClose(CloseEvent(recentGridView->GetWndPtr(), NULL, NULL));
				}
			}, shared_from_this());

			//recentGridView->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
			AddChildControlPtr(recentGridView);
			recentGridView->PostUpdate(Updates::All);
			recentGridView->SubmitUpdate();
			recentGridView->Measure(CSizeF(FLT_MAX, FLT_MAX));
			recentGridView->Arrange(CRectF(m_spTextBox->GetRectInWnd().left, m_spTextBox->GetRectInWnd().bottom,
				m_spTextBox->GetRectInWnd().right, m_spTextBox->GetRectInWnd().bottom + (std::min)(300.f, recentGridView->DesiredSize().height)));
			recentGridView->SubmitUpdate();
		}

	}, m_spRecentButton);

	AddChildControlPtr(m_spTextBox);
	AddChildControlPtr(m_spRecentButton);
	AddChildControlPtr(m_spFavoriteGrid);
	AddChildControlPtr(m_spFileGrid);

}

CFilerView::~CFilerView() = default;

//void CFilerView::OnCreate(const CreateEvt& e)
//{
//	CD2DWControl::OnCreate(e);
//
//	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
//	m_spRecentButton->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
//	m_spFavoriteGrid->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
//	m_spFileGrid->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
//}

void CFilerView::OnRect(const RectEvent& e)
{
	Measure(e.Rect.Size());
	Arrange(e.Rect);
}

void CFilerView::Measure(const CSizeF& availableSize)
{
	m_spTextBox->Measure(availableSize, L"A");
	m_spRecentButton->Measure(availableSize);
	m_spFavoriteGrid->Measure(availableSize);
	m_spFileGrid->Measure(availableSize);

	m_size.width = m_spFavoriteGrid->DesiredSize().width + m_spFileGrid->DesiredSize().width;
	m_size.height = m_spTextBox->DesiredSize().height + (std::max)(m_spFavoriteGrid->DesiredSize().height, m_spFileGrid->DesiredSize().height);
}

void CFilerView::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);

	m_spTextBox->Arrange(CRectF(rc.left, rc.top, rc.right-m_spRecentButton->DesiredSize().width, rc.top + m_spTextBox->DesiredSize().height));
	m_spRecentButton->Arrange(CRectF(rc.right - m_spRecentButton->DesiredSize().width, rc.top, rc.right, rc.top + m_spTextBox->DesiredSize().height));
	m_spFavoriteGrid->Arrange(CRectF(rc.left, m_spTextBox->ArrangedRect().bottom, rc.left + m_spFavoriteGrid->DesiredSize().width, rc.bottom));
	m_spFileGrid->Arrange(CRectF(m_spFavoriteGrid->ArrangedRect().right, m_spTextBox->ArrangedRect().bottom, rc.right, rc.bottom));
}