// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Matrix.h"

#include "Ephere/NativeTools/Asserts.h"
#include "Ephere/NativeTools/Span.h"

#include <vector>

namespace Ephere { namespace Geometry
{
	enum class Interpolation : int
	{
		Step,
		Linear,
		Smooth,
		Spline
	};

	inline std::ostream& operator<<( std::ostream& stream, Interpolation interpolation )
	{
		switch( interpolation )
		{
			case Interpolation::Step: stream << "Step"; break;
			case Interpolation::Linear: stream << "Linear"; break;
			case Interpolation::Smooth: stream << "Smooth"; break;
			case Interpolation::Spline: stream << "Spline"; break;
		}

		return stream;
	}

	template<typename T>
	struct ControlPoint
	{
		T position;
		T value;
		Interpolation type;
		int index;

		ControlPoint()
			: position( 0 ),
			value( 0 ),
			type( Interpolation::Spline ),
			index( 0 )
		{
		}

		ControlPoint( T position, T value, Interpolation type = Interpolation::Spline, int index = 0 )
			: position( position ),
			value( value ),
			type( type ),
			index( index )
		{
		}

		bool operator==( ControlPoint const& other ) const
		{
			return position == other.position && value == other.value && index == other.index && type == other.type;
		}

		bool operator<( ControlPoint const& other ) const
		{
			return position < other.position;
		}

		EPHERE_NODISCARD bool IsNearEqual( ControlPoint const& other, T epsilon = T( Epsilon ) ) const
		{
			return Ephere::NearEqual( position, other.position, epsilon ) &&
				Ephere::NearEqual( value, other.value, epsilon ) &&
				type == other.type &&
				index == other.index;
		}
	};

	template<typename T>
	std::ostream& operator<<( std::ostream& stream, ControlPoint<T> const& point )
	{
		return stream << point.type << ' ' << point.position << ' ' << point.value;
	}

	template<typename TValue, typename TPosition>
	TValue GetLinearlyInterpolatedValue( TValue const& first, TValue const& second, TPosition position )
	{
		return first * ( TPosition( 1 ) - position ) + second * position;
	}

	template<typename TValue, typename TPosition>
	TValue GetLinearlyInterpolatedValue( Span<TValue const> array, TPosition position )
	{
		ASSERT_ARGUMENT( position >= 0 && position <= 1 );

		if( fabs( TPosition( 1 ) - position ) < TPosition( 0.00001 ) )
		{
			return array[array.size() - 1];
		}

		auto const positionOnTargetArray = position * ( array.size() - 1 );
		auto const lowerIndex = static_cast<int>( floor( positionOnTargetArray ) );
		auto const coordinateBetweenValues = positionOnTargetArray - lowerIndex;
		auto const& first = array[lowerIndex];
		auto const& second = array[lowerIndex + 1];

		return GetLinearlyInterpolatedValue( first, second, coordinateBetweenValues );
	}

	template<typename T>
	T Smoothstep( T p0, T p1, T t )
	{
		auto const st = t * t * ( T( 3 ) - T( 2 ) * t );
		return GetLinearlyInterpolatedValue( p0, p1, st );
	}

	// Performs a catmull-rom curve interpolation between p1 and p2
	// Time is assumed to be given in range 0.0 < T < 1.0
	// Note that the tangents get clampped to be in time range defined by p1 and p2
	template<typename T>
	Matrix<2, 1, T> ClampedCubic( Matrix<2, 1, T> const& p0, Matrix<2, 1, T> const& p1, Matrix<2, 1, T> const& p2, Matrix<2, 1, T> const& p3, T t )
	{
		auto const t2 = t * t;
		auto const t3 = t2 * t;

		auto const h1 = T( 2 ) * t3 - T( 3 ) * t2 + T( 1 );
		auto const h2 = -T( 2 ) * t3 + T( 3 ) * t2;
		auto const h3 = t3 - T( 2 ) * t2 + t;
		auto const h4 = t3 - t2;

		auto pt1 = ( p2 - p0 ) * T( 0.5 );
		auto const xo1 = std::min( p2.x() - ( p1.x() + pt1.x() ), T( 0 ) );
		auto const clampRatio1 = CHECK( !Ephere::NearEqual( pt1.x(), T( 0 ) ) ) ? T( 1 ) - std::abs( xo1 ) / pt1.x() : T( 0 );

		auto& pt1x = pt1.x();
		pt1x *= clampRatio1;
		auto& pt1y = pt1.y();
		pt1y *= clampRatio1;

		auto pt2 = ( p3 - p1 ) * T( 0.5 );
		auto const xo2 = std::min( ( p2.x() - pt2.x() ) - p1.x(), T( 0 ) );
		auto const clampRatio2 = CHECK( !Ephere::NearEqual( pt2.x(), T( 0 ) ) ) ? T( 1 ) - std::abs( xo2 ) / pt2.x() : T( 0 );

		auto& pt2x = pt2.x();
		pt2x *= clampRatio2;
		auto& pt2y = pt2.y();
		pt2y *= clampRatio2;

		return p1 * h1 + p2 * h2 + pt1 * h3 + pt2 * h4;
	}

	// Given the control point data, sample as timed profile curve
	// The positions and values are assumed to be in range 0.0 < X < 1.0
	// We require the data to be sorted by position
	void SampleAsProfileCurve(
		std::vector<float> const& positions,
		std::vector<float> const& values,
		std::vector<Interpolation> const& interpolations,
		std::vector<float>& outPositions,
		std::vector<float>& outValues );

	template<typename T>
	T EvaluateAsProfileCurve_( Span<ControlPoint<T> const> points, T coordinate )
	{
		if( points.empty() )
		{
			return 0;
		}

		if( points.size() == 1 || coordinate < points[0].position )
		{
			return points[0].value;
		}

		auto const& lastPoint = points[points.size() - 1];
		if( coordinate > lastPoint.position )
		{
			return lastPoint.value;
		}

		auto index = 1;
		for( ; index < points.size(); ++index )
		{
			// Add an epsilon to ensure that we can get a reasonable output from maximum value
			if( coordinate < points[index].position + T( Epsilon ) )
			{
				break;
			}
		}

		if( !CHECK( index < points.size() ) )
		{
			return 0;
		}

		auto const& point0 = points[index - 1];
		auto const value0 = point0.value;

		// When using step interpolation we just need to return previous point's value
		if( point0.type == Interpolation::Step )
		{
			return value0;
		}

		auto const& point1 = points[index];

		auto const position0 = point0.position;
		auto const position1 = point1.position;
		auto const segmentRange = position1 - position0;

		// If point is located between two points which are in the same place return the value of the first point.
		// This is needed to prevent degenerate results.
		if( fabs( segmentRange ) <= T( Epsilon ) )
		{
			return value0;
		}

		auto const value1 = point1.value;
		auto const positionInsideSegment = ( coordinate - position0 ) / segmentRange;

		switch( point0.type )
		{
			case Interpolation::Linear:
				return GetLinearlyInterpolatedValue( value0, value1, positionInsideSegment );
			case Interpolation::Smooth:
				return Smoothstep( value0, value1, positionInsideSegment );
			case Interpolation::Spline:
			{
				T lp, rp, lv, rv;

				if( index == 1 )
				{
					lp = position0 + segmentRange * T( -1 );
					lv = value0;
				}
				else
				{
					lp = points[index - 2].position;
					lv = points[index - 2].value;
				}

				if( index == points.size() - 1 )
				{
					rp = position1 + segmentRange;
					rv = value1;
				}
				else
				{
					rp = points[index + 1].position;
					rv = points[index + 1].value;
				}

				auto const pt = ClampedCubic( Matrix<2, 1, T>( lp, lv ), Matrix<2, 1, T>( position0, value0 ), Matrix<2, 1, T>( position1, value1 ), Matrix<2, 1, T>( rp, rv ), positionInsideSegment );

				return pt.y();
			}

			default:
				break;
		}

		return 0.0f;
	}

	// Given the control point data, evaluate as timed profile curve
	// The positions, values and time are assumed to be in range 0.0 < X < 1.0
	// NOTE: We require the data to be sorted by position
	inline float EvaluateAsProfileCurve( Span<ControlPoint<float> const> points, float coordinate )
	{
		return EvaluateAsProfileCurve_<float>( points, coordinate );
	}

	inline double EvaluateAsProfileCurve( Span<ControlPoint<double> const> points, double coordinate )
	{
		return EvaluateAsProfileCurve_<double>( points, coordinate );
	}

	void CubicBezier( std::vector<Vector3f>& points, Vector3f const& controlPoint0, Vector3f const& controlPoint1, bool modifyLastPoint = false );

} }
