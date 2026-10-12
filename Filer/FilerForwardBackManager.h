#pragma once
#include <deque>
#include <sigslot/signal.hpp>
#include "ShellFolder.h"
#include "reactive_vector.h"

class CFilerForwardBackManager
{
private:
	static const int kMaxsize = 10;
	bool m_canForward;
	bool m_canBack;
public:
	reactive_vector_ptr<std::shared_ptr<CShellFolder>> Forwrds;
	reactive_vector_ptr<std::shared_ptr<CShellFolder>> Backs;
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
