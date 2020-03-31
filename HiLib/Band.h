#pragma once
#include "SheetEnums.h"
#include <WTypesbase.h>
#include <float.h>
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
	CSheet* m_pSheet = nullptr; // Parent sheet pointer
	bool m_bVisible = true; // Visible or not
	bool m_bSelected = false; // Selected or not

	FLOAT m_start = 0.f;
	bool m_isMeasureValid = false; // Measurement for width/height is valid or not
	FLOAT m_length = 0.f;
	bool m_isFitMeasureValid = false; // Measurement for fit width/height is valid or not
	FLOAT m_fitLength = 0.f;
	bool m_isMinLengthFit = false;
	FLOAT m_minLength = 16.f;
	bool m_isMaxLengthFit = false;
	FLOAT m_maxLength = FLT_MAX;

	int m_allIndex = kInvalidIndex;
	int m_visIndex = kInvalidIndex;
public:
	template <class Archive>
	void save(Archive& ar)
	{
		ar("visible", m_bVisible);
		ar("allindex", m_allIndex);
		ar("visindex", m_visIndex);
	}
	template <class Archive>
	void load(Archive& ar)
	{
		ar("visible", m_bVisible);
		ar("allindex", m_allIndex);
		ar("visindex", m_visIndex);
	}

public:
	//Constructor
	CBand(CSheet* pSheet = nullptr)
		:m_pSheet(pSheet){}
	//Destructor
	virtual ~CBand(){}

	CBand& ShallowCopy(const CBand& band)
	{
		m_pSheet = band.m_pSheet;
		m_bVisible = band.m_bVisible;
		m_bSelected = band.m_bSelected;
		m_isMeasureValid = band.m_isMeasureValid;
		m_isFitMeasureValid = band.m_isFitMeasureValid;	
		return *this;
	}
	CSheet* GetSheetPtr()const{return m_pSheet;}
	void SetSheetPtr(CSheet* pSheet){m_pSheet = pSheet;}
	bool GetIsMeasureValid()const{return m_isMeasureValid;}
	void SetIsMeasureValid(bool bMeasureValid){m_isMeasureValid = bMeasureValid;}
	void SetIsFitMeasureValid(bool isFitMeasureValid) { m_isFitMeasureValid = isFitMeasureValid; }
	virtual bool GetVisible()const{return m_bVisible;}
	virtual void SetVisible(const bool& bVisible, bool notify = true) = 0;
	virtual bool GetSelected()const{return m_bSelected;}
	virtual void SetSelected(const bool& bSelected) = 0;
	virtual bool IsDragTrackable()const{return false;}
	virtual bool IsTrackable()const { return false; }
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) = 0;
	virtual void OnPropertyChanged(const wchar_t* name) = 0;
	//virtual SizingType GetSizingType()const = 0;

	virtual FLOAT GetStart() { return m_start; }
	virtual void SetStart(const FLOAT start, bool notify = true);
	virtual FLOAT GetEnd() { return m_start + m_length; }

	virtual FLOAT GetLength() = 0;
	virtual FLOAT GetVirtualLength() = 0;
	virtual FLOAT GetFitLength() = 0;
	virtual void SetLength(const FLOAT length, bool notify = true);
	virtual void FitLength(bool notify = true);
	virtual void BoundLength(bool notify = true);

	
	template<typename T>
	int GetIndex() const{return kInvalidIndex; }
	template<> inline int GetIndex<AllTag>() const { return m_allIndex; }
	template<> inline int GetIndex<VisTag>() const { return m_visIndex; }

	template<typename T>
	void SetIndex(const int index){}
	template<> inline void SetIndex<AllTag>(const int index){ m_allIndex = index; }
	template<> inline void SetIndex<VisTag>(const int index){ m_visIndex = index; }
};

