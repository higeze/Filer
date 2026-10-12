#include "FilerForwardBackManager.h"

void CFilerForwardBackManager::Open(std::shared_ptr<CShellFolder> folder)
{
	Backs.push_back(folder);
	if (Backs->size() > kMaxsize) {
		Backs.erase(Backs->cbegin());
	}

	folder->Open();

	Forwrds.clear();

	SetCanBack(!Backs->empty() && Backs->size() > 0);
	SetCanForward(!Forwrds->empty() && Forwrds->size() > 0);
}

void CFilerForwardBackManager::Forward()
{
	if (GetCanForward()) {
		auto folder = Forwrds.back();
		Forwrds.erase(std::prev(Backs.cend()));

		folder->Open();

		Backs.push_back(folder);
		if (Backs->size() > kMaxsize) {
			Backs.erase(Backs->cbegin());
		}
	}

	SetCanBack(!Backs->empty() && Backs->size() > 0);
	SetCanForward(!Forwrds->empty() && Forwrds->size() > 0);
}

void CFilerForwardBackManager::Back()
{
	if (GetCanBack()) {
		auto folder = Backs.back();
		Backs.erase(std::prev(Backs->cend()));

		folder->Open();

		Forwrds.push_back(folder);
		if (Forwrds->size() > kMaxsize) {
			Forwrds.erase(Backs->cbegin());
		}
	}

	SetCanBack(!Backs->empty() && Backs->size() > 0);
	SetCanForward(!Forwrds->empty() && Forwrds->size() > 0);
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
	Backs.clear();
	Forwrds.clear();
	SetCanBack(!Backs->empty() && Backs->size() > 0);
	SetCanForward(!Forwrds->empty() && Forwrds->size() > 0);
}