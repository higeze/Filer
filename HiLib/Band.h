#pragma once
#include "SheetEnums.h"
#include <WTypesbase.h>
#include <float.h>
#include "JsonSerializer.h"

class CGridView;
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
	CGridView* m_pGrid = nullptr; // Parent sheet pointer
	std::shared_ptr<bool> m_spVisible = std::make_shared<bool>(true); // Visible or not
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
	//Constructor
	CBand(CGridView* pSheet = nullptr)
		:m_pGrid(pSheet){}
	//Destructor
	virtual ~CBand(){}

	CBand& ShallowCopy(const CBand& band)
	{
		m_pGrid = band.m_pGrid;
		*m_spVisible = *(band.m_spVisible);
		m_bSelected = band.m_bSelected;
		m_isMeasureValid = band.m_isMeasureValid;
		m_isFitMeasureValid = band.m_isFitMeasureValid;	
		return *this;
	}
	CGridView* GetGridPtr()const{return m_pGrid;}
	void SetSheetPtr(CGridView* pSheet){m_pGrid = pSheet;}
	bool GetIsMeasureValid()const{return m_isMeasureValid;}
	void SetIsMeasureValid(bool bMeasureValid){m_isMeasureValid = bMeasureValid;}
	void SetIsFitMeasureValid(bool isFitMeasureValid) { m_isFitMeasureValid = isFitMeasureValid; }
	virtual bool GetIsVisible()const{return *m_spVisible;}
	virtual void SetIsVisible(const bool& bVisible, bool notify = true) = 0;
	virtual bool GetIsSelected()const{return m_bSelected;}
	virtual void SetIsSelected(const bool& bSelected) = 0;
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

public:
	template <class Archive>
	void save(Archive& ar)
	{
		ar("visible", *m_spVisible);
		ar("allindex", m_allIndex);
		ar("visindex", m_visIndex);
	}
	template <class Archive>
	void load(Archive& ar)
	{
		ar("visible", *m_spVisible);
		ar("allindex", m_allIndex);
		ar("visindex", m_visIndex);
	}


	friend void to_json(json& j, const CBand& o)
	{
		j = json{
			{"visible", o.m_spVisible},
			{"allindex", o.m_allIndex},
			{"visindex", o.m_visIndex }
		};

	}
	friend void from_json(const json& j, CBand& o)
	{	
		j.at("visible").get_to(o.m_spVisible);
		j.at("allindex").get_to(o.m_allIndex);
		j.at("visindex").get_to(o.m_visIndex);
	}
};



