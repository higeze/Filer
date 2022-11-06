#pragma once
#include <functional>
#include <vector>
#include <tuple>
#include <sigslot/signal.hpp>
#include "JsonSerializer.h"

template <class T>
class Subject
{
private:
	sigslot::signal<T> m_onNextSignal;
public:
	Subject() {}
	virtual ~Subject() = default;//disconnect_all in default destructor
	virtual void OnNext(const T& value)
	{
		m_onNextSignal(value);
	}

	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next, sigslot::group_id id = 0)
	{
		return m_onNextSignal.connect(next, id);
	}

};

template <>
class Subject<void>
{
private:
	sigslot::signal<> m_onNextSignal;
public:
	Subject() {}
	virtual ~Subject() = default;//disconnect_all in default destructor
	virtual void OnNext()
	{
		m_onNextSignal();
	}
	virtual sigslot::connection Subscribe(std::function<void(void)> next, sigslot::group_id id = 0)
	{
		return m_onNextSignal.connect(next, id);
	}
};


template<class T>
class IReactiveProperty
{
public:
	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next, sigslot::group_id id = 0) = 0;
	virtual operator T() const { return get(); }
	virtual const T& get() const = 0;
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

template<class T>
class IReactiveCommand
{
public:
	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next, sigslot::group_id id = 0) = 0;
	virtual void Execute(const T& value) = 0;
	virtual void Dispose(void) = 0;
};

template<>
class IReactiveCommand<void>
{
public:
	virtual sigslot::connection Subscribe(std::function<void(void)> next, sigslot::group_id id = 0) = 0;
	virtual void Execute(void) = 0;
	virtual void Dispose(void) = 0;
};

template<class T> class ReactiveProperty;
template<class T> void to_json(json& j, const ReactiveProperty<T>& o);
template<class T> void from_json(const json& j, ReactiveProperty<T>& o);

template<class CharT, class Traits, class Allocator> class ReactiveBasicStringProperty;
template<class CharT, class Traits, class Allocator> void to_json(json& j, const ReactiveBasicStringProperty<CharT, Traits, Allocator>& o);
template<class CharT, class Traits, class Allocator> void from_json(const json& j, ReactiveBasicStringProperty<CharT, Traits, Allocator>& o);

template<class T, class Allocator> class ReactiveVectorProperty;
template<class T, class Allocator> void to_json(json& j, const ReactiveVectorProperty<T, Allocator>& o);
template<class T, class Allocator> void from_json(const json& j, ReactiveVectorProperty<T, Allocator>& o);

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
	virtual ~ReactiveProperty() = default;
	ReactiveProperty(const ReactiveProperty& val) = default;
	ReactiveProperty(ReactiveProperty&& val) = default;
	ReactiveProperty& operator=(const ReactiveProperty& val) = default;
	ReactiveProperty& operator=(ReactiveProperty&& val) = default;
	ReactiveProperty& operator=(const T& val) 
	{
		set(val);
		return *this;
	}
	ReactiveProperty& operator=(T&& val)
	{
		set(val);
		return *this;
	}
	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next, sigslot::group_id id = 0)
	{
		return m_pSubject->Subscribe(next, id);
	}
	virtual const T& get() const override { return m_value; }
	virtual void set(const T& value) override
	{
		if (m_value != value) {
			m_value = value;
			m_pSubject->OnNext(m_value);
		}
	}
	virtual void force_notify_set(const T& value)
	{
		m_value = value;
		m_pSubject->OnNext(m_value);
	}

	template <class Archive>
	void save(Archive& ar)
	{
		ar("Value", m_value);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("Value", m_value);
		m_pSubject->OnNext(m_value);
	}

	friend void to_json(json& j, const ReactiveProperty<T>& o)
	{
		j = {
			{"Value", o.m_value},
		};
	}

	friend void from_json(const json& j, ReactiveProperty<T>& o)
	{
		j.at("Value").get_to(o.m_value);
	}
};





template <class T>
class ReactiveCommand:public IReactiveCommand<T>
{
protected:
	std::shared_ptr<Subject<T>> m_pSubject;
public:
	ReactiveCommand():
		m_pSubject(std::make_shared<Subject<T>>()){};
	virtual ~ReactiveCommand() = default;
	ReactiveCommand(const ReactiveCommand& val) = default;
	ReactiveCommand(ReactiveCommand&& val) = default;
	ReactiveCommand& operator=(const ReactiveCommand& val) = default;
	ReactiveCommand& operator=(ReactiveCommand&& val) = default;

	virtual sigslot::connection Subscribe(std::function<void(const T& value)> next, sigslot::group_id id = 0) override
	{
		return m_pSubject->Subscribe(next, id);
	}

	virtual void Execute(const T& value) override
	{
		m_pSubject->OnNext(value);
	}
	void Dispose()
	{
		m_pSubject = std::make_shared<Subject<T>>();
	}
};

template<>
class ReactiveCommand<void>:public IReactiveCommand<void>
{
protected:
	std::shared_ptr<Subject<void>> m_pSubject;
public:
	ReactiveCommand():
		m_pSubject(std::make_shared<Subject<void>>()){};
	~ReactiveCommand() = default;
	ReactiveCommand(const ReactiveCommand& val) = default;
	ReactiveCommand(ReactiveCommand&& val) = default;
	ReactiveCommand& operator=(const ReactiveCommand& val) = default;
	ReactiveCommand& operator=(ReactiveCommand&& val) = default;

	sigslot::connection Subscribe(std::function<void(void)> next,sigslot::group_id id = 0) override
	{
		return m_pSubject->Subscribe(next, id);
	}
	void Execute(void) override
	{
		m_pSubject->OnNext();
	}
	void Dispose()
	{
		m_pSubject = std::make_shared<Subject<void>>();
	}
};

template<class T, class U>
class IReactiveDetailProperty
{
public:
	virtual sigslot::connection Subscribe(std::function<void(const U& value)> next, sigslot::group_id id = 0) = 0;
	virtual operator T() const { return get(); }
	virtual const T& get() const = 0;
	virtual void set(const T& value) = 0;
};


template <class T, class U>
class ReactiveDetailProperty :public IReactiveDetailProperty<T, U>
{
protected:
	std::shared_ptr<Subject<U>> m_pSubject;
	T m_value;
public:
	ReactiveDetailProperty() :
		m_pSubject(std::make_shared<Subject<U>>()),
		m_value()
	{};

	ReactiveDetailProperty(const T& value) :
		m_pSubject(std::make_shared<Subject<U>>()),
		m_value(value)
	{};

	virtual ~ReactiveDetailProperty() = default;
	ReactiveDetailProperty(const ReactiveDetailProperty& val) = default;
	ReactiveDetailProperty(ReactiveDetailProperty&& val) = default;
	ReactiveDetailProperty& operator=(const ReactiveDetailProperty& val) = default;
	ReactiveDetailProperty& operator=(ReactiveDetailProperty&& val) = default;

	virtual sigslot::connection Subscribe(std::function<void(const U& value)> next, sigslot::group_id id = 0)
	{
		return m_pSubject->Subscribe(next, id);
	}
	virtual const T& get() const override { return m_value; }
	virtual void set(const T& value) override
	{
		if (m_value != value) {
			force_notify_set(value);
		}
	}

	virtual void force_notify_set(const T& value) = 0;

	template <class Archive>
	void save(Archive& ar)
	{
		ar("Value", m_value);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("Value", m_value);
	}

	friend void to_json(json& j, const ReactiveDetailProperty<T, U>& o)
	{
		j = {
			{"Value", o.m_value},
		};
	}

	friend void from_json(const json& j, ReactiveDetailProperty<T, U>& o)
	{
		j.at("Value").get_to(o.m_value);
	}
};


//
//template<class T, class U>
//class ReactiveDetailProperty :public ReactiveProperty<T>
//{
//	using base = ReactiveProperty<T>;
//protected:
//	std::shared_ptr<Subject<U>> m_pDetailSubject;
//public:
//	ReactiveDetailProperty() :
//		base(),
//		m_pDetailSubject(std::make_shared <Subject<U>>()){}
//
//	ReactiveDetailProperty(const T& value) :
//		base(value),
//		m_pDetailSubject(std::make_shared <Subject<U>>())
//	{};
//
//	virtual ~ReactiveDetailProperty() = default;
//
//	virtual sigslot::connection SubscribeDetail(std::function<void(const U& value)> next, sigslot::group_id id = 0)
//	{
//		return m_pDetailSubject->Subscribe(next, id);
//	}
//};
//



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
	int NewStartingIndex = -1;
	int OldStartingIndex = -1;
};

template <class CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
class ReactiveBasicStringProperty
	: public ReactiveDetailProperty<std::basic_string<CharT, Traits, Allocator>, NotifyStringChangedEventArgs<CharT>>
{
	using base = typename ReactiveDetailProperty<std::basic_string<CharT, Traits, Allocator>, NotifyStringChangedEventArgs<CharT>>;
	using str_type = typename std::basic_string<CharT, Traits, Allocator>;
	using notify_type = typename NotifyStringChangedEventArgs<CharT>;
	using size_type = typename  std::basic_string<CharT, Traits, Allocator>::size_type;
	using const_iterator = typename std::basic_string<CharT, Traits, Allocator>::const_iterator;
	using const_reference = typename std::basic_string<CharT, Traits, Allocator>::const_reference;
//	using reference = typename std::basic_string<CharT, Traits, Allocator>::reference;

public:
	ReactiveBasicStringProperty()
		:base(){}
	
	ReactiveBasicStringProperty(const str_type& value) 
		:base(value){}
	
	~ReactiveBasicStringProperty() = default;

	virtual void force_notify_set(const str_type& value) override
	{
		str_type old(this->m_value);
		this->m_value.assign(value);
		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Assign,
				value,
				old,
				0, 
				0
		});
	}

	const_reference operator[](size_type pos) const noexcept
	{
		return this->m_value.operator[](pos);
	}

	//reference operator[](size_type pos) noexcept
	//{
	//	return m_value.operator[](pos);
	//}

	const_iterator cbegin() const noexcept
	{
		return this->m_value.cbegin();
	}

	const_iterator cend() const noexcept
	{
		return this->m_value.cend();
	}

	size_type size() const noexcept
	{
		return this->m_value.size();
	}

	bool empty() const noexcept
	{
		return this->m_value.empty();
	}

	const CharT& front() const
	{
		return this->m_value.front();
	}
	
	const CharT& back() const
	{
		return this->m_value.back();
	}

	str_type substr(size_type pos = 0, size_type n = std::basic_string<CharT, Traits, Allocator>::npos) const
	{
		return this->m_value.substr(pos, n);
	}

	const CharT* c_str() const noexcept
	{
		return this->m_value.c_str();
	}

	const CharT* data() const noexcept
	{
		return this->m_value.data();
	}
	
	ReactiveBasicStringProperty<CharT, Traits, Allocator>& assign(const str_type& value)
	{
		this->set(value);
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& insert(size_type index, const str_type& value)
	{
		this->m_value.insert(index, value);
		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Insert,
			value,
			str_type(),
			(int)index, 
			-1
		});
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& erase(size_type index = 0, size_type count = std::basic_string<CharT, Traits, Allocator>::npos)
	{
		str_type old(this->m_value.substr(index, count));
		this->m_value.erase(index, count);
		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Erase,
			str_type(),
			old,
			-1,
			(int)index
		});
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& replace(size_type pos, size_type count, const str_type& value)
	{
		str_type old(this->m_value.substr(pos, count));
		this->m_value.replace(pos, count, value);
		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Replace,
			value,
			old,
			(int)pos,
			(int)pos
		});
		return *this;
	}

	ReactiveBasicStringProperty<CharT, Traits, Allocator>& replace(size_type pos, size_type n1, const CharT* s, size_type n2)
	{
		str_type neww(s, n2);
		str_type old(this->m_value.substr(pos, n1));
		this->m_value.replace(pos, n1, s, n2);
		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Replace,
			neww,
			old,
			(int)pos,
			(int)pos
		});
		return *this;
	}

	void clear()
	{
		str_type old(this->m_value);
		this->m_value.clear();
		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Clear,
			nullptr,
			old,
			-1,
			0
		});
	}

	//template <class Archive>
	//void save(Archive& ar)
	//{
	//	ReactiveDetailProperty<str_type, notify_type>::load(ar);
	//}

	//template <class Archive>
	//void load(Archive& ar)
	//{
	//	str_type old(this->m_value);
	//	ar("Value", this->m_value);
	//	if (old != this->m_value) {
	//		this->m_pSubject->OnNext(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
	//			NotifyStringChangedAction::Assign,
	//			this->m_value,
	//			old,
	//			0, (int)old.size(), 0
	//		});
	//	}
	//}

	friend void to_json(json& j, const ReactiveBasicStringProperty<CharT, Traits, Allocator>& o)
	{
		j = {
			{"Value", o.get()}
		};
	}

	friend void from_json(const json& j, ReactiveBasicStringProperty<CharT, Traits, Allocator>& o)
	{
		std::basic_string<CharT, Traits, Allocator> value;
		j.at("Value").get_to(value);
		o.set(value);
	}
};




using ReactiveWStringProperty = ReactiveBasicStringProperty<wchar_t>;

template<typename... Args>
class ReactiveTupleProperty:public ReactiveProperty<std::tuple<Args...>>
{
	using base = ReactiveProperty<std::tuple<Args...>>;
	using tuple_type = std::tuple<Args...>;
public:
	ReactiveTupleProperty(const Args& ...args) :
		base(std::make_tuple(args...)){}

	void set(const Args& ...args)
	{
		base::set(std::make_tuple(args...));
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
class ReactiveVectorProperty:public ReactiveDetailProperty<std::vector<T, Allocator>, NotifyVectorChangedEventArgs<T>>
{
	using base = ReactiveDetailProperty<std::vector<T, Allocator>, NotifyVectorChangedEventArgs<T>>;
	using vector_type = std::vector<T, Allocator>;
	using notify_type = NotifyVectorChangedEventArgs<T>;
	using str_type = typename std::vector<T, Allocator>;
	using size_type = typename  std::vector<T, Allocator>::size_type;
	using iterator = typename std::vector<T, Allocator>::iterator;
	using const_iterator = typename std::vector<T, Allocator>::const_iterator;
	using const_reference = typename std::vector<T, Allocator>::const_reference;
	using reference = typename std::vector<T, Allocator>::reference;

public:
	ReactiveVectorProperty() :
		base(){}

	ReactiveVectorProperty(const vector_type& value) :
		base(value){}

	virtual ~ReactiveVectorProperty() = default;

	virtual void force_notify_set(const vector_type& value) override
	{
		vector_type old(this->m_value);
		this->m_value.assign(value.cbegin(), value.cend());
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Reset,
			this->m_value,
			0,
			old,
			0
		});
	}

	const_reference operator[](size_type pos) const noexcept
	{
		return this->m_value.operator[](pos);
	}

	//TODO
	reference operator[](size_type pos) noexcept
	{
		return this->m_value.operator[](pos);
	}
	//TODO
	iterator begin() noexcept
	{
		return this->m_value.begin();
	}
	//TODO
	iterator end() noexcept
	{
		return this->m_value.end();
	}

	const_iterator cbegin() const noexcept
	{
		return this->m_value.cbegin();
	}

	const_iterator cend() const noexcept
	{
		return this->m_value.cend();
	}

	size_type size() const noexcept
	{
		return this->m_value.size();
	}

	bool empty() const noexcept
	{
		return this->m_value.empty();
	}

	void push_back(const T& x)
	{
		this->m_value.push_back(x);
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Add,
			{ x },
				(int)size() - 1,
			{},
				-1
		});
	}

	iterator insert(const_iterator position, const T& x)
	{
		auto ret = this->m_value.insert(position, x);
		auto index = std::distance(this->m_value.begin(), ret);
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Insert,
			{ x },
			index,
			{},
			-1
		});
		return ret;
	}

	iterator replace(iterator position, const T& x)
	{
		auto oldItem = *position;
		*position = x;
		auto index = std::distance(this->m_value.begin(), position);
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Replace,
			{ x },
			index,
			{ oldItem},
			-1
		});
		return position;
	}

	iterator erase(const_iterator where)
	{
		auto oldItem = *where;
		auto index = std::distance(this->m_value.cbegin(), where);
		iterator ret = this->m_value.erase(where);
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Remove,
			{},
			-1,
			{ oldItem },
			index
		});
		return ret;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		auto oldItems = vector_type(first, last);
		auto index = std::distance(this->m_value.cbegin(), first);
		iterator ret = this->m_value.erase(first, last);
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Remove,
			{},
				-1,
				oldItems,
				index
		});
		return ret;
	}


	void clear()
	{
		auto old = this->m_value;
		this->m_value.clear();
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Reset,
			{},
			-1,
			old,
			0
		});
		return;
	}


	template <class Archive>
	void save(Archive& ar)
	{
		ReactiveDetailProperty<vector_type, notify_type >::load(ar);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		vector_type old(this->m_value);
		ar("Value", this->m_value);
		if (old != this->m_value) {
			this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
			{
				NotifyVectorChangedAction::Reset,
				this->m_value,
				0,
				old,
				0
			});
		}
	}

	friend void to_json(json& j, const ReactiveVectorProperty<T, Allocator>& o)
	{
		j = {
			{"Value", o.get()}
		};
	}

	friend void from_json(const json& j, ReactiveVectorProperty<T, Allocator>& o)
	{
		std::vector<T, Allocator> value;
		j.at("Value").get_to(value);
		o.set(value);
	}
};




class CBinding
{
public:
	CBinding() {}

	template<typename T>
	void Attach(IReactiveProperty<T>& source, IReactiveProperty<T>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		if (m_sourceConnection.connected()) { m_sourceConnection.disconnect(); }
		if (m_targetConnection.connected()) { m_targetConnection.disconnect(); }

		target.set(source.get());
		m_sourceConnection = source.Subscribe(
			[&](T value)->void
			{
				target.set(value);
			}, idSource);
		m_targetConnection = target.Subscribe(
			[&](T value)->void
			{
				source.set(value);
			});
	}

	template<typename T>
	CBinding(IReactiveProperty<T>& source, IReactiveProperty<T>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		Attach(source, target, idSource, idTarget);
	}

	template<typename T,typename CharT> void copy_notify_string_changed( T& target, const NotifyStringChangedEventArgs<CharT>& e)
	{
		switch (e.Action) {
			case NotifyStringChangedAction::Assign://new,old,0,0
				target.assign(e.NewString);
				break;
			case NotifyStringChangedAction::Insert://new,null,idx,-1
				target.insert(e.NewStartingIndex, e.NewString);
				break;
			case NotifyStringChangedAction::Erase://null,old,-1, idx
				target.erase(e.OldStartingIndex, e.OldString.size());
				break;
			case NotifyStringChangedAction::Replace://new,old,idx,idx
				target.replace(e.OldStartingIndex, e.OldString.size(), e.NewString);
				break;
			case NotifyStringChangedAction::Clear://null,old,-1,idx
				target.clear();
				break;
			default:
				break;
		}
	}


	template <class CharT, class Traits, class Allocator>
	void Attach(ReactiveBasicStringProperty<CharT, Traits, Allocator>& source, ReactiveBasicStringProperty<CharT, Traits, Allocator>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		if (m_sourceConnection.connected()) { m_sourceConnection.disconnect(); }
		if (m_targetConnection.connected()) { m_targetConnection.disconnect(); }
		target.set(source.get());
		m_sourceConnection = source.Subscribe(
			[&](const NotifyStringChangedEventArgs<CharT>& notify)->void
			{
				if (target.get() != source.get()) {
					copy_notify_string_changed(target, notify);
				}
			}, idSource);
		m_targetConnection = target.Subscribe(
			[&](const NotifyStringChangedEventArgs<CharT>& notify)->void
			{
				if (target.get() != source.get()) {
					copy_notify_string_changed(source, notify);
				}
			});
	}

	template <class CharT, class Traits, class Allocator>
	CBinding(ReactiveBasicStringProperty<CharT, Traits, Allocator>& source, ReactiveBasicStringProperty<CharT, Traits, Allocator>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		Attach(source, target, idSource, idTarget);
	}



	template <class CharT, class Traits, class Allocator>
	CBinding(std::basic_string<CharT, Traits, Allocator>& source, ReactiveBasicStringProperty<CharT, Traits, Allocator>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		target.set(source);
		m_targetConnection = target.Subscribe(
			[&](const NotifyStringChangedEventArgs<CharT>& notify)->void
			{
				if (target.get() != source) {
					copy_notify_string_changed(source, notify);
				}
			});
	}

	template<class T, class Allocator>
	CBinding(ReactiveVectorProperty<T, Allocator>& source, ReactiveVectorProperty<T, Allocator>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		target.set(source.get());
		m_sourceConnection = source.Subscribe(
			[&](const NotifyVectorChangedEventArgs<T>& notify)->void
			{
				target.set(notify.NewItems);
			}, idSource);
		m_targetConnection = target.Subscribe(
			[&](const NotifyVectorChangedEventArgs<T>& notify)->void
			{
				source.set(notify.NewItems);
			});
	}

	template<typename T>
	void Attach(IReactiveCommand<T>& source, IReactiveCommand<T>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		if (m_targetConnection.connected()) { m_targetConnection.disconnect(); }
		m_targetConnection = target.Subscribe(
			[&](T value)->void
			{
				source.Execute(value);
			}, idTarget);
	}

	template<typename T>
	CBinding(IReactiveCommand<T>& source, IReactiveCommand<T>& target, sigslot::group_id idSource = 0, sigslot::group_id idTarget = 0)
	{
		Attach(source, target, idSource, idTarget);
	}

	template<>
	inline CBinding(IReactiveCommand<void>& source, IReactiveCommand<void>& target, sigslot::group_id idSource, sigslot::group_id idTarget)
	{
		m_targetConnection = target.Subscribe(
			[&](void)->void
			{
				source.Execute();
			}, idTarget);
	}

	CBinding(CBinding&& rhs)
	{
		if (m_sourceConnection.connected()) { m_sourceConnection.disconnect(); }
		if (m_targetConnection.connected()) { m_targetConnection.disconnect(); }

		m_sourceConnection = std::move(rhs.m_sourceConnection);
		m_targetConnection = std::move(rhs.m_targetConnection);
	}

	CBinding& operator=(CBinding&& rhs)
	{
		if (this != &rhs) {
			if (m_sourceConnection.connected()) { m_sourceConnection.disconnect(); }
			if (m_targetConnection.connected()) { m_targetConnection.disconnect(); }

			m_sourceConnection = std::move(rhs.m_sourceConnection);
			m_targetConnection = std::move(rhs.m_targetConnection);
		}
		return *this;
	}

	virtual ~CBinding()
	{
		if (m_sourceConnection.connected()) { m_sourceConnection.disconnect(); }
		if (m_targetConnection.connected()) { m_targetConnection.disconnect(); }
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