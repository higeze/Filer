#include "ImageView.h"
#include "D2DWWindow.h"
#include "Scroll.h"
#include "Debug.h"
#include "MyMenu.h"
#include "ResourceIDFactory.h"
#include <functional>
#include <ranges>
#include "HiLibResource.h"
#include "PDFViewport.h"
#include "PdfViewStateMachine.h"
#include "TextBoxDialog.h"
#include "DialogProperty.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Button.h"
#include "ButtonProperty.h"
#include "strconv.h"
#include "Dispatcher.h"
#include "D2DImage.h"
#include "ImageDrawer.h"
#include "D2DWTypes.h"


/**************************/
/* Constructor/Destructor */
/**************************/

CImageView::CImageView(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl),
	m_pMachine(std::make_unique<CImageViewStateMachine>(this)),
	m_spVScroll(std::make_shared<CVScroll>(this)),
	m_spHScroll(std::make_shared<CHScroll>(this)),
	Rotate(D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT), m_prevScale(0.f), m_initialScaleMode(ImageScaleMode::Width),
	Dummy(std::make_shared<int>(0)), Image(CD2DImage()), m_imgDrawer(std::make_unique<CImageDrawer>()), Scale(1.f)
{
	Image.subscribe([this](auto doc) {
		Open(doc.GetPath());
	}, Dummy);

	AddChildControlPtr(m_spVScroll);
	AddChildControlPtr(m_spHScroll);
}

CImageView::~CImageView() = default;

void CImageView::Open(const std::wstring& path)
{
	if (::PathFileExists(path.c_str())) {

		Close();

		m_pFileIsInUse = CFileIsInUseImpl::CreateInstance(GetWndPtr()->m_hWnd, path.c_str(), FUT_DEFAULT, OF_CAP_DEFAULT);
		GetWndPtr()->AddMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, [this](UINT, LPARAM, WPARAM, BOOL&)->LRESULT
		{
			Close();
			return 0;
		});

		Image.get_unconst()->Open(path);

		if (*Scale < 0 && Image->IsValid()) {// < 0 means auto-scale
			CSizeF sz = Image->GetSizeF();

			FLOAT scaleX = GetRenderSize().width / sz.width;
			FLOAT scaleY = GetRenderSize().height / sz.height;
			switch (m_initialScaleMode) {
				case ImageScaleMode::MinWidthHeight:
					Scale.set((std::min)(scaleX, scaleY));
					break;
				case ImageScaleMode::Width:
					Scale.set(scaleX);
					break;
				case ImageScaleMode::Height:
					Scale.set(scaleY);
					break;
				default:
					Scale.set(1.f);
			}
		} else if (*Scale < 0) {
			auto b = 1.f;
		}
	}
}
void CImageView::Close()
{
	m_imgDrawer->WaitAll();
	m_imgDrawer->Clear();

	GetWndPtr()->RemoveMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE);
	m_pFileIsInUse.Release();
	Image.get_unconst()->Close();
}

/****************/
/* EventHandler */
/****************/
//
//void CImageView::OnCreate(const CreateEvt& e)
//{
//	CD2DWControl::OnCreate(e);
//	auto [rcVertical, rcHorizontal] = GetRects();
//	m_spVScroll->OnCreate(CreateEvt(GetWndPtr(), this, rcVertical));
//	m_spHScroll->OnCreate(CreateEvt(GetWndPtr(), this, rcHorizontal));
//}

CRectF CImageView::GetRenderRectInWnd()
{
    CRectF rc = GetRectInWnd();
	rc.DeflateRect(GetFocusedBorder().Width * 0.5f);
	rc.DeflateRect(GetPadding());
	return rc;
}

CSizeF CImageView::GetRenderSize()
{
	return GetRenderRectInWnd().Size();
}

CSizeF CImageView::GetRenderContentSize()
{
	if (Image->IsValid()) {
		CSizeF sz = Image->GetSizeF();
		sz.width *= *Scale;
		sz.height *= *Scale;
		return sz;
	} else {
		return CSizeF(0.f, 0.f);
	}
};

void CImageView::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CImageView::OnDestroy(const DestroyEvent& e)
{
	CD2DWControl::OnDestroy(e);
}

void CImageView::OnEnable(const EnableEvent& e)
{
	if (e.Enable) {

	} else {
		Close();
	}
}

void CImageView::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	UpdateScroll();
}

void CImageView::OnMouseWheel(const MouseWheelEvent& e)
{
	if(::GetAsyncKeyState(VK_CONTROL)){
		FLOAT factor = static_cast<FLOAT>(std::pow(1.1f, (std::abs(e.Delta) / WHEEL_DELTA)));
		FLOAT multiply = (e.Delta > 0) ? factor : 1/factor;
		FLOAT prevScale = *Scale;
		Scale.set(std::clamp(*Scale * multiply, GetMinScale(), GetMaxScale()));
		m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() * *Scale / prevScale/* + m_spVScroll->GetScrollPage() / 2.f *(m_scale / prevScale - 1.f)*/);
		m_spHScroll->SetScrollPos(m_spHScroll->GetScrollPos() * *Scale / prevScale/*+ m_spHScroll->GetScrollPage() / 2.f *(m_scale / prevScale - 1.f)*/);

	} else {
		m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() - m_spVScroll->GetDeltaScroll() * e.Delta / WHEEL_DELTA);
	}
}

/**********/
/* Normal */
/**********/

void CImageView::Normal_LButtonDown(const LButtonDownEvent& e)
{
}

void CImageView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
}

void CImageView::Normal_Paint(const PaintEvent& e)
{
	if (!Image->IsValid()) { return; }
	//Clip
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());

	//PaintContent
	auto callback = [this]()->void { GetWndPtr()->GetDispatcherPtr()->PostInvoke([pWnd = GetWndPtr()]() { pWnd->InvalidateRect(NULL, FALSE); }); };

	CSizeF szImage(Image->GetSizeF());
	CRectF rcInWnd(GetRenderRectInWnd());
	CRectF rcFullInPage(Image->GetSizeF());
	CRectF rcScaledFullInPage(rcFullInPage * *Scale);

	CRectF rcScaledClipInPage(rcScaledFullInPage.IntersectRect(
		CRectF(m_spHScroll->GetScrollPos(), m_spVScroll->GetScrollPos(), m_spHScroll->GetScrollPos() + rcInWnd.Width(), m_spVScroll->GetScrollPos() + rcInWnd.Height())));
	CRectF rcClipInPage(rcScaledClipInPage / *Scale);

	CSizeU szBitmap(m_imgDrawer->GetPrimaryBitmapSize());

	CPointF ptDstClipInWnd(rcInWnd.LeftTop());
	CPointF ptDstLeftTopInWnd(rcInWnd.LeftTop() - CPointF(m_spHScroll->GetScrollPos(), m_spVScroll->GetScrollPos()));

	CRectF rcDstInWnd(
		std::round(ptDstLeftTopInWnd.x),
		std::round(ptDstLeftTopInWnd.y),
		std::round(ptDstLeftTopInWnd.x) + std::round(rcScaledFullInPage.Width()),
		std::round(ptDstLeftTopInWnd.y) + std::round(rcScaledFullInPage.Height()));
	CRectF rcDstClipInWnd(rcDstInWnd.IntersectRect(rcInWnd));

	FLOAT blurScale = ((std::min)(512.f / szImage.width, 512.f / szImage.height));

	bool drawFullPage = (rcScaledFullInPage.Width() * rcScaledFullInPage.Height()) < (szBitmap.width * szBitmap.height / 8);
	
	bool drawOnePageOneScale = *Scale >= 1.f && szImage.width <= szBitmap.width/2 && szImage.height <= szBitmap.height/2;
	bool drawOnePageLessScale = *Scale < 1.f && szImage.width * *Scale <= szBitmap.width/2 && szImage.height * *Scale <= szBitmap.height/2;
	bool drawClipPageOneScale = *Scale >= 1.f && !drawOnePageLessScale && !drawOnePageLessScale;
	bool drawClipPageLessScale = *Scale < 1.f && !drawOnePageLessScale && !drawOnePageLessScale;

	ImgBmpKey blurKey{ .ImagePtr = &(*Image), .Scale = blurScale, .Rotate = 0, .Rect = CRectF2CRectU(CRectF(szImage * blurScale)) };
	ImgBmpKey oneKey{ .ImagePtr = &(*Image), .Scale = 1.f, .Rotate = 0, .Rect = CRectF2CRectU(CRectF(szImage)) };
	ImgBmpKey scaleKey{ .ImagePtr = &(*Image), .Scale = *Scale, .Rotate = 0, .Rect = CRectF2CRectU(rcFullInPage) };
	ImgBmpKey cliponeKey{ .ImagePtr = &(*Image), .Scale = 1.f, .Rotate = 0, .Rect = CRectF2CRectU(rcClipInPage) };
	ImgBmpKey clipscaleKey{ .ImagePtr = &(*Image), .Scale = *Scale, .Rotate = 0, .Rect = CRectF2CRectU(rcScaledClipInPage) };

	if (cliponeKey.Rect.IsRectNull() || clipscaleKey.Rect.IsRectNull()) {
		auto a = 1;
	}

	if (drawOnePageOneScale) {
		if (!m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), oneKey, rcDstInWnd, callback)) {
			m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback);
		}
	} else if (drawOnePageLessScale) {
		if (!m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), scaleKey, ptDstLeftTopInWnd, callback)) {
			m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback);
		}
	} else if (drawClipPageOneScale) {
		if (!m_imgDrawer->DrawClipBitmap(GetWndPtr()->GetDirectPtr(), cliponeKey, rcDstClipInWnd, callback)) {
			m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback);

			//std::vector<ImgBmpKey> keys = m_imgDrawer->FindClipKeys([cliponeKey, pPage = &(m_image.get()), scale = m_scale](const ImgBmpKey& key)->bool{
			//	return 
			//		key != cliponeKey &&
			//		key.ImagePtr->GetPath() == cliponeKey.ImagePtr->GetPath() && 
			//		key.Scale == cliponeKey.Scale &&
			//		key.Rotate == cliponeKey.Rotate &&
			//		!key.Rect.IsRectNull();
			//});
			//for (const ImgBmpKey& key : keys) {
			//	CPointF ptClipInWnd = Ctrl2Wnd(Doc2Ctrl(Page2Doc(i, key.Rect.LeftTop())));
			//	m_imgDrawer->DrawClipBitmap(GetWndPtr()->GetDirectPtr(), key, ptClipInWnd, callback);
			//}
		}
	} else if (drawClipPageLessScale) {
		if (!m_imgDrawer->DrawClipBitmap(GetWndPtr()->GetDirectPtr(), clipscaleKey, ptDstClipInWnd, callback)) {
			m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback);

			std::vector<ImgBmpKey> keys = m_imgDrawer->FindClipKeys([clipscaleKey, pPage = &(*Image), scale = *Scale](const ImgBmpKey& key)->bool{
				return 
					key != clipscaleKey &&
					key.ImagePtr->GetPath() == clipscaleKey.ImagePtr->GetPath() && 
					key.Scale == clipscaleKey.Scale &&
					key.Rotate == clipscaleKey.Rotate &&
					!key.Rect.IsRectNull();
			});
			for (const ImgBmpKey& key : keys) {
				CPointF ptClipInWnd = ptDstClipInWnd + CPointF(m_spHScroll->GetScrollPos(), m_spVScroll->GetScrollPos()) + CPointU2CPointF(key.Rect.LeftTop());
				m_imgDrawer->DrawClipBitmap(GetWndPtr()->GetDirectPtr(), key, ptClipInWnd, callback);
			}
		}
	}

	//if(true){
	//	std::vector<int> spetskvec = m_pdf->GetPDFiumPtr()->GetQueuedSpecificTaskCounts();
	//	std::wstring spetskcnts = std::to_wstring(spetskvec[0]);
	//	for (size_t i = 1; i < spetskvec.size(); i++) {
	//		spetskcnts += L", " + std::to_wstring(spetskvec[i]);
	//	}
	//	std::wstring debugText = std::format(
	//		L"PDF Threads\r\n\tThread:{}/{}\r\n\tTask:{}\r\n\tSpecificTask:{}\r\nThread Pool\r\n\tThread:{}/{}\r\n\tTask:{}\r\n",
	//		m_pdf->GetPDFiumPtr()->GetActiveThreadCount(),m_pdf->GetPDFiumPtr()->GetTotalThreadCount(),
	//		m_pdf->GetPDFiumPtr()->GetQueuedTaskCount(),
	//		spetskcnts,
	//		m_pdfDrawer->GetThreadPoolPtr()->GetActiveThreadCount(),m_pdfDrawer->GetThreadPoolPtr()->GetTotalThreadCount(),
	//		m_pdfDrawer->GetThreadPoolPtr()->GetQueuedTaskCount());
	//	CSizeF debugTextSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), debugText);
	//	GetWndPtr()->GetDirectPtr()->DrawTextLayout(*(m_pProp->Format), debugText,
	//		CRectF(GetRenderRectInWnd().left,
	//		GetRenderRectInWnd().top,
	//		GetRenderRectInWnd().left + debugTextSize.width,
	//		GetRenderRectInWnd().top + debugTextSize.height));
	//}



	//if (drawFullPage) {
	//	if (!m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), fullKey, ptDstLeftTopInWnd, callback)) {
	//		m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback);
	//	}
	//} else {
	//	if (m_imgDrawer->ExistInPrimary(clipKey) &&
	//		m_imgDrawer->DrawClipBitmap(GetWndPtr()->GetDirectPtr(), clipKey, ptDstClipInWnd, callback)) {

	//	} else {
	//		m_imgDrawer->DrawBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback);
	//		m_imgDrawer->DrawClipBitmap(GetWndPtr()->GetDirectPtr(), clipKey, ptDstClipInWnd, callback);//Just order

	//		//std::vector<ImgBmpKey> keys = m_imgDrawer->FindClipKeys([clipKey, pPage = &(m_image.get()), scale = m_scale](const ImgBmpKey& key)->bool{
	//		//	return 
	//		//		key != clipKey &&
	//		//		key.ImagePtr->GetPath() == clipKey.ImagePtr->GetPath() && 
	//		//		key.Scale == clipKey.Scale &&
	//		//		key.Rotate == clipKey.Rotate &&
	//		//		!key.Rect.IsRectNull();
	//		//});
	//		//for (const ImgBmpKey& key : keys) {
	//		//	CPointF ptClipInWnd = Ctrl2Wnd(Doc2Ctrl(Page2Doc(i, key.Rect.LeftTop())));
	//		//	m_imgDrawer->DrawPDFPageClipBitmap(GetWndPtr()->GetDirectPtr(), key, ptClipInWnd, callback);
	//		//}
	//	}
	//}


	//CRectF rc(GetRenderContentSize());
	//rc.OffsetRect(GetRenderRectInWnd().LeftTop());
	//rc.OffsetRect(-m_spHScroll->GetScrollPos(), -m_spVScroll->GetScrollPos());
	//GetWndPtr()->GetDirectPtr()->DrawBitmap(
	//	m_image.get().GetBitmapPtr(),
	//	rc,
	//	1.0f,
	//	D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

	//PaintScroll
	UpdateScroll();
	m_spVScroll->OnPaint(e);
	m_spHScroll->OnPaint(e);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CImageView::Normal_KeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
		case 'O':
			if (ctrl) {
				Open();
			}
			break;
	case 'A':
	{
		if (ctrl) {
			*e.HandledPtr = TRUE;
		}
	}
	case 'C':
	{
		if (ctrl) {
			*e.HandledPtr = TRUE;
		}
		break;
	}

		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CImageView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	//CreateMenu
	CMenu menu(::CreatePopupMenu());
	//Add Row
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateClockwise");
	mii.dwTypeData = const_cast<LPWSTR>(L"Rotate Clockwise");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateCounterClockwise");
	mii.dwTypeData = const_cast<LPWSTR>(L"Rotate Counter Clockwise");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	::SetForegroundWindow(GetWndPtr()->m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		e.PointInScreen.x,
		e.PointInScreen.y,
		GetWndPtr()->m_hWnd);

	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateClockwise")) {
		switch (*Rotate) {
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90);
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180);
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270);
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT);
				break;
			default:
				break;
		}
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateCounterClockwise")) {
		switch (*Rotate) {
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270);
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_DEFAULT);
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE90);
				break;
			case D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE270:
				Rotate.set(D2D1_BITMAPSOURCE_ORIENTATION::D2D1_BITMAPSOURCE_ORIENTATION_ROTATE_CLOCKWISE180);
				break;
			default:
				break;
		}
	}
	*e.HandledPtr = TRUE;
}

void CImageView::Normal_KillFocus(const KillFocusEvent& e)
{
}

/*************/
/* NormalPan */
/*************/

void CImageView::NormalPan_SetCursor(const SetCursorEvent& e)
{
	if (!(*e.HandledPtr)) {
		HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
		::SetCursor(hCur);
		*(e.HandledPtr) = TRUE;
	}
}

/***************/
/* VScrollDrag */
/***************/
void CImageView::VScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spVScroll->SetState(UIElementState::Dragged);
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
void CImageView::VScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spVScroll->SetState(UIElementState::Normal);
	m_spVScroll->SetStartDrag(0.f);
}
bool CImageView::VScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_spVScroll->GetIsVisible() && m_spVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CImageView::VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spVScroll->GetIsVisible() && m_spVScroll->GetThumbRect().PtInRect(e.PointInWnd);
}
void CImageView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spVScroll->SetScrollPos(
		m_spVScroll->GetScrollPos() +
		(e.PointInWnd.y - m_spVScroll->GetStartDrag()) *
		m_spVScroll->GetScrollDistance() /
		m_spVScroll->GetRectInWnd().Height());
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}

void CImageView::VScrlDrag_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

/***************/
/* HScrollDrag */
/***************/
void CImageView::HScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spHScroll->SetState(UIElementState::Dragged);
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CImageView::HScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spHScroll->SetState(UIElementState::Normal);
	m_spHScroll->SetStartDrag(0.f);
}
bool CImageView::HScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_spHScroll->GetIsVisible() && m_spHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CImageView::HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spHScroll->GetIsVisible() && m_spHScroll->GetThumbRect().PtInRect(e.PointInWnd);
}

void CImageView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spHScroll->SetScrollPos(
		m_spHScroll->GetScrollPos() +
		(e.PointInWnd.x - m_spHScroll->GetStartDrag()) *
		m_spHScroll->GetScrollDistance() /
		m_spHScroll->GetRectInWnd().Width());
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CImageView::HScrlDrag_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

/***********/
/* Panning */
/***********/
void CImageView::Panning_OnEntry(const LButtonBeginDragEvent& e)
{
	HScrlDrag_OnEntry(e);
	VScrlDrag_OnEntry(e);
}

void CImageView::Panning_OnExit(const LButtonEndDragEvent& e)
{
	HScrlDrag_OnExit(e);
	VScrlDrag_OnExit(e);
}

void CImageView::Panning_MouseMove(const MouseMoveEvent& e)
{
	m_spHScroll->SetScrollPos(
		m_spHScroll->GetScrollPos() + m_spHScroll->GetStartDrag() - e.PointInWnd.x);
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));

	m_spVScroll->SetScrollPos(
		m_spVScroll->GetScrollPos() + m_spVScroll->GetStartDrag() - e.PointInWnd.y);
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));

	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
}

void CImageView::Panning_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

bool CImageView::Panning_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pMachine->IsStateNormalPan();
}

void CImageView::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(GetWndPtr()->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
}

void CImageView::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpstrDefExt = L"txt";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			throw std::exception(FILE_LINE_FUNC);
		}
	} else {
		::ReleaseBuffer(path);
		CD2DImage image(path);
		Image.set(image);
		Open();
	}
}

std::tuple<CRectF, CRectF> CImageView::GetRects() const
{
	CRectF rcClient(GetRectInWnd());
	CRectF rcVertical;
	FLOAT lineHalfWidth = GetFocusedBorder().Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_spHScroll->GetIsVisible() ? (m_spHScroll->GetBandWidth() + lineHalfWidth) : lineHalfWidth);

	CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_spVScroll->GetIsVisible() ? (m_spVScroll->GetBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;

	return { rcVertical, rcHorizontal };
}

void CImageView::UpdateScroll()
{
	//VScroll
	m_spVScroll->SetScrollRangePage(0, GetRenderContentSize().height, GetRenderSize().height);

	//HScroll
	m_spHScroll->SetScrollRangePage(0, GetRenderContentSize().width, GetRenderSize().width);

	//VScroll/HScroll Rect
	auto [rcVertical, rcHorizontal] = GetRects();
	m_spVScroll->OnRect(RectEvent(GetWndPtr(), rcVertical));
	m_spHScroll->OnRect(RectEvent(GetWndPtr(), rcHorizontal));
}


void CImageView::Update() {}
