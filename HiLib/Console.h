#pragma once

class CConsole
{
private:
	static std::function<BOOL(DWORD)> s_callback_function;
	FILE* m_pFile = nullptr;
public:
	CConsole();
	~CConsole();

	BOOL Alloc();
	BOOL Free();
	bool IsOpen()const;
private:
	//static BOOL WINAPI callback_helper(DWORD dwCtrlType) { return s_callback_function(dwCtrlType); }
	//static BOOL SetConsoleCtrlHandler(std::function<BOOL(DWORD)> callback, BOOL Add)
	//{
	//	s_callback_function = callback;
	//	return ::SetConsoleCtrlHandler(callback_helper, Add);
	//}

};
