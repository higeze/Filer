#include "FilerForwardBackManager.h"

void CFilerForwardBackManager::Open(std::shared_ptr<CShellFolder> folder)
{
	m_backs.push_back(folder);
	if (m_backs.size() > kMaxsize) {
		m_backs.pop_front();
	}

	folder->Open();

	m_backs.clear();

	SetCanBack(!m_backs.empty() && m_backs.size() > 0);
	SetCanForward(!m_forwards.empty() && m_forwards.size() > 0);
}

void CFilerForwardBackManager::Forward()
{
	if (GetCanForward()) {
		auto folder = m_forwards.back();
		m_forwards.pop_back();

		folder->Open();

		m_backs.push_back(folder);
		if (m_backs.size() > kMaxsize) {
			m_backs.pop_front();
		}
	}

	SetCanBack(!m_backs.empty() && m_backs.size() > 0);
	SetCanForward(!m_forwards.empty() && m_forwards.size() > 0);
}

void CFilerForwardBackManager::Back()
{
	if (GetCanBack()) {
		auto folder = m_backs.back();
		m_backs.pop_back();

		folder->Open();

		m_forwards.push_back(folder);
		if (m_forwards.size() > kMaxsize) {
			m_forwards.pop_front();
		}
	}

	SetCanBack(!m_backs.empty() && m_backs.size() > 0);
	SetCanForward(!m_forwards.empty() && m_forwards.size() > 0);
}

bool CFilerForwardBackManager::GetCanBack()const
{
	return m_canBack;//!m_backs.empty() && m_backs.size()>0;
}

bool CFilerForwardBackManager::GetCanForward()const
{
	return m_canForward;//!m_forwards.empty() && m_forwards.size()>0;
}

void CFilerForwardBackManager::SetCanBack(bool canBack)
{
	if (m_canBack != canBack) {
		m_canBack = canBack;
		PropertyChanged(L"CanBack");
	}
}
void CFilerForwardBackManager::SetCanForward(bool canForward)
{
	if (m_canForward != canForward) {
		m_canForward = canForward;
		PropertyChanged(L"CanForward");
	}
}

void CFilerForwardBackManager::Clear()
{
	m_backs.clear();
	m_forwards.clear();
	SetCanBack(!m_backs.empty() && m_backs.size() > 0);
	SetCanForward(!m_forwards.empty() && m_forwards.size() > 0);
}