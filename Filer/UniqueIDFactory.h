#pragma once

class CUniqueIDFactory
{
private:
	unsigned int m_id;
public:
	CUniqueIDFactory(unsigned int firstId = 0):m_id(0){}
	virtual ~CUniqueIDFactory(void){}
	unsigned int NewID();
	void DeleteID(unsigned int id);
};

