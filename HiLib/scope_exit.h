#pragma once

template<typename Func>
class scope_exit
{
private:
	Func m_func;
public:
	scope_exit(Func func) :
		m_func(func)
	{}
	
	~scope_exit()
	{
		m_func();
	}
};

template<typename Func>
static scope_exit<Func> make_scope_exit(Func func)
{
	return scope_exit<Func>(func);
}