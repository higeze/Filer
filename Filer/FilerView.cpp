#include "FilerView.h"
#include "ButtonProperty.h"
#include "FilerBindGridView.h"
#include "ShellFileFactory.h"

CRecentFolderGridView::CRecentFolderGridView(CD2DWControl* pParentControl,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CFilerBindGridView(
	pParentControl,
	spFilerGridViewProp,
	//arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr, spFilerGridViewProp->HeaderPropPtr),
	//arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>>>(nullptr, L"Name"),
	arg<"frzcolcnt"_s>() = 0,
	arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
		std::make_shared<CFileIconPathColumn<std::shared_ptr<CShellFile>>>(this, L"Path"),
		std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(nullptr, spFilerGridViewProp->FileTimeArgsPtr)},
	arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr, spFilerGridViewProp->HeaderPropPtr),
	arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr, spFilerGridViewProp->CellPropPtr),
	arg<"frzrowcnt"_s>() = 1)
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
			FAILED_CONTINUE(pShellLink->Resolve(NULL, SLR_NONE));
			CIDL absoluteIdl;
			FAILED_CONTINUE(pShellLink->GetIDList(absoluteIdl.ptrptr()));
			CIDL parentIdl = absoluteIdl.CloneParentIDL();
			CComPtr<IShellFolder> pParentFolder = shell::DesktopBindToShellFolder(parentIdl);

			if (auto p = std::dynamic_pointer_cast<CShellFolder>(CShellFileFactory::GetInstance()->CreateShellFilePtr(pParentFolder, parentIdl, absoluteIdl.CloneLastID()))) {
				GetItemsSource().push_back(p);
			}
		}
	}
}

void CRecentFolderGridView::OnPaint(const PaintEvent& e)
{
	CFilerBindGridView::OnPaint(e);
	e.WndPtr->GetDirectPtr()->DrawSolidRectangle(*m_spGridViewProp->CellPropPtr->Line, GetRectInWnd());
}

CFilerView::CFilerView(CD2DWControl* pParentControl,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp)
	:m_spGridView(std::make_shared<CFilerGridView>(this, spFilerGridViewProp)),
	m_spTextBox(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_spRecentButton(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>()))
{
	m_spTextBox->SetIsEnterText(true);
	m_spRecentButton->Content.set(L"R");

	//Folder-Text Binding
	m_spTextBox->EnterText.subscribe([this](auto notify) {
		m_spGridView->SetPath(*m_spTextBox->EnterText);
		m_spGridView->SubmitUpdate();
	}, m_spGridView->Folder.life());
	m_spGridView->Folder.subscribe([this](auto value) {
		m_spTextBox->Text.set(value->GetPath());
	}, m_spTextBox);
}

CFilerView::~CFilerView() = default;

void CFilerView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_spRecentButton->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	m_spGridView->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));

	m_spRecentButton->Command.subscribe([this]()->void {

		if (auto iter = std::ranges::find_if(m_childControls, [](const std::shared_ptr<CD2DWControl>& spControl)->bool {return typeid(*spControl) == typeid(CRecentFolderGridView); });
			iter != m_childControls.cend()) {
			(*iter)->OnClose(CloseEvent(GetWndPtr(), NULL, NULL));
		} else {
			auto recentGridView = std::make_shared<CRecentFolderGridView>(this, std::make_shared<FilerGridViewProperty>());
			recentGridView->SelectedItem.subscribe([this, recentGridView](const std::shared_ptr<CShellFile>& spFile) {
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					m_spGridView->Folder.set(spFolder);
					recentGridView->OnClose(CloseEvent(recentGridView->GetWndPtr(), NULL, NULL));
				}
			}, shared_from_this());

			recentGridView->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
			recentGridView->PostUpdate(Updates::All);
			recentGridView->SubmitUpdate();
			recentGridView->Measure(CSizeF(FLT_MAX, FLT_MAX));
			recentGridView->Arrange(CRectF(m_spTextBox->GetRectInWnd().left, m_spTextBox->GetRectInWnd().bottom,
				m_spTextBox->GetRectInWnd().right, m_spTextBox->GetRectInWnd().bottom + (std::min)(300.f, recentGridView->DesiredSize().height)));
			recentGridView->SubmitUpdate();
		}

	}, shared_from_this());
}

void CFilerView::OnRect(const RectEvent& e)
{
	Measure(e.Rect.Size());
	Arrange(e.Rect);
}

void CFilerView::Measure(const CSizeF& availableSize)
{
	m_spTextBox->Measure(availableSize, L"A");
	m_spRecentButton->Measure(availableSize);
	m_spGridView->Measure(availableSize);
}

void CFilerView::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	m_spTextBox->Arrange(CRectF(rc.left, rc.top, rc.right-m_spRecentButton->DesiredSize().width, rc.top + m_spTextBox->DesiredSize().height));
	m_spRecentButton->Arrange(CRectF(rc.right - m_spRecentButton->DesiredSize().width, rc.top, rc.right, rc.top + m_spTextBox->DesiredSize().height));
	m_spGridView->Arrange(CRectF(rc.left, m_spTextBox->GetRectInWnd().bottom, rc.right, rc.bottom));
}