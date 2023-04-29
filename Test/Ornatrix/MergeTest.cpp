#include "PrecompiledHeaders.h"

#include "Utilities.h"

#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Merger.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Merge" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );

	Merger merger;
	merger.parameters.targetHair = &hair;
	merger.parameters.distributionMesh = &distributionMesh;

	SECTION( "MergeWithDifferentTransforms" )
	{
		GenerateHairs( hair, array { SurfacePosition { 0u, Vector2f::Zero() } }, 1, &distributionMesh, 0, 2 );

		merger.parameters.mergedStrands.PushBack( HairParameter( make_shared<SharedHair>( sharedHair ), Xform3::XRotation( -HalfPif ) ) );
		merger.Apply();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		auto const vertices = hair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE( NearEqual( ( vertices[1] - vertices[0] ).normalize(), Vector3::ZAxis() ) );
		REQUIRE( NearEqual( ( vertices[3] - vertices[2] ).normalize(), Vector3::YAxis() ) );
	}

	SECTION( "IncreaseSurfacePolygonCount" )
	{
		GenerateHairs( hair, array { SurfacePosition{ 0u, Vector2f::Zero() } }, 1, &distributionMesh, 0, 2 );

		// Generate hair on a mesh which has 4 faces (as opposed to original mesh which has 1 face)
		auto const meshWithMorePolygons = GeneratePlane<Real>( 1, 3, 3 );
		SharedHair mergedHair;
		GenerateHairs( mergedHair, array { SurfacePosition{ 3u, { 0.5f, 0.5f } } }, 1, &meshWithMorePolygons, 0, 2 );
		mergedHair.ValidateStrandToObjectTransforms( &meshWithMorePolygons );
		auto const mergedHairVertices = mergedHair.GetVertices( IHair::Object );
		auto const mergedTransform = HairUtilities( mergedHair ).GetStrandToObjectTransform( 0 );

		mergedHair.SetCoordinateSpace( IHair::CoordinateSpace::Strand );
		merger.parameters.mergedStrands.PushBack( HairParameter( make_shared<SharedHair>( mergedHair ), Xform3::Identity() ) );
		merger.Apply();		
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		// The merged in hair will reference an invalid face, but that should not matter and we should get the original vertices as a result
		auto const vertices = hair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE( NearEqual( vertices[2], mergedHairVertices[0] ) );
		REQUIRE( NearEqual( vertices[3], mergedHairVertices[1] ) );
		REQUIRE( NearEqual( HairUtilities( hair ).GetStrandToObjectTransform( 1 ), mergedTransform ) );
	}

	SECTION( "IncreaseSurfacePolygonCountPreservePositionsForGrounding" )
	{
		// Generate hair on a mesh which has 4 faces (as opposed to original mesh which has 1 face)
		auto const meshWithMorePolygons = GeneratePlane<Real>( 1, 2, 3 );
		SharedHair mergedHair;
		GenerateVertexHairs( mergedHair, 1, &meshWithMorePolygons, 0, 2 );
		mergedHair.ValidateStrandToObjectTransforms( &meshWithMorePolygons );
		auto const mergedHairVertices = mergedHair.GetVertices( IHair::Object );
		auto const mergedTransforms = HairUtilities( mergedHair ).GetStrandToObjectTransforms();

		merger.parameters.mergedStrands.PushBack( HairParameter( make_shared<SharedHair>( mergedHair ), Xform3::Identity() ) );
		merger.Apply();

		// The merged vertices and transforms should not have been touched during merging, even though the resulting hair has some invalid surface dependencies with the new mesh
		auto const vertices = hair.GetVertices( IHair::CoordinateSpace::Object );
		auto const transforms = HairUtilities( hair ).GetStrandToObjectTransforms();
		REQUIRE_THAT( mergedHairVertices, IsVectorNearEqual( vertices, 0.0001_r ) );
		REQUIRE_THAT( mergedTransforms, IsVectorNearEqual( transforms, 0.0001_r ) );
	}
}
}
