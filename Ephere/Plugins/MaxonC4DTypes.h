// Must compile with VC 2012 / GCC 4.8

#pragma once

#ifdef MAXON_C4D

#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Ornatrix/Types.h"
#include "Ephere/Plugins/Maxon/C4D/BaseShaderWrapper.h"
#include "Ephere/Plugins/Maxon/C4D/HostVector.h"

#ifdef _MSC_VER
#	pragma warning( push )
#	pragma warning( disable : 4100 )
#endif
#include "c4d.h"
#ifdef _MSC_VER
#	pragma warning( pop )
#endif

#include <memory>
#include <sstream>
#include <vector>

namespace Ephere::Plugins::Maxon::C4D
{
// From C4DUtilities.h
Vector FlipUvwCoordinate( Vector const& uvwCoordinate );
BaseObject* GetBaseLinkObject( BaseDocument* doc, BaseLink const* baseLink );
std::string C4DRenderResultToString( INITRENDERRESULT type );

namespace Ornatrix
{
	class PolygonMeshC4D;
	class TriangleMeshAccess;

// From OxC4DUtilities.h
BaseTag* GetDistributionMeshTag( BaseTag* originalTag, PolygonMeshC4D const* meshAccessHandle );
}
}

namespace Ephere::Plugins
{

using Maxon::C4D::Ornatrix::TriangleMeshAccess;

class PolygonObjectWrapper
{
public:

	PolygonObjectWrapper( bool isGenerator = false )
		: isGenerator_( isGenerator )
	{
		Make( nullptr, nullptr );
	}

	PolygonObjectWrapper( PolygonObject* polygon, BaseObject* originalObject, Matrix const xform = Matrix() )
		: xform_( xform )
	{
		Make( polygon, originalObject );
	}

	PolygonObjectWrapper( PolygonObjectWrapper const& src )
	{
		Set( src );
	}

	void Set( PolygonObjectWrapper const& src )
	{
		polygon_ = src.GetPolygonObjectSharedPointer();
		baseLink_ = src.GetBaseLinkSharedPointer();
		mesh_ = src.GetMeshSharedPointer();
		isGenerator_ = src.IsGenerator();
	}

	void Reset()
	{
		polygon_.reset();
		baseLink_.reset();
		mesh_.reset();
	}

	std::shared_ptr<PolygonObject> const& GetPolygonObjectSharedPointer() const
	{
		return polygon_;
	}

	PolygonObject* GetPolygonObject() const
	{
		return polygon_.get();
	}

	std::shared_ptr<BaseLink> const& GetBaseLinkSharedPointer() const
	{
		return baseLink_;
	}

	BaseLink* GetBaseLink() const
	{
		return baseLink_.get();
	}

	bool HasOriginalObject() const
	{
		return baseLink_ != nullptr;
	}

	std::shared_ptr<Ornatrix::SharedPolygonMesh> const& GetMeshSharedPointer() const
	{
		return mesh_;
	}

	Ornatrix::SharedPolygonMesh* GetMesh() const
	{
		return mesh_.get();
	}

	void SetTriangleMeshAccess( std::shared_ptr<TriangleMeshAccess> triangleMeshAccess ) const
	{
		triangleMeshAccess_ = std::move( triangleMeshAccess );
	}

	TriangleMeshAccess* GetTriangleMeshAccess() const
	{
		return triangleMeshAccess_.get();
	}

	Matrix64 GetMatrix( BaseDocument* doc ) const
	{
		return GetMatrix( doc, false );
	}

	Matrix64 GetLocalMatrix( BaseDocument* doc ) const
	{
		return GetMatrix( doc, true );
	}

	Matrix GetXformMatrix() const
	{
		return xform_;
	}

	bool IsGenerator() const
	{
		return isGenerator_;
	}

private:

	void Make( PolygonObject* polygon, BaseObject* originalObject )
	{
		if( isGenerator_ )
		{
			// Create empty PolygonObject
			polygon_ = std::shared_ptr<PolygonObject>( PolygonObject::Alloc( 0, 0 ), []( PolygonObject* polygon )
			{
				PolygonObject::Free( polygon );
			} );
		}
		else if( polygon != nullptr )
		{
			// Retain PolygonObject
			polygon_ = std::shared_ptr<PolygonObject>( polygon, []( PolygonObject* polygon )
			{
				PolygonObject::Free( polygon );
			} );

			// Save reference to original object in a BaseLink
			if( originalObject != nullptr )
			{
				baseLink_ = std::shared_ptr<BaseLink>( static_cast<BaseLink*>( BaseLink::Alloc() ), []( BaseLink* baseLink )
				{
					BaseLink::Free( baseLink );
				} );

				baseLink_->SetLink( originalObject );
			}
		}

		mesh_ = std::make_shared<Ornatrix::SharedPolygonMesh>();
	}

	Matrix64 GetMatrix( BaseDocument* doc, bool local ) const
	{
		Matrix64 result;
		if( baseLink_ != nullptr )
		{
			auto const linkedObject = Maxon::C4D::GetBaseLinkObject( doc, baseLink_.get() );
			if( linkedObject != nullptr )
			{
				result = local ? linkedObject->GetMl() : linkedObject->GetMg();
				return result;
			}
		}
		if( polygon_ != nullptr )
		{
			result = local ? polygon_->GetMl() : polygon_->GetMg();
		}
		return result;
	}

	// A single cached polygon
	std::shared_ptr<PolygonObject> polygon_;

	// BaseLink to the source object
	// May be different than the polygon (like a primitive or selection tag)
	std::shared_ptr<BaseLink> baseLink_;

	// The parsed TriangleMesh
	std::shared_ptr<Ornatrix::SharedPolygonMesh> mesh_;

	// Created by HostTriangleMeshAccess()
	mutable std::shared_ptr<TriangleMeshAccess> triangleMeshAccess_;

	// Transform applied to all points on parsing
	Matrix xform_;

	bool isGenerator_ = false;
};

class BaseBitmapWrapper
{
public:

	BaseBitmapWrapper()
	{
	}

	BaseBitmapWrapper( BaseBitmapWrapper const& source )
	{
		source.GetBitmap().CopyTo( bitmap_ );
	}

	BaseBitmapWrapper& operator=( BaseBitmapWrapper const& source )
	{
		source.GetBitmap().CopyTo( bitmap_ );
		return *this;
	}

	BaseBitmap const& GetBitmap() const
	{
		return bitmap_;
	}

	BaseBitmap& GetBitmap()
	{
		return bitmap_;
	}

private:

	AutoAlloc<BaseBitmap> bitmap_;
};


struct C4DCurveWrapper
{
	C4DCurveWrapper()
		: splineObject_( nullptr ),
		splineInstance_( nullptr ),
		segmentIndex_( static_cast<unsigned>( -1 ) )
	{
	}

	explicit C4DCurveWrapper( SplineObject& obj, Matrix objectToWorldTransform, unsigned segmentIndex = 0 )
		: splineObject_( &obj ),
		splineInstance_( nullptr ),
		startTransform_( ~objectToWorldTransform * ~obj.GetMg()* objectToWorldTransform ),
		segmentIndex_( segmentIndex )
	{
	}

	explicit C4DCurveWrapper( std::shared_ptr<SplineObject> instance, Matrix objectToWorldTransform, unsigned segmentIndex = 0 )
		: splineObject_( instance.get() ),
		splineInstance_( instance ),
		startTransform_( ~objectToWorldTransform * ~instance->GetMg()* objectToWorldTransform ),
		segmentIndex_( segmentIndex )
	{
	}

	static bool FromC4DObject( BaseObject* obj, std::vector<C4DCurveWrapper>& result, Matrix const& objectToWorldTransform );

	static bool FromDeformedSpline( BaseObject* obj, std::vector<C4DCurveWrapper>& result, Matrix const& objectToWorldTransform );

	static bool FromNewSpline( SplineObject* splineObject, std::vector<C4DCurveWrapper>& result, Matrix const& objectToWorldTransform );

	Maxon::C4D::HostVector Evaluate( float positionAlongCurve, bool useNormalizedSpace = false ) const;

	SplineObject* splineObject_;
	std::shared_ptr<SplineObject> splineInstance_;
	Matrix startTransform_;
	unsigned segmentIndex_;
};

typedef Vector2d64 HostVector2;
typedef Maxon::C4D::HostVector HostVector3;
typedef Maxon::C4D::HostVector HostTextureCoordinate;
typedef Matrix64 HostXform3;
typedef Ornatrix::Box3 HostAxisAlignedBox3;
typedef PolygonObjectWrapper HostTriangleMesh;
typedef PolygonObjectWrapper HostPolygonMesh;
typedef C4DCurveWrapper HostCurve;
typedef CPolygon HostTriangleFace;
typedef CPolygon HostTriangleTextureFace;
typedef Maxon::C4D::BaseShaderWrapper HostTextureMap;
typedef std::vector<bool> HostElementSelection;
typedef BaseBitmapWrapper HostImage;
typedef void* HostRay; // TODO
typedef void* HostTextureAccessHandle;	// TODO
typedef std::shared_ptr<TriangleMeshAccess> HostTriangleMeshAccessHandle;
typedef void* HostMeshIntersector; // Used in Maya only


inline void Invert( HostElementSelection& selection )
{
	selection.flip();
}

inline HostXform3 HostInverse( HostXform3 const& xform )
{
	return ~xform;
}

inline HostVector3 HostTransform( HostXform3 const& xform, HostVector3 const& point )
{
	auto const result = xform * point;
	return HostVector3( result );
}

Ornatrix::Xform3 GetCommonValue( HostXform3 const& hostValue );
inline HostAxisAlignedBox3 HostTransform( HostXform3 const& xform, HostAxisAlignedBox3 const& box )
{
	auto const common = GetCommonValue( xform );
	return common * box;
}

inline HostVector3 HostBoundingBoxMin( HostAxisAlignedBox3 const& box )
{
	auto const result = box.pmin();
	return HostVector3( result );
}

inline HostVector3 HostBoundingBoxMax( HostAxisAlignedBox3 const& box )
{
	auto const result = box.pmax();
	return HostVector3( result );
}

inline void HostBoundingBoxExpand( HostAxisAlignedBox3& box, HostAxisAlignedBox3 const& other )
{
	box += other;
}

inline HostVector3 CrossProd( HostVector3 const& left, HostVector3 const& right )
{
	auto const result = Cross( left, right );
	return HostVector3( result );
}

inline float Length( HostVector3 const& vector )
{
	return static_cast<float>( vector.GetLength() );
}

}

#ifdef C4D_VERSION
#include "Ephere/NativeTools/Types.h"
EPHERE_DECLARE_TYPE_SIZE( Ephere::Plugins::HostVector3, 24 )
#endif

#elif defined(MAXON_TARGET_OSX) || defined(MAXON_TARGET_WINDOWS)
#error "Need to define MAXON_C4D"
#endif
