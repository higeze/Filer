#pragma once
#include "D2DWControl.h"
#include <optional>

class CD2DWWindow;
class CDialogStateMachine;

class CD2DWDialog: public CD2DWControl
{
public:
	const FLOAT kSizeWidth = 5.f;
	const FLOAT kTitlePadding = 3.f;
public:
	reactive_wstring_ptr Title;
	reactive_property_ptr<bool> IsModal;
	virtual const SolidLine& GetNormalBorder() const
	{
		static const SolidLine value(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.0f, 1.0f); return value;
	}


protected:
	std::unique_ptr<CDialogStateMachine> m_pDialogMachine;

	mutable CSizeF m_titleSize;
	std::optional<CPointF> m_startPoint;

public:
	CD2DWDialog(CD2DWControl* pParentControl = nullptr);
	virtual ~CD2DWDialog();

	//virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnPaint(const PaintEvent& e) override;

	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;

	bool Guard_LButtonBeginDrag_Normal_To_Moving(const LButtonBeginDragEvent& e);
	bool Guard_LButtonBeginDrag_Normal_To_LeftSizing(const LButtonBeginDragEvent& e);
	bool Guard_LButtonBeginDrag_Normal_To_RightSizing(const LButtonBeginDragEvent& e);
	bool Guard_LButtonBeginDrag_Normal_To_TopSizing(const LButtonBeginDragEvent& e);
	bool Guard_LButtonBeginDrag_Normal_To_BottomSizing(const LButtonBeginDragEvent& e);

	void Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void Normal_LButtonEndDrag(const LButtonEndDragEvent& e);
	void Normal_MouseMove(const MouseMoveEvent& e);
	void Normal_MouseLeave(const MouseLeaveEvent& e);
	void Normal_SetCursor(const SetCursorEvent& e);
	
	void Moving_OnEntry(const LButtonBeginDragEvent& e);
	void Moving_OnExit();
	void Moving_MouseMove(const MouseMoveEvent& e);

	void Sizing_OnEntry(const LButtonBeginDragEvent& e);
	void Sizing_OnExit();

	void LeftSizing_MouseMove(const MouseMoveEvent& e);
	void RightSizing_MouseMove(const MouseMoveEvent& e);
	void TopSizing_MouseMove(const MouseMoveEvent& e);
	void BottomSizing_MouseMove(const MouseMoveEvent& e);

	void LeftSizing_SetCursor(const SetCursorEvent& e);
	void RightSizing_SetCursor(const SetCursorEvent& e);
	void TopSizing_SetCursor(const SetCursorEvent& e);
	void BottomSizing_SetCursor(const SetCursorEvent& e);


	void Error_StdException(const std::exception& e);

	CSizeF GetTitleSize() const;
	CRectF GetTitleRect();
	virtual void PaintTitle();
	virtual void PaintBorder();

private:
	bool IsPtInLeftSizingRect(const CPointF& pt);
	bool IsPtInRightSizingRect(const CPointF& pt);
	bool IsPtInTopSizingRect(const CPointF& pt);
	bool IsPtInBottomSizingRect(const CPointF& pt);


};
