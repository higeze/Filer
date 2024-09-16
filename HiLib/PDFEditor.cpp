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
#include "PDFBitmapDrawer.h"

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

CPDFEditor::CPDFEditor(CD2DWControl* pParentControl)
	:CDockPanel(pParentControl),
	m_spFilterBox(std::make_shared<CTextBox>(this, L"")),
	m_spFilterCountBlock(std::make_shared<CTextBlock>(this)),
	m_spPageBox(std::make_shared<CTextBox>(this, L"")),
	m_spTotalPageBlock(std::make_shared<CTextBlock>(this)),
	m_spScaleBox(std::make_shared<CTextBox>(this, L"")),
	m_spPercentBlock(std::make_shared<CTextBlock>(this)),
	m_spPDFView(std::make_shared<CPdfView>(this)),
	m_spStatusBar(std::make_shared<CStatusBar>(this))
{
	using pr = std::pair<std::shared_ptr<CD2DWControl>, DockEnum>;

	m_spFilterBox->SetIsEnterText(true);
	m_spScaleBox->SetIsEnterText(true);
	m_spPageBox->SetIsEnterText(true);

	m_spFilterBox->SetIsTabStop(true);
	m_spPageBox->SetIsTabStop(true);
	m_spScaleBox->SetIsTabStop(true);
	m_spPDFView->SetIsTabStop(true);

	m_spFilterBox->SetIsScrollable(false);
	m_spStatusBar->IsFocusable.set(false);

	/*******/
	/* Top */
	/*******/
	auto spTopDock = std::make_shared<CDockPanel>(this);
	spTopDock->Add(
		pr(m_spTotalPageBlock, DockEnum::Right),
		pr(m_spPageBox, DockEnum::Right),
		pr(m_spPercentBlock, DockEnum::Right),
		pr(m_spScaleBox, DockEnum::Right),
		pr(m_spFilterCountBlock, DockEnum::Right),
		pr(m_spFilterBox, DockEnum::Fill)
	);

	/********/
	/* Dock */
	/********/
	this->Add(
		pr(spTopDock, DockEnum::Top),
		pr(m_spStatusBar, DockEnum::Bottom),
		pr(m_spPDFView, DockEnum::Fill)
	);

	/***********/
	/* Binding */
	/***********/
	m_spScaleBox->Text.set(ratio_to_percent(*m_spPDFView->Scale));
	m_spPercentBlock->Text.set(L"%");
	m_spPDFView->Scale.subscribe([this](const FLOAT& ratio)
		{
			std::wstring percent = ratio_to_percent(ratio);
			if (percent != *m_spScaleBox->Text) {
				m_spScaleBox->Text.set(percent);
			}
		}, Life);
	m_spScaleBox->EnterText.subscribe([this](auto notify)
		{
			FLOAT ratio = percent_to_ratio(*m_spScaleBox->EnterText);
			if (ratio != *m_spPDFView->Scale) {
				//Validate
				if (ratio == 0.f || m_spPDFView->GetMinScale() > ratio || m_spPDFView->GetMaxScale() < ratio) {
					m_spScaleBox->Text.set(ratio_to_percent(*m_spPDFView->Scale));
				}
				else {
					m_spPDFView->Scale.set(ratio);
				}
			}
		}, Life);

	m_spPDFView->CurrentPage.subscribe([this](const int& value)
		{
			wchar_t* endptr = nullptr;
			int page = std::wcstol(m_spPageBox->EnterText->c_str(), &endptr, 10);
			if (page != value) {
				m_spPageBox->Text.set(std::to_wstring(value));
				m_spPageBox->EnterText.set(std::to_wstring(value));
			}
		}, Life);
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
		}, Life);

	m_spPDFView->TotalPage.binding(m_spTotalPageBlock->Text);
	m_spFilterBox->Text.binding(m_spPDFView->Find);
	m_spPDFView->FindCount.subscribe([this](auto value) { m_spFilterCountBlock->Text.set(std::to_wstring(value)); }, Life);











}
//
//std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> CPDFEditor::GetRects() const
//{
//	CRectF rcClient = GetRectInWnd();
//
//	FLOAT filterHeight = m_spFilterBox->MeasureSize(L"").height;
//	CSizeF pageSize = m_spPageBox->MeasureSize(L"000");
//	CSizeF totalPageSize = m_spTotalPageBlock->MeasureSize(L"000");
//	CSizeF scaleSize = m_spScaleBox->MeasureSize(L"000.0");
//	CSizeF percentSize = m_spPercentBlock->MeasureSize();
//	FLOAT statusHeight = m_spStatusBar->MeasureSize(L"").height;
//
//	FLOAT maxHeight = (std::max)({filterHeight, pageSize.height, totalPageSize.height, scaleSize.height, percentSize.height});
//
//	CRectF rcPercent(rcClient.right - percentSize.width, 
//		rcClient.top + (maxHeight - percentSize.height)*0.5f, 
//		rcClient.right, 
//		rcClient.top + (maxHeight - percentSize.height)*0.5f+ percentSize.height);
//	CRectF rcScale(rcPercent.left - scaleSize.width -2.f,
//		rcClient.top + (maxHeight - scaleSize.height)*0.5f,
//		rcPercent.left -2.f,
//		rcClient.top + (maxHeight - scaleSize.height)*0.5f + scaleSize.height);
//
//	CRectF rcTotalPage(rcScale.left - totalPageSize.width -2.f,
//		rcClient.top + (maxHeight - totalPageSize.height)*0.5f, 
//		rcScale.left -2.f,
//		rcClient.top + (maxHeight - totalPageSize.height)*0.5f + totalPageSize.height);
//	CRectF rcPage(rcTotalPage.left - pageSize.width -2.f,
//		rcClient.top + (maxHeight - pageSize.height)*0.5f,
//		rcTotalPage.left -2.f, 
//		rcClient.top +(maxHeight - pageSize.height)*0.5f+ pageSize.height);
//	
//	CRectF rcFilter(rcClient.left,
//		rcClient.top + (maxHeight - filterHeight)*0.5f, 
//		rcPage.left - 2.f, 
//		rcClient.top +(maxHeight - filterHeight)*0.5f + filterHeight);
//
//	CRectF rcPDF(rcClient.left, rcClient.top + filterHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
//	CRectF rcStatus(rcClient.left, rcPDF.bottom, rcClient.right, rcClient.bottom);
//
//	return { rcFilter, rcPage, rcTotalPage, rcScale, rcPercent, rcPDF, rcStatus };
//}


void CPDFEditor::OnCreate(const CreateEvt& e)
{
	CDockPanel::OnCreate(e);
	//CD2DWControl::OnCreate(e);
	////auto [rcFilter, rcrcPage, rcTotalPage, rcScale, rcPercent, rcPDF, rcStatus] = GetRects();
	//m_spFilterBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spFilterCountBlock->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spPageBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spTotalPageBlock->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spScaleBox->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spPercentBlock->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spPDFView->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	//m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));


	////Bindings
	//m_spScaleBox->Text.set(ratio_to_percent(*m_spPDFView->Scale));

	//m_spPDFView->Scale.subscribe([this](const FLOAT& ratio)
	//{
	//	std::wstring percent = ratio_to_percent(ratio);
	//	if (percent != *m_spScaleBox->Text) {
	//		m_spScaleBox->Text.set(percent);
	//	}
	//}, shared_from_this());
	//m_spScaleBox->EnterText.subscribe([this](auto notify)
	//{
	//	FLOAT ratio = percent_to_ratio(*m_spScaleBox->EnterText);
	//	if (ratio != *m_spPDFView->Scale) {
	//		//Validate
	//		if (ratio == 0.f || m_spPDFView->GetMinScale() > ratio || m_spPDFView->GetMaxScale() < ratio) {
	//			m_spScaleBox->Text.set(ratio_to_percent(*m_spPDFView->Scale));
	//		} else {
	//			m_spPDFView->Scale.set(ratio);
	//		}
	//	}
	//}, shared_from_this());

	//m_spPDFView->CurrentPage.subscribe([this](const int& value)
	//{
	//	wchar_t* endptr = nullptr;
	//	int page = std::wcstol(m_spPageBox->EnterText->c_str(), &endptr, 10);
	//	if (page != value) {
	//		m_spPageBox->Text.set(std::to_wstring(value));
	//		m_spPageBox->EnterText.set(std::to_wstring(value));
	//	}
	//}, shared_from_this());
	//m_spPageBox->EnterText.subscribe([this](auto notify)
	//{
	//	wchar_t* endptr = nullptr;
	//	int pageAtBox = std::wcstol(m_spPageBox->EnterText->c_str(), &endptr, 10);
	//	int pageAtView = *m_spPDFView->CurrentPage;
	//	if (pageAtBox != pageAtView) {
	//		if (!m_spPDFView->Jump(pageAtBox)) {
	//			m_spPageBox->Text.set(std::to_wstring(*m_spPDFView->CurrentPage));
	//			m_spPageBox->EnterText.set(std::to_wstring(*m_spPDFView->CurrentPage));
	//		}
	//	}
	//}, shared_from_this());

	//m_spPDFView->TotalPage.binding(m_spTotalPageBlock->Text);
	//m_spFilterBox->Text.binding(m_spPDFView->Find);
}

void CPDFEditor::OnPaint(const PaintEvent& e)
{
	CDockPanel::OnPaint(e);
	/*m_spFilterBox->OnPaint(e);
	m_spPageBox->OnPaint(e);
	m_spTotalPageBlock->OnPaint(e);
	m_spScaleBox->OnPaint(e);
	m_spPercentBlock->OnPaint(e);
	m_spPDFView->OnPaint(e);
	m_spStatusBar->OnPaint(e);*/
}

void CPDFEditor::Measure(const CSizeF& availableSize)
{
	CDockPanel::Measure(availableSize);
}
void CPDFEditor::Arrange(const CRectF& rc)
{
	CDockPanel::Arrange(rc);

	//CD2DWControl::Arrange(e);

	//CRectF rcClient = GetRectInWnd();

	//FLOAT filterHeight = m_spFilterBox->MeasureSize(L"").height;
	//CSizeF pageSize = m_spPageBox->MeasureSize(L"000");
	//CSizeF totalPageSize = m_spTotalPageBlock->MeasureSize(L"000");
	//CSizeF scaleSize = m_spScaleBox->MeasureSize(L"000.0");
	//CSizeF percentSize = m_spPercentBlock->MeasureSize();
	//FLOAT statusHeight = m_spStatusBar->MeasureSize(L"").height;

	//FLOAT maxHeight = (std::max)({ filterHeight, pageSize.height, totalPageSize.height, scaleSize.height, percentSize.height });

	//CRectF rcPercent(rcClient.right - percentSize.width,
	//	rcClient.top + (maxHeight - percentSize.height) * 0.5f,
	//	rcClient.right,
	//	rcClient.top + (maxHeight - percentSize.height) * 0.5f + percentSize.height);
	//CRectF rcScale(rcPercent.left - scaleSize.width - 2.f,
	//	rcClient.top + (maxHeight - scaleSize.height) * 0.5f,
	//	rcPercent.left - 2.f,
	//	rcClient.top + (maxHeight - scaleSize.height) * 0.5f + scaleSize.height);

	//CRectF rcTotalPage(rcScale.left - totalPageSize.width - 2.f,
	//	rcClient.top + (maxHeight - totalPageSize.height) * 0.5f,
	//	rcScale.left - 2.f,
	//	rcClient.top + (maxHeight - totalPageSize.height) * 0.5f + totalPageSize.height);
	//CRectF rcPage(rcTotalPage.left - pageSize.width - 2.f,
	//	rcClient.top + (maxHeight - pageSize.height) * 0.5f,
	//	rcTotalPage.left - 2.f,
	//	rcClient.top + (maxHeight - pageSize.height) * 0.5f + pageSize.height);

	//CRectF rcFilter(rcClient.left,
	//	rcClient.top + (maxHeight - filterHeight) * 0.5f,
	//	rcPage.left - 2.f,
	//	rcClient.top + (maxHeight - filterHeight) * 0.5f + filterHeight);

	//CRectF rcPDF(rcClient.left, rcClient.top + filterHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	//CRectF rcStatus(rcClient.left, rcPDF.bottom, rcClient.right, rcClient.bottom);

	//m_spFilterBox->Arrange(rcFilter);
	//m_spPageBox->Arrange(rcPage);
	//m_spTotalPageBlock->Arrange(rcTotalPage);
	//m_spScaleBox->Arrange(rcScale);
	//m_spPercentBlock->Arrange(rcPercent);
	//m_spPDFView->Arrange(rcPDF);
	//m_spStatusBar->Arrange(rcStatus);
	////m_spTextBox->UpdateAll();
}

void CPDFEditor::OnKeyDown(const KeyDownEvent& e)
{
	bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	bool shift = ::GetAsyncKeyState(VK_SHIFT);
	switch (e.Char) {
		case 'F':
			if (ctrl) {
				GetWndPtr()->SetFocusToControl(m_spFilterBox);
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
	m_spPDFView->GetPDFDrawer()->WaitAll();
	OpenCommand.execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::OpenAs()
{
	m_spPDFView->GetPDFDrawer()->WaitAll();
	OpenAsCommand.execute(GetWndPtr()->m_hWnd);
}

void CPDFEditor::Save()
{
	m_spPDFView->GetPDFDrawer()->WaitAll();
	m_spPDFView->PDF.get_unconst()->Save();
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
			m_spPDFView->PDF.get_unconst()->SaveAs(path, m_spPDFView->PDF->GetFileVersion(), false);
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

