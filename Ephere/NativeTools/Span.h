// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

#include "Ephere/NativeTools/Asserts.h"

#include <array>

namespace Ephere
{
	/*! A much simpler variant of GSL's dynamic span<T> class. To be used mostly as a function parameter passed by value */
	template <typename T>
	struct Span
	{
		typedef T element_type;
		typedef typename std::remove_cv<T>::type value_type;
		typedef int size_type;
		typedef size_type index_type;
		typedef T* pointer;
		typedef T& reference;

		typedef T* iterator;
		typedef T* const_iterator;

		Span() :
			data_( nullptr ),
			count_( 0 )
		{
		}

		Span( T* data, size_type count )
			: data_( data ),
			count_( count )
		{
			DEBUG_ONLY( ASSERT_RANGE( data == nullptr ? size() == 0 : size() >= 0 ) );
		}

		Span( T* begin, T* end )
			: data_( begin )
		{
			auto distance = end - begin;
			DEBUG_ONLY( ASSERT_RANGE( distance >= 0 ) );
			count_ = static_cast<size_type>( distance );
		}

		// ReSharper disable once CppNonExplicitConvertingConstructor
		template <std::size_t N>
		Span( element_type( &array )[N] )
			: data_( std::addressof( array[0] ) ),
			count_( static_cast<size_type>( N ) )
		{
		}

		// ReSharper disable once CppNonExplicitConvertingConstructor
		template <std::size_t N>
		Span( std::array<typename std::remove_const<element_type>::type, N>& array )
			: data_( array.data() ),
			count_( static_cast<size_type>( N ) )
		{
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1900
		template <class Container,
			class = typename std::enable_if<
			std::is_convertible<typename Container::pointer, pointer>::value
			&& std::is_convertible<typename Container::pointer, decltype( std::declval<Container>().data() )>::value>::type
		>
		Span( Container& cont )
			: Span( cont.data(), static_cast<size_type>( cont.size() ) )
		{
		}

		template <class Container,
			class = typename std::enable_if<
			std::is_convertible<typename Container::pointer, pointer>::value
			&& std::is_convertible<typename Container::pointer, decltype( std::declval<Container>().data() )>::value>::type
		>
		Span( Container const& cont )
			: Span( cont.data(), static_cast<size_type>( cont.size() ) )
		{
			static_assert( std::is_const<element_type>::value, "Span element type must be const" );
		}
#else
		template <class Container>
		Span( Container& cont )
			: data_( cont.data() ),
			count_( size_type( cont.size() ) )
		{
		}

		template <class Container>
		Span( const Container& cont )
			: data_( cont.data() ),
			count_( size_type( cont.size() ) )
		{
			static_assert( std::is_const<element_type>::value, "Span element type must be const" );
		}
#endif

		EPHERE_NODISCARD size_type size() const
		{
			return count_;
		}

		EPHERE_NODISCARD bool empty() const
		{
			return count_ == 0;
		}

		EPHERE_NODISCARD T* begin() const
		{
			return data_;
		}

		EPHERE_NODISCARD T* end() const
		{
			return data_ + count_;
		}

		EPHERE_NODISCARD T* data() const
		{
			return data_;
		}

		EPHERE_NODISCARD T& front() const
		{
			DEBUG_ONLY( ASSERT_RANGE( size() > 0 ) );
			return data_[0];
		}

		T& operator[]( size_type index ) const
		{
			DEBUG_ONLY( ASSERT_RANGE( 0 <= index && index < size() ) );
			return data_[index];
		}

		EPHERE_NODISCARD Span<element_type> subspan( size_type offset, size_type count = -1 ) const
		{
			DEBUG_ONLY( ASSERT_RANGE( offset >= 0 && size() - offset >= 0 ) );

			if( count < 0 )
			{
				return Span<element_type>( data() + offset, size() - offset );
			}

			DEBUG_ONLY( ASSERT_RANGE( size() - offset >= count ) );
			return Span<element_type>( data() + offset, count );
		}

	private:

		T* data_;
		size_type count_;
	};

	template <typename T, typename U>
	bool IsEmptyOrSameSizeAs( Span<T> const& span, Span<U> const& target )
	{
		return span.empty() || span.size() == target.size();
	}

	template <typename T>
	T* GetElementIfNotEmpty( Span<T> const& span, int index )
	{
		return !span.empty() ? &span[index] : nullptr;
	}

	template <typename T>
	Span<T> MakeSingleElementSpan( T& element )
	{
		return Span<T>( &element, &element + 1 );
	}

#if defined( EPHERE_HAVE_CPP17 )
	template <class Container, class = typename std::enable_if<( sizeof( typename Container::value_type ) > 0 )>::type>
	Span< typename std::remove_reference<decltype( *std::declval<Container>().data() )>::type > MakeSpan( Container& container )
	{
		typedef typename std::remove_reference<decltype( *std::declval<Container>().data() )>::type ElementType;
		return Span<ElementType>( container );
	}

	template <typename TByte, typename TContainer>
	auto AsBytes( TContainer&& container ) -> Span<std::conditional_t<std::is_const_v<std::remove_reference_t<decltype( container.front() )>>, TByte const, TByte>>
	{
		static_assert( sizeof( TByte ) == 1 );
		using ValueType = std::remove_reference_t<decltype( container.front() )>;
		using ResultByteType = std::conditional_t<std::is_const_v<ValueType>, TByte const, TByte>;
		return Span<ResultByteType>( reinterpret_cast<ResultByteType*>( container.data() ), static_cast<int>( container.size() * sizeof( ValueType ) ) );
	}
#endif
}
