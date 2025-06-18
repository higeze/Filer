#include "FavoritesGridView.h"
#include "BindColumn.h"
#include "BindRow.h"
#include "FavoritesColumn.h"
#include "FavoriteCell.h"

#include "ShellFile.h"
#include "ShellFolder.h"

#include "Debug.h"
#include "ApplicationProperty.h"
#include "SheetEventArgs.h"
#include "Cursorer.h"
#include "FavoritesItemDragger.h"
#include "FilerWnd.h"
#include "Celler.h"
#include "Debug.h"
#include "MyMenu.h"
#include "TextBlock.h"
#include "DockPanel.h"
#include "Dispatcher.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFavoritesGridView::CFavoritesGridView(CD2DWControl* pParentControl)
	:CBindGridView(pParentControl,
		arg<"bindtype"_s>() = BindType::Row,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{std::make_shared<CFavoritesColumn<CFavorite>>(this)})
{
	IsFocusable.set(false);
	m_pVScroll->SetVisibility(Visibility::Hidden);
	m_pHScroll->SetVisibility(Visibility::Disabled);

	m_spItemDragger = std::make_shared<CFavoritesItemDragger>();

	CellLButtonDblClk.connect(std::bind(&CFavoritesGridView::OnCellLButtonDblClk,this,std::placeholders::_1));
}

void CFavoritesGridView::OnCreate(const CreateEvt& e)
{
	//Base Create
	CBindGridView::OnCreate(e);

	//List
	OpenFavorites();

	SubmitUpdate();
}

void CFavoritesGridView::OnKeyDown(const KeyDownEvent& e)
{
	//m_keepEnsureVisibleFocusedCell = false;
	switch (e.Char) {
		case 'R':
			if (::IsKeyDown(VK_CONTROL)) {
				Reload();
				(*e.HandledPtr) = true;
			}
			break;
		default:
			CBindGridView::OnKeyDown(e);
	}
}

void CFavoritesGridView::OnCommandDelete()
{
	ItemsSource.erase(ItemsSource.get_unconst()->cbegin() + m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>());
	SubmitUpdate();
}

class CFavoritePropertyDlg : public CD2DWDialog
{
protected:
	std::shared_ptr<int> Dummy;
	std::shared_ptr<CDockPanel> m_spDock;
	std::shared_ptr<CButton> m_spButtonOkay;
	std::shared_ptr<CButton> m_spButtonCancel;
	std::shared_ptr<CTextBlock> m_spTextBlockName;
	std::shared_ptr<CTextBox> m_spTextBoxName;
	std::shared_ptr<CTextBlock> m_spTextBlockPath;
	std::shared_ptr<CTextBox> m_spTextBoxPath;

	CFavorite m_favorite;

public:
	CFavoritePropertyDlg(CD2DWControl* pParentControl, const CFavorite& favorite)
		:m_favorite(favorite)
	{
		Title.set(L"Favorite Property");

		m_spTextBlockName->Text.set(L"Name");
		m_spTextBoxName->Text.set(m_favorite.GetShortName());

		m_spTextBlockName->Text.set(L"Path");
		m_spTextBoxPath->Text.set(m_favorite.GetPath());

		m_spButtonOkay->Content.set(L"OK");
		m_spButtonCancel->Content.set(L"Cancel");

		m_spButtonOkay->Command.subscribe([this]()->void
			{
				m_favorite.SetShortName(*m_spTextBoxName->Text);
				m_favorite.SetPath(*m_spTextBoxPath->Text);

				GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
			}, Dummy);

		m_spButtonCancel->Command.subscribe([this]()->void
			{
				GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
			}, Dummy);

	}
	virtual ~CFavoritePropertyDlg() = default;

	void OnCreate(const CreateEvt& e)
	{
		using pr = std::pair<std::shared_ptr<CD2DWControl>, DockEnum>;

		auto spBottomDock = std::make_shared<CDockPanel>(this);
		spBottomDock->Add(
			pr(m_spButtonOkay, DockEnum::Right),
			pr(m_spButtonCancel, DockEnum::Right)
		);

		m_spDock->Add(
			pr(m_spTextBlockName, DockEnum::Top),
			pr(m_spTextBoxName, DockEnum::Top),
			pr(m_spTextBlockPath, DockEnum::Top),
			pr(m_spTextBoxPath, DockEnum::Top),
			pr(spBottomDock, DockEnum::Bottom)
		);

		//Base
		CD2DWDialog::OnCreate(e);
	}

	//CSizeF MeasureOverride(const CSizeF& availableSize)
	//{
	//	m_spDock->Measure(availableSize);

	//	return m_spDock->DesiredSize();
	//}


	//void ArrangeOverride(const CRectF& finalRect)
	//{
	//	CPdfViewDlgBase::ArrangeOverride(finalRect);

	//	CRectF rcTitle = GetTitleRect();
	//	m_spParameter->Arrange(CRectF(
	//		finalRect.left, finalRect.top + rcTitle.Height(),
	//		finalRect.right, finalRect.top + rcTitle.Height() + m_spParameter->DesiredSize().height));
	//	CRectF rcBtnCancel(finalRect.right - m_spButtonCancel->DesiredSize().width, finalRect.bottom - m_spButtonCancel->DesiredSize().height, finalRect.right, finalRect.bottom);
	//	CRectF rcBtnDo(rcBtnCancel.left - m_spButtonDo->DesiredSize().width, finalRect.bottom - m_spButtonDo->DesiredSize().height, rcBtnCancel.left, finalRect.bottom);
	//	m_spButtonCancel->Arrange(rcBtnCancel);
	//	m_spButtonDo->Arrange(rcBtnDo);
	//}


};






void CFavoritesGridView::OnCommandProperty()
{
	auto spDlg = std::make_shared<CFavoritePropertyDlg>(this, ItemsSource->at(m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>()));

	spDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), CRectF()));
	spDlg->Measure(CSizeF(FLT_MAX, FLT_MAX));
	spDlg->Arrange(CalcCenterRectF(spDlg->DesiredSize()));
	GetWndPtr()->SetFocusToControl(spDlg);
}


void CFavoritesGridView::OnContextMenu(const ContextMenuEvent& e)
{
	//
	auto me = std::dynamic_pointer_cast<CFavoritesGridView>(shared_from_this());
	CContextMenu2 menu;
	menu.Add(
		std::make_unique<CMenuItem2>(L"Delete", &CFavoritesGridView::OnCommandDelete, me),
		std::make_unique<CMenuItem2>(L"Property", &CFavoritesGridView::OnCommandProperty, me)
	);
	menu.Popup(GetWndPtr()->m_hWnd, CPointU(e.PointInScreen.x, e.PointInScreen.y));
	*e.HandledPtr = TRUE;
}


void CFavoritesGridView::OpenFavorites()
{
	LOG_THIS_1("CFavoritesGridView::OpenFavorites");

	LOG_SCOPED_TIMER_THIS_1("OpenFavorites Total");

	//Celler
	m_spCeller->Clear();
	//Cursor
	m_spCursorer->Clear();

	m_allCells.clear();
	//for (auto colPtr : m_allCols) {
	//	std::dynamic_pointer_cast<CMapColumn>(colPtr)->Clear();
	//	colPtr->SetIsMeasureValid(false);
	//}

	PostUpdate(Updates::Sort);
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Invalidate);
}

void CFavoritesGridView::OnCellLButtonDblClk(const CellEventArgs& e)
{
	if(auto p = dynamic_cast<CFavoriteCell<CFavorite>*>(e.CellPtr)){
		auto pFile = p->GetShellFile();
		if (pFile != nullptr && typeid(*pFile) != typeid(CShellInvalidFile)) {
			FileChosen(p->GetShellFile());
		}
	}
	SubmitUpdate();
}

void CFavoritesGridView::MoveRow(int indexTo, typename RowTag::SharedPtr spFrom)
{
	int from = spFrom->GetIndex<VisTag>();
	int to = indexTo > from ? indexTo - 1 : indexTo;

	auto fromIter = ItemsSource->cbegin() + (from - GetFrozenCount<RowTag>());
	auto temp = *fromIter;
	ItemsSource.erase(fromIter);
	auto toIter = ItemsSource->cbegin() + (to - GetFrozenCount<RowTag>());
	ItemsSource.insert(toIter, temp);

	Reload();
}

void CFavoritesGridView::Reload()
{
	for (auto iter = ItemsSource.get_unconst()->begin(); iter != ItemsSource.get_unconst()->end(); ++iter) {
		iter->SetLockShellFile(nullptr);
	}
	OpenFavorites();
	SubmitUpdate();
}
