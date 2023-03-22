// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "Ephere/NativeTools/StringView.h"

#include <algorithm>
#include <array>
#include <string>
#include <type_traits>

namespace Ephere
{

// Zero-terminated character array of a constant size with string-like interface
template <typename C, int NSize>
class FixedString
{
public:

	typedef C value_type;

	enum : int
	{
		MaxLength = NSize - 1,
		Size = NSize,
	};

	static_assert( MaxLength > 0, "NSize must be more than 1" );

	FixedString()
	{
		data_[0] = 0;
	}

	template <typename U>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FixedString( U const& source, typename std::enable_if<std::is_same<C, typename U::value_type>::value, int>::type = 0 )
	{
		auto length = std::min( static_cast<int>( source.length() ), static_cast<int>( MaxLength ) );
		std::char_traits<C>::copy( data_.data(), source.data(), length );
		data_[length] = 0;
	}

	template <typename U>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FixedString( U const* source, typename std::enable_if< std::is_integral<U>::value && std::is_same<C, U>::value, int>::type = 0 )
	{
		auto sourceLength = static_cast<int>( std::char_traits<U>::length( source ) );
		*this = FixedString( std::basic_string_view<U>( source, sourceLength ) );
	}

	template <typename U>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FixedString( U const& source, typename std::enable_if<std::is_array<U>::value && std::is_same<C, typename std::remove_extent<U>::type>::value, int>::type = 0 )
	{
		*this = FixedString( &source[0] );
	}

	template <typename U>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FixedString( U const& source, typename std::enable_if< std::is_integral<typename U::value_type>::value && !std::is_same<C, typename U::value_type>::value && ( sizeof( C ) > 1 ), int>::type = 0 )
	{
		auto length = std::min( static_cast<int>( source.length() ), static_cast<int>( MaxLength ) );
		std::transform( source.data(), source.data() + length, data_.data(), []( typename U::value_type c )
		{
			return C( c );
		} );
		data_[length] = 0;
	}

	template <typename U>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FixedString( U const* source, typename std::enable_if< std::is_integral<U>::value && !std::is_same<C, U>::value && ( sizeof( C ) > 1 ), int>::type = 0 )
	{
		auto sourceLength = static_cast<int>( std::char_traits<U>::length( source ) );
		*this = FixedString( std::basic_string_view<U>( source, sourceLength ) );
	}

	template <typename U>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FixedString( U const& source, typename std::enable_if<std::is_array<U>::value && !std::is_same<C, typename std::remove_extent<U>::type>::value && ( sizeof( C ) > 1 ), int>::type = 0 )
	{
		*this = FixedString( &source[0] );
	}

	bool operator==( std::basic_string_view<C> other ) const
	{
		return std::basic_string_view<C>( data_.data() ) == other;
	}

	bool operator!=( std::basic_string_view<C> other ) const
	{
		return !( *this == other );
	}

	C operator[]( int index ) const
	{
		return data_[index];
	}

	C& operator[]( int index )
	{
		return data_[index];
	}

	EPHERE_NODISCARD bool empty() const
	{
		return data_[0] == 0;
	}

	EPHERE_NODISCARD int length() const
	{
		return static_cast<int>( std::char_traits<C>::length( data_.data() ) );
	}

	EPHERE_NODISCARD int size() const
	{
		return length();
	}

	EPHERE_NODISCARD C const* data() const
	{
		return data_.data();
	}

	EPHERE_NODISCARD C* data()
	{
		return data_.data();
	}

	EPHERE_NODISCARD C const* c_str() const
	{
		return data();
	}

	operator std::basic_string<C>() const
	{
		return std::basic_string<C>( data_.data() );
	}

	operator std::basic_string_view<C>() const
	{
		return std::basic_string_view<C>( data_.data() );
	}

	void clear()
	{
		data_[0] = 0;
	}

	template <typename U, size_t N>
	typename std::enable_if< ( sizeof( U ) > 1 && std::is_integral<U>::value ) >::type CopyToCArray( U( &destination )[N] ) const
	{
		auto targetLength = std::min( length(), static_cast<int>( N - 1 ) );
		std::transform( data_.data(), data_.data() + targetLength, destination, []( C c )
		{
			return U( c );
		} );
		destination[targetLength] = 0;
	}

private:

	std::array<C, Size> data_;
};

template <class T, typename C, int NSize>
bool operator==( T const& string, FixedString<C, NSize> const& fixed )
{
	return fixed == std::basic_string_view<C>( string );
}

}
