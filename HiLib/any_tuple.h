#pragma once
#include <tuple>
#include <memory>
#include <typeinfo>
#include <any>

namespace detail {
    template<class T, class Tuple, std::size_t... I>
    constexpr std::unique_ptr<T> make_unique_from_tuple_impl(Tuple&& t, std::index_sequence<I...>)
    {
        static_assert(std::is_constructible_v<T,
            decltype(std::get<I>(std::declval<Tuple>()))...>);
    #if __cpp_lib_reference_from_temporary >= 202202L
        if constexpr (std::tuple_size_v<std::remove_reference_t<Tuple>> == 1) {
            using tuple_first_t = decltype(std::get<0>(std::declval<Tuple>()));
            static_assert(!std::reference_constructs_from_temporary_v<T, tuple_first_t>);
        }
    #endif
        return std::make_unique<T>(std::get<I>(std::forward<Tuple>(t))...);
    }
}

template<class T, class Tuple>
constexpr std::unique_ptr<T> make_unique_from_tuple(Tuple&& t)
{
    return detail::make_unique_from_tuple_impl<T>(std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

class any_tuple
{
    struct base
    {
        virtual ~base() {}

        virtual void * get(std::type_info const&) = 0;

        virtual std::unique_ptr<base> clone() const = 0;

        virtual bool equal(const base&) const = 0;
    };

    template<class ... Ts>
    struct impl : base
    {
        template<class ... Us>
        impl(Us && ... us) : data_(std::forward<Us>(us) ... ) 
        {
            //Maybe check for duplicated types and throw.
        }

        virtual void * get( std::type_info const & ti ) override
        {
            return get_helper( ti, std::index_sequence_for<Ts...>() );
        }

        template<std::size_t ... Indices>
        void* get_helper( std::type_info const & ti, std::index_sequence<Indices...> )
        {
            //If you know that only one element of a certain type is available, you can refactor this to avoid comparing all the type_infos
            const bool valid[] = { (ti == typeid(Ts)) ... };

            const std::size_t c = std::count( std::begin(valid), std::end(valid), true );
            if ( c != 1 )
            {
                throw std::runtime_error(""); // something here
            }

            // Pack the addresses of all the elements in an array
            void * result[] = { static_cast<void*>(& std::get<Indices>(data_) ) ... };

            // Get the index of the element we want
            const int which = std::find( std::begin(valid), std::end(valid), true ) - std::begin(valid);

            return result[which];
        }

        virtual std::unique_ptr<base> clone() const override
        {
            return make_unique_from_tuple<impl<Ts...>>(data_);
        }

        virtual bool equal(const base& rhs) const override
        {
            try {
                const impl<Ts...> rhs_impl = dynamic_cast<const impl<Ts...>&>(rhs);
                const std::tuple<Ts...>& rhs_data = dynamic_cast<const std::tuple<Ts...>&>(rhs_impl.data_);
                return data_ == rhs_data;
            }
            catch (std::bad_cast) {
                return false;
            }
        }

        std::tuple<Ts...> data_;
    };

public:
     any_tuple() = default; // allow empty state

     template<class ... Us>
     any_tuple(Us && ... us) :
            m_( new impl< std::remove_reference_t< std::remove_cv_t<Us> > ... >( std::forward<Us>(us) ... ) ){}
     virtual ~any_tuple() = default;

	any_tuple(any_tuple& value)
        :m_(value.m_->clone()){}
	any_tuple(const any_tuple& value)
        :m_(value.m_->clone()){}
    any_tuple& operator=(const any_tuple& rhs)
    {
        any_tuple(rhs).swap(*this);
        return *this;
    }
	any_tuple(any_tuple&& rhs) noexcept
        :m_(std::move(rhs.m_)){}
    any_tuple& operator=(any_tuple&& rhs) noexcept
    {
        any_tuple(std::move(rhs)).swap(*this);
        return *this;
    }

    bool operator == (const any_tuple& rhs) const
    {
        return m_->equal(*rhs.m_);
    }

    void swap(any_tuple &rhs) noexcept {
        std::swap(m_, rhs.m_);
    }

     template<class T>
     T& get()
     {
        if ( !m_ )
        {
            throw std::runtime_error(""); // something
        }
        return *reinterpret_cast<T*>( m_->get( typeid(T) ) );
     }

     template<class T>
     const T& get() const
     {
         return const_cast<any_tuple&>(*this).get<T>();
     }

     bool valid() const { return bool(m_); }

 private:
     std::unique_ptr< base > m_; //Possibly use small buffer optimization
 };
