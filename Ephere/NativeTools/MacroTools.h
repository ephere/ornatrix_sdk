#pragma once

#if defined( __cplusplus ) && __cplusplus >= 201703L || defined( _MSVC_LANG ) && _MSVC_LANG >= 201703L
#	define EPHERE_HAVE_CPP17 1
#endif

#if defined( EPHERE_HAVE_CPP17 )
#	define EPHERE_NODISCARD [[nodiscard]]
#	define EPHERE_NOEXCEPT noexcept
#else
#	define EPHERE_NODISCARD
#	define EPHERE_NOEXCEPT
#endif

#define EPHERE_NO_UTILITIES

#ifndef UNUSED_VALUE
#	define UNUSED_VALUE( x ) ( (void)( x ) )
#endif

#ifndef DEBUG_ONLY
#	ifdef NDEBUG
#		define DEBUG_ONLY( expression )
#	else
#		define DEBUG_ONLY( expression ) expression
#	endif
#endif
