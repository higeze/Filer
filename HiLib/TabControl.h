#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"
#include "observable.h"
#include <Shlwapi.h>


class CFilerGridView;
struct FilerGridViewProperty;
class CToDoGridView;
struct GridViewProperty;
class CTextboxWnd;
class CShellFolder;
class CTextEditor;


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

class CTabControl :public CUIElement
{
private:
	CWnd* m_pWnd;
	d2dw::CRectF m_rect;
	observable_vector<std::shared_ptr<TabData>> m_itemsSource;

	std::unordered_map<std::string, std::function<std::wstring(const std::shared_ptr<TabData>&)>> m_itemsHeaderTemplate;
	std::unordered_map<std::string, std::function<std::shared_ptr<CUIElement>(const std::shared_ptr<TabData>&)>> m_itemsContentTemplate;
	std::shared_ptr<CUIElement> m_spCurControl;
	observable<int> m_selectedIndex = -1;
public:
	CTabControl(CWnd* pWnd);
	virtual ~CTabControl();

	//Getter Setter
	CWnd* GetWndPtr() const override { return m_pWnd; }
	d2dw::CRectF GetRectInWnd() const override { return m_rect; }

	observable_vector<std::shared_ptr<TabData>>& GetItemsSource(){ return m_itemsSource; }
	int GetSelectedIndex()const{ return m_selectedIndex.get(); }

	std::function<std::vector<d2dw::CRectF>&()> GetHeaderRects;
	std::function<d2dw::CRectF&()> GetContentRect;


	/***************/
	/* UI MessageÅ@*/
	/***************/
	virtual void OnCreate(const CreateEvent& e) {}
	virtual void OnPaint(const PaintEvent& e) {}
	//virtual void OnClose(const CloseEvent& e) {}
	virtual void OnCommand(const CommandEvent& e) {}
	virtual void OnRect(const RectEvent& e) { m_rect = e.Rect; }

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e) {}
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) {}
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) {}
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) {}

	virtual void OnRButtonDown(const RButtonDownEvent& e) {}

	virtual void OnMButtonDown(const MouseEvent& e){}//TODO
	virtual void OnMButtonUp(const MouseEvent& e){}//TODO

	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseEnter(const MouseEvent& e);//TODO
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e){}

	virtual void OnKeyDown(const KeyDownEvent& e) {}
	virtual void OnSysKeyDown(const SysKeyDownEvent& e){}
	virtual void OnChar(const CharEvent& e) {}
	virtual void OnContextMenu(const ContextMenuEvent& e){}
	virtual void OnSetFocus(const SetFocusEvent& e) {}
	virtual void OnSetCursor(const SetCursorEvent& e) {}
	virtual void OnKillFocus(const KillFocusEvent& e) {}

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
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);

		ar("ItemsSource", static_cast<std::vector<std::shared_ptr<TabData>>&>(m_itemsSource));
		ar("SelectedIndex", m_selectedIndex);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);

		ar("ItemsSource", static_cast<std::vector<std::shared_ptr<TabData>>&>(m_itemsSource));
		ar("SelectedIndex", m_selectedIndex);
	}
};
