#include "FilerView.h"

CFilerView::CFilerView(CD2DWControl* pParentControl,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp)
	:m_spGridView(std::make_shared<CFilerGridView>(this, spFilerGridViewProp)),
	m_spTextBox(std::make_shared<CTextBox>(this, spTextBoxProp, L"")) 
{
	m_spTextBox->SetIsEnterText(true);
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
	m_spGridView->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
}

void CFilerView::OnRect(const RectEvent& e)
{
	Measure(e.Rect.Size());
	Arrange(e.Rect);
}

void CFilerView::Measure(const CSizeF& availableSize)
{
	m_spTextBox->Measure(availableSize, L"A");
}

void CFilerView::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	m_spTextBox->Arrange(CRectF(rc.LeftTop(), CSizeF(rc.Width(), m_spTextBox->DesiredSize().height)));
	m_spGridView->Arrange(CRectF(rc.left, m_spTextBox->GetRectInWnd().bottom, rc.right, rc.bottom));
}