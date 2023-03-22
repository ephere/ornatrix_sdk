#pragma once

#include "Ephere/NativeTools/MacroTools.h"

#if defined( EPHERE_HAVE_CPP17 ) || defined( __APPLE__ )
#   include <string_view>
#	define EPHERE_HAS_STRING_VIEW 1
#else
#	define EPHERE_HAS_STRING_VIEW 0
#endif

namespace Ephere
{

//! Binary-compatible std::string_view mini-alternative (most implementations of string_view use pointer+size_t, but GCC's libstdc++ uses size_t+pointer)
template <typename C>
class BasicStringView
{
public:

	typedef C value_type;

	typedef std::size_t size_type;


	// ReSharper disable once CppNonExplicitConvertingConstructor
	BasicStringView( C const* begin = nullptr )
		: data_( begin )
		, length_( data_ != nullptr ? std::char_traits<C>::length( data_ ) : 0 )
	{
	}

	BasicStringView( C const* begin, std::size_t length )
		: data_( begin )
		, length_( length )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <class TStringLike>
	BasicStringView( TStringLike const& s, typename std::enable_if<std::is_same<typename TStringLike::value_type, C>::value, int>::type = 0 )
		: data_( s.data() )
		, length_( s.length() )
	{
	}

	EPHERE_NODISCARD bool empty() const EPHERE_NOEXCEPT
	{
		return length() == 0;
	}

	EPHERE_NODISCARD std::size_t length() const EPHERE_NOEXCEPT
	{
		return length_;
	}

	C const& operator[]( size_type pos ) const
	{
		return data_[pos];
	}

	EPHERE_NODISCARD C const* data() const EPHERE_NOEXCEPT
	{
		return data_;
	}

	EPHERE_NODISCARD C const* begin() const EPHERE_NOEXCEPT
	{
		return data_;
	}

	EPHERE_NODISCARD C const* end() const EPHERE_NOEXCEPT
	{
		return data_ + length();
	}

	EPHERE_NODISCARD int compare( BasicStringView const& right ) const EPHERE_NOEXCEPT
	{
		int const result = std::char_traits<C>::compare( data_, right.data_, std::min( length_, right.length_ ) );

		if( result != 0 )
		{
			return result;
		}

		if( length_ < right.length_ )
		{
			return -1;
		}

		if( length_ > right.length_ )
		{
			return 1;
		}

		return 0;
	}

	void remove_prefix( size_type count ) EPHERE_NOEXCEPT
	{
		data_ += count;
		length_ -= count;
	}

	void remove_suffix( size_type count ) EPHERE_NOEXCEPT
	{
		length_ -= count;
	}

	EPHERE_NODISCARD BasicStringView substr( size_type pos = 0, size_type count = std::basic_string<C>::npos ) const
	{
		if( pos > length_ )
		{
			// Diverging from the standard which expects an out_of_range exception here
			return BasicStringView();
		}

		count = std::min( count, length_ - pos );
		return BasicStringView( data_ + pos, count );
	}

#if EPHERE_HAS_STRING_VIEW
	operator std::basic_string_view<C>() const
	{
		return data_ != nullptr ? std::basic_string_view<C>( data_, length() ) : std::basic_string_view<C>();
	}
#endif

	EPHERE_NODISCARD std::basic_string<C> ToString() const
	{
		return data_ != nullptr ? std::basic_string<C>( data_, length() ) : std::basic_string<C>();
	}

	friend bool operator==( BasicStringView const& a, BasicStringView const& b ) EPHERE_NOEXCEPT
	{
		return a.compare( b ) == 0;
	}

	friend bool operator!=( BasicStringView const& a, BasicStringView const& b ) EPHERE_NOEXCEPT
	{
		return !( a == b );
	}

	friend bool operator<( BasicStringView const& a, BasicStringView const& b ) EPHERE_NOEXCEPT
	{
		return a.compare( b ) < 0;
	}

private:
	C const* data_;

	std::size_t length_;
};

typedef BasicStringView<char> StringView;

}

#if !EPHERE_HAS_STRING_VIEW

namespace std
{

template <typename C>
class basic_string_view : public Ephere::BasicStringView<C>
{
public:

	typedef Ephere::BasicStringView<C> BaseType;
	typedef std::size_t size_type;

	// ReSharper disable once CppNonExplicitConvertingConstructor
	basic_string_view( C const* begin = nullptr )
		: BaseType( begin )
	{
	}

	basic_string_view( C const* begin, std::size_t length )
		: BaseType( begin, length )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <class TStringLike>
	basic_string_view( TStringLike const& s, typename std::enable_if<std::is_same<typename TStringLike::value_type, C>::value, int>::type = 0 )
		: BaseType( s )
	{
	}

	explicit operator std::basic_string<C>() const
	{
		return std::basic_string<C>( this->data(), this->length() );
	}
};

typedef basic_string_view<char> string_view;
typedef basic_string_view<wchar_t> wstring_view;
typedef basic_string_view<char16_t> u16string_view;

}

#endif
