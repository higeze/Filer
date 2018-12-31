#pragma once


enum class LineType
{
	None,
	OneLine,
	OneLineFitAlways,
	MultiLine,
};

enum class SizeType
{
	Trackable,
	Untrackable,
};


/**
 *  Sort
 *  Sort state
 */
enum class Sorts
{
	None,
	Up,
	Down
};


/**
 *  Compare
 *  Compare state
 */
enum class Compares{
	Same,
	Diff,
	DiffNE
};

enum class Updates{
	None,
	Sort,
	Filter,
	Rect,
	RowVisible,
	ColumnVisible,
	Column,
	Row,
	Scrolls,
	EnsureVisibleFocusedCell,
	Invalidate,
	All
};