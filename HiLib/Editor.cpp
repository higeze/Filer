#include "Editor.h"
#include "EditorProperty.h"
#include <regex>
#include <nameof/nameof.hpp>

#include "TextBox.h"
#include "EditorTextBox.h"
#include "EditorScroll.h"
#include "EditorProperty.h"

/***************/
/* CTextEditor */
/***************/

CEditor::CEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<EditorProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_spProp(spProp),
	m_spFilterBox(std::make_shared<CTextBox>(this, spProp->EditorTextBoxPropPtr, L"")),
	m_spTextBox(std::make_shared<CEditorTextBox>(this, spProp->EditorTextBoxPropPtr, L"")),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr))
{
	m_spFilterBox->GetText().Subscribe([this](auto)
	{
		m_spTextBox->ClearHighliteRects();
	});

	m_spFilterBox->SetIsScrollable(false);

	m_spTextBox->SetIsScrollable(true);

	m_spStatusBar->GetIsFocusable().set(false);
	
	m_open.Subscribe([this](HWND hWnd)
	{
		m_spTextBox->UpdateAll();
	}, 
	100);
	m_encoding.Subscribe([this](const encoding_type& e)
	{
		m_spStatusBar->SetText(str2wstr(std::string(nameof::nameof_enum(e))));
	});
}



std::tuple<CRectF, CRectF, CRectF> CEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT filterHeight = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spFilterBox->GetTextBoxPropertyPtr()->Format), L"").height
		+ m_spFilterBox->GetTextBoxPropertyPtr()->Padding->top + m_spFilterBox->GetTextBoxPropertyPtr()->Padding->bottom;
	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;
	CRectF rcFilter(rcClient.left, rcClient.top, rcClient.right, rcClient.top + filterHeight);
	CRectF rcText(rcClient.left, rcClient.top + filterHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcText.bottom, rcClient.right, rcClient.bottom);

	return { rcFilter, rcText, rcStatus };
}


void CEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcFilter, rcText, rcStatus] = GetRects();
	m_spFilterBox->OnCreate(CreateEvt(GetWndPtr(), this, rcFilter));
	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, rcText));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
}

void CEditor::OnPaint(const PaintEvent& e)
{
	m_spFilterBox->OnPaint(e);
	m_spTextBox->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcFilter, rcText, rcStatus] = GetRects();
	m_spFilterBox->OnRect(RectEvent(GetWndPtr(), rcFilter));
	m_spTextBox->OnRect(RectEvent(GetWndPtr(), rcText));
	m_spStatusBar->OnRect(RectEvent(GetWndPtr(), rcStatus));
	m_spTextBox->UpdateAll();
}


void CEditor::OnKeyDown(const KeyDownEvent& e)
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

void CEditor::Open()
{
	m_open.Execute(GetWndPtr()->m_hWnd);
}

void CEditor::OpenAs()
{
	m_open_as.Execute(GetWndPtr()->m_hWnd);
}

void CEditor::Save()
{
	m_save.Execute(GetWndPtr()->m_hWnd);
}

void CEditor::SaveAs()
{
	m_save_as.Execute(GetWndPtr()->m_hWnd);
}

void CEditor::Update()
{
	m_spFilterBox->UpdateAll();
	m_spTextBox->UpdateAll();
}

