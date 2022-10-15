#pragma once

template<class TRect>
class CGdiObj
{
protected:
	TRect m_hGdiObj;
	bool m_isManaged;

public:
	CGdiObj(TRect hGdiObj=NULL, bool isManaged = true):m_hGdiObj(hGdiObj),m_isManaged(isManaged){}
	virtual ~CGdiObj()
	{
		if(m_isManaged && m_hGdiObj!=NULL){
			::DeleteObject(m_hGdiObj);
			m_hGdiObj = NULL;
		}
	}
	void Attach(TRect hGdiObj)
	{
		if(m_hGdiObj != NULL && m_hGdiObj != hGdiObj)
			::DeleteObject(m_hGdiObj);
		m_hGdiObj = hGdiObj;
	}

	TRect Detach()
	{
		TRect hGdiObj = m_hGdiObj;
		m_hGdiObj = NULL;
		return hGdiObj;
	}
	operator TRect()const{return m_hGdiObj;}
	bool IsNull() const { return (m_hGdiObj == NULL); }
	void SetIsManaged(bool isManaged){m_isManaged = isManaged;}


};