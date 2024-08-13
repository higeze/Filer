#pragma once
#include "Direct2DWrite.h"

class CWICImagingFactory
{
public:
	static const CComPtr<IWICImagingFactory2>& GetInstance()
	{
		static CComPtr<IWICImagingFactory2> pFactory;
		if (!pFactory) {
			FAILED_THROW(pFactory.CoCreateInstance(
				CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER));
		}
		return pFactory;
	}
};
