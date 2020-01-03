#pragma once
#include "GridView.h"

class CToDo;
struct ToDoGridViewProperty;


class CToDoGridView :public CGridView
{
public:
	CToDoGridView(std::shared_ptr<ToDoGridViewProperty>& spToDoGridViewProp);
	~CToDoGridView();

	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	/******************/
	/* Window Message */
	/******************/
	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};