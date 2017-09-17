#pragma once

class CIcon
{
private:
	std::shared_ptr<std::remove_pointer<HICON>::type> m_pIcon;
public:
	//Constructor
	CIcon(HICON hIcon=NULL);
	//Destructor
	virtual ~CIcon(){}
	//Cast
	operator HICON()const{return m_pIcon.get();}
	//IsNull
	bool IsNull()const{return m_pIcon.get() == NULL;}
};