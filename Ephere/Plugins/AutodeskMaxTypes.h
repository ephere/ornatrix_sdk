// Must compile with VC 2012 / GCC 4.8

#pragma once

#ifdef AUTODESK_3DSMAX

#pragma warning( push, 2 )

// Needed by older Max headers
#include <algorithm>

// Max headers include Windows.h
#define NOMINMAX

#include <max.h>
#include <simpshp.h>
#include <spline3d.h>
#include <surf_api.h>

#include <array>
#include <memory>
#include <tuple>
#include <vector>

#pragma warning( pop )

#include "Ephere/NativeTools/Asserts.h"

namespace Ephere { namespace Plugins
{
	namespace Autodesk
	{
		namespace Max
		{
			class PolygonMeshMax;
		}
	}

	struct MaxTriangleMeshAccess
{
	MaxTriangleMeshAccess() :
		mesh_( nullptr ),
		isReadOnly( false ),
		isEditingTopology( false ),
		isEditingGeometry( false ),
		isEditingNormals( false ),
		faceVertexCount( 3 )
	{
	}

	explicit MaxTriangleMeshAccess( MNMesh* mesh ) :
		mesh_( std::make_shared<MNMesh>( *mesh ) ),
		isReadOnly( false ),
		isEditingTopology( false ),
		isEditingGeometry( false ),
		isEditingNormals( false ),
		faceVertexCount( 3 )
	{
	}

	explicit MaxTriangleMeshAccess( std::shared_ptr<MNMesh> const& meshSharedPointer ) :
		mesh_( meshSharedPointer ),
		isReadOnly( false ),
		isEditingTopology( false ),
		isEditingGeometry( false ),
		isEditingNormals( false ),
		faceVertexCount( 3 )
	{
	}

	MNMesh* Get() const
	{
		//return hasMeshWeakPointer_ && meshWeakPointer_.expired() ? static_cast<MNMesh*>( nullptr ) : mesh_;
		return mesh_.get();
	}

	MNMesh& operator*() const
	{
		ASSERT( *this != nullptr );
		return *mesh_;
	}

	MNMesh* operator->() const
	{
		ASSERT( *this != nullptr );
		return mesh_.get();
	}

	bool operator==( std::nullptr_t ) const
	{
		return mesh_ == nullptr;
	}

	bool operator!=( std::nullptr_t ) const
	{
		return mesh_ != nullptr;
	}

	void Reset()
	{
		mesh_.reset();
		trianglePolygonIndices_.clear();
	}

	std::vector<std::tuple<int, int, std::array<int, 3>>> const& GetTrianglePolygonIndices() const
	{
		ValidateTriangleIndices();
		return trianglePolygonIndices_;
	}

	std::vector<int> const& GetPolygonStartingTriangleIndices() const
	{
		ValidateTriangleIndices();
		return polygonStartingTriangleIndices_;
	}

private:

	// Warning! The order of members must not be changed because clients depend on it
	std::shared_ptr<MNMesh> mesh_;

public:
	bool isReadOnly;
	bool isEditingTopology;
	bool isEditingGeometry;
	bool isEditingNormals;

	//! This is set by user in SetFaceCount and is used to alter multiple triangles at the same time in subsequent calls
	int faceVertexCount;

private:

	void ValidateTriangleIndices() const;

	//! If triangulated mesh is requested
	mutable std::vector<std::tuple<int, int, std::array<int, 3>>> trianglePolygonIndices_;
	mutable std::vector<int> polygonStartingTriangleIndices_;
};

struct MaxCurveWrapper
{
	MaxCurveWrapper()
		: nurbsCurve( nullptr ),
		spline( nullptr ),
		simpleShape( nullptr ),
		curveIndex( -1 )
	{
	}

	explicit MaxCurveWrapper( NURBSCurve& obj )
		: nurbsCurve( &obj ),
		spline( nullptr ),
		simpleShape( nullptr ),
		curveIndex( -1 )
	{
	}

	explicit MaxCurveWrapper( Spline3D& obj )
		: nurbsCurve( nullptr ),
		spline( &obj ),
		simpleShape( nullptr ),
		curveIndex( -1 )
	{
	}

	explicit MaxCurveWrapper( SimpleShape& obj, int const index )
		: nurbsCurve( nullptr ),
		spline( nullptr ),
		simpleShape( &obj ),
		curveIndex( index )
	{
	}

	static bool FromMaxObject( Object&, TimeValue, std::vector<MaxCurveWrapper>& result, std::vector<std::shared_ptr<NURBSSet>>& nurbsSets );

	NURBSCurve* nurbsCurve;
	Spline3D* spline;
	SimpleShape* simpleShape;
	int curveIndex;

	Point3 Evaluate( float positionAlongCurve, bool useNormalizedSpace = false ) const;

private:

	static bool GetSplineShapeCurves( Object& obj, TimeValue t, std::vector<MaxCurveWrapper>& result );

	//! This is used internally
	std::shared_ptr<Object> ownedObject_;

};

typedef Point2 HostVector2;
typedef Point3 HostVector3;
typedef UVVert HostTextureCoordinate;
typedef Matrix3 HostXform3;
typedef MNMesh HostTriangleMesh;
typedef MNMesh HostPolygonMesh;
typedef MNFace HostTriangleFace;
typedef TVFace HostTriangleTextureFace;
typedef Box3 HostAxisAlignedBox3;
typedef ::Ray HostRay;
typedef BitArray HostElementSelection;
typedef Texmap HostTextureMap;
typedef void* HostImage; // Used in Maya so far has/needs no 3dsmax equivalent
typedef void* HostMeshIntersector; // Used in Maya only
typedef MaxTriangleMeshAccess HostTriangleMeshAccessHandle;
typedef MaxCurveWrapper HostCurve;
typedef Autodesk::Max::PolygonMeshMax HostIPolygonMesh;

inline void Invert( HostElementSelection& selection )
{
	selection = ~selection;
}

inline HostXform3 HostInverse( HostXform3 const& xform )
{
	return Inverse( xform );
}

inline HostVector3 HostTransform( HostXform3 const& xform, HostVector3 const& point )
{
	return xform * point;
}

inline HostAxisAlignedBox3 HostTransform( HostXform3 const& xform, HostAxisAlignedBox3 const& box )
{
	return box * xform;
}

inline HostVector3 HostBoundingBoxMin( HostAxisAlignedBox3 const& box )
{
	return box.Min();
}

inline HostVector3 HostBoundingBoxMax( HostAxisAlignedBox3 const& box )
{
	return box.Max();
}

inline void HostBoundingBoxExpand( HostAxisAlignedBox3& box, HostAxisAlignedBox3 const& other )
{
	box += other;
}

} }

template <typename C>
std::basic_ostream<C>& operator <<( std::basic_ostream<C>& stream, Point3 const& vector )
{
	stream << '[' << ' ' << vector.x << ' ' << vector.y << ' ' << vector.z << ' ' << ']';
	return stream;
}

#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Plugins { namespace Autodesk { namespace Max { namespace Ornatrix
{

struct MaxTriangleMesh final : Ephere::Ornatrix::ITriangleMesh
{
	MaxTriangleMesh( MaxTriangleMeshAccess const& mesh )
		: meshAccess( mesh )
	{
	}

	void PrepareVertexColorSet( int colorSetIndex ) override;

	int GetVertexColorSetCount() const override;

	Ephere::Ornatrix::Vector3 GetVertexColor( int faceIndex, Ephere::Ornatrix::Vector3 const& barycentricCoordinate, int colorSetIndex ) const override;


	MaxTriangleMeshAccess meshAccess;
};

} } } } }

#endif
