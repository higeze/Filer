#pragma once


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
	RowVisible,
	ColumnVisible,
	Column,
	Row,
	EnsureVisibleFocusedCell,
	Scrolls,
	Invalidate,
	All
};