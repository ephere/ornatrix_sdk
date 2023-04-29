// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

#include "Ephere/NativeTools/MacroTools.h"

#ifndef ASSERT
#	define CHECK( x, ... ) ( x )
#	define ASSERT( x, ... ) ( x )
#	define ASSERT_ARGUMENT( x, ... ) ( x )
#	define ASSERT_FAIL( ... )
#	define ASSERT_RANGE( x, ... ) ( x )
#	define ASSERT_( x, e, ... ) ( x )
#endif
