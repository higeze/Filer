#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "Button.h"
#include "index_vector.h"
#include <Shlwapi.h>

#include "IStateMachine.h"
#include <boost/sml.hpp>
#include "reactive_property.h"
#include "reactive_vector.h"
#include "JsonSerializer.h"



class CFilerGridView;
struct FilerGridViewProperty;
class CToDoGridView;
struct GridViewProperty;
class CShellFolder;
class CEditor;
class CD2DWWindow;

class CTabTemplate;

struct TabData:public std::enable_shared_from_this<TabData>
{
	reactive_property_ptr<bool> Unlock;

	TabData()
		:Unlock(true){}
	virtual ~TabData() = default;

	virtual std::shared_ptr<TabData> ClonePtr() const = 0;
	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing);

	//In case of REGISTER_POLYMORPHIC_RELATION, Base and Derived class have to be same function structure
	template<class Archive>
	void save(Archive& ar){}
	template<class Archive>
	void load(Archive& ar){}


	friend void to_json(json& j, const TabData& o)
	{
		j = json{
			{"Unlock", o.Unlock},
		};
	}
	friend void from_json(const json& j, TabData& o) 
	{
		get_to(j, "Unlock", o.Unlock);
	}
};

class CTabControl;
/*********************/
/* CTabHeaderControl */
/*********************/

class CTabHeaderControl :public CD2DWControl
{
public:
	static CRectF FOCUS_PADDING;
	
private:
	std::shared_ptr<CButton> m_spButton;

	CSizeF m_size = CSizeF();
	CSizeF m_iconSize = CSizeF();
	CSizeF m_textSize = CSizeF();
	CSizeF m_buttonSize = CSizeF();

	FLOAT m_minWidth = 30.f;
	FLOAT m_maxWidth = 300.f;


	int m_index = -1;
	bool m_isMeasureValid = false;

public:
	const SolidFill& GetNormalBackground() const override
	{
		const static SolidFill value(239.f / 255.f, 239.f / 255.f, 239.f / 255.f, 1.0f); return value;
	}
	const SolidFill& GetSelectedOverlay() const override
	{
		const static SolidFill value(255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1.0f); return value;
	}
	const SolidFill& GetUnfocusSelectedOverlay() const override
	{
		const static SolidFill value(247.f / 255.f, 247.f / 255.f, 247.f / 255.f, 1.0f); return value;
	}
	const SolidFill& GetHotOverlay() const override
	{
		const static SolidFill value(1.0f, 1.0f, 1.0f, 0.3f); return value;
	}

public:
	CTabHeaderControl(CTabControl* pTabControl);
	virtual ~CTabHeaderControl() = default;
	bool GetMeasureValid()const { return m_isMeasureValid; }
	void SetMeasureValid(const bool& b) { m_isMeasureValid = b; }
	int GetIndex() const { return m_index; }
	void SetIndex(const int index) { m_index = index; }
	bool GetIsSelected()const;

	std::tuple<CSizeF, CSizeF, CSizeF, CSizeF> MeasureSizes();
	std::tuple<CSizeF, CSizeF, CSizeF, CSizeF> GetSizes();
	virtual CSizeF GetSize();
	std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects();

	virtual void OnCreate(const CreateEvt& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnRect(const RectEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	//virtual void OnContextMenu(const ContextMenuEvent& e);

};

/************************/
/* CAddTabHeaderControl */
/************************/

class CAddTabHeaderControl :public CD2DWControl
{
private:
	std::shared_ptr<CButton> m_spButton;

	CSizeF m_buttonSize = CSizeF();
	CSizeF m_size = CSizeF();

	FLOAT m_minWidth = 30.f;
	FLOAT m_maxWidth = 300.f;

	bool m_isMeasureValid = false;

public:
	CAddTabHeaderControl(CTabControl* pTabControl);
	virtual ~CAddTabHeaderControl() = default;
	bool GetMeasureValid()const { return m_isMeasureValid; }
	void SetMeasureValid(const bool& b) { m_isMeasureValid = b; }

	bool GetIsSelected()const;

	std::tuple<CSizeF, CSizeF> MeasureSizes();
	std::tuple<CSizeF, CSizeF> GetSizes();
	virtual CSizeF GetSize();
	std::tuple<CRectF, CRectF> GetRects();

	virtual void OnCreate(const CreateEvt& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnRect(const RectEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	//virtual void OnContextMenu(const ContextMenuEvent& e) {//Do Nothing}

};

/***************/
/* CTabControl */
/***************/

class CTabControl :public CD2DWControl
{
	friend class CTabHeaderControl;
	friend class CAddTabHeaderControl;
	static CRectF MARGIN;

protected:
	index_vector<std::shared_ptr<CTabHeaderControl>> m_headers;
	std::shared_ptr<CAddTabHeaderControl> m_addHeader;

	std::unordered_map<std::string, std::shared_ptr<CTabTemplate>> m_templates;

	std::unordered_map<std::string, std::function<std::wstring(const std::shared_ptr<TabData>&)>> m_itemsHeaderTemplate;
	std::unordered_map<std::string, std::function<void(const std::shared_ptr<TabData>&, const CRectF&)>> m_itemsHeaderIconTemplate;
	std::unordered_map<std::string, std::function<std::shared_ptr<CD2DWControl>(const std::shared_ptr<TabData>&)>> m_itemsControlTemplate;
	std::unordered_map<std::string, std::function<void(const std::shared_ptr<TabData>&, const std::shared_ptr<TabData>&)>> m_itemsBindingTemplate;
	std::shared_ptr<CD2DWControl> m_spCurControl;
	std::optional<size_t> m_contextIndex;
public:
	reactive_property_ptr<int> SelectedIndex;
	int m_prevSelectedIndex = -1;
	reactive_vector_ptr<std::shared_ptr<TabData>> ItemsSource;

public:
	CTabControl(CD2DWControl* pParentControl = nullptr);
	virtual ~CTabControl();

	//Getter Setter
	std::shared_ptr<CD2DWControl>& GetCurrentControlPtr() { return m_spCurControl; }
	std::optional<size_t> GetPtInHeaderRectIndex(const CPointF& pt)const;

	void UpdateHeaderRects();

	virtual CRectF GetRectInWnd()const override { return m_rect; }
	std::function<CRectF&()> GetContentRect;
	std::function<CRectF&()> GetControlRect;


	virtual void Arrange(const CRectF& rc) override;

	/***************/
	/* UI MessageÅ@*/
	/***************/

	virtual void OnCreate(const CreateEvt& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnClosing(const ClosingEvent& e);
	virtual void OnRect(const RectEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);

	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);


	/***********/
	/* Command */
	/***********/
	virtual void OnCommandLockTab();
	virtual void OnCommandCloneTab();
	virtual void OnCommandNewTab() {}
	virtual void OnCommandCloseTab();
	virtual void OnCommandCloseAllButThisTab();

private:
	struct Machine;
	std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;
	int m_dragFrom;
	int m_dragTo;

public:
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;

	bool Guard_LButtonBeginDrag_Normal_To_Dragging(const LButtonBeginDragEvent& e);

	void Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void Normal_LButtonEndDrag(const LButtonEndDragEvent& e);
	void Normal_MouseMove(const MouseMoveEvent& e);
	
	void Dragging_OnEntry(const LButtonBeginDragEvent& e);
	void Dragging_OnExit(const LButtonEndDragEvent& e);
	void Dragging_MouseMove(const MouseMoveEvent& e);

	void Error_StdException(const std::exception& e);

private:
	index_vector<std::shared_ptr<CTabHeaderControl>>::const_iterator FindPtInDraggingHeaderRect(const CPointF& pt);

//public:
//	virtual void process_event(const LButtonBeginDragEvent& e) override;
//	virtual void process_event(const LButtonEndDragEvent& e) override;
//	virtual void process_event(const MouseMoveEvent& e) override;
//	virtual void process_event(const MouseLeaveEvent& e) override;
//	virtual void process_event(const SetCursorEvent& e) override;
//
//	virtual void process_event(const PaintEvent& e) override {}
//	virtual void process_event(const LButtonDownEvent& e) override{}
//	virtual void process_event(const LButtonUpEvent& e) override{}
//	virtual void process_event(const LButtonClkEvent& e) override{}
//	virtual void process_event(const LButtonSnglClkEvent& e) override{}
//	virtual void process_event(const LButtonDblClkEvent& e) override{}
//	virtual void process_event(const RButtonDownEvent& e) override{}
//	virtual void process_event(const ContextMenuEvent& e) override{}
//	virtual void process_event(const SetFocusEvent& e) override{}
//	virtual void process_event(const KillFocusEvent& e) override{}
//	virtual void process_event(const KeyDownEvent& e) override{}
//	virtual void process_event(const KeyUpEvent& e) override{}
//	virtual void process_event(const CharEvent& e) override{}
//	virtual void process_event(const BeginEditEvent& e) override {}
//	virtual void process_event(const EndEditEvent& e) override {}




public:

	//NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
	//	CTabControl,
	//	ItemsSource,
	//	SelectedIndex);
	friend void to_json(json& j, const CTabControl& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));

		j["ItemsSource"] = o.ItemsSource;
		j["SelectedIndex"] = o.SelectedIndex;
	}

	friend void from_json(const json& j, CTabControl& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));

		get_to(j, "ItemsSource", o.ItemsSource);
		get_to(j, "SelectedIndex", o.SelectedIndex);
	}

};

