#include "PDFEditor.h"
#include "PDFEditorProperty.h"
#include "PDFView.h"
#include "PDFDoc.h"
#include "TextBox.h"
#include "TextBlock.h"
#include "EditorScroll.h"
#include "EditorProperty.h"

#include <regex>
#include <nameof/nameof.hpp>

#include "ReactiveProperty.h"


/***************/
/* CPDFEditor */
/***************/

CPDFEditor::CPDFEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<PDFEditorProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_spProp(spProp),
	m_spFilterBox(std::make_shared<CTextBox>(this, spProp->TextBoxPropPtr, L"")),
	m_spPageBox(std::make_shared<CTextBox>(this, spProp->TextBoxPropPtr, L"")),
	m_spTotalPageBlock(std::make_shared<CTextBlock>(this, spProp->TextBlockPropPtr)),
	m_spScaleBox(std::make_shared<CTextBox>(this, spProp->TextBoxPropPtr, L"")),
	m_spPercentBlock(std::make_shared<CTextBlock>(this, spProp->TextBlockPropPtr)),
	m_spPDFView(std::make_shared<CPdfView>(this, spProp->PDFViewPropPtr)),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr)),
	m_bindFilterText(m_spFilterBox->GetText(), m_spPDFView->GetFind())
{
	m_spFilterBox->SetIsScrollable(false); 
	m_spPercentBlock->GetText().set(L"%");
	m_spStatusBar->GetIsFocusable().set(false);
}

std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> CPDFEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT filterHeight = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spFilterBox->GetTextBoxPropertyPtr()->Format), L"").height
		+ m_spFilterBox->GetTextBoxPropertyPtr()->Padding->top + m_spFilterBox->GetTextBoxPropertyPtr()->Padding->bottom + m_spFilterBox->GetTextBoxPropertyPtr()->Line->Width;

	CSizeF pageSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		*(m_spPageBox->GetTextBoxPropertyPtr()->Format), L"000");
	pageSize.width += m_spScaleBox->GetTextBoxPropertyPtr()->Padding->top
		+ m_spPageBox->GetTextBoxPropertyPtr()->Padding->bottom
		+ m_spPageBox->GetTextBoxPropertyPtr()->Line->Width;
	pageSize.height += m_spScaleBox->GetTextBoxPropertyPtr()->Padding->left
		+ m_spPageBox->GetTextBoxPropertyPtr()->Padding->right
		+ m_spPageBox->GetTextBoxPropertyPtr()->Line->Width;

	CSizeF totalPageSize = m_spTotalPageBlock->MeasureSize();

	CSizeF scaleSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(
		*(m_spScaleBox->GetTextBoxPropertyPtr()->Format), L"000.0");
	scaleSize.width += m_spScaleBox->GetTextBoxPropertyPtr()->Padding->top
		+ m_spScaleBox->GetTextBoxPropertyPtr()->Padding->bottom
		+ m_spScaleBox->GetTextBoxPropertyPtr()->Line->Width;
	scaleSize.height += m_spScaleBox->GetTextBoxPropertyPtr()->Padding->left
		+ m_spScaleBox->GetTextBoxPropertyPtr()->Padding->right
		+ m_spScaleBox->GetTextBoxPropertyPtr()->Line->Width;

	CSizeF percentSize = m_spPercentBlock->MeasureSize();

	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;


	CRectF rcPercent(rcClient.right - percentSize.width, rcClient.top, rcClient.right, rcClient.top + percentSize.height);
	CRectF rcScale(rcPercent.left - scaleSize.width -2.f, rcClient.top, rcPercent.left -2.f, rcClient.top + scaleSize.height);

	CRectF rcTotalPage(rcScale.left - totalPageSize.width -2.f, rcClient.top, rcScale.left -2.f, rcClient.top + totalPageSize.height);
	CRectF rcPage(rcTotalPage.left - pageSize.width -2.f, rcClient.top, rcTotalPage.left -2.f, rcClient.top + pageSize.height);
	
	CRectF rcFilter(rcClient.left, rcClient.top, rcPage.left - 2.f, rcClient.top + filterHeight);
	CRectF rcPDF(rcClient.left, rcClient.top + filterHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcPDF.bottom, rcClient.right, rcClient.bottom);

	return { rcFilter, rcPage, rcTotalPage, rcScale, rcPercent, rcPDF, rcStatus };
}


void CPDFEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcFilter, rcPage, rcTotalPage, rcScale, rcPercent, rcPDF, rcStatus] = GetRects();
	m_spFilterBox->OnCreate(CreateEvt(GetWndPtr(), this, rcFilter));
	m_spPageBox->OnCreate(CreateEvt(GetWndPtr(), this, rcPage));
	m_spTotalPageBlock->OnCreate(CreateEvt(GetWndPtr(), this, rcTotalPage));
	m_spScaleBox->OnCreate(CreateEvt(GetWndPtr(), this, rcScale));
	m_spPercentBlock->OnCreate(CreateEvt(GetWndPtr(), this, rcPercent));
	m_spPDFView->OnCreate(CreateEvt(GetWndPtr(), this, rcPDF));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));

	m_spFilterBox->SetIsTabStop(true);
	m_spPageBox->SetIsTabStop(true);
	m_spScaleBox->SetIsTabStop(true);
	m_spPDFView->SetIsTabStop(true);

	auto ratio_to_percent = [](const FLOAT& value)->std::wstring {
		return std::format(L"{:.1f}", std::round(value * 1000.f) / 1000.f * 100.f).c_str();
	};

	auto percent_to_ratio = [](const std::wstring& percent)->FLOAT {
		wchar_t* stopstring;
		return std::round(std::wcstof(percent.c_str(), &stopstring) * 1000.f) / 1000.f / 100.f;
	};

	m_spScaleBox->GetText().set(ratio_to_percent(m_spPDFView->GetScale().get()));
	m_spPDFView->GetScale().Subscribe(
	[&](const FLOAT& ratio)->void{
		std::wstring percent = ratio_to_percent(ratio);
		if (percent != m_spScaleBox->GetText().get()) {
			m_spScaleBox->GetText().set(percent);
		}
	});

	m_spScaleBox->GetEnterText().Subscribe(
	[&](const NotifyStringChangedEventArgs<wchar_t>& notify)->void {
		FLOAT ratio = percent_to_ratio(m_spScaleBox->GetText().get());
		if (ratio != m_spPDFView->GetScale().get()) {
			//Validate
			if(ratio == 0.f || m_spPDFView->GetMinScale() > ratio || m_spPDFView->GetMaxScale() < ratio){ 
				m_spScaleBox->GetText().set(ratio_to_percent(m_spPDFView->GetScale().get()));
			} else {
				m_spPDFView->GetScale().set(ratio);
			}
		}
	});
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
	auto [rcFilter, rcPage, rcTotalPage, rcScale, rcPercent, rcPDF, rcStatus] = GetRects();
	m_spFilterBox->OnRect(RectEvent(GetWndPtr(), rcFilter));
	m_spPageBox->OnRect(RectEvent(GetWndPtr(), rcPage));
	m_spTotalPageBlock->OnRect(RectEvent(GetWndPtr(), rcTotalPage));
	m_spScaleBox->OnRect(RectEvent(GetWndPtr(), rcScale));
	m_spPercentBlock->OnRect(RectEvent(GetWndPtr(), rcPercent));
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
	m_spPDFView->GetDocPtr()->Save();
}

void CPDFEditor::SaveAs()
{
	//m_save_as.Execute(GetWndPtr()->m_hWnd);
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	ofn.lpstrFilter = L"PDF file(*.pdf)\0*.pdf\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Save as";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"pdf";

	if (!GetSaveFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			//throw std::exception(FILE_LINE_FUNC);
		}
	} else {
		::ReleaseBuffer(path);
		bool same = path == m_spPDFView->GetDocPtr()->GetPath();
		if (!same) {
			m_spPDFView->GetDocPtr()->SaveWithVersion(path, 0, m_spPDFView->GetDocPtr()->GetFileVersion());
			m_spPDFView->Open(path);
		} else {
			m_spPDFView->GetDocPtr()->Save();
			/*auto doc(m_spPDFView->GetDocPtr()->Clone());
			m_spPDFView->GetDocPtr().reset();
			doc.SaveAsCopy(path, 0);*/
		}
	}
}

void CPDFEditor::Update()
{
	m_spFilterBox->UpdateAll();
	//m_spPDFView->UpdateAll();
}

