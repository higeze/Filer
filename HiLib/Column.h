#pragma once
#include "Band.h"
#include "SheetEnums.h"
class CCell;
class CRow;
class Sheet;

class CColumn:public CBand
{
public:
	static const coordinates_type kMinWidth = 16;
protected:
	Sorts m_sort; //Indicate sort state
	coordinates_type m_left; //left position from parent sheet
	coordinates_type m_width; //width
	bool m_isInit; //if init is set, initial width is used
	std::wstring m_filter; //Filter string
public:
	//boost::serialization
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
		ar & boost::serialization::make_nvp("Band", boost::serialization::base_object<CBand>(*this));

		ar & boost::serialization::make_nvp("sort", m_sort);
		ar & boost::serialization::make_nvp("left", m_left);
		ar & boost::serialization::make_nvp("width", m_width);
		m_isInit = false;
		//m_isSerialized = true;
		ar & boost::serialization::make_nvp("filter", m_filter);
    }
public:
	//Constructor
	CColumn(CSheet* pSheet = nullptr)
		:CBand(pSheet),m_sort(Sorts::None),m_left(0),m_width(0),m_isInit(true)/*,m_isSerialized(false)*/{}
	//Destructor
	virtual ~CColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)
	{
		CBand::ShallowCopy(column);
		m_sort =  column.m_sort;
		m_left = column.m_left;
		m_width = column.m_width;
		m_isInit = column.m_isInit;
		m_filter = column.m_filter;
		return *this;
	}

	virtual CColumn* CloneRaw()const{return nullptr;}
	std::shared_ptr<CColumn> Clone()const{return std::shared_ptr<CColumn>(CloneRaw());}

	virtual std::wstring GetFilter()const{return m_filter;}
	virtual void SetFilter(const std::wstring& filter){m_filter = filter;}
	virtual coordinates_type GetWidth();
	virtual void SetWidth(const coordinates_type& width);
	virtual void SetWidthWithoutSignal(const coordinates_type& width){m_width=max(width,kMinWidth);}
	virtual coordinates_type GetLeft()const{return  m_left + Offset();}
	virtual void SetSheetLeft(const coordinates_type left){m_left=left;}
	virtual void SetSheetLeftWithoutSignal(const coordinates_type left){m_left=left;}
	virtual coordinates_type GetRight(){return GetLeft() + GetWidth();}
	virtual Sorts GetSort()const{return m_sort;};
	virtual void SetSort(const Sorts& sort);
	virtual void SetVisible(const bool& bVisible, bool notify = true)override;
	virtual void SetSelected(const bool& bSelected)override;
	template<typename TAV>
	size_type GetIndex()const
	{
		return m_pSheet->Pointer2Index<ColTag, TAV>(this);
	}

	virtual bool Paste(std::shared_ptr<CCell> spCellDst, std::shared_ptr<CCell> spCellSrc){return false;}
	virtual bool Paste(std::shared_ptr<CCell> spCellDst, std::wstring source){return false;}
	virtual void Delete(std::shared_ptr<CCell> spCellDst){}
	virtual cell_type& Cell(CRow* pRow)=0;
	virtual cell_type HeaderCellTemplate( CRow* pRow,  CColumn* pColumn)=0;
	virtual cell_type FilterCellTemplate( CRow* pRow,  CColumn* pColumn)=0;
	virtual cell_type CellTemplate( CRow* pRow,  CColumn* pColumn)=0;
	virtual void InsertNecessaryRows(){};
};

BOOST_CLASS_EXPORT_KEY(CColumn);

class CGridView;

class CParentColumn:public CColumn
{
public:
	static const coordinates_type kMinWidth = 2;
protected:
public:
	//boost::serialization
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
		ar & boost::serialization::make_nvp("Column", boost::serialization::base_object<CColumn>(*this));
    }
public:
	//Constructor
	CParentColumn(CGridView* pGrid = nullptr);
	//Destructor
	virtual ~CParentColumn(){}
	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CColumn::ShallowCopy(column);
		return *this;
	}
	virtual CParentColumn* CloneRaw()const{return nullptr;}
	std::shared_ptr<CParentColumn> Clone()const{return std::shared_ptr<CParentColumn>(CloneRaw());}
	virtual coordinates_type Offset()const;
	virtual cell_type HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn) = 0;
	virtual cell_type NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) = 0;
};

BOOST_CLASS_EXPORT_KEY(CParentColumn);

class CSheetCell;

class CChildColumn:public CColumn
{
public:
	//Constructor
	CChildColumn(CSheetCell* pSheetCell);
	//Destructor
	virtual ~CChildColumn(){}
	virtual coordinates_type Offset()const;
};
