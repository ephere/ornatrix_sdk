// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedCopyWithUserProvidedDtor
#pragma once

#include "Ephere/Geometry/Native/Matrix.h"

namespace Ephere { namespace Geometry
{

template <typename T>
struct ICurvesT
{
	typedef Matrix<3, 1, T> Vector;
	typedef Matrix<4, 1, T> VectorH;
	typedef Matrix<3, 4, T> Xform;

	virtual ~ICurvesT()
	{
	}

	EPHERE_NODISCARD virtual int GetSplineCount() const = 0;

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return GetSplineCount() == 0;
	}

	EPHERE_NODISCARD virtual Xform GetSplineStartTransform( int splineIndex ) const = 0;

	/** Evaluates a point on a parametric curve
		@param splineIndex Index of the curve if there are multiple curves
		@param param The coordinate at which to evaluate the point
		@param useNormalizedSpace When true force the coordinate along the curve to be normalized to the curve length
	*/
	EPHERE_NODISCARD virtual Vector EvaluateSpline( int splineIndex, T param, bool useNormalizedSpace = false ) const = 0;

	/** Evaluates tangent on a parametric curve
		@param splineIndex Index of the curve if there are multiple curves
		@param param The coordinate at which to evaluate the point
		@param useNormalizedSpace When true force the coordinate along the curve to be normalized to the curve length
	*/
	EPHERE_NODISCARD virtual Vector EvaluateSplineTangent( int splineIndex, T param, bool useNormalizedSpace = false ) const = 0;

	//! Get first the degree and the controlPointCount (D and N), on the second call provide controlPoints and/or knots of at least N and N+D+1 lengths
	virtual bool GetNurbsCurve( int splineIndex, int* degree = nullptr, int* controlPointCount = nullptr, Span<VectorH> controlPoints = Span<VectorH>(), Span<T> knots = Span<T>() ) const = 0;
};


template <typename T>
bool IsNurbs( ICurvesT<T> const& curves )
{
	for( auto index = curves.GetSplineCount() - 1; index >= 0; --index )
	{
		if( !curves.GetNurbsCurve( index ) )
		{
			return false;
		}
	}

	return true;
}

} }
