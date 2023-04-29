#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/AdoptExternalGuides.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

void CheckGuideToHairInfluence( IHair const& hair, IHair& controlHair, AdoptExternalGuides& op )
{
	auto const verticesBeforeControlChange = hair.GetVertices( IHair::CoordinateSpace::Object );

	MoveTips( controlHair, Vector3::XAxis() );
	REQUIRE( op.Apply() );

	auto const verticesAfterControlChange = hair.GetVertices( IHair::CoordinateSpace::Object );

	REQUIRE_THAT( verticesBeforeControlChange, !IsVectorNearEqual( verticesAfterControlChange ) );
}

TEST_CASE( "AdoptExternalGuides" )
{
	SharedHair hair, referenceHair, controlHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh );

	GenerateVertexHairs( referenceHair, 1, &distributionMesh );
	controlHair.CopyFrom( referenceHair );

	AdoptExternalGuides op;
	op.parameters.objects.reference = &referenceHair;
	op.parameters.objects.control = &controlHair;
	op.parameters.targetHair = &hair;

	SECTION( "Apply" )
	{
		REQUIRE( op.Apply() );
		CheckGuideToHairInfluence( hair, controlHair, op );
	}

	SECTION( "BindingsNotRecalculatedWhenInputDoesntChange" )
	{
		// First application creates the initial bindings
		REQUIRE( op.Apply() );
		
		REQUIRE( op.bindingCalculationCount == 1 );

		// Assign different target hair object which is an identical copy of initial hair
		auto anotherHair = hair;
		op.parameters.targetHair = &anotherHair;

		// Apply again, bindings shouldn't have been re-created
		REQUIRE( op.bindingCalculationCount == 1 );
	}

	SECTION( "BindingsRecalculatedWhenInputGeometryChanges" )
	{
		// First application creates the initial bindings
		REQUIRE( op.Apply() );

		// Assign different target hair object which is an identical copy of initial hair
		auto anotherHair = hair;
		MoveTips( anotherHair, Vector3::XAxis() );
		op.parameters.targetHair = &anotherHair;
		REQUIRE( op.Apply() );

		// Apply again, bindings shouldn't have been re-created
		REQUIRE( op.bindingCalculationCount == 2 );
	}

	SECTION( "NoReferenceHair" )
	{
		op.parameters.objects.reference = nullptr;
		REQUIRE( op.Apply() );
		REQUIRE( op.bindingCalculationCount == 1 );
		CheckGuideToHairInfluence( hair, controlHair, op );
	}

	SECTION( "NoReferenceAndControlHair" )
	{
		op.parameters.objects.reference = nullptr;
		op.parameters.objects.control = nullptr;
		REQUIRE( !op.Apply() );
	}
}

}