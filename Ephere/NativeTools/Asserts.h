// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

#ifndef ASSERT
#	define CHECK( x, ... ) ( x )
#	define ASSERT( x, ... )
#	define ASSERT_ARGUMENT( x, ... )
#	define ASSERT_FAIL( ... )
#	define ASSERT_RANGE( x, ... )
#	define ASSERT_( x, e, ... )
#	define DEBUG_ONLY( x )
#endif
