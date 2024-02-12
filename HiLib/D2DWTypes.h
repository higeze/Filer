#pragma once
#include <d2d1_1.h>
#include "JsonSerializer.h"
// 
template<typename TRect, typename TSize, typename TPoint, typename T> struct CPointT;
template<typename TRect, typename TSize, typename TPoint, typename T> struct CSizeT;
template<typename TRect, typename TSize, typename TPoint, typename T> struct CRectT;

struct XTag;
struct YTag;

template<typename ...>
constexpr bool false_v = false;

template<typename TRect, typename TSize, typename TPoint, typename T>
struct CPointT :public TPoint
{
	using rect_type = CRectT<TRect, TSize, TPoint, T>;
	using size_type = CSizeT<TRect, TSize, TPoint, T>;
	using point_type = CPointT<TRect, TSize, TPoint, T>;

	struct XTag{};
	struct YTag{};

	CPointT() :TPoint{ 0.0f } {}
	CPointT(const T& a, const T& b) :TPoint{ a, b } {}
	CPointT(const point_type& pt) :CPointT(pt.x, pt.y) {}
	CPointT(const TPoint& pt) : CPointT(pt.x, pt.y) {}
	void SetPoint(const T& a, const T& b) { this->x = a; this->y = b; }
	void Offset(const T& xOffset, const T& yOffset) { this->x += xOffset; this->y += yOffset; }
	void Offset(const point_type& pt) { this->x += pt.x; this->y += pt.y; }
	point_type OffsetCopy(const point_type& pt) { return point_type(this->x + pt.x, this->y + pt.y); }
	point_type OffsetXCopy(const T& xOffset) { return point_type(this->x + xOffset, this->y); }
	const TPoint* operator&() const
	{
		return static_cast<const TPoint*>(this);
	}
	point_type operator -() const { return point_type(-this->x, -this->y); }
	point_type operator +(point_type pt) const
	{
		return point_type(this->x + pt.x, this->y + pt.y);
	}
	point_type operator -(point_type pt) const
	{
		return point_type(this->x - pt.x, this->y - pt.y);
	}
	point_type& operator +=(const point_type& pt)
	{
		this->x += pt.x; this->y += pt.y; return *this;
	}
	point_type operator *(const T& z) const
	{
		return point_type(this->x * z, this->y * z);
	}
	point_type operator /(const T& z) const
	{
		return point_type(this->x / z, this->y / z);
	}
	bool operator==(const point_type& pt)const
	{
		return (this->x == pt.x && this->y == pt.y);
	}
	bool operator!=(const point_type& pt)const
	{
		return (this->x != pt.x || this->y != pt.y);
	}

	template<typename TXY>
	FLOAT Get() const
	{
		if constexpr (std::is_same_v<TXY, XTag>) {
			return this->x;
		} else if constexpr (std::is_same_v<TXY, YTag>) {
			return this->y;
		} else {
			static_assert(false_v<TXY>);
		}
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		point_type,
		x,
		y)
};
	
template<typename TRect, typename TSize, typename TPoint, typename T>
struct CSizeT :public TSize
{
	using rect_type = CRectT<TRect, TSize, TPoint, T>;
	using size_type = CSizeT<TRect, TSize, TPoint, T>;
	using point_type = CPointT<TRect, TSize, TPoint, T>;

	CSizeT() :TSize{ 0.0f } {}
	CSizeT(const T& w, const T& h) :TSize{ w, h } {}
	CSizeT(const size_type& size) :CSizeT(size.width, size.height) {}
	CSizeT(const TSize& size) :CSizeT(size.width, size.height) {}
	const TSize* operator&() const
	{
		return static_cast<const TSize*>(this);
	}
	bool operator==(const size_type& rhs) const
	{
		return this->width == rhs.width && this->height == rhs.height;
	}

	bool operator!=(const size_type& rhs) const
	{
		return !operator==(rhs);
	}

	size_type operator+(size_type rhs)const
	{
		return size_type(this->width + rhs.width, this->height + rhs.height);
	}


	std::size_t GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(this->width)>()(this->width));
		boost::hash_combine(seed, std::hash<decltype(this->height)>()(this->height));
		return seed;
	}

	size_type operator*(FLOAT z)const
	{
		return size_type(this->width * z, this->height * z);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		size_type,
		width,
		height)
};

template<typename TRect, typename TSize, typename TPoint, typename T>
struct CRectT :public TRect
{
	using rect_type = CRectT<TRect, TSize, TPoint, T>;
	using size_type = CSizeT<TRect, TSize, TPoint, T>;
	using point_type = CPointT<TRect, TSize, TPoint, T>;

	CRectT() :TRect{ 0 } {}
	CRectT(const T& l, const T& t, const T& r, const T& b) :TRect{ l, t, r, b } {}
	CRectT(const rect_type& rc) :CRectT(rc.left, rc.top, rc.right, rc.bottom) {}
	CRectT(const TRect& rc) :CRectT(rc.left, rc.top, rc.right, rc.bottom) {}
	CRectT(const TPoint& pt, const TSize& sz) :TRect(pt.x, pt.y, pt.x + sz.width, pt.y + sz.height) {}
	CRectT(const TPoint& pt1, const TPoint& pt2) :TRect(pt1.x, pt1.y, pt2.x, pt2.y) {}
	CRectT(const size_type& size) :TRect{ 0, 0, size.width, size.height } {}
	void SetRect(const T& l, const T& t, const T& r, const T& b)
	{
		this->left = l; this->top = t; this->right = r; this->bottom = b;
	}
	void SetPointSize(const TPoint& pt, const TSize& sz)
	{
		this->left = pt.x; this->top = pt.y; this->right = pt.x + sz.width; this->bottom = pt.y + sz.height;
	}
	bool IsRectNull() const
	{
		return this->left == 0 && this->top == 0 && this->right == 0 && this->bottom == 0;
	}
	void MoveToX(T x) { this->right = x + this->right - this->left; this->left = x; }
	void MoveToY(T y) { this->bottom = y + this->bottom - this->top; this->top = y; }
	void MoveToXY(T x, T y)
	{
		this->right += x - this->left;
		this->bottom += y - this->top;
		this->left = x;
		this->top = y;
	}
	void MoveToXY(const point_type& pt)
	{
		MoveToXY(pt.x, pt.y);
	}
	void OffsetRect(T x, T y) { this->left += x; this->right += x; this->top += y;this-> bottom += y; }
	void OffsetRect(const point_type& pt) { OffsetRect(pt.x, pt.y); }
	void OffsetX(T x) { this->left += x; this->right += x; }
	void OffsetY(T y) { this->top += y;this-> bottom += y; }
	T Width()const { return this->right - this->left; }
	T Height()const { return this->bottom - this->top; }

	void InflateRect(T x, T y) { this->left -= x; this->right += x; this->top -= y; this->bottom += y; }
	void DeflateRect(T x, T y) { this->left += x; this->right -= x; this->top += y; this->bottom -= y; }
	void InflateRect(T x) { this->left -= x; this->right += x; this->top -= x; this->bottom += x; }
	void DeflateRect(T x) { this->left += x; this->right -= x; this->top += x; this->bottom -= x; }
	rect_type DeflateRectCopy(T x) { return rect_type(this->left + x, this->top + x, this->right - x, this->bottom - x); }
	bool PtInRect(const point_type& pt) const { return pt.x >= this->left && pt.x <= this->right && pt.y >= this->top && pt.y <= this->bottom; }
	bool RectInRect(const rect_type& rc) const { return rc.left >= this->left && rc.top >= this->top && rc.right <= this->right && rc.bottom <= this->bottom; }
	bool SizeInRect(const size_type& sz) const { return sz.width <= this->Width() && sz.height <= this->Height(); }
	rect_type IntersectRect(const rect_type& rc) const 
	{
		T l = (std::max)(this->left, rc.left);
		T t = (std::max)(this->top, rc.top);
		T r = (std::min)(this->right, rc.right);
		T b = (std::min)(this->bottom, rc.bottom);
		if (l > r || t > b) {
			return rect_type();
		} else {
			return rect_type(l, t, r, b);
		}
	}
	point_type LeftTop() const { return point_type(this->left, this->top); }
	point_type RightBottom() const { return point_type(this->right, this->bottom); }
	point_type CenterPoint() const
	{
		return point_type((this->left + this->right) / 2, (this->top + this->bottom) / 2);
	}

	T CenterX() const
	{
		return (this->left + this->right) / 2.f;
	}
	T CenterY() const
	{
		return (this->top + this->bottom) / 2.f;
	}

	size_type Size() const
	{
		return size_type(Width(), Height());
	}
	void InflateRect(const rect_type& rc)
	{
		this->left -= rc.left;
		this->top -= rc.top;
		this->right += rc.right;
		this->bottom += rc.bottom;
	}
	void DeflateRect(const rect_type& rc)
	{
		this->left += rc.left;
		this->top += rc.top;
		this->right -= rc.right;
		this->bottom -= rc.bottom;
	}

	rect_type DeflateRectCopy(const rect_type& rc) const
	{
		return rect_type(
			this->left + rc.left,
			this->top + rc.top,
			this->right - rc.right,
			this->bottom - rc.bottom);
	}

	//const TRect* operator&() const
	//{
	//	return static_cast<const TRect*>(this);
	//}

	rect_type operator+(rect_type rc)const
	{
		rect_type ret(*this);
		ret.InflateRect(rc);
		return ret;
	}
	rect_type operator+(point_type pt)const
	{
		rect_type ret(*this);
		ret.OffsetRect(pt);
		return ret;
	}
	rect_type& operator+=(rect_type rc)
	{
		this->InflateRect(rc);
		return *this;
	}

	rect_type operator-(rect_type rc)const
	{
		rect_type ret(*this);
		ret.DeflateRect(rc);
		return ret;
	}
	rect_type operator-(point_type pt)const
	{
		rect_type ret(*this);
		ret.OffsetRect(-pt);
		return ret;
	}
	rect_type& operator-=(rect_type rc)
	{
		this->DeflateRect(rc);
		return *this;
	}
	bool operator==(const rect_type& rc)const 
	{ 
		return this->left == rc.left && this->top == rc.top && this->right == rc.right && this->bottom == rc.bottom;
	}
	bool operator!=(const rect_type& rc)const 
	{ 
		return !operator==(rc);
	}
	rect_type operator*(FLOAT z)const
	{
		return rect_type(this->left * z, this->top * z, this->right * z, this->bottom * z);
	}
	rect_type operator/(FLOAT z)const
	{
		return rect_type(this->left / z, this->top / z, this->right / z, this->bottom / z);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(rect_type,
		left,
		top,
		right,
		bottom)
};

using CRectF = CRectT<D2D1_RECT_F, D2D1_SIZE_F, D2D1_POINT_2F, FLOAT>;
using CSizeF = CSizeT<D2D1_RECT_F, D2D1_SIZE_F, D2D1_POINT_2F, FLOAT>;
using CPointF = CPointT<D2D1_RECT_F, D2D1_SIZE_F, D2D1_POINT_2F, FLOAT>;

using CRectU = CRectT<D2D1_RECT_U, D2D1_SIZE_U, D2D1_POINT_2U, UINT32>;
using CSizeU = CSizeT<D2D1_RECT_U, D2D1_SIZE_U, D2D1_POINT_2U, UINT32>;
using CPointU = CPointT<D2D1_RECT_U, D2D1_SIZE_U, D2D1_POINT_2U, UINT32>;

CRectF CRectU2CRectF(const CRectU& rc);
CRectU CRectF2CRectU(const CRectF& rc);
CSizeU CSizeF2CSizeU(const CSizeF& sz);
CSizeF CSizeU2CSizeF(const CSizeU& sz);
CPointU CPointF2CPointU(const CPointF& pt);
CPointF CPointU2CPointF(const CPointU& pt);
