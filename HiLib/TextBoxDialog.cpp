#include "TextBoxDialog.h"
#include "TextBlock.h"
#include "Button.h"
#include "TextBox.h"
#include "TextBoxProperty.h"

CTextBoxDialog::CTextBoxDialog(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp)
	:CD2DWDialog(pParentControl, spDialogProp),
	m_spTextBlock(std::make_shared<CTextBlock>(this, std::make_shared<TextBlockProperty>())),
	m_spTextBox(std::make_shared<CTextBox>(this, std::make_shared<TextBoxProperty>(), L"")),
	m_spButtonOK(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_spButtonCancel(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>()))
{
	m_spTextBox->SetHasBorder(true);
}
CTextBoxDialog::~CTextBoxDialog() = default;
//message, textbox, ok, cancel
std::tuple<CRectF, CRectF, CRectF, CRectF> CTextBoxDialog::GetRects()
{
	CRectF rc = GetRectInWnd();
	CRectF rcTitle = GetTitleRect();
	//TODO CalcRect
	CRectF rcTextBlock(rc.left + 5.f, rcTitle.bottom + 5.f, rc.right - 5.f, rcTitle.bottom + 30);

	FLOAT textBoxHeight = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spTextBox->GetTextBoxPropertyPtr()->Format), L"").height
	+ m_spTextBox->GetTextBoxPropertyPtr()->Padding->top + m_spTextBox->GetTextBoxPropertyPtr()->Padding->bottom + m_spTextBox->GetTextBoxPropertyPtr()->Line->Width;
	CRectF rcTextBox(rc.left + 5.f, rcTextBlock.bottom + 5.f, rc.right - 5.f, rcTextBlock.bottom + 5 + textBoxHeight);

	CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnOK(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);

	return { rcTextBlock, rcTextBox, rcBtnOK, rcBtnCancel };
}

void CTextBoxDialog::OnCreate(const CreateEvt& e)
{
	//Base
	CD2DWDialog::OnCreate(e);
	
	//Size
	auto [rcTextBlock, rcTextBox, rcBtnOK, rcBtnCancel] = GetRects();

	//Textbox
	m_spTextBlock->OnCreate(CreateEvt(GetWndPtr(), this, rcTextBlock));

	//Textbox
	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, rcTextBox));

	//OK button
	m_spButtonOK->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnOK));

	//Cancel button
	m_spButtonCancel->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnCancel));

	//Focus
	SetFocusedControlPtr(m_spButtonOK);
}

void CTextBoxDialog::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);

	auto [rcMessage, rcText, rcBtnOK, rcBtnCancel] = GetRects();
	m_spTextBlock->OnRect(RectEvent(GetWndPtr(), rcMessage));
	m_spTextBox->OnRect(RectEvent(GetWndPtr(), rcText));
	m_spButtonOK->OnRect(RectEvent(GetWndPtr(), rcBtnOK));
	m_spButtonCancel->OnRect(RectEvent(GetWndPtr(), rcBtnCancel));
}