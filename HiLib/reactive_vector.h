#pragma once
#include "reactive_container.h"
#include <vector>

template<typename T>
struct adl_vector_item
{
	static T clone(const T& item) { return T(item); }
	static void bind(T& src, T& dst) { return reactive_binding(src, dst); }
};

template<class T, class Allocator = std::allocator<T>>
class reactive_vector: public reactive_container<std::vector<T, Allocator>>
{
public:
	using base = typename reactive_container<std::vector<T, Allocator>>;
	using container_type = base::container_type;
	using notify_type = base::notify_type;

	using base::reactive_container;

    template <class... _Valty>
	void emplace_back(_Valty&&... _Val)
	{
		this->m_value.emplace_back(std::forward<_Valty>(_Val)...);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::push_back,
			{ this->m_value.back() },
			(int)this->size() - 1,
			{},
			-1,
			this->get_const()
		});
	}


	void observe(base::notify_type& notify)
	{
		if (this->get_const() == notify.all_items) { return; }

		switch (notify.action) {
			case notify_container_changed_action::push_back:
				this->push_back(adl_vector_item<base::value_type>::clone(notify.new_items.front()));
				adl_vector_item<T>::bind(
					notify.new_items.front(),
					this->get_unconst().operator[](notify.new_starting_index));
				break;
			case notify_container_changed_action::insert:
				this->insert(this->get_const().cbegin() + notify.new_starting_index, adl_vector_item<base::value_type>::clone(notify.new_items.front()));
				adl_vector_item<base::value_type>::bind(
					notify.new_items.front(),
					this->get_unconst().operator[](notify.new_starting_index));
				break;
			case notify_container_changed_action::Move:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::erase:
				this->erase(this->cbegin() + notify.old_starting_index);
				break;
			case notify_container_changed_action::replace:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::reset:
				for (size_t i = 0; i < notify.new_items.size(); i++) {
					this->push_back(adl_vector_item<base::value_type>::clone(notify.new_items[i]));
					adl_vector_item<base::value_type>::bind(
						notify.new_items[i],
						this->get_unconst().operator[](i));
				}
				break;
		}		this->unblock_subject();
	}

	friend void to_json(json& j, const reactive_vector& o)
	{
		j = {
			{"Value", o.get_const()}
		};
	}

	friend void from_json(const json& j, reactive_vector& o)
	{
		container_type value;
		o.set(j.at("Value").get_to(value));
	}

};


//#include "subject.h"
//#include "JsonSerializer.h"
//#include "Debug.h"

//enum class notify_container_changed_action
//{
//	push_back,
//	insert,
//	Move,
//	erase,
//	replace,
//	reset,
//};
//
//template<typename T>
//struct notify_container_changed_event_args
//{
//	notify_container_changed_action action;
//	std::vector<T> new_items;
//	int new_starting_index = -1;
//	std::vector<T> old_items;
//	int old_starting_index = -1;
//	std::vector<T> all_items;
//};
//
//template<typename T>
//struct adl_vector_item
//{
//	static T clone(const T& item) { return T(item); }
//	static void bind(T& src, T& dst) { return reactive_binding(src, dst); }
//};
//
//template<class T, class Allocator = std::allocator<T>>
//class reactive_vector
//{
//public:
//	using subject_type = typename subject<notify_container_changed_event_args<T>>;
//	using vector_type = typename std::vector<T, Allocator>;
//	using notify_type = typename notify_container_changed_event_args<T>;
//	using size_type = typename  std::vector<T, Allocator>::size_type;
//	using iterator = typename std::vector<T, Allocator>::iterator;
//	using const_iterator = typename std::vector<T, Allocator>::const_iterator;
//	using const_reference = typename std::vector<T, Allocator>::const_reference;
//	using reference = typename std::vector<T, Allocator>::reference;
//
//private:
//	subject_type m_subject;
//	vector_type m_value;
//
//public:
//	explicit reactive_vector(const vector_type& value = vector_type())
//		:m_subject(), m_value(value){};
//
//	virtual ~reactive_vector() = default;
//	auto operator<=>(const reactive_vector& rhs) const
//	{
//		return this->get_const() <=> rhs.get_const();
//	}
//	bool operator==(const reactive_vector& rhs) const
//	{
//		return this->get_const() == rhs.get_const();
//	}
//	reactive_vector(const reactive_vector& val) = delete;
//	reactive_vector& operator=(const reactive_vector& val) = delete;
//	reactive_vector(reactive_vector&& val) noexcept = default;
//	reactive_vector& operator=(reactive_vector&& val) noexcept = default;
//	//reactive_vector& operator=(const vector_type& val) 
//	//{
//	//	set(val);
//	//	return *this;
//	//}
//	//reactive_vector& operator=(vector_type&& val) 
//	//{
//	//	set(val);
//	//	return *this;
//	//}
//
//  //  subscriber_type get_subscriber() const {
//  //      return m_subject.get_subscriber();
//  //  }
//
//  //  rxcpp::observable<T> get_observable() const {
//		//return m_subject.get_observable();
//  //  }
//
//	template<class... Args>
//	auto subscribe(Args&&... args) const -> sigslot::connection
//	{
//		return m_subject.subscribe(std::forward<Args>(args)...);
//	}
//	const vector_type& get_const() const 
//	{ 
//		return m_value;
//	}
//	vector_type& get_unconst() 
//	{ 
//		return m_value;
//	}
//	void set(const vector_type& value)
//	{
//		if (this->get_const() != value) {
//			vector_type old(this->get_const());
//			this->m_value.assign(value.cbegin(), value.cend());
//			this->m_subject.on_next(notify_type
//			{
//				notify_container_changed_action::reset,
//				this->get_const(),
//				0,
//				old,
//				0,
//				this->get_const()
//			});
//		}
//	}
//
//	void block_subject()
//	{
//		m_subject.block();
//	}
//
//	void unblock_subject()
//	{
//		m_subject.unblock();
//	}
//
//	const_reference operator[](size_type pos) const noexcept
//	{
//		return this->m_value.operator[](pos);
//	}
//
//	////TODO
//	//reference operator[](size_type pos) noexcept
//	//{
//	//	return this->m_value.operator[](pos);
//	//}
//	////TODO
//	//iterator begin() noexcept
//	//{
//	//	return this->m_value.begin();
//	//}
//	////TODO
//	//iterator end() noexcept
//	//{
//	//	return this->m_value.end();
//	//}
//
//	const_iterator cbegin() const noexcept
//	{
//		return this->m_value.cbegin();
//	}
//
//	const_iterator cend() const noexcept
//	{
//		return this->m_value.cend();
//	}
//
//	size_type size() const noexcept
//	{
//		return this->m_value.size();
//	}
//
//	bool empty() const noexcept
//	{
//		return this->m_value.empty();
//	}
//
//	void push_back(const T& x)
//	{
//		this->m_value.push_back(x);
//		this->m_subject.on_next(notify_container_changed_event_args<T>
//		{
//			notify_container_changed_action::push_back,
//			{ this->m_value.back() },
//			(int)size() - 1,
//			{},
//			-1,
//			this->get_const()
//		});
//	}
//
//    template <class... _Valty>
//	void emplace_back(_Valty&&... _Val)
//	{
//		this->m_value.emplace_back(std::forward<_Valty>(_Val)...);
//		this->m_subject.on_next(notify_container_changed_event_args<T>
//		{
//			notify_container_changed_action::push_back,
//			{ this->m_value.back() },
//			(int)size() - 1,
//			{},
//			-1,
//			this->get_const()
//		});
//	}
//
//	iterator insert(const_iterator position, const T& x)
//	{
//		auto ret = this->m_value.insert(position, x);
//		auto index = std::distance(this->m_value.begin(), ret);
//		this->m_subject.on_next(notify_container_changed_event_args<T>
//		{
//			notify_container_changed_action::insert,
//			{ *ret },
//			index,
//			{},
//			-1,
//			this->get_const()
//		});
//		return ret;
//	}
//
//	iterator replace(iterator position, const T& x)
//	{
//		auto oldItem = *position;
//		*position = x;
//		auto index = std::distance(this->m_value.begin(), position);
//		this->m_subject.on_next(notify_container_changed_event_args<T>
//		{
//			notify_container_changed_action::replace,
//			{ *position },
//			index,
//			{ oldItem},
//			-1,
//			this->get_const()
//		});
//		return position;
//	}
//
//	iterator erase(const_iterator where)
//	{
//		auto oldItem = *where;
//		auto index = std::distance(this->m_value.cbegin(), where);
//		iterator ret = this->m_value.erase(where);
//		this->m_subject.on_next(notify_container_changed_event_args<T>
//		{
//			notify_container_changed_action::erase,
//			{},
//			-1,
//			{ oldItem },
//			index,
//			this->get_const()
//		});
//		return ret;
//	}
//
//	iterator erase(const_iterator first, const_iterator last)
//	{
//		auto oldItems = vector_type(first, last);
//		auto index = std::distance(this->m_value.cbegin(), first);
//		iterator ret = this->m_value.erase(first, last);
//		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
//		{
//			notify_container_changed_action::erase,
//			{},
//			-1,
//			oldItems,
//			index,
//			this->get_const()
//		});
//		return ret;
//	}
//
//
//	void clear()
//	{
//		auto old = this->m_value;
//		this->m_value.clear();
//		this->m_subject.on_next(notify_container_changed_event_args<T>
//		{
//			notify_container_changed_action::reset,
//			{},
//			-1,
//			old,
//			0,
//			this->get_const()
//		});
//		return;
//	}
//
//	void observe(notify_container_changed_event_args<T>& notify)
//	{
//		if (this->get_const() == notify.all_items) { return; }
//
//		switch (notify.action) {
//			case notify_container_changed_action::push_back:
//				this->push_back(adl_vector_item<T>::clone(notify.new_items.front()));
//				adl_vector_item<T>::bind(
//					notify.new_items.front(),
//					this->get_unconst().operator[](notify.new_starting_index));
//				break;
//			case notify_container_changed_action::insert:
//				this->insert(this->get_const().cbegin() + notify.new_starting_index, adl_vector_item<T>::clone(notify.new_items.front()));
//				adl_vector_item<T>::bind(
//					notify.new_items.front(),
//					this->get_unconst().operator[](notify.new_starting_index));
//				break;
//			case notify_container_changed_action::Move:
//				THROW_FILE_LINE_FUNC;
//				break;
//			case notify_container_changed_action::erase:
//				this->erase(this->cbegin() + notify.old_starting_index);
//				break;
//			case notify_container_changed_action::replace:
//				THROW_FILE_LINE_FUNC;
//				break;
//			case notify_container_changed_action::reset:
//				for (size_t i = 0; i < notify.new_items.size(); i++) {
//					this->push_back(adl_vector_item<T>::clone(notify.new_items[i]));
//					adl_vector_item<T>::bind(
//						notify.new_items[i],
//						this->get_unconst().operator[](i));
//				}
//				break;
//		}		this->unblock_subject();
//	}
//
//	friend void to_json(json& j, const reactive_vector<T, Allocator>& o)
//	{
//		j = {
//			{"Value", o.get_const()}
//		};
//	}
//
//	friend void from_json(const json& j, reactive_vector<T, Allocator>& o)
//	{
//		vector_type value;
//		j.at("Value").get_to(value);
//		o.set(value);
//	}
//};

template<class T, class Allocator = std::allocator<T>> using reactive_vector_ptr = std::shared_ptr<reactive_vector<T, Allocator>>;

template<class T, class Allocator  = std::allocator<T>, class... Args>
auto make_reactive_vector(Args&&... args) -> reactive_vector_ptr<T, Allocator>
{
	return std::make_shared<reactive_vector<T, Allocator>>(std::forward<Args>(args)...);
}

