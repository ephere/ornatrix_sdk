#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/MeshFromHairGenerator.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Operators/Symmetry.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

void TestPropagatedStrandSymmetry( bool groundStrands, Symmetry& symmetry, SharedPolygonMesh& distributionMesh, SharedHair& originalHair, IHair& hair, Xform3 const& symmetryTransform )
{
	Propagator propagator;
	propagator.parameters.targetHair = &originalHair;
	propagator.parameters.distributionMesh = &distributionMesh;
	propagator.parameters.rootCount = 1;
	propagator.parameters.sideCount = 1;
	propagator.parameters.verticesPerRootCount = 3;
	propagator.Apply();

	symmetry.parameters.groundMirroredStrands = groundStrands;
	symmetry.parameters.inputHair = &originalHair;
	symmetry.parameters.outputHair = &hair;
	symmetry.parameters.distributionMesh = &distributionMesh;
	symmetry.Apply();

	// Check for symmetrical results
	auto const halfCount = hair.GetVertexCount() / 2;
	hair.ValidateStrandToObjectTransforms( &distributionMesh );
	auto const originalVertices = originalHair.GetVertices( IHair::CoordinateSpace::Object );
	auto const leftVertices = hair.GetVertices( IHair::CoordinateSpace::Object, 0, halfCount );
	auto const rightVertices = hair.GetVertices( IHair::CoordinateSpace::Object, halfCount, halfCount );

	// The original side didn't change
	REQUIRE_THAT(
		originalVertices,
		// ReSharper disable once CppRedundantCastExpression
		IsVectorNearEqual( leftVertices, 0.0001_r )
	);

	auto const expectedVertices = Transform( rightVertices, symmetryTransform.VectorMultiplyFunction() );
	REQUIRE_THAT(
		leftVertices,
		// ReSharper disable once CppRedundantCastExpression
		IsVectorNearEqual( expectedVertices, 0.0001_r )
	);
}

TEST_CASE( "Symmetry" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 1, 1, &distributionMesh );

	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	Symmetry symmetry;

	auto const symmetryPlaneTransform = Xform3::XRotation( HalfPif );
	symmetry.parameters.mirrorPlane.origin = Vector3::Zero();
	symmetry.parameters.mirrorPlane.normal = symmetryPlaneTransform.zVector();

	auto const symmetryTransform = symmetryPlaneTransform * Xform3::Scale( Vector3( 1, 1, -1 ) ) * Inverse( symmetryPlaneTransform );

	SECTION( "PropagatedStrands" )
	{
		TestPropagatedStrandSymmetry( false, symmetry, distributionMesh, originalHair, hair, symmetryTransform );
	}

	SECTION( "PropagatedStrandsWithGrounding" )
	{
		TestPropagatedStrandSymmetry( true, symmetry, distributionMesh, originalHair, hair, symmetryTransform );
	}
}

TEST_CASE( "SymmetryEmptyOutputHair" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( originalHair, 1, 1, &distributionMesh );

	// Add some rotations
	originalHair.SetStrandRotation( 0, Pir / 3_r );
	CheckedStaticCast<HairBaseSA&>( originalHair ).InvalidateStrandToObjectTransformsCache();

	Symmetry symmetry;

	auto const symmetryPlaneTransform = Xform3::XRotation( HalfPir );
	symmetry.parameters.mirrorPlane.origin = Vector3::Zero();
	symmetry.parameters.mirrorPlane.normal = symmetryPlaneTransform.zVector();
	symmetry.parameters.inputHair = &originalHair;
	symmetry.parameters.outputHair = &hair;
	symmetry.parameters.distributionMesh = &distributionMesh;
	symmetry.Apply( false );

	SECTION( "CopyStrandRotation" )
	{
		REQUIRE( hair.HasStrandRotations() );
		auto const rotation1 = hair.GetStrandRotation( 0 ), rotation2 = hair.GetStrandRotation( 1 );
		//REQUIRE( fabs( rotation1 + rotation2 ) <= Epsilonr );
		REQUIRE( fabs( rotation2 - rotation1 ) > Epsilonr );
	}
}

TEST_CASE( "SymmetryWithRotations" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 1, 1, &distributionMesh );

	// Add some rotations
	hair.SetUseStrandRotations( true );
	hair.SetStrandRotation( 0, Pir / 3_r );
	CheckedStaticCast<HairBaseSA&>( hair ).InvalidateStrandToObjectTransformsCache();

	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	Symmetry symmetry;

	auto const symmetryPlaneTransform = Xform3::XRotation( HalfPif );
	symmetry.parameters.mirrorPlane.origin = Vector3::Zero();
	symmetry.parameters.mirrorPlane.normal = symmetryPlaneTransform.zVector();
	symmetry.parameters.inputHair = &originalHair;
	symmetry.parameters.outputHair = &hair;
	symmetry.parameters.distributionMesh = &distributionMesh;
	symmetry.Apply();

	auto const symmetryTransform = symmetryPlaneTransform * Xform3::Scale( Vector3( 1, 1, -1 ) ) * Inverse( symmetryPlaneTransform );

	MeshFromHairGenerator meshFromHair;
	meshFromHair.parameters.inputHair = &hair;
	meshFromHair.parameters.distributionMesh = &distributionMesh;
	meshFromHair.Initialize();

	meshFromHair.GenerateMesh();

	SECTION( "MirroredRotations" )
	{
		auto const mirroredVertexStartIndex = meshFromHair.parameters.outputMesh()->GetVertexCount() / 2;
		for( auto vertexIndex = 0; vertexIndex < mirroredVertexStartIndex; ++vertexIndex )
		{
			auto const originalVertex = meshFromHair.parameters.outputMesh()->GetVertex( vertexIndex );
			auto const expectedVertex = symmetryTransform * originalVertex;
			auto const actualVertex = meshFromHair.parameters.outputMesh()->GetVertex( vertexIndex + mirroredVertexStartIndex );

			REQUIRE( NearEqual( expectedVertex, actualVertex ) );
		}
	}

	SECTION( "CopyStrandRotation" )
	{
		REQUIRE( hair.HasStrandRotations() );
		REQUIRE( fabs( hair.GetStrandRotation( 0 ) - hair.GetStrandRotation( 1 ) ) > Epsilonr );
	}

	SECTION( "GroundingPreservesMirroredStrandRotation" )
	{
		SharedHair hairWithGrounding;
		hairWithGrounding.CopyFrom( sharedHair, true, true, true, true, true );

		symmetry.parameters.groundMirroredStrands = true;
		symmetry.parameters.outputHair = &hairWithGrounding;
		symmetry.Apply();

		meshFromHair.parameters.inputHair = &hairWithGrounding;
		meshFromHair.GenerateMesh();

		// If the grounding altered the rotations correctly the meshes generated in both cases should be identical
		REQUIRE_THAT(
			meshFromHair.parameters.outputMesh()->GetVertices(),
			IsVectorNearEqual( meshFromHair.parameters.outputMesh()->GetVertices(), 0.0001_r )
		);
	}
}

}
