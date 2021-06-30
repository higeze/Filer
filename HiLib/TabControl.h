#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "Button.h"
#include "ReactiveProperty.h"
#include "index_vector.h"
#include <Shlwapi.h>



class CFilerGridView;
struct FilerGridViewProperty;
class CToDoGridView;
struct GridViewProperty;
class CShellFolder;
class CTextEditor;
class CD2DWWindow;

struct TabHeaderControlProperty
{
	std::shared_ptr<FormatF> Format;
	std::shared_ptr<CRectF> Padding;
	std::shared_ptr<ButtonProperty> ButtonProp;
	TabHeaderControlProperty()
		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
		ButtonProp(std::make_shared<ButtonProperty>())
	{
		ButtonProp->BorderLine = SolidLine(0.f, 0.f, 0.f, 0.f, 0.f);
	};
};

struct TabControlProperty
{
	std::shared_ptr<TabHeaderControlProperty> HeaderProperty;
	std::shared_ptr<SolidLine> Line;
	std::shared_ptr<SolidFill> SelectedFill;
	std::shared_ptr<SolidFill> UnfocusSelectedFill;
	std::shared_ptr<SolidFill> NormalFill;
	std::shared_ptr<SolidFill> HotFill;
	std::shared_ptr<CRectF> Padding;

	TabControlProperty()
		:HeaderProperty(std::make_shared<TabHeaderControlProperty>()),
		Line(std::make_shared<SolidLine>(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f)),
		NormalFill(std::make_shared<SolidFill>(239.f/255.f, 239.f/255.f, 239.f/255.f, 1.0f)),
		SelectedFill(std::make_shared<SolidFill>(255.f/255.f, 255.f/255.f, 255.f/255.f, 1.0f)),
		UnfocusSelectedFill(std::make_shared<SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
		HotFill(std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)){};
};


struct TabData
{
	ReactiveProperty<bool> Unlock = true;

	TabData(){}
	virtual ~TabData() = default;

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
		get_to_nothrow(j, "Unlock", o.Unlock);
	}
};

class CTabControl;
/*********************/
/* CTabHeaderControl */
/*********************/

class CTabHeaderControl :public CD2DWControl
{
private:
	std::shared_ptr<TabHeaderControlProperty> m_spProp;
	std::shared_ptr<CButton> m_spButton;
	std::unique_ptr<CBinding> m_isEnableBinding;

	CSizeF m_size = CSizeF();
	CSizeF m_iconSize = CSizeF();
	CSizeF m_textSize = CSizeF();
	CSizeF m_buttonSize = CSizeF();

	FLOAT m_minWidth = 30.f;
	FLOAT m_maxWidth = 300.f;

	int m_index = -1;
	bool m_isMeasureValid = false;

public:
	CTabHeaderControl(CTabControl* pTabControl, const std::shared_ptr<TabHeaderControlProperty>& spProp);
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
	std::shared_ptr<TabHeaderControlProperty> m_spProp;
	std::shared_ptr<CButton> m_spButton;

	CSizeF m_buttonSize = CSizeF();
	CSizeF m_size = CSizeF();

	FLOAT m_minWidth = 30.f;
	FLOAT m_maxWidth = 300.f;

	bool m_isMeasureValid = false;

public:
	CAddTabHeaderControl(CTabControl* pTabControl, const std::shared_ptr<TabHeaderControlProperty>& spProp);
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
protected:
	std::shared_ptr<TabControlProperty> m_spProp;
	ReactiveVectorProperty<std::shared_ptr<TabData>> m_itemsSource;
	index_vector<std::shared_ptr<CTabHeaderControl>> m_headers;
	std::shared_ptr<CAddTabHeaderControl> m_addHeader;

	std::unordered_map<std::string, std::function<std::wstring(const std::shared_ptr<TabData>&)>> m_itemsHeaderTemplate;
	std::unordered_map<std::string, std::function<CComPtr<ID2D1Bitmap>(const std::shared_ptr<TabData>&)>> m_itemsHeaderIconTemplate;
	std::unordered_map<std::string, std::function<std::shared_ptr<CD2DWControl>(const std::shared_ptr<TabData>&)>> m_itemsControlTemplate;
	std::shared_ptr<CD2DWControl> m_spCurControl;
	ReactiveProperty<int> m_selectedIndex;
	std::optional<size_t> m_contextIndex;
public:
	CTabControl(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<TabControlProperty>& spProp = nullptr);
	virtual ~CTabControl();

	//Getter Setter
	ReactiveVectorProperty<std::shared_ptr<TabData>>& GetItemsSource(){ return m_itemsSource; }
	int GetSelectedIndex()const{ return m_selectedIndex.get(); }
	std::shared_ptr<CD2DWControl>& GetCurrentControlPtr() { return m_spCurControl; }
	std::optional<size_t> GetPtInHeaderRectIndex(const CPointF& pt)const;

	void UpdateHeaderRects();

	std::function<CRectF&()> GetContentRect;
	std::function<CRectF&()> GetControlRect;

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
	virtual void OnCommandLockTab(const CommandEvent& e);
	virtual void OnCommandCloneTab(const CommandEvent& e);
	virtual void OnCommandNewTab(const CommandEvent& e) {}
	virtual void OnCommandCloseTab(const CommandEvent& e);
	virtual void OnCommandCloseAllButThisTab(const CommandEvent& e);

public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		ar("ItemsSource", m_itemsSource);
		ar("SelectedIndex", m_selectedIndex);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("ItemsSource", m_itemsSource);
		ar("SelectedIndex", m_selectedIndex);
	}

	friend void to_json(json& j, const CTabControl& o)
	{
		j = json{
			{"ItemsSource", o.m_itemsSource},
			{"SelectedIndex", o.m_selectedIndex }
		};
	}
	friend void from_json(const json& j, CTabControl& o)
	{
		j.at("ItemsSource").get_to(o.m_itemsSource);
		j.at("SelectedIndex").get_to(o.m_selectedIndex);
	}
};

