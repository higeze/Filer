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

#include "reactive_property.h"
#include "reactive_binding.h"

/***********/
/* Globals */
/***********/
auto ratio_to_percent = [](const FLOAT& value)->std::wstring {
	return std::format(L"{:.1f}", std::round(value * 1000.f) / 1000.f * 100.f).c_str();
};

auto percent_to_ratio = [](const std::wstring& percent)->FLOAT {
	wchar_t* stopstring;
	return std::round(std::wcstof(percent.c_str(), &stopstring) * 1000.f) / 1000.f / 100.f;
};

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
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr))
{
	m_spFilterBox->SetIsScrollable(false); 
	m_spPercentBlock->Text.set(L"%");
	m_spStatusBar->IsFocusable.set(false);
}

std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> CPDFEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT filterHeight = m_spFilterBox->MeasureSize(L"").height;
	CSizeF pageSize = m_spPageBox->MeasureSize(L"000");
	CSizeF totalPageSize = m_spTotalPageBlock->MeasureSize(L"000");
	CSizeF scaleSize = m_spScaleBox->MeasureSize(L"000.0");
	CSizeF percentSize = m_spPercentBlock->MeasureSize();
	FLOAT statusHeight = m_spStatusBar->MeasureSize(L"").height;

	FLOAT maxHeight = (std::max)({filterHeight, pageSize.height, totalPageSize.height, scaleSize.height, percentSize.height});

	CRectF rcPercent(rcClient.right - percentSize.width, 
		rcClient.top + (maxHeight - percentSize.height)*0.5f, 
		rcClient.right, 
		rcClient.top + (maxHeight - percentSize.height)*0.5f+ percentSize.height);
	CRectF rcScale(rcPercent.left - scaleSize.width -2.f,
		rcClient.top + (maxHeight - scaleSize.height)*0.5f,
		rcPercent.left -2.f,
		rcClient.top + (maxHeight - scaleSize.height)*0.5f + scaleSize.height);

	CRectF rcTotalPage(rcScale.left - totalPageSize.width -2.f,
		rcClient.top + (maxHeight - totalPageSize.height)*0.5f, 
		rcScale.left -2.f,
		rcClient.top + (maxHeight - totalPageSize.height)*0.5f + totalPageSize.height);
	CRectF rcPage(rcTotalPage.left - pageSize.width -2.f,
		rcClient.top + (maxHeight - pageSize.height)*0.5f,
		rcTotalPage.left -2.f, 
		rcClient.top +(maxHeight - pageSize.height)*0.5f+ pageSize.height);
	
	CRectF rcFilter(rcClient.left,
		rcClient.top + (maxHeight - filterHeight)*0.5f, 
		rcPage.left - 2.f, 
		rcClient.top +(maxHeight - filterHeight)*0.5f + filterHeight);

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

	m_spFilterBox->SetIsEnterText(true);
	m_spScaleBox->SetIsEnterText(true);
	m_spPageBox->SetIsEnterText(true);

	m_spFilterBox->SetIsTabStop(true);
	m_spPageBox->SetIsTabStop(true);
	m_spScaleBox->SetIsTabStop(true);
	m_spPDFView->SetIsTabStop(true);

	//Bindings
	m_spScaleBox->Text.set(ratio_to_percent(*m_spPDFView->Scale));

	m_spPDFView->Scale.subscribe([this](const FLOAT& ratio)
	{
		std::wstring percent = ratio_to_percent(ratio);
		if (percent != *m_spScaleBox->Text) {
			m_spScaleBox->Text.set(percent);
		}
	}, shared_from_this());
	m_spScaleBox->EnterText.subscribe([this](auto notify)
	{
		FLOAT ratio = percent_to_ratio(*m_spScaleBox->EnterText);
		if (ratio != *m_spPDFView->Scale) {
			//Validate
			if (ratio == 0.f || m_spPDFView->GetMinScale() > ratio || m_spPDFView->GetMaxScale() < ratio) {
				m_spScaleBox->Text.set(ratio_to_percent(*m_spPDFView->Scale));
			} else {
				m_spPDFView->Scale.set(ratio);
			}
		}
	}, shared_from_this());

	m_spPDFView->CurrentPage.subscribe([this](const int& value)
	{
		wchar_t* endptr = nullptr;
		int page = std::wcstol(m_spPageBox->EnterText->c_str(), &endptr, 10);
		if (page != value) {
			m_spPageBox->Text.set(std::to_wstring(value));
			m_spPageBox->EnterText.set(std::to_wstring(value));
		}
	}, shared_from_this());
	m_spPageBox->EnterText.subscribe([this](auto notify)
	{
		wchar_t* endptr = nullptr;
		int pageAtBox = std::wcstol(m_spPageBox->EnterText->c_str(), &endptr, 10);
		int pageAtView = *m_spPDFView->CurrentPage;
		if (pageAtBox != pageAtView) {
			if (!m_spPDFView->Jump(pageAtBox)) {
				m_spPageBox->Text.set(std::to_wstring(*m_spPDFView->CurrentPage));
				m_spPageBox->EnterText.set(std::to_wstring(*m_spPDFView->CurrentPage));
			}
		}
	}, shared_from_this());

	m_spPDFView->TotalPage.binding(m_spTotalPageBlock->Text);
	m_spFilterBox->Text.binding(m_spPDFView->Find);
}

void CPDFEditor::OnPaint(const PaintEvent& e)
{
	m_spFilterBox->OnPaint(e);
	m_spPageBox->OnPaint(e);
	m_spTotalPageBlock->OnPaint(e);
	m_spScaleBox->OnPaint(e);
	m_spPercentBlock->OnPaint(e);
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
		case 'F':
			if (ctrl) {
				SetFocusedControlPtr(m_spFilterBox);
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
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CPDFEditor::Open()
{
	OpenCommand.execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::OpenAs()
{
	OpenAsCommand.execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::Save()
{
	m_spPDFView->PDF.get_unconst()->Save(*m_spPDFView->PDF->Path);
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
		bool same = path == *m_spPDFView->PDF->Path;
		if (!same) {
			m_spPDFView->PDF.get_unconst()->SaveWithVersion(path, 0, m_spPDFView->PDF->GetFileVersion());
			m_spPDFView->Open(path);
		} else {
			Save();
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

