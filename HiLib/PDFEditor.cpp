#include "PDFEditor.h"
#include "PDFEditorProperty.h"
#include "PDFView.h"
#include "TextBox.h"
#include "EditorScroll.h"
#include "EditorProperty.h"

#include <regex>
#include <nameof/nameof.hpp>

#include "ReactiveProperty.h"


/***************/
/* CTextEditor */
/***************/

CPDFEditor::CPDFEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<PDFEditorProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_spProp(spProp),
	m_spFilterBox(std::make_shared<CTextBox>(this, spProp->TextBoxPropPtr, L"")),
	m_spPDFView(std::make_shared<CPdfView>(this, spProp->PDFViewPropPtr)),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr)),
	m_bindFilterText(m_spFilterBox->GetText(), m_spPDFView->GetFind())
{
	m_spFilterBox->SetIsScrollable(false); 

	m_spStatusBar->GetIsFocusable().set(false);
}

std::tuple<CRectF, CRectF, CRectF> CPDFEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT filterHeight = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spFilterBox->GetTextBoxPropertyPtr()->Format), L"").height
		+ m_spFilterBox->GetTextBoxPropertyPtr()->Padding->top + m_spFilterBox->GetTextBoxPropertyPtr()->Padding->bottom + m_spFilterBox->GetTextBoxPropertyPtr()->Line->Width;
	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;
	CRectF rcFilter(rcClient.left, rcClient.top, rcClient.right, rcClient.top + filterHeight);
	CRectF rcPDF(rcClient.left, rcClient.top + filterHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcPDF.bottom, rcClient.right, rcClient.bottom);

	return { rcFilter, rcPDF, rcStatus };
}


void CPDFEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcFilter, rcPDF, rcStatus] = GetRects();
	m_spFilterBox->OnCreate(CreateEvt(GetWndPtr(), this, rcFilter));
	m_spPDFView->OnCreate(CreateEvt(GetWndPtr(), this, rcPDF));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
	m_spFilterBox->SetIsTabStop(true);
	m_spPDFView->SetIsTabStop(true);
}

void CPDFEditor::OnPaint(const PaintEvent& e)
{
	m_spFilterBox->OnPaint(e);
	m_spPDFView->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CPDFEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcFilter, rcPDF, rcStatus] = GetRects();
	m_spFilterBox->OnRect(RectEvent(GetWndPtr(), rcFilter));
	m_spPDFView->OnRect(RectEvent(GetWndPtr(), rcPDF));
	m_spStatusBar->OnRect(RectEvent(GetWndPtr(), rcStatus));
	//m_spTextBox->UpdateAll();
}

void CPDFEditor::OnKeyDown(const KeyDownEvent& e)
{
	bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	bool shift = ::GetAsyncKeyState(VK_SHIFT);
	switch (e.Char) {
		case 'O':
			if (ctrl && shift) {
				OpenAs();
				*e.HandledPtr = TRUE;
			} else if (ctrl) {
				Open();
				*e.HandledPtr = TRUE;
			}
			break;
		case 'S':
			if (ctrl && shift) {
				SaveAs();
				*e.HandledPtr = TRUE;
			}else if (ctrl) {
				Save();
				*e.HandledPtr = TRUE;
			}
			break;
		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CPDFEditor::Open()
{
	m_open.Execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::OpenAs()
{
	m_open_as.Execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::Save()
{
	m_save.Execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::SaveAs()
{
	m_save_as.Execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::Update()
{
	m_spFilterBox->UpdateAll();
	//m_spPDFView->UpdateAll();
}

