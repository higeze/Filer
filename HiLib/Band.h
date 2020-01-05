#pragma once
#include "MyFriendSerializer.h"
#include "SheetEnums.h"
class CSheet;
class CCell;

struct AllTag{};
struct VisTag{};
struct PntTag {};

class CBand
{
public:
	static const int kMaxIndex = 10000;
	static const int kMinIndex = -10;
	static const int kInvalidIndex = -9999;
	inline static const FLOAT kResizeAreaHarfWidth = 4.0f;
protected:
	//Field
	CSheet* m_pSheet; // Parent sheet pointer
	bool m_bVisible; // Visible or not
	bool m_bSelected; // Selected or not
	bool m_bMeasureValid; // Measurement for width/height is valid or not

	int m_allIndex = -9999;
	int m_visIndex = -9999;
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
	virtual bool GetVisible()const{return m_bVisible;}
	virtual void SetVisible(const bool& bVisible, bool notify = true) = 0;
	virtual bool GetSelected()const{return m_bSelected;}
	virtual void SetSelected(const bool& bSelected) = 0;
	virtual bool IsDragTrackable()const{return false;}
	virtual bool IsTrackable()const { return false; }
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) = 0;
	virtual void OnPropertyChanged(const wchar_t* name) = 0;
	virtual SizingType GetSizingType()const = 0;
	
	template<typename T>
	int GetIndex() const{return kInvalidIndex; }
	template<> inline int GetIndex<AllTag>() const { return m_allIndex; }
	template<> inline int GetIndex<VisTag>() const { return m_visIndex; }

	template<typename T>
	void SetIndex(const int index){}
	template<> inline void SetIndex<AllTag>(const int index){ m_allIndex = index; }
	template<> inline void SetIndex<VisTag>(const int index){ m_visIndex = index; }

};

