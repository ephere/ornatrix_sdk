// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
#pragma once

#include "Ephere/Geometry/Native/Box.h"
#include "Ephere/Geometry/Native/Matrix.h"
#include "Ephere/NativeTools/Mathematics.h"

//#define DOUBLE_PRECISION

namespace Ephere { namespace Geometry
{
template <unsigned N, typename T>
class Ray;

template<unsigned N, typename T>
class NearestNeighbourFinderCommon;

template<typename T>
class IPolygonMeshT;

template<typename T>
class PolygonMeshUtilitiesT;

template<typename T>
struct PolygonMeshQueryContextT;

template<typename T>
class SharedPolygonMeshT;

template<typename T>
struct ICurvesT;

template<typename T>
class SharedNurbsCurvesT;

template<typename T>
class ISphere;
} }

namespace Ephere { namespace Ornatrix
{
#ifdef DOUBLE_PRECISION
typedef double Real;
#else
typedef float Real;
#endif

typedef Geometry::Matrix<2, 1, Real> Vector2;
typedef Geometry::Matrix<3, 1, Real> Vector3;
typedef Geometry::Matrix<4, 1, Real> Vector4;
typedef Geometry::Matrix<3, 4, Real> Xform3;
typedef Geometry::Matrix<2, 2, Real> Matrix2;
typedef Geometry::Matrix<3, 3, Real> Matrix3;
typedef Geometry::Matrix<4, 4, Real> Matrix4;

typedef Vector3 TextureCoordinate;

typedef Geometry::Box<2, Real> Box2;
typedef Geometry::Box<3, Real> Box3;

typedef Geometry::Ray<3, Real> Ray3;

typedef unsigned StrandId;

typedef Geometry::NearestNeighbourFinderCommon<3, Real> NearestNeighbourFinder3;
typedef NearestNeighbourFinder3 NearestNeighbourFinder;

typedef Geometry::IPolygonMeshT<Real> IPolygonMeshSA;
typedef Geometry::ISphere<Real> ISphere;
typedef Geometry::SharedPolygonMeshT<Real> SharedPolygonMesh;
typedef Geometry::PolygonMeshUtilitiesT<Real> PolygonMeshUtilities;
typedef Geometry::PolygonMeshQueryContextT<Real> PolygonMeshQueryContext;

typedef Geometry::ICurvesT<Real> ICurves;
typedef Geometry::SharedNurbsCurvesT<Real> SharedNurbsCurves;

#if defined( EPHERE_HAVE_CPP11 )
static constexpr StrandId InvalidStrandId = static_cast<StrandId>( -1 );
static constexpr auto Pir = static_cast<Real>( Pi );
static constexpr auto TwoPir = static_cast<Real>( TwoPi );
static constexpr auto HalfPir = static_cast<Real>( HalfPi );
static constexpr auto Epsilonr = static_cast<Real>( Geometry::Epsilon );

constexpr Real operator "" _r( long double x )
{
	return static_cast<Real>( x );
}

constexpr Real operator "" _r( unsigned long long int x )
{
	return static_cast<Real>( x );
}
#else
StrandId const InvalidStrandId = static_cast<StrandId>( -1 );
double const Pir = static_cast<Real>( Pi );
double const TwoPir = static_cast<Real>( TwoPi );
double const HalfPir = static_cast<Real>( HalfPi );
double const Epsilonr = static_cast<Real>( Geometry::Epsilon );
#endif


struct ITriangleMesh
{
	virtual void PrepareVertexColorSet( int colorSetIndex ) = 0;

	EPHERE_NODISCARD virtual int GetVertexColorSetCount() const = 0;

	EPHERE_NODISCARD virtual Vector3 GetVertexColor( int faceIndex, Vector3 const& barycentricCoordinate, int colorSetIndex ) const = 0;

protected:
	virtual ~ITriangleMesh()
	{
	}
};

class IHair1;
class IHair2;
class IHair3;
typedef IHair3 IHair;

} }

// For compatibility with the old code. TODO: Remove
namespace Ephere { namespace Plugins { namespace Ornatrix
{

using namespace Ephere::Ornatrix;

} } }
