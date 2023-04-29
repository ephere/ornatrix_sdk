#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/StrandDataGenerator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "StrandDataGenerator" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateVertexHairs( hair, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	StrandDataGenerator op;
	op.parameters.targetHair = &hair;

#ifdef SEEXPR_ENABLED
	SECTION( "StrandChannelToGroupsWithFilter" )
	{
		hair.SetUsesStrandGroups( true );
		hair.SetStrandGroups( vector { 1, 2, 3, 4 } );

		hair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 1 );
		HairUtilities( hair ).SetStrandChannelData( vector<Real> { 5, 6, 7, 8 } );

		op.parameters.strandGroup.pattern = "2,4";
		op.parameters.generationMethod = StrandDataGeneratorParameters::GenerationMethodType::SeExpr;
		op.parameters.seExprString = string( "sc_" ) + ToAscii( HairUtilities( hair ).GetStrandChannelName() );
		op.parameters.sourceValueRange.enabled = false;
		op.parameters.targetValueRange.enabled = false;
		op.parameters.targetData = StrandDataGeneratorParameters::TargetDataType::StrandGroups;
		
		op.Evaluate();

		REQUIRE( hair.GetStrandGroups() == vector<int> { 1, 6, 3, 8 } );
	}
#endif

	SECTION( "StrandGroupsOuptutIsEven" )
	{
		GenerateRandomHairs( hair, 10, 1, &distributionMesh );

		op.parameters.generationMethod = StrandDataGeneratorParameters::GenerationMethodType::Random;
		op.parameters.targetValueRange.enabled = true;
		op.parameters.targetValueRange.minimum = 1;
		op.parameters.targetValueRange.maximum = 3;
		op.parameters.targetData = StrandDataGeneratorParameters::TargetDataType::StrandGroups;

		op.Evaluate();

		auto const strandGroups = hair.GetStrandGroups();

		REQUIRE( CountIf( strandGroups, []( auto const groupId ) { return groupId == 1; } ) > 2 );
		REQUIRE( CountIf( strandGroups, []( auto const groupId ) { return groupId == 2; } ) > 2 );
		REQUIRE( CountIf( strandGroups, []( auto const groupId ) { return groupId == 3; } ) > 2 );
	}

	// #6181
	SECTION( "SeexprInexistentChannel" )
	{
		GenerateRandomHairs( hair, 10, 1, &distributionMesh );

		op.parameters.generationMethod = StrandDataGeneratorParameters::GenerationMethodType::SeExpr;
		op.parameters.seExprString = "sc_Nonexistent";
		op.parameters.targetData = StrandDataGeneratorParameters::TargetDataType::StrandGroups;

		op.Evaluate();

		// If no assertions happened we are good
	}
}
}
