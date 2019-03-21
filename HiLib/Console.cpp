#include "Console.h"

std::function<BOOL(DWORD)> CConsole::s_callback_function;

CConsole::CConsole():m_pFile(nullptr){}

CConsole::~CConsole()
{
	if (IsOpen()) {
		Free();
	}
}

BOOL CConsole::Alloc()
{
	if (!::AllocConsole()) { return FALSE; }

	//::_wfreopen_s(&m_pFile, L"CON", L"r", stdin);
	::_wfreopen_s(&m_pFile, L"CON", L"w", stdout);
	//std::function<BOOL(DWORD)> callback = [this](DWORD dwCtrlType) -> BOOL {
	//	if (dwCtrlType == CTRL_C_EVENT) {
	//		Free();
	//		return TRUE;
	//	} else {
	//		return FALSE;
	//	}
	//};
	//SetConsoleCtrlHandler(callback, TRUE);
	//SetConsoleCtrlHandler([](DWORD dwCtrlType) -> BOOL {
	//		if (dwCtrlType == CTRL_C_EVENT) {
	//			FreeConsole();
	//			return TRUE;
	//		} else {
	//			return FALSE;
	//		}
	//	}, TRUE);
	HMENU hMenu = ::GetSystemMenu(::GetConsoleWindow(), FALSE);
	::RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
	return TRUE;
}

BOOL CConsole::Free()
{
	//auto b = ::fclose(stdin);
	//auto c = ::fclose(stdout);
	auto a = ::fclose(m_pFile);
	m_pFile = NULL;
	if(::FreeConsole()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

bool CConsole::IsOpen()const
{
	return (bool)m_pFile;
}