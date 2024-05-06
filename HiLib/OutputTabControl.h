//#pragma once
//#include "D2DWControl.h"
//#include "TabControl.h"
//
///*************/
///* CFilerTab */
///*************/
//class COutputTabControl :public CTabControl
//{
//private:
//
//public:
//	COutputTabControl(CD2DWControl* pParentControl = nullptr);
//	virtual ~COutputTabControl();
//
//	/****************/
//	/* Pure Virtual */
//	/****************/
//
//	/**********/
//	/* Getter */
//	/**********/
//	SHAREDPTR_GETTER(CColorTextBox, FilerView)
//	SHAREDPTR_GETTER(CEditor, TextView)
//
//	/**************/
//	/* UI Message */
//	/**************/
//	void OnCreate(const CreateEvt& e) override;
//	void OnContextMenu(const ContextMenuEvent& e) override;
//
//	/***********/
//	/* Command */
//	/***********/
//	void OnCommandNewTab() override { return OnCommandNewFilerTab(); }
//	void OnCommandNewFilerTab();
//	void OnCommandNewToDoTab();
//	void OnCommandNewTextTab();
//	void OnCommandNewPdfTab();
//	void OnCommandNewImageTab();
//	void OnCommandNewPreviewTab();
//
//	void OnCommandAddToFavorite();
//	void OnCommandOpenSameAsOther();
//
//public:
//	friend void to_json(json& j, const CFilerTabGridView& o)
//	{
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ImageTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PreviewTabData);
//
//		to_json(j, static_cast<const CTabControl&>(o));
//
//		//j["FilerView"] = o.m_spFilerView;
//		//j["ToDoView"] = o.m_spToDoGridView;
//	}
//
//	friend void from_json(const json& j, CFilerTabGridView& o)
//	{
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ImageTabData);
//		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PreviewTabData);
//
//		from_json(j, static_cast<CTabControl&>(o));
//
//		//get_to(j, "FilerView", o.m_spFilerView);
//		//get_to(j, "ToDoView", o.m_spToDoGridView);
//	}
//};