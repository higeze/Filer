#pragma once
#include "MyFriendSerializer.h"
class CSheet;
class CCell;

class CBand
{
public:
	typedef int size_type;
	typedef int coordinates_type;
	typedef std::shared_ptr<CCell> cell_type;
	typedef std::wstring string_type;
	static const size_type kMaxIndex = 10000;
	static const size_type kMinIndex = -10;
	static const size_type kInvalidIndex = -9999;
	static const coordinates_type kResizeAreaHarfWidth = 4;
protected:
	//Field
	CSheet* m_pSheet; // Parent sheet pointer
	bool m_bVisible; // Visible or not
	bool m_bSelected; // Selected or not
	bool m_bMeasureValid; // Measurement for width/height is valid or not
public:
	FRIEND_SERIALIZER;
	template <class Archive>
	void save(Archive& ar)
	{
		ar("visible", m_bVisible);
	}
	template <class Archive>
	void load(Archive& ar)
	{
		//TODO m_pSheet = static_cast<sheet_xml_wiarchive&>(ar).GetSheetPtr();

		ar("visible", m_bVisible);
		m_bMeasureValid = true;//Width or Height are serialized
	}

public:
	//Constructor
	CBand(CSheet* pSheet = nullptr)
		:m_pSheet(pSheet),m_bVisible(true),m_bSelected(false),m_bMeasureValid(false){}
	//Destructor
	virtual ~CBand(){}

	CBand& ShallowCopy(const CBand& band)
	{
		m_pSheet = band.m_pSheet;
		m_bVisible = band.m_bVisible;
		m_bSelected = band.m_bSelected;
		m_bMeasureValid = band.m_bMeasureValid;	
		return *this;
	}
	CSheet* GetSheetPtr()const{return m_pSheet;}
	void SetSheetPtr(CSheet* pSheet){m_pSheet = pSheet;}
	bool GetMeasureValid()const{return m_bMeasureValid;}
	void SetMeasureValid(bool bMeasureValid){m_bMeasureValid = bMeasureValid;}
	virtual coordinates_type Offset()const {return kInvalidIndex;}// = 0;
	virtual bool GetVisible()const{return m_bVisible;}
	virtual void SetVisible(const bool& bVisible, bool notify = true) {}//=0;
	virtual bool GetSelected()const{return m_bSelected;}
	virtual void SetSelected(const bool& bSelected) {}//=0;
	virtual bool IsDragTrackable()const{return false;}
	virtual coordinates_type GetLeftTop()const { return kInvalidIndex; }//= 0;
	virtual coordinates_type GetRightBottom()/*TODO*/ { return kInvalidIndex; }//= 0;
	virtual void SetWidthHeightWithoutSignal(const coordinates_type&) {}//= 0;
};

