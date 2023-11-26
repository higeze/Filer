#pragma once
#include "MenuItem.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"
#include "ResourceIDFactory.h"

class CMenuItem2
{
protected:
	MENUITEMINFO mii = { 0 };
public:
	reactive_wstring_ptr Header;
	reactive_command_ptr<void> Command;

	template<class... Args>
	CMenuItem2(std::wstring&& header, Args&&... args)
		:Header(header), Command(std::forward<Args>(args)...)
	{
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;	
	}
	CMenuItem2()
	{
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
	}

	virtual ~CMenuItem2(){}

	virtual const MENUITEMINFO& GetMII()
	{
		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, Header->c_str());
		mii.dwTypeData = const_cast<LPWSTR>(Header->c_str());
		return mii;
	}
};

class CMenuSeparator2:public CMenuItem2
{
public:
	CMenuSeparator2()
	{
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE;
		mii.fType = MFT_SEPARATOR;
	}
	virtual const MENUITEMINFO& GetMII() override
	{
		return mii;
	}
};

class CContextMenu2
{
private:
    struct delete_menu
    {
	    void operator()(HMENU p)
        { 
            if(p){
                ::DestroyMenu(p);
            }
        }
    };

	std::vector<std::unique_ptr<CMenuItem2>> m_items;
public:

	void Add() {}
	template<class _Head>
	void Add(_Head&& item)
	{
		m_items.emplace_back(std::forward<_Head>(item));
	}
	template<class _Head, class... _Tail>
	void Add(_Head&& head, _Tail&&... tail)
	{
		Add(std::forward<_Head>(head));
		Add(std::forward<_Tail>(tail)...);
	}

	void Popup(HWND hWnd, const CPointU& pt)
	{
		std::unique_ptr<std::remove_pointer_t<HMENU>, delete_menu> pMenu(::CreatePopupMenu());

		for (const auto& item : m_items) {
			::InsertMenuItemW(pMenu.get(), ::GetMenuItemCount(pMenu.get()), TRUE, &item->GetMII());
		}
			::SetForegroundWindow(hWnd);
		int id = ::TrackPopupMenu(pMenu.get(), 
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
			pt.x, pt.y, 0, hWnd, NULL);

		auto iter = std::find_if(m_items.cbegin(), m_items.cend(),
			[id](const std::unique_ptr<CMenuItem2>& item) {
			return CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, item->Header->c_str()) == id;
		});

		if (iter != m_items.cend()) {
			(*iter)->Command.execute();
		}
	}
};


class CMenu
{
private:
	HMENU m_hMenu;
public:
	explicit CMenu():m_hMenu(NULL){}
	explicit CMenu(HMENU hMenu):m_hMenu(hMenu){}
	virtual ~CMenu(){
		if(m_hMenu!=NULL){
			::DestroyMenu(m_hMenu);
			m_hMenu = NULL;
		}
	}
	CMenu(const CMenu& menu)
	{
		Copy(menu);
	}

	CMenu& operator=(const CMenu& menu)
	{
		Copy(menu);
		return *this;
	}

	void Copy(const CMenu& menu)
	{
		CreatePopupMenu();
		int cnt = menu.GetMenuItemCount();
		for( int nItem = 0; nItem < cnt; nItem++)
		{
			MENUITEMINFO mii = {0};
			mii.cbSize = sizeof(MENUITEMINFO);

			//Get MENUITEMINFO
			mii.fMask = MIIM_TYPE|MIIM_ID|MIIM_STATE;
			BOOL b = menu.GetMenuItemInfo(nItem, TRUE, &mii); 

			if( mii.wID == -1)
			{
				throw std::exception("CMenu::Copy couldn't support this type of menuitem.");
			}else{
				std::basic_string<TCHAR> name;
				if(mii.fType == MFT_STRING){
					mii.cch++;
					mii.dwTypeData = GetBuffer(name, mii.cch);
					menu.GetMenuItemInfo(nItem, TRUE, &mii);	
				}else if(mii.fType == MFT_SEPARATOR){
				}else{
					throw std::exception("CMenu::Copy couldn't support this type of menuitem.");
				}
				InsertMenuItem(GetMenuItemCount(), TRUE, &mii);			
			}
		}
	}

	bool IsNull() const { return (m_hMenu == NULL); }
	void Attach(HMENU hMenu)
	{
		if(m_hMenu != NULL && m_hMenu != hMenu)
			::DestroyMenu(m_hMenu);
		m_hMenu = hMenu;
	}

	HMENU Detach()
	{
		HMENU hMenu = m_hMenu;
		m_hMenu = NULL;
		return hMenu;
	}
	//InsertMenuItem(0, TRUE, &mii);

	HMENU CreatePopupMenu()
	{
		Attach(::CreatePopupMenu());
		return m_hMenu;
	}

	int GetMenuItemCount()const
	{
		return ::GetMenuItemCount(m_hMenu);
	}

	std::basic_string<TCHAR> GetMenuString(UINT uIDItem,UINT flags)const
	{
		std::basic_string<TCHAR> string;
		::GetMenuString(m_hMenu,uIDItem,GetBuffer(string,256),256,flags);
		ReleaseBuffer(string);
		return string;
	}

	UINT GetMenuItemID(int nPos)const
	{
		return ::GetMenuItemID(m_hMenu, nPos);
	}

	UINT GetMenuState(UINT uId,UINT uFlags)const
	{
		return ::GetMenuState(m_hMenu,uId,uFlags);
	}
	HMENU GetSubMenu(int nPos)const
	{
		return ::GetSubMenu(m_hMenu,nPos);
	}
	BOOL GetMenuItemInfo(UINT item, BOOL fByPosition, LPMENUITEMINFO lpmii)const
	{
		return ::GetMenuItemInfo(m_hMenu, item, fByPosition, lpmii);
	}

	BOOL InsertMenuItem(UINT uItem, BOOL fByPosition, LPCMENUITEMINFO lpmii)
	{
		return ::InsertMenuItem(m_hMenu, uItem, fByPosition, lpmii);
	}

	BOOL InsertMenuItem(UINT uItem, BOOL fByPosition,CMenuItem* pMenuItem)
	{
		return InsertMenuItem(uItem,fByPosition,pMenuItem->GetMenuItemInfoPtr());
	}

	BOOL InsertSeparator(UINT uItem, BOOL fByPosition)
	{
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE;
		mii.fType = MFT_SEPARATOR;
		return InsertMenuItem(uItem, TRUE, &mii);
	}

	BOOL TrackPopupMenu(UINT uFlags, int x, int y, HWND hWnd)
	{
		return ::TrackPopupMenu(m_hMenu, uFlags, x, y, 0, hWnd, NULL);
	}

	BOOL DeleteMenu(UINT uPosition, UINT uFlags)
	{
		return ::DeleteMenu(m_hMenu, uPosition, uFlags);
	}

	operator HMENU()const{return m_hMenu;}
	operator bool()const { return m_hMenu!=NULL; }
};