#include "ImageEditor.h"
#include "ImageEditorProperty.h"
#include "ImageView.h"
#include "StatusBar.h"
#include "D2DImage.h"
#include "ImageEditorProperty.h"

#include <regex>
#include <nameof/nameof.hpp>

#include "ReactiveProperty.h"


/***************/
/* CImageEditor */
/***************/

CImageEditor::CImageEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<ImageEditorProperty>& spProp)
	:CD2DWControl(pParentControl),
	m_spProp(spProp),
	m_spImageView(std::make_shared<CImageView>(this, spProp->ImageViewPropPtr)),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr)),
	m_spScaleBox(std::make_shared<CTextBox>(this, spProp->TextBoxPropPtr, L"0"))//,
	//m_bindScaleText(m_spScaleBox->GetEnterText(), m_spImageView->GetScale())
{


	//m_spStatusBar->GetIsFocusable().set(false);
}

std::tuple<CRectF, CRectF, CRectF> CImageEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT scaleHeight = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spScaleBox->GetTextBoxPropertyPtr()->Format), L"").height
		+ m_spScaleBox->GetTextBoxPropertyPtr()->Padding->top + m_spScaleBox->GetTextBoxPropertyPtr()->Padding->bottom + m_spScaleBox->GetTextBoxPropertyPtr()->Line->Width;

	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;
	CRectF rcScale(rcClient.left, rcClient.top, rcClient.right, rcClient.top + scaleHeight);
	CRectF rcImage(rcClient.left, rcClient.top + scaleHeight + 2.f, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcImage.bottom, rcClient.right, rcClient.bottom);

	return { rcScale, rcImage, rcStatus };
}


void CImageEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcScale, rcImage, rcStatus] = GetRects();
	m_spScaleBox->OnCreate(CreateEvt(GetWndPtr(), this, rcScale));
	m_spImageView->OnCreate(CreateEvt(GetWndPtr(), this, rcImage));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
	m_spImageView->SetIsTabStop(true);

	m_spScaleBox->SetIsEnterText(true);

	auto ratio_to_percent = [](const FLOAT& value)->std::wstring {
		return std::format(L"{:.1f}", std::round(value * 1000.f) / 1000.f * 100.f).c_str();
	};

	auto percent_to_ratio = [](const std::wstring& percent)->FLOAT {
		wchar_t* stopstring;
		return std::round(std::wcstof(percent.c_str(), &stopstring) * 1000.f) / 1000.f / 100.f;
	};

	m_spScaleBox->GetText().set(ratio_to_percent(m_spImageView->GetScale().get()));
	m_spImageView->GetScale().Subscribe(
	[&](const FLOAT& ratio)->void{
		std::wstring percent = ratio_to_percent(ratio);
		if (percent != m_spScaleBox->GetText().get()) {
			m_spScaleBox->GetText().set(percent);
		}
	});

	m_spScaleBox->GetEnterText().Subscribe(
	[&](const NotifyStringChangedEventArgs<wchar_t>& notify)->void {
		FLOAT ratio = percent_to_ratio(m_spScaleBox->GetText().get());
		if (ratio != m_spImageView->GetScale().get()) {
			//Validate
			if(ratio == 0.f || m_spImageView->GetMinScale() > ratio || m_spImageView->GetMaxScale() < ratio){ 
				m_spScaleBox->GetText().set(ratio_to_percent(m_spImageView->GetScale().get()));
			} else {
				m_spImageView->GetScale().set(ratio);
			}
		}
	});


}

void CImageEditor::OnPaint(const PaintEvent& e)
{
	m_spScaleBox->OnPaint(e);
	m_spImageView->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CImageEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcScale, rcImage, rcStatus] = GetRects();
	m_spScaleBox->OnRect(RectEvent(GetWndPtr(), rcScale));
	m_spImageView->OnRect(RectEvent(GetWndPtr(), rcImage));
	m_spStatusBar->OnRect(RectEvent(GetWndPtr(), rcStatus));
}

void CImageEditor::OnKeyDown(const KeyDownEvent& e)
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

void CImageEditor::Open()
{
	m_open.Execute(GetWndPtr()->m_hWnd);
}

void CImageEditor::OpenAs()
{
	m_open_as.Execute(GetWndPtr()->m_hWnd);
}

void CImageEditor::Save()
{
	//m_spPDFView->GetDocPtr()->Save();
}

void CImageEditor::SaveAs()
{
	////m_save_as.Execute(GetWndPtr()->m_hWnd);
	//std::wstring path;
	//OPENFILENAME ofn = { 0 };
	//ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//ofn.lpstrFilter = L"PDF file(*.pdf)\0*.pdf\0\0";
	//ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	//ofn.nMaxFile = MAX_PATH;
	//ofn.lpstrTitle = L"Save as";
	//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	//ofn.lpstrDefExt = L"pdf";

	//if (!GetSaveFileName(&ofn)) {
	//	DWORD errCode = CommDlgExtendedError();
	//	if (errCode) {
	//		//throw std::exception(FILE_LINE_FUNC);
	//	}
	//} else {
	//	::ReleaseBuffer(path);
	//	bool same = path == m_spPDFView->GetDocPtr()->GetPath();
	//	if (!same) {
	//		m_spPDFView->GetDocPtr()->SaveWithVersion(path, 0, m_spPDFView->GetDocPtr()->GetFileVersion());
	//		m_spPDFView->Open(path);
	//	} else {
	//		m_spPDFView->GetDocPtr()->Save();
	//		/*auto doc(m_spPDFView->GetDocPtr()->Clone());
	//		m_spPDFView->GetDocPtr().reset();
	//		doc.SaveAsCopy(path, 0);*/
	//	}
	//}
}

void CImageEditor::Update()
{
}

