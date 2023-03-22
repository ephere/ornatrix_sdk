// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppVariableCanBeMadeConstexpr
#pragma once

#include "Ephere/NativeTools/SmartPointers.h"

#include <memory>

namespace Ephere
{

/*! Interface identifier.
Recommended scheme: 0xCCCIIIVV, CCC is company code, III - interface number, VV - version */
typedef unsigned InterfaceId;

InterfaceId const InterfaceIdNone = 0;

InterfaceId const InterfaceId_Company_Private = 0;
InterfaceId const InterfaceId_Company_Ephere = 0x100000;

/*!
Returns requested interfaces, if supported. Similar to COM IUnknown.
Retrieving an interface through IInterfaceProvider::GetInterface has the following advantages over C++ dynamic_cast:

- The target interface may be renamed inside the provider DLL. The client will continue to work using the old name because the interface is identified by a number,
not by its name. dynamic_cast won't work if the class name doesn't match in both the provider and the client DLLs.

- Returning an owning pointer allows the provider to build a new object that implements the interface and pass its ownership to the client.
Also, if the implementation is held by a shared pointer inside the provider DLL, the client DLL can share the ownership.
*/
struct IInterfaceProvider
{
	static unsigned const CurrentVersion = 1;


	EPHERE_NODISCARD virtual unsigned Version() const
	{
		return CurrentVersion;
	}

	virtual UniquePtr<void> GetInterface( char const* sourceName, InterfaceId interfaceId ) = 0;


	template <class T>
	std::shared_ptr<T> GetInterface( char const* sourceName = nullptr )
	{
		std::shared_ptr<T> result;
		if( auto pointer = GetInterface( sourceName, T::IID ) )
		{
			return std::static_pointer_cast<T>( pointer.ToShared() );
		}

		return result;
	}

protected:

	~IInterfaceProvider()
	{
	}
};

}
