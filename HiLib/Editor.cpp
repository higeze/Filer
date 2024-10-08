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

CEditor::CEditor(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl),
	m_spFilterBox(std::make_shared<CTextBox>(this, L"")),
	m_spTextBox(std::make_shared<CEditorTextBox>(this, L"")),
	m_spStatusBar(std::make_shared<CStatusBar>(this)),
	Path(), Encoding(encoding_type::UNKNOWN), Status(FileStatus::None)
{
	m_spFilterBox->SetIsScrollable(false);

	m_spTextBox->SetIsScrollable(true);

	m_spStatusBar->IsFocusable.set(false);
}



std::tuple<CRectF, CRectF, CRectF> CEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT filterHeight = m_spFilterBox->MeasureSize(L"").height;
	FLOAT statusHeight = m_spStatusBar->MeasureSize(L"").height;
	CRectF rcFilter(rcClient.left, rcClient.top, rcClient.right, rcClient.top + filterHeight);
	CRectF rcText(rcClient.left, rcClient.top + filterHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcText.bottom, rcClient.right, rcClient.bottom);

	return { rcFilter, rcText, rcStatus };
}


void CEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	OpenCommand.subscribe([this](HWND hWnd)
	{
		m_spTextBox->Caret.set(CTextCaret());
		m_spTextBox->UpdateAll();
	}, shared_from_this());

	Encoding.subscribe([this](const encoding_type& e)
	{
		m_spStatusBar->Text.set(str2wstr(std::string(nameof::nameof_enum(e))));
	}, shared_from_this());

	auto [rcFilter, rcText, rcStatus] = GetRects();
	m_spFilterBox->OnCreate(CreateEvt(GetWndPtr(), this, rcFilter));
	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, rcText));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));

	m_spFilterBox->SetIsEnterText(true);
	//m_spFilterBox->SetIsTabStop(true);
	//m_spTextBox->SetIsTabStop(true);

	m_spFilterBox->Text.subscribe([this](auto)
	{
		m_spTextBox->ClearHighliteRects();
	}, shared_from_this());
}

void CEditor::OnPaint(const PaintEvent& e)
{
	m_spFilterBox->OnPaint(e);
	m_spTextBox->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CEditor::ArrangeOverride(const CRectF& finalRect)
{
	CD2DWControl::ArrangeOverride(finalRect);

	auto [rcFilter, rcText, rcStatus] = GetRects();
	m_spFilterBox->Arrange(rcFilter);
	m_spTextBox->Arrange(rcText);
	m_spStatusBar->Arrange(rcStatus);
	m_spTextBox->UpdateAll();
}


void CEditor::OnKeyDown(const KeyDownEvent& e)
{
	bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	bool shift = ::GetAsyncKeyState(VK_SHIFT);
	switch (e.Char) {
		case 'F':
			if (ctrl) {
				GetWndPtr()->SetFocusToControl(m_spFilterBox);
				*e.HandledPtr = TRUE;
			}
			break;
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
			break;
	}
}

void CEditor::Open()
{
	OpenCommand.execute(GetWndPtr()->m_hWnd);
}

void CEditor::OpenAs()
{
	OpenAsCommand.execute(GetWndPtr()->m_hWnd);
}

void CEditor::Save()
{
	SaveCommand.execute(GetWndPtr()->m_hWnd);
}

void CEditor::SaveAs()
{
	SaveAsCommand.execute(GetWndPtr()->m_hWnd);
}

void CEditor::Update()
{
	m_spFilterBox->UpdateAll();
	m_spTextBox->UpdateAll();
}

