#pragma once

namespace V4 
{


struct D2D_OBJECT_UPDATE
{
	enum TYP{ NONE,NEWPARENT,TRYNEWPARENT_FAIL,MOVE,SUCCESS,TRY_NEWPARENT };
	
	TYP stat;
	d2dw::CPointF globalpt;
	void* object;
};








};





