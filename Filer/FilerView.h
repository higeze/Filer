#pragma once
#include "D2DWControl.h"
#include "FilerGridView.h"
#include "TextBox.h"
#include "Button.h"
#include "TextBoxProperty.h"
#include "JsonSerializer.h"

class CShellFile;

class CRecentFolderGridView :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
public:
	CRecentFolderGridView(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp = nullptr);
	virtual void OnPaint(const PaintEvent& e) override;


};

class CFilerView:public CD2DWControl
{
public:
	SHAREDPTR_GETTER(CTextBox, TextBox)
	SHAREDPTR_GETTER(CButton, RecentButton)
	SHAREDPTR_GETTER(CFilerGridView, GridView)

	

	CFilerView(CD2DWControl* pParentControl = nullptr, 
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp = nullptr,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp = nullptr);

	virtual ~CFilerView();

	virtual void OnCreate(const CreateEvt& e) override;
	//virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;

	virtual void Measure(const CSizeF& availableSize) override;
	//virtual CSizeF DesiredSize() const { return m_size; }
	//virtual CSizeF RenderSize() const { return CSizeF(m_size.width - Margin->Width(), m_size.height - Margin->Height()); }
	virtual void Arrange(const CRectF& rc) override;

	friend void to_json(json& j, const CFilerView& o)
	{
		j["GridView"] = o.m_spGridView;
	}

	friend void from_json(const json& j, CFilerView& o)
	{
		get_to(j, "GridView", o.m_spGridView);
	}
};