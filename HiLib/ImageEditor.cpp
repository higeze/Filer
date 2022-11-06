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
	m_spStatusBar(std::make_shared<CStatusBar>(this, spProp->StatusBarPropPtr))
{
	m_spStatusBar->GetIsFocusable().set(false);
}

std::tuple<CRectF, CRectF> CImageEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;
	CRectF rcImage(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcImage.bottom, rcClient.right, rcClient.bottom);

	return { rcImage, rcStatus };
}


void CImageEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcImage, rcStatus] = GetRects();
	m_spImageView->OnCreate(CreateEvt(GetWndPtr(), this, rcImage));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
	m_spImageView->SetIsTabStop(true);
}

void CImageEditor::OnPaint(const PaintEvent& e)
{
	m_spImageView->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CImageEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcImage, rcStatus] = GetRects();
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

