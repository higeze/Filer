#pragma once


//enum class SizingType
//{
//	None,
//	AlwaysFit,
//	Depend,
//	Independ,
//};

//enum class ColumnSizingType
//{
//	AlwaysFit_UnTrackable,
//	MinFitMaxInf_Trackable,
//	MinWrappableMaxInf_Trackable,
//	Free_Trackable,
//};
//
//enum class LengthType
//{
//	Value,
//	Fit,
//};
//
//enum class RowHeightType
//{
//	Value,
//	Fit
//};



//If Cell-Value, Cell-Size Changed
//Column:Fit Width to Max Cells Wi3dth // If Cell is Wrappable, Keep Width // If Cell is NotWrappable, Min-Fit, Max-Inf 

//MinColumn
//       Num Fit
//  Wrap O   O
//NoWrap O   O

//       Num, Fit, INF


//enum class RowSizingType
//{
//	AlwaysFit_UnTrackable,
//	MinFitMaxInf_Trackable,
//	Free_Trackable,
//};
//
//enum class CellSizingType
//{
//	Wrap,
//	NoWrap
//};
//

//enum class LineType
//{
//	None,
//	OneLine,
//	OneLineFitAlways,
//	MultiLine,
//	Both,
//};

enum class SizeType : std::uint8_t
{
	Trackable,
	Untrackable,
};


/**
 *  Sort
 *  Sort state
 */
enum class Sorts : std::uint8_t

{
	None,
	Up,
	Down
};


/**
 *  Compare
 *  Compare state
 */
enum class Compares : std::uint8_t
{
	Same,
	Diff,
	DiffNE
};

enum class Updates : std::uint8_t
{
	None,
	Filter,//Update Row IsVisible
	//Rect,//Move Window
	RowVisible,//AllContainer->VisContainer
	ColumnVisible,//AllContainer->VisContainer
	Sort,//Sort VisContainer
	Column,//Start&Length
	Row,//Start&Length
	Scrolls,
	EnsureVisibleFocusedCell,
	Invalidate,
	All
};