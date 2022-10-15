//#pragma once
#include <d2d1_1.h>
#include <D2d1_1helper.h>
#pragma comment (lib, "D2d1.lib")
#include <dwrite_1.h>
#pragma comment (lib, "dwrite.lib")
#include <d3d11_1.h>
#pragma comment (lib, "d3d11.lib")

#include <wincodec.h>
//#include <wrl/client.h>
//using namespace Microsoft::WRL;
#include "atlcomcli.h"
#include <exception>
#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "MyPoint.h"
#include "MyRect.h"
#include "MyFont.h"
#include "Debug.h"
#include <fmt/format.h>

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
		CPointT(T a, T b) :TPoint{ a, b } {}
		void SetPoint(T a, T b) { this->x = a; this->y = b; }
		void Offset(T xOffset, T yOffset) { this->x += xOffset; this->y += yOffset; }
		void Offset(point_type& pt) { this->x += pt.x; this->y += pt.y; }
		point_type OffsetCopy(point_type& pt) { return point_type(this->x + pt.x, this->y + pt.y); }

		point_type operator -() const { return point_type(-this->x, -this->y); }
		point_type operator +(point_type pt) const
		{
			return CPointF(this->x + pt.x, this->y + pt.y);
		}
		point_type operator -(point_type pt) const
		{
			return CPointF(this->x - pt.x, this->y - pt.y);
		}
		point_type& operator +=(const point_type& pt)
		{
			this->x += pt.x; this->y += pt.y; return *this;
		}
		point_type operator *(const T& z) const
		{
			return CPointF(this->x * z, this->y * z);
		}
		point_type operator /(const T& z) const
		{
			return CPointF(this->x / z, this->y / z);
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
		CSizeT(T w, T h) :TSize{ w, h } {}
		CSizeT(const size_type& size)
		{
			this->width = size.width;
			this->height = size.height;
		}
		bool operator==(const size_type& rhs) const
		{
			return this->width == rhs.width && this->height == rhs.height;
		}

		bool operator!=(const size_type& rhs) const
		{
			return !operator==(rhs);
		}

		std::size_t GetHashCode() const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, std::hash<decltype(this->width)>()(this->width));
			boost::hash_combine(seed, std::hash<decltype(this->height)>()(this->height));
			return seed;
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

		CRectT() :TRect{ 0.0f } {}
		CRectT(T l, T t, T r, T b) :TRect{ l,t,r,b } {}
		CRectT(const size_type& size) :TRect{ 0, 0, size.width, size.height } {}
		void SetRect(T l, T t, T r, T b)
		{
			this->left = l; this->top = t; this->right = r; this->bottom = b;
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
		FLOAT Width()const { return this->right - this->left; }
		FLOAT Height()const { return this->bottom - this->top; }

		void InflateRect(T x, T y) { this->left -= x; this->right += x; this->top -= y; this->bottom += y; }
		void DeflateRect(T x, T y) { this->left += x; this->right -= x; this->top += y; this->bottom -= y; }
		void InflateRect(T x) { this->left -= x; this->right += x; this->top -= x; this->bottom += x; }
		void DeflateRect(T x) { this->left += x; this->right -= x; this->top += x; this->bottom -= x; }
		bool PtInRect(const point_type& pt) const { return pt.x >= this->left && pt.x <= this->right && pt.y >= this->top && pt.y <= this->bottom; }
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
		rect_type& operator+=(point_type rc)
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
		rect_type& operator-=(point_type rc)
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
//
//#include <list>
//#include <map>
//
//template <class T>
//class CUndoInfoStack 
//{
//	protected :
//		CUndoInfoStack( int maxsize ):m_max_size(maxsize){};
//		~CUndoInfoStack(){ flush(); }
//	public :
//		void push( void* addr, T t )
//		{
//			if ( m_map.find( addr ) == m_map.end() ) 
//			{
//				m_map[addr] = new std::list<T>();
//			}
//
//			std::list<T>* ls = m_map[addr];
//			ls->push_back( t );
//			
//			while ( (int)ls->size() > m_max_size )
//			{
//				ls->pop_front();
//			}						
//								
//		}
//		
//		T pop( void* addr )
//		{
//			std::list<T>* ls = m_map[addr];
//			T t = ls->back();			
//			ls->pop_back();
//			
//			return t;		
//		}
//		int size( void* addr )
//		{
//			return (int)m_map[addr]->size();
//		}			 
//		void flush()
//		{
//			std::map<void*, std::list<T>* >::iterator it = m_map.begin();
//			
//			while( it != m_map.end() )
//			{
//				std::list<T>* ls = it->second;
//				
//				delete ls;
//				
//				it++;
//				
//			}
//			
//			m_map.clear();
//		
//		}
//		
//		static CUndoInfoStack<T>& FactorySingleton();
//	
//	protected :
//		
//		std::map<void*, std::list<T>* > m_map;
//		int m_max_size;
//		
//
//
//};
//
//
//
//class CUndoInfo
//{
//	public :
//		CUndoInfo(){};
//		enum TYPE{ DEL, ADD };
//		TYPE m_type;
//		int m_textPosition;
//		int m_length;
//		std::wstring m_text;
//};
//
//typedef CUndoInfoStack<CUndoInfo>	CUndoInfoStackEx;	
//
//
//class CSelected
//{
//	public :
//		CSelected( int textPostion ):m_textposition_start(textPostion),m_textposition_end(textPostion){}
//
//		void SetEnd( int textPosition ){ m_textposition_end = textPosition; }
//		int StartPostion(){ return (std::min)(m_textposition_start,m_textposition_end); }
//		int Length(){ return std::abs(m_textposition_start-m_textposition_end); }
//			
//	protected :
//		int m_textposition_start;
//		int m_textposition_end;
//};