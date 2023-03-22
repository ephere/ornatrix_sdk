// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/NativeTools/FixedString.h"

namespace Ephere { namespace Ornatrix
{

struct StrandChannelName
{
	// Enum preferred to static const int, see https://stackoverflow.com/questions/5391973/undefined-reference-to-static-const-int
	enum : int
	{
		MaximumNameLength = 16
	};

	FixedString<wchar_t, MaximumNameLength> name;
};

}
}
