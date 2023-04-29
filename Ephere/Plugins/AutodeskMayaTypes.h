// Must compile with VC 2012 / GCC 4.8

#pragma once

#ifdef AUTODESK_MAYA

#if defined( _MSC_VER ) && !defined( __clang__ )
#	pragma warning( push, 3 )
#else
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wold-style-cast"
#	pragma clang diagnostic ignored "-Wduplicate-enum"
#	pragma clang diagnostic ignored "-Wreserved-id-macro"
#	pragma clang diagnostic ignored "-Wdocumentation"
#	pragma clang diagnostic ignored "-Wdeprecated"
#	pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#	pragma clang diagnostic ignored "-Wextra-semi"
#endif

// Needed by MTypes.h for intptr_t on Linux
#include <cstdint>

#include <maya/MBoundingBox.h>
#include <maya/MImage.h>
#include <maya/MMatrix.h>
#include <maya/MMeshIntersector.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>

#if defined( _MSC_VER ) && !defined( __clang__ )
#	pragma warning( pop )
#else
#	pragma GCC diagnostic pop
#endif

#include <vector>

#if !defined(_MSC_VER) || _MSC_VER > 1600
#include <mutex>
#endif

namespace Ephere { namespace Plugins
{

namespace Autodesk { namespace Maya { namespace Ornatrix
{
class TextureAccess;
class TriangleMeshAccess;
} } }

struct MayaMap
{
	MayaMap()
		: firstEvaluationDone( false ),
		missingFile( -1 )
	{
	}

	explicit MayaMap( MPlug const& plug )
		: mapPlug( plug ),
		firstEvaluationDone( false ),
		missingFile( -1 )
	{
	}

	MayaMap( MayaMap const& other )
		: mapPlug( other.mapPlug ),
		firstEvaluationDone( other.firstEvaluationDone ),
		missingFile( other.missingFile )
	{
	}

	MayaMap& operator=( MayaMap const& other )
	{
		mapPlug = other.mapPlug;
		firstEvaluationDone = other.firstEvaluationDone;
		missingFile = other.missingFile;
		return *this;
	}


	MPlug mapPlug;

	mutable std::mutex blocker;

	// The first evaluation of the map must happen in isolation, without concurrency.
	mutable bool firstEvaluationDone;

	// < 0 = unknown, 0 = no, > 0 = yes
	mutable char missingFile;
};

typedef MPoint HostVector2;					// actually 4
typedef MPoint HostVector3;					// actually 4
typedef MPoint HostTextureCoordinate;		// actually 4
typedef MMatrix HostXform3;					// actually 4x4
typedef MObject HostTriangleMesh;				// MFn::Type = kMesh?
typedef MObject HostPolygonMesh;
typedef MObject HostCurve;					// MFn::Type = kNurbsCurve
typedef MIntArray HostTriangleFace;			// 3 indices of triangle vertices
typedef MIntArray HostTriangleTextureFace;	// 3 indices of triangle vertices
typedef MBoundingBox HostAxisAlignedBox3;
typedef std::vector<bool> HostElementSelection;
typedef MayaMap HostTextureMap;
typedef MImage HostImage;
typedef MObject HostRay; // TODO
typedef Ephere::Plugins::Autodesk::Maya::Ornatrix::TextureAccess* HostTextureAccessHandle;
//typedef Ephere::Plugins::Autodesk::Maya::Ornatrix::TriangleMeshAccess* HostTriangleMeshAccessHandle;
typedef std::shared_ptr<Ephere::Plugins::Autodesk::Maya::Ornatrix::TriangleMeshAccess> HostTriangleMeshAccessHandle;
typedef MMeshIntersector* HostMeshIntersector;

inline void Invert( HostElementSelection& selection )
{
	selection.flip();
}

inline HostXform3 HostInverse( HostXform3 const& xform )
{
	return xform.inverse();
}

inline HostVector3 HostTransform( HostXform3 const& xform, HostVector3 const& point )
{
	return point * xform;
}

inline HostAxisAlignedBox3 HostTransform( HostXform3 const& xform, HostAxisAlignedBox3 const& box )
{
	HostAxisAlignedBox3 result( box );
	result.transformUsing( xform );
	return result;
}

inline HostVector3 HostBoundingBoxMin( HostAxisAlignedBox3 const& box )
{
	return box.min();
}

inline HostVector3 HostBoundingBoxMax( HostAxisAlignedBox3 const& box )
{
	return box.max();
}

inline void HostBoundingBoxExpand( HostAxisAlignedBox3& box, HostAxisAlignedBox3 const& other )
{
	box.expand( other );
}

} }

#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Plugins { namespace Autodesk { namespace Maya { namespace Ornatrix
{

struct MayaTriangleMesh final : Ephere::Ornatrix::ITriangleMesh
{
	explicit MayaTriangleMesh( std::shared_ptr<TriangleMeshAccess> const& meshAccess )
		: meshAccess( meshAccess )
	{
	}

	void PrepareVertexColorSet( int colorSetIndex ) override;

	int GetVertexColorSetCount() const override;

	Ephere::Ornatrix::Vector3 GetVertexColor( int faceIndex, Ephere::Ornatrix::Vector3 const& barycentricCoordinate, int colorSetIndex ) const override;


	std::shared_ptr<TriangleMeshAccess> meshAccess;
};

} } } } }

#ifdef MAYA_VERSION
#	include "Ephere/NativeTools/Types.h"
EPHERE_DECLARE_TYPE_SIZE( Ephere::Plugins::HostVector3, 32 )
#endif

#endif
