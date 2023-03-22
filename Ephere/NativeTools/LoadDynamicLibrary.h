// Must compile as C and C++ with VC 2012 / GCC 4.8

// Minimal cross-platform tools to load a shared library

// ReSharper disable CppVariableCanBeMadeConstexpr
// ReSharper disable CppClangTidyModernizeReturnBracedInitList
// ReSharper disable CppClangTidyReadabilityRedundantDeclaration
// ReSharper disable CppCStyleCast
#pragma once

#ifndef __cplusplus
#	define nullptr ((void*)0)
#else
#	include <memory>
#	include <string>
#endif

#ifdef __cplusplus
#	define EPHERE_EXTERN_C extern "C"
#else
#	define EPHERE_EXTERN_C extern
#endif

#ifdef _MSC_VER
#	define EPHERE_API_EXPORT EPHERE_EXTERN_C __declspec( dllexport )
#	define EPHERE_API_IMPORT EPHERE_EXTERN_C __declspec( dllimport )
#else
#	define EPHERE_API_EXPORT EPHERE_EXTERN_C __attribute__(( visibility( "default" ) ))
#	define EPHERE_API_IMPORT
#endif

#ifdef _WIN32

// Copy the stuff we need from Windows.h to avoid including it

// ReSharper disable once CppUnusedIncludeDirective - for va_list
#include <stdarg.h>

typedef struct HINSTANCE__ *HINSTANCE;
typedef HINSTANCE HMODULE;
typedef unsigned long DWORD;

#ifdef __cplusplus
extern "C" {
#endif

__declspec( dllimport ) HMODULE __stdcall LoadLibraryA( char const* lpLibFileName );
__declspec( dllimport ) int __stdcall FreeLibrary( HMODULE hLibModule );
typedef __int64( __stdcall *FARPROC )( );  // NOLINT(clang-diagnostic-language-extension-token)
__declspec( dllimport ) FARPROC __stdcall GetProcAddress( HMODULE hModule, char const* lpProcName );
__declspec( dllimport ) DWORD __stdcall GetLastError( void );
__declspec( dllimport ) DWORD __stdcall FormatMessageA(
	DWORD dwFlags, void const* lpSource, DWORD dwMessageId, DWORD dwLanguageId, char* lpBuffer, DWORD nSize, va_list *Arguments );
__declspec( dllimport ) void* __stdcall LocalFree( void* hMem );
__declspec( dllimport ) int __stdcall GetModuleHandleExA( DWORD dwFlags, char const* lpModuleName, HMODULE* phModule );
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA( HMODULE hModule, char* lpFilename, DWORD nSize );

#ifdef __cplusplus
}

namespace Ephere
{
	inline std::string GetLoadDynamicLibraryError()
	{
		std::string result;
		if( GetLastError() != 0 )
		{
			char* errorPointer = nullptr;
			FormatMessageA(
				0x3100 /* FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY */,
				nullptr,
				GetLastError(),
				0 /*LANG_NEUTRAL*/,
				reinterpret_cast<char*>( &errorPointer ),
				0,
				nullptr );

			if( errorPointer != nullptr )
			{
				result = errorPointer;
				//remove cr and newline character
				result.erase( result.length() - 2, 2 );
				LocalFree( errorPointer );
			}
		}

		return result;
	}

	inline std::string GetDynamicLibraryPath( void const* functionAddress )
	{
		HMODULE module = nullptr;
		auto const maxPath = 260;
		char modName[maxPath] = { 0 };
		if( functionAddress != nullptr
			// ReSharper disable once CppReinterpretCastFromVoidPtr
			&& GetModuleHandleExA( 0x4 /*GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS*/ | 0x2 /*GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT*/, reinterpret_cast<char const*>( functionAddress ), &module )
			&& GetModuleFileNameA( module, modName, maxPath ) )
		{
			return modName;
		}

		return "";
	}
}

#endif

#define MAKE_DYNLIB_FILENAME( basename ) basename ".dll"

inline void* Ephere_LoadDynamicLibrary( char const* libraryFilePath )
{
	return LoadLibraryA( libraryFilePath );
}

inline void Ephere_FreeDynamicLibrary( void* handle )
{
	if( handle != nullptr )
	{
		FreeLibrary( (HMODULE)handle );
	}
}

inline void* Ephere_GetDynamicLibraryFunction( void* handle, char const* functionName )
{
	return handle != nullptr ? (void*)( GetProcAddress( (HMODULE)handle, functionName ) ) : nullptr;
}

#else // _WIN32

// On MacOS dlfcn.h is enabled only if _POSIX_C_SOURCE is not defined
#undef _POSIX_C_SOURCE

#include <dlfcn.h>

#if defined( __APPLE__ )
	#define MAKE_DYNLIB_FILENAME( basename ) "lib" basename ".dylib"
#else
	#define MAKE_DYNLIB_FILENAME( basename ) "lib" basename ".so"
#endif

void* Ephere_LoadDynamicLibrary( const char* libraryFilePath );
inline void* Ephere_LoadDynamicLibrary( const char* libraryFilePath )
{
	return dlopen( libraryFilePath, RTLD_NOW | RTLD_GLOBAL );
}

void Ephere_FreeDynamicLibrary( void* handle );
inline void Ephere_FreeDynamicLibrary( void* handle )
{
	if( handle != nullptr )
	{
		dlclose( handle );
	}
}

void* Ephere_GetDynamicLibraryFunction( void* handle, char const* functionName );
inline void* Ephere_GetDynamicLibraryFunction( void* handle, char const* functionName )
{
	return handle != nullptr ? dlsym( handle, functionName ) : nullptr;
}

#ifdef __cplusplus

namespace Ephere
{
	inline std::string GetLoadDynamicLibraryError()
	{
		auto errorText = dlerror();
		return errorText != nullptr ? std::string( errorText ) : std::string();
	}

	inline std::string GetDynamicLibraryPath( void* functionAddress )
	{
		Dl_info info;
		return dladdr( functionAddress, &info ) ? info.dli_fname : "";
	}
}

#endif

#endif // _WIN32

#ifndef __cplusplus
#	undef nullptr
#else

namespace Ephere
{

inline std::string MakeDynamicLibraryFilename( std::string const& rawName )
{
#if defined( _WIN32 )
	return rawName + ".dll";
#elif defined( __APPLE__ )
	return "lib" + rawName + ".dylib";
#else
	return "lib" + rawName + ".so";
#endif
}

inline std::shared_ptr<void> LoadDynamicLibrary( char const* libraryFilePath )
{
	return std::shared_ptr<void>( Ephere_LoadDynamicLibrary( libraryFilePath ), Ephere_FreeDynamicLibrary );
}

//! "Raw" library name means the name of the library without any prefixes ('lib' on Unix) or extensions
inline std::shared_ptr<void> LoadDynamicLibrary( std::string const& libraryRawName, std::string const& libraryDirectory, std::string* fullPath = nullptr )
{
	std::string const libraryFileName = MakeDynamicLibraryFilename( libraryRawName );
	auto const libraryFilePath = libraryDirectory.empty() ? libraryFileName : libraryDirectory + '/' + libraryFileName;
	if( fullPath != nullptr )
	{
		*fullPath = libraryFilePath;
	}

	return LoadDynamicLibrary( libraryFilePath.c_str() );
}

}

#endif
