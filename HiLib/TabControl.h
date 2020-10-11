#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "ReactiveProperty.h"
#include <Shlwapi.h>


class CFilerGridView;
struct FilerGridViewProperty;
class CToDoGridView;
struct GridViewProperty;
class CShellFolder;
class CTextEditor;
class CD2DWWindow;

struct TabControlProperty
{
	std::shared_ptr<FormatF> Format;
	std::shared_ptr<SolidLine> Line;
	std::shared_ptr<SolidFill> SelectedFill;
	std::shared_ptr<SolidFill> UnfocusSelectedFill;
	std::shared_ptr<SolidFill> NormalFill;
	std::shared_ptr<SolidFill> HotFill;
	std::shared_ptr<CRectF> Padding;

	TabControlProperty()
		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
		Line(std::make_shared<SolidLine>(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f)),
		NormalFill(std::make_shared<SolidFill>(239.f/255.f, 239.f/255.f, 239.f/255.f, 1.0f)),
		SelectedFill(std::make_shared<SolidFill>(255.f/255.f, 255.f/255.f, 255.f/255.f, 1.0f)),
		UnfocusSelectedFill(std::make_shared<SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
		HotFill(std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)){};
};


struct TabData
{
	TabData(){}
	virtual ~TabData() = default;

	//In case of REGISTER_POLYMORPHIC_RELATION, Base and Derived class have to be same function structure
	template<class Archive>
	void save(Archive& ar){}
	template<class Archive>
	void load(Archive& ar){}
};

class CTabControl :public CD2DWControl
{
protected:
	std::shared_ptr<TabControlProperty> m_spProp;
	ReactiveVectorProperty<std::shared_ptr<TabData>> m_itemsSource;

	std::unordered_map<std::string, std::function<std::wstring(const std::shared_ptr<TabData>&)>> m_itemsHeaderTemplate;
	std::unordered_map<std::string, std::function<CComPtr<ID2D1Bitmap>(const std::shared_ptr<TabData>&)>> m_itemsHeaderIconTemplate;
	std::unordered_map<std::string, std::function<std::shared_ptr<CD2DWControl>(const std::shared_ptr<TabData>&)>> m_itemsControlTemplate;
	std::shared_ptr<CD2DWControl> m_spCurControl;
	ReactiveProperty<int> m_selectedIndex;
	std::optional<size_t> m_contextIndex;
public:
	CTabControl(CD2DWControl* pParentControl, const std::shared_ptr<TabControlProperty>& spProp);
	virtual ~CTabControl();

	//Getter Setter
	ReactiveVectorProperty<std::shared_ptr<TabData>>& GetItemsSource(){ return m_itemsSource; }
	int GetSelectedIndex()const{ return m_selectedIndex.get(); }
	std::shared_ptr<CD2DWControl>& GetCurrentControlPtr() { return m_spCurControl; }
	std::optional<size_t> GetPtInHeaderRectIndex(const CPointF& pt)const;


	std::function<std::vector<CRectF>&()> GetHeaderRects;
	std::function<CRectF&()> GetContentRect;
	std::function<CRectF&()> GetControlRect;


	/***************/
	/* UI MessageÅ@*/
	/***************/

	virtual void OnCreate(const CreateEvt& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnClose(const CloseEvent& e);
	virtual void OnRect(const RectEvent& e);

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e);
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e);
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e);

	virtual void OnRButtonDown(const RButtonDownEvent& e);

	virtual void OnMButtonDown(const MouseEvent& e);
	virtual void OnMButtonUp(const MouseEvent& e);

	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseEnter(const MouseEnterEvent& e);
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e);

	virtual void OnKeyDown(const KeyDownEvent& e);
	virtual void OnSysKeyDown(const SysKeyDownEvent& e);
	virtual void OnChar(const CharEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);

	/***********/
	/* Command */
	/***********/
	void OnCommandCloneTab(const CommandEvent& e);
	void OnCommandCloseTab(const CommandEvent& e);
	void OnCommandCloseAllButThisTab(const CommandEvent& e);

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
};
