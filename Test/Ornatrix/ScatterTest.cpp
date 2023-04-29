#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Operators/Scatter.h"
#include "Ephere/Ornatrix/Operators/Symmetry.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Scatter" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;

	const auto inputStrandCount = 4;
	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, inputStrandCount, 1, &distributionMesh );

	SharedHair referenceHair;
	GenerateRandomHairs( referenceHair, 1, 1, &distributionMesh );

	Propagator propagator;
	propagator.parameters.targetHair = &referenceHair;
	propagator.parameters.distributionMesh = &distributionMesh;
	propagator.parameters.rootCount = 3;

	propagator.Apply();

	SharedHair resultHair;
	resultHair.SetUseSurfaceDependency( true );

	Scatter scatter;
	scatter.parameters.inputHair = &hair;
	scatter.parameters.outputHair = &resultHair;
	scatter.parameters.distributionMesh = &distributionMesh;
	scatter.parameters.referenceHairs = { &referenceHair };

	SECTION( "PropagationScatter" )
	{
		REQUIRE( scatter.Apply() );
		REQUIRE( resultHair.GetStrandCount() == referenceHair.GetStrandCount() * hair.GetStrandCount() );

		REQUIRE( resultHair.ValidateStrandToObjectTransforms( &distributionMesh ) );
	}

	SECTION( "InputWidth" )
	{
		scatter.parameters.targetWidthMethod = ScatterParameters::TargetWidthMethodType::InputWidth;
		REQUIRE( scatter.Apply() );

		// TODO: Check results
	}

	SECTION( "WithSymmetry" )
	{
		// NOLINT(performance-unnecessary-copy-initialization)
		const auto hairCopy = sharedHair;

		Symmetry symmetry;
		symmetry.parameters.inputHair = &hairCopy;
		symmetry.parameters.outputHair = &hair;
		symmetry.parameters.distributionMesh = &distributionMesh;
		symmetry.Apply();

		REQUIRE( scatter.Apply() );
		REQUIRE( resultHair.ValidateStrandToObjectTransforms( &distributionMesh ) );
	}

#if false
	SECTION( "RandomizeReference" )
	{
		scatter.parameters.randomizeReference = true;
		REQUIRE( scatter.Apply() );

		REQUIRE( resultHair.ValidateStrandToObjectTransforms( &distributionMesh ) );
		auto resultVerticesStrand1 = resultHair.GetVertices( IHair::CoordinateSpace::Object, 0, resultHair.GetVertexCount() / inputStrandCount );
		Transform( resultVerticesStrand1, resultVerticesStrand1, [firstPoint = resultVerticesStrand1[0]]( const auto& vertex ) { return vertex - firstPoint; } );

		for( auto resultStrandIndex = 1; resultStrandIndex < inputStrandCount; ++resultStrandIndex )
		{
			auto resultVerticesStrand = resultHair.GetVertices( IHair::CoordinateSpace::Object, resultStrandIndex * resultHair.GetVertexCount(), resultHair.GetVertexCount() / inputStrandCount );
			Transform( resultVerticesStrand, resultVerticesStrand, [firstPoint = resultVerticesStrand[0]]( const auto& vertex ) { return vertex - firstPoint; } );

			REQUIRE_THAT( resultVerticesStrand1, !IsVectorNearEqual( resultVerticesStrand ) );
		}
	}
#endif
}
}
