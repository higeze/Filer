#pragma once

#include "OGLUIElement.h"
#include "OGLEventArgs.h"
#include "Vertex.h"
#include "UIElement.h"

class COGLSeriesProperty;
//COGLSeries
class COGLSeries:public COGLUIElement
{
private:
	bool m_bVisible;
	std::wstring m_name;
public:
	std::shared_ptr<COGLSeriesProperty> m_spSeriesProperty;
	boost::multi_array<CVertex,2> m_mVertex;
public:
	COGLSeries(COGLUIElement* pParent, std::shared_ptr<COGLSeriesProperty> spSeriesProperty, unsigned int xSize, unsigned int ySize)
		:COGLUIElement(pParent),m_spSeriesProperty(spSeriesProperty),m_mVertex(boost::extents[xSize][ySize]),m_bVisible(true){}
	virtual ~COGLSeries(){}
	std::shared_ptr<COGLSeriesProperty> GetSeriesPropertyPtr()const{return m_spSeriesProperty;}
	void SetSeriesPropertyPtr(const std::shared_ptr<COGLSeriesProperty>& spProperty){m_spSeriesProperty = spProperty;}

	bool GetVisible()const{return m_bVisible;}
	void SetVisible(const bool& bVisible){m_bVisible=bVisible;}
	void SetName(const std::wstring& name){m_name = name;}
	std::wstring GetName()const{return m_name;}

	virtual void OnPaint(OGLPaintEventArgs& e);

	virtual void SetState(const UIElementState::Type& state)override;
};