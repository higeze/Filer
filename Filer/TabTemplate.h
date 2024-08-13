#pragma once
#include <string>
#include <memory>

#include "Direct2DWrite.h"
#include "D2DWWindow.h"
#include "D2DWControl.h"
#include "TabControl.h"

class CTabTemplate
{
protected:
	std::shared_ptr<TabData> m_pTabData;
public:
	void Set(const std::shared_ptr<TabData>& pTabData)
	{
		if (m_pTabData) {
			Detach(m_pTabData);
		}
		Attach(pTabData);
	}
	virtual std::wstring Header() const = 0;
	virtual void DrawHeaderIcon(CD2DWWindow* pWnd, CDirect2DWrite* pDirect, const CRectF& rect) const = 0;
	virtual std::shared_ptr<CD2DWControl> Control() const = 0;
	virtual void Detach(const std::shared_ptr<TabData>& pTabData) = 0;
	virtual void Attach(const std::shared_ptr<TabData>& pTabData) = 0;
};

#include "FilerTabData.h"
#include "Dispatcher.h"
#include "D2DFileIconDrawer.h"
#include "FilerGridView.h"

class CFilerTabTemplate: public CTabTemplate
{
	std::shared_ptr<CFilerGridView> m_spFilerGridView;

	CFilerTabTemplate(CD2DWControl* pParentControl, const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
		:m_spFilerGridView(std::make_shared<CFilerGridView>(pParentControl, spFilerGridViewProp)){}

	virtual std::wstring Header() const override
	{
		if (auto p = std::dynamic_pointer_cast<FilerTabData>(m_pTabData)) {
			return p->Folder->GetDispNameWithoutExt().c_str();
		} else {
			return L"nullptr";
		}
	}
	virtual void DrawHeaderIcon(CD2DWWindow* pWnd, CDirect2DWrite* pDirect, const CRectF& rect) const override
	{
		auto updated = [pWnd]()->void
		{
			pWnd->GetDispatcherPtr()->PostInvoke([pWnd]()->void
			{
				pWnd->InvalidateRect(NULL, FALSE);
			});
		};

		if (auto p = std::dynamic_pointer_cast<FilerTabData>(m_pTabData)) {
			pDirect->GetFileIconDrawerPtr()->DrawFileIconBitmap(
				pDirect, rect.LeftTop(), p->Folder->GetAbsoluteIdl(), p->Folder->GetPath(), p->Folder->GetDispExt(), p->Folder->GetAttributes(), updated);
		} else {
			//Do nothing
		}	
	}
	virtual std::shared_ptr<CD2DWControl> Control() const override
	{
		return m_spFilerGridView;
	}
	virtual void Detach(const std::shared_ptr<TabData>& pTabData) {}
	virtual void Attach(const std::shared_ptr<TabData>& pTabData) 
	{
		if (auto p = std::dynamic_pointer_cast<FilerTabData>(m_pTabData)) {
			m_spFilerGridView->OpenFolder(p->Folder);
		}
	}
};
