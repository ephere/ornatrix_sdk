#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Ground.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Ground" )
{
	SharedHair sharedHair;
	//sharedHair.SetSurfaceTangentComputeMethod( SurfaceTangentComputeMethod::FirstEdge );

	IHair& hair = sharedHair;
	HairUtilities const hairUtilities( hair );
	SharedHair originalHair;

	auto surfaceMesh = GeneratePlane<Real>( 1 );

	Ground ground;
	ground.parameters.targetHair = &hair;
	ground.parameters.distributionMesh = &surfaceMesh;

	SECTION( "DynamicGrounding" )
	{
		ground.parameters.dynamicGrounding = true;

		GenerateRandomHairs( hair, 1, 1, &surfaceMesh );

		// Evaluate object-space positions so that grounding can happen
		hair.ValidateStrandToObjectTransforms( &surfaceMesh );
		auto const verticesAfterGeneration = hair.GetVertices( IHair::CoordinateSpace::Object );
		auto const strand1SurfaceDependency = hair.GetSurfaceDependency2( 0 );

		// Clear away any surface dependency
		hair.SetUseSurfaceDependency2( false );

		REQUIRE( ground.Apply() );
		REQUIRE( hair.HasSurfaceDependency2() );
		auto const groundedStrand1Position = hair.GetSurfaceDependency2( 0 );
		REQUIRE( strand1SurfaceDependency.IsNearEqual( groundedStrand1Position ) );

		// Add another strand
		GenerateRandomHairs( hair, 1, 1, &surfaceMesh, 0, 2, -1, false, false, 1, true );
		hair.ValidateStrandToObjectTransforms( &surfaceMesh );

		// Intentionally change the surface dependency for new strand so we can re-ground it
		auto const strand2SurfaceDependency = hair.GetSurfaceDependency2( 1 );
		hair.SetSurfaceDependency2( 1, { 0u, Vector2f::Zero() } );

		REQUIRE( ground.Apply() );
		auto const groundedStrand2Position = hair.GetSurfaceDependency2( 0 );

		// The second strand should have been re-grounded
		REQUIRE( strand1SurfaceDependency.IsNearEqual( groundedStrand1Position ) );
		REQUIRE( strand2SurfaceDependency.IsNearEqual( groundedStrand2Position ) );

		// Apply again to make sure we have no crashes
		REQUIRE( ground.Apply() );
	}

	SECTION( "DynamicGroundingIncreaseStrands" )
	{
		ground.parameters.dynamicGrounding = true;

		GenerateRandomHairs( hair, 10, 1, &surfaceMesh );

		// Evaluate object-space positions so that grounding can happen
		hair.ValidateStrandToObjectTransforms( &surfaceMesh );
		MoveTipsRandomly( hair );

		auto const verticesAfterGeneration = hair.GetVertices( IHair::CoordinateSpace::Object );

		// Clear away any surface dependency
		hair.SetUseSurfaceDependency2( false );

		SharedHair subsetHair;
		HairUtilities( hair ).GetSubset( subsetHair, 0.1_r, &surfaceMesh );

		ground.parameters.targetHair = &subsetHair;
		REQUIRE( ground.Apply() );

		ground.parameters.targetHair = &hair;
		REQUIRE( ground.Apply() );
		REQUIRE( hair.ValidateStrandToObjectTransforms( &surfaceMesh ) );

		REQUIRE_THAT(
			verticesAfterGeneration,
			IsVectorNearEqual( hair.GetVertices( IHair::CoordinateSpace::Object ), 0.0001_r )
		);
	}

	SECTION( "GroundWithRotations" )
	{
		GenerateRandomHairs( hair, 1, 1, &surfaceMesh, 1 );
		hair.ValidateStrandToObjectTransforms( &surfaceMesh );
		auto generatedHair = sharedHair;

		auto const verticesAfterGeneration = hair.GetVertices( IHair::CoordinateSpace::Object );
		hair.SetUseSurfaceDependency2( false );

		originalHair.CopyFrom( hair );

		//ground.parameters.set.rotations = false;
		ground.parameters.positionBlendRamp = Ramp( 1.0f );

		// Apply once to original mesh. This will ground the strand.
		REQUIRE( ground.Apply() );

		// Move and rotate the surface mesh by some amount
		// ReSharper disable once CppRedundantCastExpression
		auto const groundMeshXform = Xform3::XYZRotation( 0.2f, 0.1f, 0.4f ) * Xform3::Translation( -Vector3::ZAxis() * 0.5_r );
		surfaceMesh.SetVertices( Transform( surfaceMesh.GetVertices(), groundMeshXform.VectorMultiplyFunction() ) );

		// Apply grounding again, this should modify the strand based on the rotated surface
		hair.CopyFrom( originalHair );
		REQUIRE( ground.Apply() );

		auto const verticesAfterGrounding = hair.GetVertices( IHair::CoordinateSpace::Object );

		// Calculate what the expected vertices are due to the changes mesh
		generatedHair.InvalidateStrandToObjectTransformsCache();
		generatedHair.ValidateStrandToObjectTransforms( &surfaceMesh );
		auto const expectedVertices = generatedHair.GetVertices( IHair::CoordinateSpace::Object );

		// Length was preserved
		REQUIRE_THAT( ( verticesAfterGeneration[1] - verticesAfterGeneration[0] ).length(), IsNearEqual( ( verticesAfterGrounding[1] - verticesAfterGrounding[0] ).length() ) );

		// Root was correctly moved and rotated
		REQUIRE_THAT( groundMeshXform * verticesAfterGeneration[0], IsNearEqual( verticesAfterGrounding[0] ) );

		// Tip was correctly moved and rotated
		REQUIRE_THAT( expectedVertices, IsVectorNearEqual( verticesAfterGrounding ) );
	}

	SECTION( "DontSetPositionsAndRotations" )
	{
		ground.parameters.set.positions = false;
		ground.parameters.set.rotations = false;

		hair.SetUseStrandToObjectTransforms( true );
		hair.SetStrandCount( 1 );
		hair.SetGlobalStrandPointCount( 2 );
		HairUtilities( hair ).SetStrandToObjectTransform( 0, Xform3::Translation( surfaceMesh.GetVertex( 0 ) ) );
		
		REQUIRE( hairUtilities.SetVertices( vector{ Vector3::Zero(), Vector3::ZAxis() } ) );

		ground.parameters.distributionMesh = &surfaceMesh;
		ground.GroundHair( hair, Xform3::Identity(), surfaceMesh, Xform3::Identity() );
		ground.Apply();

		auto const originalSurfaceDependency = hair.GetSurfaceDependency2( 0 );

		// ReSharper disable once CppRedundantCastExpression
		auto const changedSurfaceTransform = Xform3::Translation( Vector3::XAxis() * 0.1_r ) * Xform3::XRotation( 0.5_r );
		vector const expectedVertices = { changedSurfaceTransform * surfaceMesh.GetVertex( 0 ), Vector3( 0.6_r, 0.1_r, 2.3_r ) };
		REQUIRE( hairUtilities.SetVertices( expectedVertices, IHair::CoordinateSpace::Object ) );

		ground.parameters.distributionMesh = { ground.parameters.distributionMesh(), changedSurfaceTransform };
		ground.Apply();
		auto const verticesAfterApply = hair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( expectedVertices, IsVectorNearEqual( verticesAfterApply ) );
		REQUIRE( hair.GetSurfaceDependency2( 0 ).IsNearEqual( originalSurfaceDependency ) );
	}

	SECTION( "GroundToScaledMesh" )
	{
		GenerateRandomHairs( hair, 1, 1, &surfaceMesh );

		REQUIRE( hair.ValidateStrandToObjectTransforms( &surfaceMesh ) );
		auto const verticesBeforeApply = hair.GetVertices( IHair::CoordinateSpace::Object );

		// Clear away any surface dependency
		hair.SetUseSurfaceDependency2( false );

		static auto const ScaledMeshTransform = Xform3::Translation( Vector3( 0.1_r, 0, 0 ) ) * Xform3::Scale( Vector3( 0.5_r, 0.5_r, 0.5_r ) );

		ground.parameters.distributionMesh = PolygonMeshParameter( &surfaceMesh, ScaledMeshTransform );
		ground.GroundHair( hair, Xform3::Identity(), surfaceMesh, ScaledMeshTransform );
		REQUIRE( ground.Apply() );

		auto const verticesAfterApply = hair.GetVertices( IHair::CoordinateSpace::Object );

		// The length of the resulting strand should'nt have changed
		//REQUIRE_THAT( verticesBeforeApply[1].z(), IsNearEqual( verticesAfterApply[1].z() ) );

		REQUIRE_THAT( verticesBeforeApply, IsVectorNearEqual( verticesAfterApply ) );
	}
}

}
