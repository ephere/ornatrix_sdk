// Must compile with VC 2012 / GCC 4.8
// ReSharper disable CppVariableCanBeMadeConstexpr

#pragma once

#include "Ephere/NativeTools/Span.h"

#ifdef __APPLE__
#include <tgmath.h>
#else
#include <cmath>
#endif

#include <limits>
#include <tuple>
#include <type_traits>

namespace Ephere
{

#if defined( EPHERE_HAVE_CPP11 )
//!...
static constexpr auto Pi = 3.14159265358979323846;

//!...
static constexpr auto TwoPi = Pi * 2;

//!...
static constexpr auto HalfPi = Pi / 2;

//!...
static constexpr auto Pif = static_cast<float>( Pi );

//!...
static constexpr auto TwoPif = static_cast<float>( TwoPi );

//!...
static constexpr auto HalfPif = static_cast<float>( HalfPi );
#else
//!...
double const Pi = 3.14159265358979323846;

//!...
double const TwoPi = Pi * 2;

//!...
double const HalfPi = Pi / 2;

//!...
float const Pif = static_cast<float>( Pi );

//!...
float const TwoPif = static_cast<float>( TwoPi );

//!...
float const HalfPif = static_cast<float>( HalfPi );
#endif

/** Tests that provided value is not a "not a number". A float/double can become a NaN during certain operations, or if left uninitialized in some cases.
 * @param value Value to check
 * @return true if value is "not a number"
 **/
template<typename T>
bool IsNaN( T value )
{
	// ReSharper disable once CppIdenticalOperandsInBinaryExpression
	return value != value;
}

//!...
template <typename T>
T Infinity()
{
	return std::numeric_limits<T>::max();
}

template <typename T>
T NegativeInfinity()
{
	return std::numeric_limits<T>::lowest();
}

//! Convert degrees to radians
template <typename T>
typename std::enable_if< std::is_floating_point<T>::value, T >::type DegToRad( T deg )
{
	return T( deg * Pi / 180 );
}

//! Convert radians to degrees
template <typename T>
typename std::enable_if< std::is_floating_point<T>::value, T >::type RadToDeg( T rad )
{
	return T( rad * 180 / Pi );
}


//! Comparison for near equality
template <typename T>
bool NearEqual( T a, T b, T epsilon = T( 0.000001 ) )
{
	static_assert( std::is_floating_point<T>::value, "T must be float or double" );
	if( epsilon < 0 )
	{
		return false;
	}

	auto temp = a - b;
	return -epsilon < temp&& temp < epsilon;
}

//! Linear interpolation between two values
template <typename T, typename T2>
typename std::enable_if<std::is_floating_point<T2>::value, T>::type Interpolate( T a, T b, T2 t )
{
	return T( a * ( T2( 1 ) - t ) + b * t );
}

//! Linear interpolation between the closest two of multiple values (assumed equidistant)
template <typename T, typename T2>
typename std::enable_if<std::is_floating_point<T2>::value, T>::type Interpolate( T const* values, int const count, T2 t )
{
	auto const epsilon = 0.00001f;

	if( count < 1 )
	{
		return T( 0 );
	}

	if( count == 1 || t <= epsilon )
	{
		return values[0];
	}

	if( t >= T2( 1 ) - epsilon )
	{
		return values[count - 1];
	}

	auto const firstIndexScalar = t * ( count - 1 );
	auto const firstIndex = static_cast<int>( firstIndexScalar );
	auto const secondIndex = firstIndex + 1;
	auto const segmentPosition = firstIndexScalar - T2( firstIndex );

	return Interpolate( values[firstIndex], values[secondIndex], segmentPosition );
}

template <typename T, typename T2>
typename std::enable_if<std::is_floating_point<T2>::value, T>::type Interpolate( Span<T const> values, T2 t )
{
	return Interpolate( values.data(), Size( values ), t );
}

template <typename T>
T Clamp( T val, T valMin, T valMax )
{
	return val < valMin ? valMin : ( val > valMax ? valMax : val );
}

//! Return if val is inside [valMin,valMax]
template <typename T>
bool InRange( T val, T valMin, T valMax )
{
	return valMin <= val && val <= valMax;
}

inline double Round( double x, double factor )
{
	return floor( x * factor + 0.5 ) / factor;
}

template<typename T1, typename T2>
float GetFraction( T1 value1, T2 value2 )
{
	return static_cast<float>( value1 ) / static_cast<float>( value2 );
}

template<typename T1, typename T2, typename TResult>
TResult GetFractionT( T1 value1, T2 value2 )
{
	return static_cast<TResult>( value1 ) / static_cast<TResult>( value2 );
}

template<typename TIndex>
float GetIndexPosition( TIndex index, TIndex maximumIndexExclusive )
{
	return maximumIndexExclusive <= 1 ? 0.0f : GetFraction( index, maximumIndexExclusive - 1 );
}

template<typename TIndex, typename TPosition>
TPosition GetIndexPositionT( TIndex index, TIndex maximumIndexExclusive )
{
	return maximumIndexExclusive <= 1 ? TPosition( 0 ) : GetFractionT<TIndex, TIndex, TPosition>( index, maximumIndexExclusive - 1 );
}

template<typename TIndex, typename TPosition>
TIndex GetPositionIndex( TPosition position, TIndex maximumIndexExclusive )
{
	return std::min( static_cast<TIndex>( round( position * static_cast<TPosition>( maximumIndexExclusive ) ) ), maximumIndexExclusive - 1 );
}

template<typename TIndex>
TIndex TransformIndex( TIndex index, TIndex sourceIndexCount, TIndex destinationIndexCount )
{
	return static_cast<TIndex>( GetIndexPosition( index, sourceIndexCount ) * static_cast<float>( destinationIndexCount - 1 ) );
}

template<typename TIndex>
TIndex TransformIndex( TIndex index, TIndex sourceIndexCount, TIndex destinationIndexCount, float& fraction )
{
	auto const p = GetIndexPosition( index, sourceIndexCount ) * static_cast<float>( destinationIndexCount - 1 );
	auto indexF = 0.0f;
	fraction = std::modf( p, &indexF );
	return static_cast<TIndex>( indexF );
}

/** Put the value into specified range by wrapping it
	*	For example: 5 in range 0:3 produces 2. 7 in range 1:4 produces 3
	*	@param value Value to wrap
	*	@param rangeBegin Start of range
	*	@param rangeEnd End of range
	*	@return Wrapped value
	*/
template<typename T>
T WrapInRange( T value, T rangeBegin, T rangeEnd )
{
	value = value % ( rangeEnd - rangeBegin );

	if( value < rangeBegin )
	{
		value = rangeEnd - rangeBegin * 2 + value;
	}

	return value;
}

/** Puts a specified angle in radians within {-Pi,Pi} range
@param angle Angle in radians
@return Clipped angle
*/
template<typename T>
T ClipAngle( T angle )
{
	if( angle > T( Pi ) )
	{
		angle -= 2 * T( Pi );
	}
	else if( angle < -T( Pi ) )
	{
		angle += 2 * T( Pi );
	}

	return angle;
}

//! @return val^2
template <typename T>
T Square( T val )
{
	return val * val;
}

//! @return val^3
template <typename T>
T Cube( T val )
{
	return val * val * val;
}

//! Compile-time 2^N
template <int N>
struct Pow2
{
	enum
	{
		Value = 2 * Pow2<N - 1>::Value
	};
};

template <>
struct Pow2<0>
{
	enum
	{
		Value = 1
	};
};

template <typename T>
T TrueAngle( T cosA, T sinA )
{
	auto const asinA = T( asin( sinA ) );
	if( asinA >= T( 0 ) )
	{
		if( sinA >= T( 0 ) && cosA >= T( 0 ) )
		{
			return asinA;
		}

		if( sinA >= T( 0 ) && cosA < T( 0 ) )
		{
			return T( Pi ) - asinA;
		}
	}
	// If asinA < 0
	else
	{
		if( sinA < T( 0 ) && cosA < T( 0 ) )
		{
			return T( Pi ) - asinA;
		}

		if( sinA < T( 0 ) && cosA >= T( 0 ) )
		{
			return T( TwoPi + asinA );
		}
	}

	return T( 0 );
}

template <typename T>
std::tuple<int, T, T> SolveQuadratic( T const a, T const b, T const c, T epsilon = T( 0.000001 ) )
{
	auto rootCount = 0;
	auto root1 = T( 0 ), root2 = T( 0 );

	if( std::abs( a ) < epsilon )
	{
		if( std::abs( b ) < epsilon )
		{
			// Has meaningful solutions only for c == 0
			// root count -1 means that any x is a solution
			rootCount = std::abs( c ) < epsilon ? -1 : 0;
		}
		else
		{
			// linear equation
			rootCount = 1;
			root1 = -c / b;
		}
	}
	// TODO: Is this branch needed?
	else if( std::abs( c ) < epsilon )
	{
		// One root is x = 0
		root1 = T( 0 );
		if( std::abs( b ) < epsilon )
		{
			rootCount = 1;
		}
		else
		{
			rootCount = 2;
			root2 = -b / a;
		}

	}
	else
	{
		auto w = b * b - T( 4 ) * c * a;
		if( w > 0 )
		{
			w = sqrt( w );
			// single root
			if( w < epsilon )
			{
				rootCount = 1;
				root1 = -b / ( T( 2 ) * a );
			}
			// two roots
			else if( w > T( 0 ) )
			{
				rootCount = 2;
				root1 = ( -b - w ) / ( T( 2 ) * a );
				root2 = ( -b + w ) / ( T( 2 ) * a );
			}
		}
	}
	return std::make_tuple( rootCount, root1, root2 );
}

template<typename T>
bool TestFlag( T flags, T flag )
{
	return ( static_cast<uint64_t>( flags ) & static_cast<uint64_t>( flag ) ) == static_cast<uint64_t>( flag );
}

template<typename T>
T DegreesToRadians( T radians )
{
	return radians * T( Pi ) / T( 180 );
}

template<typename T>
bool IsSameSign( T const a, T const b )
{
	return a * b >= T( 0 );
}

}
