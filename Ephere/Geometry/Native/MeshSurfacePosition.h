// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Matrix.h"
#include "Ephere/NativeTools/MacroTools.h"
#include "Ephere/NativeTools/Span.h"

#include <cmath>
#include <vector>

namespace Ephere { namespace Geometry
{

struct MeshSurfacePosition
{
	MeshSurfacePosition():
		faceIndex( -1 )
	{
	}

	MeshSurfacePosition( int faceIndexValue, Vector3f const& barycentricCoordinateValue ):
		faceIndex( faceIndexValue ),
		barycentricCoordinate( barycentricCoordinateValue )
	{
	}

	//! Index of the face on the mesh or id of the base strand for propagated strands
	int faceIndex;

	//! Barycentric coordinate on the face
	Vector3f barycentricCoordinate;

	// Checks if strand propagated (strands whose roots are not on mesh surface but on other strands).
	// In such case, the face index is the index of the strand on which the current strand is located,
	// and the X barycentric value is the position on the strand from 0 to 1.
	EPHERE_NODISCARD bool IsPropagatedStrand() const
	{
		return !barycentricCoordinate.HasNaNElements() && barycentricCoordinate.z() < -0.5f;
	}

	EPHERE_NODISCARD unsigned GetUniquePropagatedStrandAndSideId() const
	{
		// TODO: Need to find a better way of hashing in the side index
		return faceIndex + static_cast<unsigned>( barycentricCoordinate.y() ) * 1000000;
	}

	bool operator == ( MeshSurfacePosition const& other ) const
	{
		return faceIndex == other.faceIndex && barycentricCoordinate == other.barycentricCoordinate;
	}
};

struct SurfacePosition
{
	enum : unsigned
	{
		InvalidFaceIndex = static_cast<unsigned>( -1 ),
	};

	//! Index of the face on the mesh or id of the base strand for propagated strands
	unsigned faceIndex;

	/** Parametric (UV) coordinates along the two leading edges for triangles and quads, or position along strand for propagated strands (y coordinate is not used in such case)
	 * If the .x value is above or equal to 1 then the whole amount represents sub-face index for Ngons or side index for propagated strands and fractional amount is the coordinate value.
	 * NOTE: Whole value of .y coordinate is currently unused but may be assigned to something in the future.
	 **/
	Vector2f surfaceCoordinate;

	SurfacePosition()
		: faceIndex( 0 ),
		surfaceCoordinate( Vector2f::Zero() )
	{
	}

	SurfacePosition( unsigned faceIndex, Vector2f surfaceCoordinate )
		: faceIndex( faceIndex ),
		surfaceCoordinate( surfaceCoordinate )
	{
	}

	void Set( int const faceIndexValue, int const faceSubIndex, Vector2f const& surfaceCoordinateValue )
	{
		faceIndex = faceIndexValue;
		surfaceCoordinate = surfaceCoordinateValue;
		if( faceSubIndex > 0 )
		{
			// Prevent rounding of floating point values due to precision
			auto const valueWithSubFaceIndex = static_cast<float>( faceSubIndex ) + surfaceCoordinate.x();

			// Move to NativeTools/Mathematics.h if needed in other VC 2012 code
#if defined( _MSC_VER ) && _MSC_VER < 1800
#	define Ephere_NextAfter _nextafterf
#else
#	define Ephere_NextAfter std::nextafter
#endif

			surfaceCoordinate.x() = valueWithSubFaceIndex == static_cast<float>( faceSubIndex + 1 ) ? Ephere_NextAfter( valueWithSubFaceIndex, 0.0f ) : valueWithSubFaceIndex;

#undef Ephere_NextAfter
		}
	}

	void Get( int& faceIndexValue, int& faceSubIndex, Vector2f& surfaceCoordinateValue ) const
	{
		faceIndexValue = static_cast<int>( faceIndex );
		surfaceCoordinateValue = surfaceCoordinate;
		if( surfaceCoordinate.x() > 1.0f )
		{
			faceSubIndex = static_cast<int>( surfaceCoordinate.x() );
			surfaceCoordinateValue.x() = surfaceCoordinateValue.x() - static_cast<float>( faceSubIndex );
		}
	}

	EPHERE_NODISCARD bool IsNearEqual( SurfacePosition const& other, float const epsilon = Epsilon ) const
	{
		return faceIndex == other.faceIndex && fabs( surfaceCoordinate.x() - other.surfaceCoordinate.x() ) <= epsilon && fabs( surfaceCoordinate.y() - other.surfaceCoordinate.y() ) <= epsilon;
	}

	EPHERE_NODISCARD float GetPropagatedSurfaceCoordinate() const
	{
		return surfaceCoordinate.x();
	}

	EPHERE_NODISCARD int GetPropagatedSideIndex() const
	{
		return -static_cast<int>( surfaceCoordinate.y() + 1.0f );
	}

	void SetPropagatedSurfaceCoordinate( float const value, int const side = 0 )
	{
		surfaceCoordinate.x() = value;
		surfaceCoordinate.y() = -1.0f - static_cast<float>( side );
	}

	// Checks if strand is propagated (strands whose roots are not on mesh surface but on other strands).
	// In such case, the face index is the index of the strand on which the current strand is located, and the x surface coordinate is the position on the strand from 0 to 1.
	EPHERE_NODISCARD bool IsPropagatedStrand() const
	{
		return surfaceCoordinate.y() < -0.1f;
	}

	EPHERE_NODISCARD unsigned GetUniquePropagatedStrandAndSideId() const
	{
		// TODO: Need to find a better way of hashing in the side index
		return faceIndex + static_cast<unsigned>( GetPropagatedSideIndex() ) * 1000000;
	}

	bool operator == ( SurfacePosition const& other ) const
	{
		return faceIndex == other.faceIndex && surfaceCoordinate == other.surfaceCoordinate;
	}

	bool operator != ( SurfacePosition const& other ) const
	{
		return !( *this == other );
	}

	EPHERE_NODISCARD bool IsValid() const
	{
		return faceIndex != InvalidFaceIndex;
	}

	static SurfacePosition Invalid()
	{
		return SurfacePosition( InvalidFaceIndex, Vector2f() );
	}
};

// TODO: Move to utilities header

template <typename C>
std::basic_ostream<C>& operator << ( std::basic_ostream<C>& stream, MeshSurfacePosition const& value )
{
	stream << value.faceIndex;
	stream << ",";
	stream << value.barycentricCoordinate;
	return stream;
}

template <typename C>
std::basic_ostream<C>& operator << ( std::basic_ostream<C>& stream, SurfacePosition const& value )
{
	stream << value.faceIndex;
	stream << ",";
	stream << value.surfaceCoordinate;
	return stream;
}

inline std::vector<SurfacePosition> GetSurfacePositionsFromFloatArray( Span<float const> values )
{
	ASSERT( values.size() % 3 == 0 );
	std::vector<SurfacePosition> surfacePos;
	auto const elementCount = values.size() / 3;
	surfacePos.reserve( elementCount );
	for( auto index = 0, sourceIndex = 0; index < elementCount; ++index, sourceIndex += 3 )
	{
		surfacePos.emplace_back( static_cast<unsigned>( values[sourceIndex] ), Geometry::Vector2f( values[sourceIndex + 1], values[sourceIndex + 2] ) );
	}

	return surfacePos;
}

inline std::vector<float> SetSurfacePositionsToFloatArray( Span<SurfacePosition const> values )
{
	std::vector<float> surfacePositionsAsVector;
	surfacePositionsAsVector.reserve( values.size() * 3 );
	for( auto const& value : values )
	{
		surfacePositionsAsVector.push_back( static_cast<float>( value.faceIndex ) );
		surfacePositionsAsVector.push_back( value.surfaceCoordinate.x() );
		surfacePositionsAsVector.push_back( value.surfaceCoordinate.y() );
	}

	return surfacePositionsAsVector;
}

} }
