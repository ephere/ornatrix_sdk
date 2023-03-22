// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/NativeTools/Asserts.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Ephere/NativeTools/StringView.h"

#include <cctype>

namespace Ephere
{

typedef std::basic_string<unsigned char> bstring;

template <typename C>
C const* EmptyString()
{
	return "";
}

template <>
inline wchar_t const* EmptyString()
{
	return L"";
}

template <typename C>
C const* Whitespace()
{
	return " \t\r\n";
}

template <>
inline wchar_t const* Whitespace()
{
	return L" \t\r\n";
}

// Returns true if the pointer is null or the first character is 0
template <typename C>
bool IsEmpty( C const* text )
{
	return text == nullptr || text[0] == 0;
}

template <int N, typename C>
std::basic_string_view<C> TakeN( C const* text )
{
	if( text == nullptr )
	{
		return std::basic_string_view<C>();
	}

	auto length = 0;
	while( length < N && text[length] != 0 )
	{
		++length;
	}

	return std::basic_string_view<C>( text, length );
}

template <typename C>
std::basic_string_view<C> TakeN( C const* text, int maximumLength )
{
	if( text == nullptr )
	{
		return std::basic_string_view<C>();
	}

	auto length = 0;
	while( length < maximumLength && text[length] != 0 )
	{
		++length;
	}

	return std::basic_string_view<C>( text, length );
}

template <typename C>
int Compare( C const* a, C const* b )
{
	if( IsEmpty( a ) )
	{
		return IsEmpty( b ) ? 0 : -1;
	}

	if( IsEmpty( b ) )
	{
		return 1;
	}

	auto const lengthA = static_cast<int>( std::char_traits<C>::length( a ) );
	auto const lengthB = static_cast<int>( std::char_traits<C>::length( b ) );
	auto result = std::char_traits<C>::compare( a, b, std::min( lengthA, lengthB ) );
	return result != 0 ? result : lengthA - lengthB;
}

template <typename C>
bool AreEqual( C const* a, C const* b )
{
	return Compare( a, b ) == 0;
}

template <typename C, class Traits = std::char_traits<C>>
bool StartsWithImpl( std::basic_string_view<C> text, std::basic_string_view<C> prefix )
{
	return prefix.length() <= text.length()
		&& Traits::compare( text.data(), prefix.data(), prefix.length() ) == 0;
}

template <class Traits = std::char_traits<char>>
bool StartsWith( std::string_view text, std::string_view prefix )
{
	return StartsWithImpl<char, Traits>( text, prefix );
}

template <class Traits = std::char_traits<wchar_t>>
bool StartsWith( std::wstring_view text, std::wstring_view prefix )
{
	return StartsWithImpl<wchar_t, Traits>( text, prefix );
}

template <typename C, class Traits = std::char_traits<C>>
bool EndsWithImpl( std::basic_string_view<C> text, std::basic_string_view<C> suffix )
{
	return suffix.length() <= text.length()
		&& Traits::compare( text.data() + text.length() - suffix.length(), suffix.data(), suffix.length() ) == 0;
}

template <class Traits = std::char_traits<char>>
bool EndsWith( std::string_view text, std::string_view suffix )
{
	return EndsWithImpl<char, Traits>( text, suffix );
}

template <class Traits = std::char_traits<wchar_t>>
bool EndsWith( std::wstring_view text, std::wstring_view suffix )
{
	return EndsWithImpl<wchar_t, Traits>( text, suffix );
}


template <typename C, class Traits = std::char_traits<C>>
std::basic_string_view<C> RemovePrefixImpl( std::basic_string_view<C> text, std::basic_string_view<C> prefix )
{
	if( StartsWithImpl<C, Traits>( text, prefix ) )
	{
		text.remove_prefix( prefix.length() );
	}

	return text;
}

template <class Traits = std::char_traits<char>>
std::string_view RemovePrefix( std::string_view text, std::string_view prefix )
{
	return RemovePrefixImpl<char, Traits>( text, prefix );
}

template <class Traits = std::char_traits<wchar_t>>
std::wstring_view RemovePrefix( std::wstring_view text, std::wstring_view prefix )
{
	return RemovePrefixImpl<wchar_t, Traits>( text, prefix );
}

template <typename C, class Traits = std::char_traits<C>>
std::basic_string_view<C> RemoveSuffixImpl( std::basic_string_view<C> text, std::basic_string_view<C> suffix )
{
	if( EndsWithImpl<C, Traits>( text, suffix ) )
	{
		text.remove_suffix( suffix.length() );
	}

	return text;
}

template <class Traits = std::char_traits<char>>
std::string_view RemoveSuffix( std::string_view text, std::string_view suffix )
{
	return RemoveSuffixImpl<char, Traits>( text, suffix );
}

template <class Traits = std::char_traits<wchar_t>>
std::wstring_view RemoveSuffix( std::wstring_view text, std::wstring_view suffix )
{
	return RemoveSuffixImpl<wchar_t, Traits>( text, suffix );
}

template <typename C, class Traits = std::char_traits<C>>
std::basic_string<C> AddSuffixImpl( std::basic_string_view<C> text, std::basic_string_view<C> suffix )
{
	return EndsWithImpl<C, Traits>( text, suffix ) ? std::basic_string<C>( text ) : std::basic_string<C>( text ) + suffix.data();
}

template <class Traits = std::char_traits<char>>
std::string AddSuffix( std::string_view text, std::string_view suffix )
{
	return AddSuffixImpl<char, Traits>( text, suffix );
}

template <class Traits = std::char_traits<wchar_t>>
std::wstring AddSuffix( std::wstring_view text, std::wstring_view suffix )
{
	return AddSuffixImpl<wchar_t, Traits>( text, suffix );
}


// Taken from https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
template<typename C>
std::basic_string<C> ReplaceAllImpl( std::basic_string_view<C> text, std::basic_string_view<C> from, std::basic_string_view<C> to )
{
	ASSERT_ARGUMENT( !from.empty() );
	size_t position = 0;
	std::basic_string<C> result{ text };
	while( ( position = result.find( from.data(), position ) ) != std::basic_string<C>::npos )
	{
		result.replace( position, from.length(), to.data() );
		// Handles case where 'to' is a substring of 'from'
		position += to.length();
	}

	return result;
}

inline std::string ReplaceAll( std::string_view text, std::string_view from, std::string_view to )
{
	return ReplaceAllImpl<char>( text, from, to );
}

inline std::wstring ReplaceAll( std::wstring_view text, std::wstring_view from, std::wstring_view to )
{
	return ReplaceAllImpl<wchar_t>( text, from, to );
}

// Taken from http://en.cppreference.com/w/cpp/string/char_traits / http://www.gotw.ca/gotw/029.htm
template <typename C>
struct CaseInsensitiveCharTraits : public std::char_traits<C>
{
	static bool eq( C c1, C c2 )
	{
		return std::toupper( c1 ) == std::toupper( c2 );
	}

	static bool lt( C c1, C c2 )
	{
		return std::toupper( c1 ) < std::toupper( c2 );
	}

	static int compare( C const* s1, C const* s2, size_t n )
	{
		while( n-- != 0 )
		{
			if( std::toupper( *s1 ) < std::toupper( *s2 ) )
			{
				return -1;
			}

			if( std::toupper( static_cast<int>( *s1 ) ) > std::toupper( static_cast<int>( *s2 ) ) )
			{
				return 1;
			}

			++s1;
			++s2;
		}

		return 0;
	}

	static C const* find( C const* s, int n, C a )
	{
		auto const ua( std::toupper( a ) );
		while( n-- != 0 )
		{
			if( std::toupper( *s ) == ua )
			{
				return s;
			}

			++s;
		}
		return nullptr;
	}
};

//!...
inline bool IsValidFilenameCharacter( wchar_t c )
{
	// The matter of valid file names is rather complex, see https://en.wikipedia.org/wiki/Filename
	// This is a simple approach
	return L'0' <= c && c <= L'9'
		|| L'A' <= c && c <= L'Z'
		|| L'a' <= c && c <= L'z'
		|| L'_' == c
		|| 128 <= c;
}

//!...
inline bool IsValidFilenameCharacter( char c )
{
	return IsValidFilenameCharacter( static_cast<wchar_t>( c ) );
}

}
