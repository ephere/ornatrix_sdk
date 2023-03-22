// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppVariableCanBeMadeConstexpr
// ReSharper disable CppClangTidyModernizeUseEqualsDefault
#pragma once

#include "Ephere/Core/Parameters/Array.h"
#include "Ephere/Geometry/Native/Interpolation.h"
#include "Ephere/NativeTools/Span.h"
#include "Ephere/NativeTools/StlExtensions.h"
#include "Ephere/Ornatrix/IFunction.h"

namespace Ephere { namespace Ornatrix
{
class IProfileCurve;

//! An implementation of IFunction1 using a vector of control points that get interpolated between
class Ramp final : public IFunction1
{
public:

	static int const CountToken = 100;

	typedef Geometry::ControlPoint<Real> ControlPoint;

	Ramp()
	{
	}

	explicit Ramp( Real flatValue )
	{
		std::array<ControlPoint, 3> knots = { ControlPoint( 0, flatValue ), ControlPoint( Real( 0.6f ), flatValue ), ControlPoint( 1, flatValue ) };
		knots_ = knots;
		Validate();
	}

	Ramp( std::initializer_list<ControlPoint> points )
	{
		knots_ = points;
		Validate();
	}

	// TODO: Redundant, either derive from IProfileCurve or remove it
	explicit Ramp( IProfileCurve const& profileCurve );

	explicit Ramp( Parameters::Array<ControlPoint> knots )
	{
		SetKnots( std::move( knots ) );
	}

	explicit Ramp( Span<Real const> source )
	{
		Load( source );
	}

	explicit Ramp( std::string_view );

	static Ramp IncreasingLine()
	{
		Ramp result;
		result.SetLine( true );
		return result;
	}

	static Ramp DecreasingLine()
	{
		Ramp result;
		result.SetLine( false );
		return result;
	}

	friend bool operator==( Ramp const& a, Ramp const& b )
	{
		return a.knots_ == b.knots_;
	}

	friend bool operator!=( Ramp const& a, Ramp const& b )
	{
		return !( a == b );
	}

	EPHERE_NODISCARD bool IsNearEqual( Ramp const& other, Real epsilon = Epsilonr ) const
	{
		if( knots_.size() != other.knots_.size() )
		{
			return false;
		}

		for( auto index = 0; index < Size( knots_ ); ++index )
		{
			if( !knots_[index].IsNearEqual( other.knots_[index], epsilon ) )
			{
				return false;
			}
		}

		return true;
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return knots_.empty();
	}

	/** Evaluate function at a specified point.
		@param coordinate Value in range {0,1}
		@return Evaluated value at specified position
		*/
	EPHERE_NODISCARD Real Evaluate( Real coordinate ) const override
	{
		return !IsEmpty() ? Geometry::EvaluateAsProfileCurve( knots_, coordinate ) : 1;
	}

	EPHERE_NODISCARD Parameters::Array<ControlPoint> const& GetKnots() const
	{
		return knots_;
	}

	void SetKnots( Parameters::Array<ControlPoint> knots )
	{
		if( knots.empty() )
		{
			*this = Ramp( 1.0f );
			return;
		}

		knots_ = std::move( knots );
		Validate();
	}

	static bool PosCompareFunc( ControlPoint const& a, ControlPoint const& b )
	{
		// In case both knots are co-located, prioritize based on index
		if( abs( a.position - b.position ) < std::numeric_limits<Real>::min() )
		{
			return a.index < b.index;
		}

		return a.position < b.position;
	}

	// Serialization

	EPHERE_NODISCARD std::string ToString() const;

	//! Copies the data of this object into a Real array
	EPHERE_NODISCARD std::vector<Real> ToFloatVector() const;

	/*! Loads the data of this object from a Real array.
	Supports both the old format that doesn't store the Interpolation type and assumes it to be Spline, and the new format which does store it.
	Returns the number of floats that were read from source.
	*/
	int Load( Span<Real const> source );

	static std::vector<Real> ToFloatVector( Span<Ramp const> ramps );

	static std::vector<Ramp> FromFloatVector( Span<Real const> data );

private:

	friend class CurveDiagramData;

	static bool GetInterpolationFromCode( std::string_view name, Geometry::Interpolation& result );

	static char const* GetInterpolationCode( Geometry::Interpolation interpolation );

	void SetLine( bool increasing = false )
	{
		std::array<ControlPoint, 3> knots =
		{
			ControlPoint( 0.0f, increasing ? 0.0f : 1.0f ),
			ControlPoint( 0.6f, 0.4f ),
			ControlPoint( 1.0f, increasing ? 1.0f : 0.0f ),
		};

		knots_ = knots;

		Validate();
	}

	void Validate()
	{
		std::sort( knots_.begin(), knots_.end(), PosCompareFunc );
		auto knotIndex = 0;
		for( auto& knot : knots_ )
		{
			knot.index = knotIndex++;
		}
	}


	//! Value points that define the function
	Parameters::Array<ControlPoint> knots_;
};

inline std::ostream& operator<<( std::ostream& stream, Ramp const& ramp )
{
	auto const& knots = ramp.GetKnots();
	stream << '[' << ' ';
	for( auto const& knot : knots )
	{
		stream << knot << ' ';
	}

	stream << ']';
	return stream;
}

} }
