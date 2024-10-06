#pragma once
#include "DockPanel.h"
#include "FilerGridView.h"
#include "JsonSerializer.h"
#include "getter_macro.h"

class CTextBox;
class CButton;
class CFavoritesGridView;
class CFilerGridView;
class CShellFile;

class CRecentFolderGridView :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
public:
	CRecentFolderGridView(CD2DWControl* pParentControl = nullptr);
	virtual void OnPaint(const PaintEvent& e) override;
};

class CFilerView:public CDockPanel
{
public:
	SHAREDPTR_GETTER(CTextBox, TextBox)
	SHAREDPTR_GETTER(CButton, RecentButton)
	SHAREDPTR_GETTER(CFavoritesGridView, FavoriteGrid)
	SHAREDPTR_GETTER(CFilerGridView, FileGrid)
public:
	//Default
	CFilerView(CD2DWControl* pParentControl = nullptr);
	virtual ~CFilerView();
	//Event
	virtual void OnCreate(const CreateEvt& e) override;
	//virtual void OnPaint(const PaintEvent& e) override;

public:
	friend void to_json(json& j, const CFilerView& o)
	{
		json_safe_to(j, "GridView", o.m_spFileGrid);
	}

	friend void from_json(const json& j, CFilerView& o)
	{
		json_safe_from(j, "GridView", o.m_spFileGrid);
	}
};

JSON_ENTRY_TYPE(CD2DWControl, CFilerView);