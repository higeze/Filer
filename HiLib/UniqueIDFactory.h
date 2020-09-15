#pragma once
#include <unordered_set>

class CUniqueIDFactory
{
private:
	std::unordered_set<unsigned int> m_ids;
public:
	CUniqueIDFactory(){}
	virtual ~CUniqueIDFactory(void){}
	unsigned int NewID();
	void DeleteID(unsigned int id);
};

