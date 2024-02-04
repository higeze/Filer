#pragma once

class CMenu;

class CShellContextMenu
{
protected:
	std::shared_ptr<IContextMenu> m_spContextMenu;
public:
	CShellContextMenu(LPCONTEXTMENU lpcm = nullptr);
	void Attach(LPCONTEXTMENU lpcm);
	~CShellContextMenu(void);

	CMenu QueryContextMenu(UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
};

