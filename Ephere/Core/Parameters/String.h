// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppVariableCanBeMadeConstexpr
#pragma once

#include "Ephere/NativeTools/Asserts.h"
#include "Ephere/NativeTools/SmartPointers.h"

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

namespace Ephere { namespace Parameters
{

//! Binary-compatible std::string replacement. Assumes little endian platform to implement short string storage.
template <typename C>
class BasicString
{
public:

	static_assert( sizeof( C ) == 1 || sizeof( C ) == 2 || sizeof( C ) == 4, "Unsupported char size" );

	typedef C value_type;

	typedef C* pointer;
	typedef C const * const_pointer;
	typedef C& reference;
	typedef C const & const_reference;

	typedef pointer iterator;
	typedef const_pointer const_iterator;
	typedef std::reverse_iterator< const_iterator > reverse_iterator;
	typedef	std::reverse_iterator< const_iterator > const_reverse_iterator;

	typedef std::int64_t size_type;
	typedef std::ptrdiff_t difference_type;

	// Must be unsigned
	typedef typename std::make_unsigned<C>::type UnsignedC;

	static size_type const MaxLength = static_cast<size_type>( ( 1ULL << ( sizeof( size_type ) * 8 - 1 ) ) - 2 );

	// The most significant bit of the last char (char may be 2 or 4 bytes) is the last byte in the storage (assuming little endian)
	static UnsignedC const IsShortMask = 1U << ( sizeof( UnsignedC ) * 8 - 1 );


	~BasicString()
	{
		Destroy();
	}

	BasicString()
	{
		SetEmptyShort();
	}

	BasicString( BasicString const& other )
	{
		if( other.IsShort() )
		{
			short_ = other.short_;
		}
		else
		{
			InitializeFrom( other.c_str(), other.length() );
		}
	}

	BasicString( BasicString&& other ) EPHERE_NOEXCEPT
		: long_( other.long_ )
	{
		if( !other.IsShort() )
		{
			other.SetEmptyShort();
		}
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	BasicString( C const* cstring )
	{
		ASSERT( cstring != nullptr );
		InitializeFrom( cstring, static_cast<size_type>( std::char_traits<C>::length( cstring ) ) );
	}

	BasicString( C const* cstring, size_type length )
	{
		ASSERT( cstring != nullptr );
		InitializeFrom( cstring, length );
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	BasicString( std::basic_string<C> const& other )
	{
		InitializeFrom( other.data(), static_cast<size_type>( other.length() ) );
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	BasicString( std::basic_string_view<C> other )
	{
		if( other.empty() )
		{
			SetEmptyShort();
		}
		else
		{
			InitializeFrom( other.data(), static_cast<size_type>( other.length() ) );
		}
	}

	explicit BasicString( size_type count, C fillCharacter )
	{
		if( count <= MaxShortLength )
		{
			short_.length = MakeShortLength( count );
			std::fill( short_.data, short_.data + count, fillCharacter );
			short_.data[count] = 0;
		}
		else
		{
			ASSERT( count < MaxLength );
			long_.length = count;
			long_.data = new C[count + 1];
			long_.owner = StaticDefaultDeleteOwner<C>::GetInstance();
			std::fill( long_.data, long_.data + count, fillCharacter );
			long_.data[count] = 0;
		}
	}

	// ReSharper disable once CppPossiblyUninitializedMember
	explicit BasicString( UniquePtr<C>&& ptr )
	{
		ASSERT( ptr.get() != nullptr );
		InitializeFrom( std::move( ptr ), static_cast<size_type>( std::char_traits<C>::length( ptr.get() ) ) );
	}

	// ReSharper disable once CppPossiblyUninitializedMember
	BasicString( UniquePtr<C>&& ptr, size_type len )
	{
		ASSERT( ptr.get() != nullptr );
		InitializeFrom( std::move( ptr ), len );
	}

	static BasicString MakeView( std::basic_string_view<C> other )
	{
		BasicString result;
		result.long_.data = const_cast<C*>( other.data() );
		result.long_.owner = nullptr;
		result.long_.length = static_cast<size_type>( other.length() );
		return result;
	}

	BasicString& operator=( BasicString const& other )
	{
		if( IsShort() )
		{
			if( other.IsShort() )
			{
				short_ = other.short_;
			}
			else
			{
				InitializeFrom( other.long_.data, other.long_.length );
			}
		}
		else if( other.IsShort() || other.long_.data != long_.data )
		{
			Destroy();
			InitializeFrom( other.c_str(), other.length() );
		}

		return *this;
	}

	BasicString& operator=( BasicString&& other ) EPHERE_NOEXCEPT
	{
		if( IsShort() )
		{
			short_ = other.short_;
			if( !other.IsShort() )
			{
				other.SetEmptyShort();
			}
		}
		else if( other.IsShort() )
		{
			Destroy();
			long_ = other.long_;
		}
		else if( other.long_.data != long_.data )
		{
			Destroy();
			long_ = other.long_;
			other.SetEmptyShort();
		}

		return *this;
	}

	BasicString& operator=( C const* cstring )
	{
		assign( cstring );
		return *this;
	}

	BasicString& assign( C const* cstring )
	{
		ASSERT( cstring != nullptr );
		return assign( cstring, static_cast<size_type>( std::char_traits<C>::length( cstring ) ) );
	}

	BasicString& assign( C const* text, size_type len )
	{
		if( begin() != text )
		{
			Destroy();
			InitializeFrom( text, len );
		}

		return *this;
	}

	EPHERE_NODISCARD bool empty() const
	{
		return length() == 0;
	}

	void clear()
	{
		Destroy();
		SetEmptyShort();
	}

	void swap( BasicString& other ) EPHERE_NOEXCEPT
	{
		std::swap( long_, other.long_ );
	}

	EPHERE_NODISCARD size_type length() const
	{
		return IsShort() ? ( short_.length & ~IsShortMask ) : long_.length;
	}

	EPHERE_NODISCARD size_type size() const
	{
		return length();
	}

	EPHERE_NODISCARD C const* c_str() const
	{
		return begin();
	}

	EPHERE_NODISCARD C const* data() const
	{
		return begin();
	}

	EPHERE_NODISCARD C const* begin() const
	{
		return IsShort() ? short_.data : long_.data;
	}

	EPHERE_NODISCARD C const* end() const
	{
		return begin() + length();
	}

	C* begin()
	{
		return IsShort() ? short_.data : long_.data;
	}

	C* end()
	{
		return begin() + length();
	}

	EPHERE_NODISCARD C front() const
	{
		DEBUG_ONLY( ASSERT( length() > 0 ) );
		return begin()[0];
	}

	EPHERE_NODISCARD C back() const
	{
		DEBUG_ONLY( ASSERT( length() > 0 ) );
		return begin()[length() - 1];
	}

	C const& operator[] ( size_type i ) const
	{
		DEBUG_ONLY( ASSERT( 0 <= i && i < length() ) );
		return begin()[i];
	}

	C& operator[] ( size_type i )
	{
		return const_cast<C&>( const_cast<BasicString<C> const&>( *this )[i] );
	}

	operator std::basic_string<C>() const
	{
		return std::basic_string<C>( c_str(), length() );
	}

	operator std::basic_string_view<C>() const
	{
		return std::basic_string_view<C>( c_str(), length() );
	}


	EPHERE_NODISCARD bool IsShort() const
	{
		return ( short_.length & IsShortMask ) != 0;
	}

	UniquePtr<C> Release()
	{
		if ( IsShort() )
		{
			UniquePtr<C> result( new C[length() + 1] );
			std::char_traits<C>::copy( result.get(), short_.data, length() + 1 );
			return result;
		}
		else
		{
			UniquePtr<C> result( long_.data, long_.owner );
			SetEmptyShort();
			return result;
		}
	}

private:

	// Forbidden
	// ReSharper disable CppFunctionIsNotImplemented
	// ReSharper disable once CppNonExplicitConvertingConstructor
	BasicString( std::nullptr_t );
	BasicString& assign( std::nullptr_t );
	// ReSharper restore CppFunctionIsNotImplemented

	void Destroy()
	{
		if( !IsShort() && long_.data != nullptr && long_.owner != nullptr )
		{
			long_.owner->Deleter( long_.data, long_.owner );
		}
	}

	static UnsignedC MakeShortLength( size_type length )
	{
		return static_cast<UnsignedC>( length ) | IsShortMask;
	}

	void InitializeFrom( C const* text, size_type length )
	{
		ASSERT( length < MaxLength );
		ASSERT( text != nullptr );
		if( length <= MaxShortLength )
		{
			short_.length = MakeShortLength( length );
			std::char_traits<C>::copy( short_.data, text, length );
			short_.data[length] = 0;
		}
		else
		{
			long_.data = new C[length + 1];
			long_.owner = StaticDefaultDeleteOwner<C>::GetInstance();
			long_.length = length;
			std::char_traits<C>::copy( long_.data, text, length );
			long_.data[length] = 0;
		}
	}

	void InitializeFrom( UniquePtr<C>&& ptr, size_type length )
	{
		ASSERT( length < MaxLength );
		ASSERT( ptr != nullptr );
		long_.owner = ptr.ReleaseOwnership();
		long_.data = ptr.release();
		long_.length = length;
	}

	void SetEmptyShort() EPHERE_NOEXCEPT
	{
		short_.data[0] = 0;
		short_.length = MakeShortLength( 0 );
	}

	struct Long  // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		C* data;

		Ephere_OwnerContainer* owner;

		// If the most significant bit of the last byte (highest byte, we assume little endian) is 1, the string is "short", i.e. stored within the structure instead of dynamically allocated
		size_type length;
	};

	static int const ShortBufferSize = sizeof( Long ) / sizeof( C ) - 1;

public:

	static int const MaxShortLength = ShortBufferSize - 1;

private:

	struct Short
	{
		C data[ShortBufferSize];

		// If the most significant bit is 1, the string is "short", i.e. stored within the structure instead of dynamically allocated
		UnsignedC length;
	};

	union
	{
		Long long_;

		Short short_;
	};

	static_assert( sizeof( Long ) == sizeof( Short ), "String internal storage failure" );
};


typedef BasicString<char> String;

typedef BasicString<wchar_t> WString;


template <typename C>
C const* begin( BasicString<C> const& text )
{
	return text.begin();
}

template <typename C>
C const* end( BasicString<C> const& text )
{
	return text.end();
}

template <typename C>
C* begin( BasicString<C>& text )
{
	return text.begin();
}

template <typename C>
C* end( BasicString<C>& text )
{
	return text.end();
}

template <typename C>
bool operator== ( BasicString<C> const& l, BasicString<C> const& r )
{
	return std::basic_string_view<C>( l ) == std::basic_string_view<C>( r );
}

template <typename C>
bool operator!= ( BasicString<C> const& l, BasicString<C> const& r )
{
	return !( l == r );
}

template <typename C>
bool operator== ( BasicString<C> const& l, C const* r )
{
	ASSERT( r != nullptr );
	return std::basic_string_view<C>( l ) == std::basic_string_view<C>( r );
}

template <typename C>
bool operator!= ( BasicString<C> const& l, C const* r )
{
	return !( l == r );
}

template <typename C>
bool operator== ( C const* l, BasicString<C> const& r )
{
	ASSERT( l != nullptr );
	return std::basic_string_view<C>( l ) == std::basic_string_view<C>( r );
}

template <typename C>
bool operator!= ( C const* l, BasicString<C> const& r )
{
	return !( l == r );
}

template <typename C>
bool operator== ( BasicString<C> const& l, std::basic_string<C> const& r )
{
	return std::basic_string_view<C>( l ) == std::basic_string_view<C>( r );
}

template <typename C>
bool operator!= ( BasicString<C> const& l, std::basic_string<C> const& r )
{
	return !( l == r );
}

template <typename C>
bool operator== ( std::basic_string<C> const& l, BasicString<C> const& r )
{
	return std::basic_string_view<C>( l ) == std::basic_string_view<C>( r );
}

template <typename C>
bool operator!= ( std::basic_string<C> const& l, BasicString<C> const& r )
{
	return !( l == r );
}

template <typename C>
bool operator== ( BasicString<C> const& l, std::basic_string_view<C> const& r )
{
	return std::basic_string_view<C>( l ) == r;
}

template <typename C>
bool operator!= ( BasicString<C> const& l, std::basic_string_view<C> const& r )
{
	return !( l == r );
}

template <typename C>
bool operator== ( std::basic_string_view<C> const& l, BasicString<C> const& r )
{
	return l == std::basic_string_view<C>( r );
}

template <typename C>
bool operator!= ( std::basic_string_view<C> const& l, BasicString<C> const& r )
{
	return !( l == r );
}

template <typename C>
bool operator< ( BasicString<C> const& l, BasicString<C> const& r )
{
	return std::basic_string_view<C>( l ) < std::basic_string_view<C>( r );
}

template <typename C>
std::basic_string<C> operator+ ( BasicString<C> const& l, std::basic_string<C> const& r )
{
	return l.c_str() ? l.c_str() + r : r;
}


template <typename C>
std::basic_string<C> operator+ ( std::basic_string<C> const& l, BasicString<C> const& r )
{
	return r.c_str() ? l + r.c_str() : l;
}

template <typename C>
std::basic_ostream<C>& operator<< ( std::basic_ostream<C>& stream, BasicString<C> const& str )
{
	if ( !str.empty() )
	{
		stream << str.c_str();
	}

	return stream;
}

//! Wide streams accept char strings
inline std::wostream& operator<< ( std::wostream& stream, String const& str )
{
	if ( !str.empty() )
	{
		stream << str.c_str();
	}

	return stream;
}

} }

// ReSharper disable once CppRedundantNamespaceDefinition
namespace std
{
template <typename C>
struct hash<Ephere::Parameters::BasicString<C>>
{
	size_t operator()( Ephere::Parameters::BasicString<C> const& value ) const EPHERE_NOEXCEPT
	{
#ifdef EPHERE_HAVE_CPP11
		return std::hash<std::basic_string_view<C>>{}( std::basic_string_view<C>( value ) );
#else
		// The pre-C++11 string_view implementations do not include a hash<> specialization or convert the string_view to string and use hash<string>, which we can do ourselves
		return std::hash<std::basic_string<C>>{}( std::basic_string<C>( value ) );
#endif
	}
};
}
