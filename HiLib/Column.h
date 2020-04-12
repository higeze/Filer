#pragma once
#include "Band.h"
#include "SheetEnums.h"
#include "MyFriendSerializer.h"
#include "named_arguments.h"
#include <float.h>

class CCell;
class CRow;
struct ColTag;
class Sheet;


class CColumn:public CBand
{
public:
	typedef ColTag Tag;
protected:
	Sorts m_sort; //Indicate sort state
	bool m_isInit = true; //if init is set, initial width is used

	SizeType m_sizeType = SizeType::Trackable;
	std::wstring m_filter; //Filter string

public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		CBand::save(ar);

		ar("sort", m_sort);
		ar("left", m_start);
		ar("width", m_length);
		//ar("minwidth", m_minWidth);
		//ar("maxwidth", m_maxWidth);
		m_isInit = false;
		ar("filter", m_filter);
	}
	template <class Archive>
	void load(Archive& ar)
	{
		CBand::load(ar);

		ar("sort", m_sort);
		ar("left", m_start);
		ar("width", m_length);
		//ar("minwidth", m_minWidth);
		//ar("maxwidth", m_maxWidth);
		m_isInit = false;
		m_isMeasureValid = true;//Width or Height are serialized
		ar("filter", m_filter);
	}
public:
	//Constructor
	template<typename... Args>
	CColumn(CSheet* pSheet = nullptr, Args... args)
		:CBand(pSheet),m_sort(Sorts::None),m_isInit(true)
	{
		m_isMinLengthFit = ::get(arg<"isminfit"_s>(), args..., default_(false));
		m_isMaxLengthFit = ::get(arg<"ismaxfit"_s>(), args..., default_(false));
		m_minLength = ::get(arg<"minwidth"_s>(), args..., default_(20.f));
		m_maxLength = ::get(arg<"maxwidth"_s>(), args..., default_(1000.f));
	}
	//Destructor
	virtual ~CColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)
	{
		CBand::ShallowCopy(column);
		m_sort =  column.m_sort;
		m_start = column.m_start;
		m_length = column.m_length;
		m_minLength = column.m_minLength;
		m_maxLength = column.m_maxLength;
		m_isInit = column.m_isInit;
		m_filter = column.m_filter;
		return *this;
	}

	virtual CColumn* CloneRaw()const{return nullptr;}
	std::shared_ptr<CColumn> Clone()const{return std::shared_ptr<CColumn>(CloneRaw());}

	virtual int GetSerializedIndex()const { return m_allIndex; }
	virtual std::wstring GetFilter()const{return m_filter;}
	virtual void SetFilter(const std::wstring& filter){m_filter = filter;}

	
	//Length
	virtual FLOAT GetLength() override;
	virtual FLOAT GetVirtualLength() override;
	virtual FLOAT GetFitLength() override;
	//Width
	virtual FLOAT GetWidth() { return GetLength(); }
	virtual FLOAT GetFitWidth() { return GetFitLength(); }
	virtual void SetWidth(const FLOAT width, bool notify = true) { SetLength(width, notify); }

	//Start/End	
	//Left/Right
	virtual FLOAT GetLeft() { return GetStart(); }
	virtual void SetLeft(const FLOAT left, bool notify = true) { SetStart(left); }
	virtual FLOAT GetRight() { return GetEnd(); }


	virtual Sorts GetSort()const{return m_sort;};
	virtual void SetSort(const Sorts& sort);
	virtual void SetVisible(const bool& bVisible, bool notify = true)override;
	virtual void SetIsSelected(const bool& bSelected)override;

	virtual bool Paste(std::shared_ptr<CCell> spCellDst, std::shared_ptr<CCell> spCellSrc){return false;}
	virtual bool Paste(std::shared_ptr<CCell> spCellDst, std::wstring source){return false;}
	virtual void Delete(std::shared_ptr<CCell> spCellDst){}
	virtual bool HasCell()const { return true; }
	virtual std::shared_ptr<CCell>& Cell(CRow* pRow) = 0;
	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; }
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; }
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; }
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) = 0;
	virtual void InsertNecessaryRows(){};
//	virtual SizingType GetSizingType()const override { return SizingType::None; }
	virtual SizeType GetSizeType()const { return m_sizeType; }
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) override;
	virtual void OnPropertyChanged(const wchar_t* name);
};