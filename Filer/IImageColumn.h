#pragma once

class IImageColumn
{
public:
	virtual UINT32 GetImageSize()const = 0;
	virtual void SetImageSize(const UINT32& size) = 0;
};
