#pragma once
#include <functional>

class IDoCommand
{
public:
	IDoCommand(){}
	virtual ~IDoCommand(){}
	virtual void Do() = 0;
	virtual void UnDo() = 0;
	virtual void ReDo() = 0;
};

class CDoCommand : public IDoCommand
{
private:
	std::function<void()> m_undo;
	std::function<void()> m_redo;
public:
	template<typename T, typename U>
	CDoCommand(T&& redo, U&& undo):m_redo(redo),m_undo(undo){}
	virtual ~CDoCommand(){}
	virtual void Do() override { m_redo(); }
	virtual void UnDo() override { m_undo(); }
	virtual void ReDo() override { m_redo(); }
};