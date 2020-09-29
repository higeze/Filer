#pragma once
#include <functional>
#include <vector>
#include <tuple>
#include <sigslot/signal.hpp>

template <class T>
class Subject
{
private:
	sigslot::signal<T> m_onNextSignal;
public:
	Subject() {}
	virtual ~Subject() = default;//disconnect_all in default destructor
	virtual void OnNext(const T& value);
	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next);
};

template<class T>
inline void Subject<T>::OnNext(const T& value)
{
	m_onNextSignal(value);
}

template<class T>
inline sigslot::connection Subject<T>::Subscribe(std::function<void(const T& value)> next)
{
	return m_onNextSignal.connect(next);
}

template<class T>
class IReactiveProperty
{
public:
	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next) = 0;
	virtual operator T() const { return get(); }
	virtual const T& get() const = 0;
	//virtual void set(const T& value) = 0;
	virtual void set(const T& value) = 0;
};

template<typename T>
std::wostream& operator<<(std::wostream& os, const IReactiveProperty<T>& reactive)
{
	os << reactive.get();
	return os;
}

template <typename T>
std::wistream& operator>>(std::wistream& is, IReactiveProperty<T>& reactive)
{
	T value;
	is >> value;
	reactive.set(value);
	return is;
}

template <class T>
class ReactiveProperty:public IReactiveProperty<T>
{
protected:
	std::shared_ptr<Subject<T>> m_pSubject;
	T m_value;
public:
	ReactiveProperty():
		m_pSubject(std::make_shared<Subject<T>>()),
		m_value(){};

	ReactiveProperty(const T& value):
		m_pSubject(std::make_shared<Subject<T>>()),
		m_value(value){};
	~ReactiveProperty() = default;
	ReactiveProperty(const ReactiveProperty& val) = default;
	ReactiveProperty(ReactiveProperty&& val) = default;
	ReactiveProperty& operator=(const ReactiveProperty& val) = default;
	ReactiveProperty& operator=(ReactiveProperty&& val) = default;

	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next)
	{
		return m_pSubject->Subscribe(next);
	}
	virtual const T& get() const override { return m_value; }
	void set(const T& value) override
	{
		if (m_value != value) {
			m_value = value;
			m_pSubject->OnNext(m_value);
		}
	}
	void force_notify_set(const T& value)
	{
		m_value = value;
		m_pSubject->OnNext(m_value);
	}
};

template<class T, class U>
class ReactiveDetailProperty :public ReactiveProperty<T>
{
protected:
	std::shared_ptr<Subject<U>> m_pDetailSubject;
public:
	ReactiveDetailProperty() :
		ReactiveProperty(),
		m_pDetailSubject(std::make_shared <Subject<U>>()){}

	ReactiveDetailProperty(const T& value) :
		ReactiveProperty(value),
		m_pDetailSubject(std::make_shared <Subject<U>>())
	{};

	virtual ~ReactiveDetailProperty() = default;

	virtual sigslot::connection SubscribeDetail(std::function<void(const U& value)> next)
	{
		return m_pDetailSubject->Subscribe(next);
	}
};




enum class NotifyStringChangedAction
{
	Assign,
	Insert,
	Erase,
	Replace,
	Clear,
};

template<typename CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
struct NotifyStringChangedEventArgs
{
	NotifyStringChangedAction Action;
	std::basic_string<CharT, Traits, Allocator> NewString;
	std::basic_string<CharT, Traits, Allocator> OldString;
	int StartIndex = -1;
	int OldEndIndex = -1;
	int NewEndIndex = -1;
};

template <class CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
class ReactiveBasicStringProperty: public ReactiveDetailProperty<std::basic_string<CharT, Traits, Allocator>, NotifyStringChangedEventArgs<CharT>>
{
	using str_type = typename std::basic_string<CharT, Traits, Allocator>;
	using size_type = typename  std::basic_string<CharT, Traits, Allocator>::size_type;
	using const_iterator = typename std::basic_string<CharT, Traits, Allocator>::const_iterator;
	using const_reference = typename std::basic_string<CharT, Traits, Allocator>::const_reference;
//	using reference = typename std::basic_string<CharT, Traits, Allocator>::reference;

public:
	ReactiveBasicStringProperty() :
		ReactiveDetailProperty(){}
	
	ReactiveBasicStringProperty(const str_type& value) :
		ReactiveDetailProperty(value){}
	
	~ReactiveBasicStringProperty() = default;

	const_reference operator[](size_type pos) const noexcept
	{
		return m_value.operator[](pos);
	}

	//reference operator[](size_type pos) noexcept
	//{
	//	return m_value.operator[](pos);
	//}

	const_iterator cbegin() const noexcept
	{
		return m_value.cend();
	}

	const_iterator cend() const noexcept
	{
		return m_value.cbegin();
	}

	size_type size() const noexcept
	{
		return m_value.size();
	}

	bool empty() const noexcept
	{
		return m_value.empty();
	}

	const CharT& front() const
	{
		return m_value.front();
	}
	
	const CharT& back() const
	{
		return m_value.back();
	}

	str_type substr(size_type pos = 0, size_type n = npos) const
	{
		return m_value.substr();
	}

	const CharT* c_str() const noexcept
	{
		return m_value.c_str();
	}

	const CharT* data() const noexcept
	{
		return m_value.data();
	}
	
	ReactiveBasicStringProperty<CharT, Traits, Allocator>& assign(const str_type& value)
	{
		if (m_value != value) {
			str_type old(m_value);
			m_value.assign(value);
			m_pDetailSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
				NotifyStringChangedAction::Assign,
					m_value,
					old,
					0, (int)old.size(), (int)m_value.size()
			});
			m_pSubject->OnNext(m_value);
		}
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& insert(size_type index, const str_type& value)
	{
		str_type old(m_value);
		m_value.insert(index, value);
		m_pDetailSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Insert,
			m_value,
			old,
			(int)index, (int)index, (int)(index + value.size())
		});
		m_pSubject->OnNext(m_value);
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& erase(size_type index = 0, size_type count = npos)
	{
		str_type old(m_value);
		m_value.erase(index, count);
		m_pDetailSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Erase,
			m_value,
			old,
			(int)index, (int)(index + count), (int)(index)
		});
		m_pSubject->OnNext(m_value);
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& replace(size_type pos, size_type count, const str_type& value)
	{
		str_type old(m_value);
		m_value.replace(pos, count, value);
		m_pDetailSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Replace,
			m_value,
			old,
			(int)pos, (int)(pos + count), int(pos + value.size())
		});
		m_pSubject->OnNext(m_value);
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& replace(size_type pos, size_type n1, const CharT* s, size_type n2)
	{
		str_type old(m_value);
		m_value.replace(pos, n1, s, n2);
		m_pDetailSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Replace,
				m_value,
				old,
				(int)pos, (int)(pos + n1), int(pos + n2)
		});
		m_pSubject->OnNext(m_value);
		return *this;
	}

	void clear()
	{
		str_type old(m_value);
		m_value.clear();
		m_pDetailSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Clear,
			m_value,
			old,
			0, (int)old.size(), 0
		});
		m_pSubject->OnNext(m_value);
	}
};

using ReactiveWStringProperty = ReactiveBasicStringProperty<wchar_t>;

template<typename... Args>
class ReactiveTupleProperty:public ReactiveProperty<std::tuple<Args...>>
{
	using tuple_type = std::tuple<Args...>;
public:
	ReactiveTupleProperty(const Args& ...args) :
		ReactiveProperty(std::make_tuple(args...)){}

	void set(const Args& ...args)
	{
		ReactiveProperty::set(std::make_tuple(args...));
	}
};

enum class NotifyVectorChangedAction
{
	Add,
	Insert,
	Move,
	Remove,
	Replace,
	Reset,
};

template<typename T>
struct NotifyVectorChangedEventArgs
{
	NotifyVectorChangedAction Action;
	std::vector<T> NewItems;
	int NewStartingIndex = -1;
	std::vector<T> OldItems;
	int OldStartingIndex = -1;
};

template<class T, class Allocator = std::allocator<T>>
class ReactiveVectorProperty:public std::vector<T, Allocator>, public ReactiveDetailProperty<std::vector<T, Allocator>, NotifyVectorChangedEventArgs<T>>
{
	using vector_type = std::vector<T, Allocator>;
public:
	ReactiveVectorProperty() :
		ReactiveDetailProperty(){}

	ReactiveVectorProperty(const vector_type& value) :
		ReactiveDetailProperty(value){}

	virtual ~ReactiveVectorProperty() = default;

	void push_back(const T& x)
	{
		m_value.push_back(x);
		m_pDetailSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Add,
			{ x },
				(int)size() - 1,
			{},
				-1
		});
		m_pSubject->OnNext(m_value);
	}

	vector_type::iterator insert(vector_type::const_iterator position, const T& x)
	{
		auto ret = m_value.insert(position, x);
		auto index = std::distance(m_value.begin(), ret);
		m_pDetailSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Insert,
			{ x },
			index,
			{},
			-1
		});
		m_pSubject->OnNext(m_value);
		return ret;
	}

	vector_type::iterator replace(vector_type::iterator position, const T& x)
	{
		auto oldItem = *position;
		*position = x;
		auto index = std::distance(m_value.begin(), position);
		m_pDetailSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Replace,
			{ x },
			index,
			{ oldItem},
			-1
		});
		m_pSubject->OnNext(m_value);
		return position;
	}

	vector_type::iterator erase(vector_type::const_iterator where)
	{
		auto oldItem = *where;
		auto index = std::distance(m_value.cbegin(), where);
		vector_type::iterator ret = m_value.erase(where);
		m_pDetailSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Remove,
			{},
			-1,
			{ oldItem },
			index
		});
		m_pSubject->OnNext(m_value);
		return ret;
	}

	vector_type::iterator erase(vector_type::const_iterator first, vector_type::const_iterator last)
	{
		auto oldItems = vector_type(first, last);
		auto index = std::distance(m_value.cbegin(), first);
		iterator ret = m_value.erase(first, last);
		m_pDetailSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Remove,
			{},
				-1,
				oldItems,
				index
		});
		m_pSubject->OnNext(m_value);
		return ret;
	}


	void clear()
	{
		m_value.clear();
		m_pDetailSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Reset,
			{},
				-1,
			{},
				-1
		});
		m_pSubject->OnNext(m_value);
		return;
	}

};


template<typename T>
class CBinding
{
public:
	CBinding(IReactiveProperty<T>& source, IReactiveProperty<T>& target)
	{
		target.set(source.get());
		m_sourceConnection = source.Subscribe(
			[&](T value)->void
			{
				target.set(value);
			});
		m_targetConnection = target.Subscribe(
			[&](T value)->void
			{
				source.set(value);
			});
	}
	virtual ~CBinding()
	{
		m_sourceConnection.disconnect();
		m_targetConnection.disconnect();
	}
private:
	sigslot::connection m_sourceConnection;
	sigslot::connection m_targetConnection;


};



//template <class CharT,
//	class Traits = std::char_traits<CharT>,
//	class Allocator = std::allocator<CharT> >
//class ReactiveBasicStringProperty :public ReactiveProperty<std::basic_string<CharT, Traits, Allocator>>
//{
//	using str_type = std::basic_string<CharT, Traits, Allocator>;
//	using size_type = std::basic_string<CharT, Traits, Allocator>::size_type;
//public:
//	using ReactiveProperty::ReactiveProperty;
//
//	sigslot::signal<const NotifyStringChangedEventArgs<CharT>&> StringChanged;
//
//	CharT& operator[](size_type index)noexcept
//	{
//		return m_value[index];
//	}
//
//	const CharT& operator[](size_type index) const noexcept
//	{
//		return m_value[index];
//	}
//
//	size_type size() const noexcept
//	{
//		return m_value.size();
//	}
//
//	bool empty() const noexcept
//	{
//		return m_value.size();
//	}
//
//
//
//	ReactiveBasicStringProperty<CharT, Traits, Allocator>& notify_assign(const str_type& str)
//	{
//		str_type old(*this);
//		str_type::operator=(str);
//		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
//			NotifyStringChangedAction::Assign,
//			*this,
//			old,
//			0, (int)old.size(), (int)this->size()
//		});
//		return *this;
//	}
//
//	ReactiveBasicStringProperty<CharT, Traits, Allocator>& notify_insert(size_type index, const str_type& str)
//	{
//		str_type old(*this);
//		insert(index, str);
//		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
//			NotifyStringChangedAction::Insert,
//			*this,
//			old,
//			(int)index, (int)index, (int)(index + str.size())
//		});
//		return *this;
//	}
//
//	ReactiveBasicStringProperty<CharT, Traits, Allocator>& notify_erase(size_type index = 0, size_type count = npos)
//	{
//		str_type old(*this);
//		erase(index, count);
//		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
//			NotifyStringChangedAction::Erase,
//			*this,
//			old,
//			(int)index, (int)(index + count), (int)(index)
//		});
//		return *this;
//	}
//
//	ReactiveBasicStringProperty<CharT, Traits, Allocator>& notify_replace(size_type pos, size_type count, const str_type& str)
//	{
//		str_type old(*this);
//		replace(pos, count, str);
//		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
//			NotifyStringChangedAction::Replace,
//			*this,
//			old,
//			(int)pos, (int)(pos + count), int(pos + str.size())
//		});
//		return *this;
//	}
//
//	ReactiveBasicStringProperty<CharT, Traits, Allocator>& notify_replace(size_type pos, size_type n1, const CharT* s, size_type n2)
//	{
//		str_type old(*this);
//		replace(pos, n1, s, n2);
//		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
//			NotifyStringChangedAction::Replace,
//				* this,
//				old,
//				(int)pos, (int)(pos + n1), int(pos + n2)
//		});
//		return *this;
//	}
//
//
//	void notify_clear()
//	{
//		str_type old(*this);
//		clear();
//		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
//			NotifyStringChangedAction::Clear,
//			*this,
//			old,
//			0, (int)old.size(), 0
//		});
//	}
//};
//
//using ReactiveWStringProperty = ReactiveBasicStringProperty<wchar_t>;





//template<std::uint64_t Bits>
//struct nm{};
//
//
//template<typename TName, typename TValue>
//struct prop
//{
//	prop(const TValue& val):value(val){}
//    ReactiveProperty<TValue> value;
//};
//
//template<typename TName>
//struct prop<TName, std::wstring>
//{
//	prop(const std::wstring& val):value(val){}
//	ReactiveWStringProperty value;
//};
//
//constexpr auto operator "" _n(char const* str, std::size_t size)
//{
//    std::uint64_t bits = 0;
//    for (std::size_t i = 0; i < size; ++i) {
//        bits = bits << 8;
//        bits = bits | str[i];
//    }
//    return bits;
//}
//
//template<typename TName, typename... TProps>
//struct find_name;
//
//template<typename TName>
//struct find_name<TName> {};
//
//template<typename TName, typename TValue, typename... TProps>
//struct find_name<TName, prop<TName, TValue>, TProps...>
//{
//    using type = prop<TName, TValue>;
//};
//
//template<typename TName, typename T, typename... TProps>
//struct find_name<TName, T, TProps...> : find_name<TName, TProps...> {};
//
//template<typename TName, typename... TProps,
//	typename TResult = typename find_name<TName, TProps...>::type>
//auto getprop(TName, std::tuple<TProps...> props)
//{
//    return std::get<TResult>(props).value;
//}