#include "TextBoxDialog.h"
#include "TextBlock.h"
#include "Button.h"
#include "TextBox.h"
#include "TextBoxProperty.h"

CTextBoxDialog::CTextBoxDialog(CD2DWControl* pParentControl)
	:CD2DWDialog(pParentControl),
	m_spTextBlock(std::make_shared<CTextBlock>(this)),
	m_spTextBox(std::make_shared<CTextBox>(this, L"")),
	m_spButtonOK(std::make_shared<CButton>(this)),
	m_spButtonCancel(std::make_shared<CButton>(this))
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

	FLOAT textBoxHeight = GetWndPtr()->GetDirectPtr()->CalcTextSize(m_spTextBox->GetFormat(), L"").height
	+ m_spTextBox->GetPadding().top + m_spTextBox->GetPadding().bottom + m_spTextBox->GetNormalBorder().Width;
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
	GetWndPtr()->SetFocusToControl(m_spButtonOK);
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