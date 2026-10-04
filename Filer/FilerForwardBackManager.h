#pragma once
#include <deque>
#include <sigslot/signal.hpp>
#include "ShellFolder.h"

class CFilerForwardBackManager
{
private:
	static const int kMaxsize = 50;
	bool m_canForward;
	bool m_canBack;

	std::deque<std::shared_ptr<CShellFolder>> m_forwards;
	std::deque<std::shared_ptr<CShellFolder>> m_backs;
public:
	sigslot::signal<const wchar_t*> PropertyChanged;
public:
	/**
	 *  Constructor
	 */
	CFilerForwardBackManager() :m_canForward(false), m_canBack(false) {}
	/**
	 *  Destructor
	 */
	virtual ~CFilerForwardBackManager() {}
	void Open(std::shared_ptr<CShellFolder> folder);
	void Forward();
	void Back();
	bool GetCanForward()const;
	bool GetCanBack()const;
	void SetCanForward(bool canForward);
	void SetCanBack(bool canBack);

	void Clear();

};
