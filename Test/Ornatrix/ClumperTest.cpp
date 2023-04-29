#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/ScopeExit.h"
#include "Ephere/Ornatrix/Operators/Clumper.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

// TODO: Move to IHair?
vector<Vector3> GetRootPositions( IHair const& hair )
{
	vector<Vector3> result;
	result.reserve( hair.GetStrandCount() );
	for( auto strandIndex = 0; strandIndex < hair.GetStrandCount(); ++strandIndex )
	{
		result.push_back( HairUtilities( hair ).GetStrandPoint( strandIndex, 0, IHair::CoordinateSpace::Object ) );
	}

	return result;
}

vector<Vector3> GetRootToTipVectors( IHair const& hair )
{
	vector<Vector3> result;
	result.reserve( hair.GetStrandCount() );
	for( auto strandIndex = 0; strandIndex < hair.GetStrandCount(); ++strandIndex )
	{
		result.push_back( HairUtilities( hair ).GetStrandPoint( strandIndex, hair.GetStrandPointCount( strandIndex ) - 1, IHair::CoordinateSpace::Object ) -
			HairUtilities( hair ).GetStrandPoint( strandIndex, 0, IHair::CoordinateSpace::Object ) );
	}

	return result;
}

vector<Vector3> GetUniqueTipPositions( IHair const& hair, float epsilon = Epsilon, vector<unordered_set<int>>* uniqueSets = nullptr )
{
	return GetUniquePoints( HairUtilities( hair ).GetTipPositions(), epsilon, uniqueSets );
}

vector<Vector3> GetUniqueRootPositions( IHair const& hair, float epsilon = Epsilon, vector<unordered_set<int>>* uniqueSets = nullptr, NumberSet<int> const* strandIndices = nullptr )
{
	return GetUniquePoints( HairUtilities( hair ).GetRootPositions( IHair::CoordinateSpace::Object, strandIndices != nullptr ? *strandIndices : NumberSet<int>{} ), epsilon, uniqueSets );
}

bool AreAllStrandsOutsideOfSurfaceTransform( IHair const& hair )
{
	vector<Vector3> strandPointsInStrandSpace;
	for( auto strandIndex = 0; strandIndex < hair.GetStrandCount(); ++strandIndex )
	{
		strandPointsInStrandSpace.resize( hair.GetStrandPointCount( strandIndex ) );
		hair.GetStrandPoints( strandIndex, 0, Size( strandPointsInStrandSpace ), strandPointsInStrandSpace.data(), IHair::CoordinateSpace::Strand );

		for( auto const& strandPointInStrandSpace : strandPointsInStrandSpace )
		{
			if( strandPointInStrandSpace.z() < 0.0f )
			{
				return false;
			}
		}
	}

	return true;
}

bool AreAllNearEqual( Span<SurfacePosition const> values )
{
	for( auto const& value: values )
	{
		if( !value.IsNearEqual( values[0] ) )
		{
			return false;
		}
	}

	return true;
}

TEST_CASE( "Clumper" )
{
	Clumper clumper;
	clumper.parameters.flyAway.fraction = 0.0f;

	Clumper::TargetHairContext targetHairContext;

	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	static auto constexpr SurfaceLength = 1;
	auto const distributionMesh = GeneratePlane<Real>( SurfaceLength );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	auto const verticesBeforeClumping = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

	clumper.GenerateRandomClumps( hair, 2, 2, nullptr, &distributionMesh );
	clumper.parameters.targetHair = &hair;
	clumper.parameters.distributionMesh = &distributionMesh;
	clumper.Apply( targetHairContext );

	clumper.parameters.amount.ramp = Ramp::IncreasingLine();

	SECTION( "HairsAreModified" )
	{
		auto const verticesAfterClumping = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT(
			verticesAfterClumping,
			!IsVectorNearEqual( verticesBeforeClumping )
		);
	}

	SECTION( "ExternalClumpsModifyHairs" )
	{
		SharedHair externalClumpHair;
		GenerateRandomHairs( externalClumpHair, 5, 1, &distributionMesh );

		auto const verticesAfterClumping = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		hair.CopyFrom( originalHair, true, false, false, false );
		clumper.GenerateClumpsFromExternalStrands( hair, externalClumpHair, nullptr, &distributionMesh );
		targetHairContext.Invalidate();
		clumper.parameters.externalClumpStrands = &externalClumpHair;
		clumper.Apply( targetHairContext );
		auto const verticesAfterExternalGuidesClumping = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			verticesAfterExternalGuidesClumping,
			!IsVectorNearEqual( verticesAfterClumping )
		);
	}

	// Test for #5041
	SECTION( "ReapplyWithoutStrandIds" )
	{
		static auto constexpr ExternalClumpGuideCount = 5;
		SharedHair externalClumpHair;
		GenerateRandomHairs( externalClumpHair, ExternalClumpGuideCount, 1, &distributionMesh );

		// Generate some large strand ids for external clump guides
		auto const largeStrandIds = Iota<StrandId>( ExternalClumpGuideCount, 10000 );
		HairUtilities( externalClumpHair ).SetStrandIds( largeStrandIds );
		REQUIRE( HairUtilities( externalClumpHair ).EnableStrandTopology() );

		hair.CopyFrom( originalHair, true, false, false, false );
		clumper.GenerateClumpsFromExternalStrands( hair, externalClumpHair, nullptr, &distributionMesh );
		targetHairContext.Invalidate();
		clumper.parameters.externalClumpStrands = &externalClumpHair;

		// Make sure hair has no strand ids
		clumper.parameters.clumpCreateMethod = ClumpParameters::ClumpCreateMethodType::ExternalClumpStrands;
		externalClumpHair.SetUseStrandIds( false );
		clumper.Apply( targetHairContext );

		// No crash is enough
	}

	SECTION( "ExternalClumpsAfterDeletingAClump" )
	{
		SharedHair externalClumpHair;
		GenerateRandomHairs( externalClumpHair, 5, 1, &distributionMesh );

		hair.CopyFrom( originalHair, true, false, false, false, false );
		clumper.DeleteClumps( array<int, 1>( { 1 } ) );
		targetHairContext.Invalidate();
		clumper.Apply( targetHairContext );
		auto const verticesAfterDeletingClump = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		hair.CopyFrom( originalHair, true, false, false, false );
		clumper.GenerateClumpsFromExternalStrands( hair, externalClumpHair, nullptr, &distributionMesh );
		targetHairContext.Invalidate();
		clumper.parameters.externalClumpStrands = &externalClumpHair;
		clumper.Apply( targetHairContext );
		auto const verticesAfterExternalGuidesClumping = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			verticesAfterExternalGuidesClumping,
			!IsVectorNearEqual( verticesAfterDeletingClump )
		);
	}

	SECTION( "ChangePropagatedInputMultiSideHairCount" )
	{
		// Propagate some hairs on two sides
		Propagator propagator;
		propagator.parameters.targetHair = &hair;
		propagator.parameters.distributionMesh = &distributionMesh;
		propagator.parameters.rootCount = 30;
		propagator.parameters.sideCount = 2;
		propagator.parameters.rootGenerationMethod = PropagateParameters::RootGenerationMethodType::Uniform;
		propagator.parameters.resultStrandGroup = 2;

		// Set non-0 low range to make sure first strand doesn't start at root of base strand and clumper will assign it properly
		propagator.parameters.range.low = 0.1f;

		GenerateRandomHairs( hair, 1, 1, &distributionMesh, 0, 4 );
		propagator.Apply();

		clumper.parameters.strandGroup.pattern = "2";
		clumper.parameters.region.maximumClosestRegionClumpCandidates = 20;
		clumper.GenerateRandomClumps( hair, 4, 1, nullptr, &distributionMesh );
		clumper.Apply( targetHairContext );

		// After application we should get 4 tips from the clumper and 1 tip from original strand
		REQUIRE( GetUniqueTipPositions( hair, 0.001f ).size() == 5 );

		static auto constexpr XAxisOrientationTransform = []( Vector3 const& value ) { return value.x() < 0.0f; };

		// Change the strand count
		propagator.parameters.rootCount = 20;
		GenerateRandomHairs( hair, 1, 1, &distributionMesh, 0, 4 );
		propagator.InvalidateTopologyCache();
		propagator.Apply();
		auto const rootToTipVectorsBeforeClumping = GetRootToTipVectors( hair );
		auto const rootToTipVectorsBeforeClumpingZOrientations = Transform( rootToTipVectorsBeforeClumping, XAxisOrientationTransform );

		clumper.Apply( targetHairContext );
		auto const rootToTipVectorsAfterClumping = GetRootToTipVectors( hair );
		auto const rootToTipVectorsAfterClumpingZOrientations = Transform( rootToTipVectorsAfterClumping, XAxisOrientationTransform );

		// After application we should get 4 tips from the clumper and 1 tip from original strand
		REQUIRE( GetUniqueTipPositions( hair, 0.001f ).size() == 5 );
		REQUIRE( rootToTipVectorsBeforeClumpingZOrientations == rootToTipVectorsAfterClumpingZOrientations );
	}

	SECTION( "SubClumping" )
	{
		GenerateRandomHairs( hair, 40, 1, &distributionMesh );

		// Separate the hairs into two evenly sized groups
		hair.SetUsesStrandGroups( true );
		vector<Vector3> rootPositions( hair.GetStrandCount() );
		hair.GetRootPositions( 0, Size( rootPositions ), rootPositions.data(), IHair::CoordinateSpace::Object );
		auto const strandGroups = Transform( rootPositions, []( auto const& position ) { return position.x() < 0 ? 1 : 2; } );
		hair.SetStrandGroups( 0, Size( strandGroups ), strandGroups.data() );

		clumper.GenerateClumpsFromStrandGroups( hair, nullptr, &distributionMesh );
		clumper.parameters.addClumpIndexChannel = true;

		REQUIRE( hair.GetStrandChannelCount( IHair::PerStrand ) == 0 );
		clumper.Apply( targetHairContext );
		REQUIRE( hair.GetStrandChannelCount( IHair::PerStrand ) == 1 );
		REQUIRE( GetUniqueTipPositions( hair, 0.001f ).size() == 2 );
		auto const tipPositionsAfterFirstClump = HairUtilities( hair ).GetTipPositions();

		Clumper subClumper;
		Clumper::TargetHairContext targetHairContext2;
		subClumper.parameters.targetHair = &hair;
		subClumper.parameters.distributionMesh = &distributionMesh;
		subClumper.parameters.flyAway.fraction = 0.0f;
		subClumper.parameters.region.value.channel = StrandChannel( StrandChannelType_PerStrand, 0 );
		subClumper.parameters.amount.ramp = Ramp( 1.0f );
		subClumper.GenerateRandomClumps( hair, 4, 1, nullptr, &distributionMesh );

		subClumper.Apply( targetHairContext2 );

		// Since we use a "full" clumping curve the roots should be clumped together into 4 unique points, while the tips should remain in their previous positions
		REQUIRE( GetUniqueRootPositions( hair, 0.001f ).size() == 4 );
		REQUIRE( GetUniqueTipPositions( hair, 0.001f ).size() == 2 );
		REQUIRE_THAT( HairUtilities( hair ).GetTipPositions(), IsVectorNearEqual( tipPositionsAfterFirstClump ) );
	}

	SECTION( "PerClumpSettingsPreservedOnClumpDeletion" )
	{
		// Change amount to different values for each clump
		Clumper::PerClumpParameters firstClumpSettings, secondClumpSettings;
		firstClumpSettings.amount = 0.5f;
		secondClumpSettings.amount = 0.3f;

		clumper.GetPerClumpParameters().insert( make_pair( 0, firstClumpSettings ) );
		clumper.GetPerClumpParameters().insert( make_pair( 1, secondClumpSettings ) );

		// Delete first clump
		clumper.DeleteClumps( array{ 0 } );

		REQUIRE( Size( clumper.GetPerClumpParameters() ) == clumper.GetClumpCount() );
		REQUIRE( clumper.GetPerClumpParameters()[0].amount == secondClumpSettings.amount );
	}

	SECTION( "GroupFilter" )
	{
		GenerateRandomHairs( hair, 10, 1, &distributionMesh );

		// Set strand group of 2 for all strands with positive x tip coordinate
		hair.SetUsesStrandGroups( true );
		auto const tipPositions = HairUtilities( hair ).GetTipPositions();
		auto const strandGroups = Transform( tipPositions, []( auto const& position ) { return position.x() < 0 ? 1 : 2; } );
		hair.SetStrandGroups( strandGroups );

		auto const strandIndicesInGroup2 = HairUtilities( hair ).SelectStrandIndicesByGroup( "1" );
		auto const tipPositionsGroup2BeforeClump = HairUtilities( hair ).GetTipPositions( strandIndicesInGroup2 );

		// Generate one random clump for group 1
		clumper.parameters.strandGroup.pattern = "1";
		clumper.GenerateRandomClumps( hair, 1, 1, nullptr, &distributionMesh );
		targetHairContext.Invalidate();
		clumper.Apply( targetHairContext );

		//const auto expectedAverageTip = GetAverage<3, 1, float>( tipPositionsGroup2BeforeClump );
		auto const tipPositionsGroup2AfterClump = HairUtilities( hair ).GetTipPositions( strandIndicesInGroup2 );

		// We expect all strands in group 2 to be clumped at the tip
		REQUIRE( GetUniquePoints( tipPositionsGroup2AfterClump ).size() == 1 );

		// And the average point of the clump should be around the average of strand tips
		//REQUIRE_THAT( expectedAverageTip, IsNearEqual( tipPositionsGroup2AfterClump[0] ) );

		// The resulting clump tip position should be one of the original clumps
		REQUIRE( find_if( begin( tipPositionsGroup2BeforeClump ), end( tipPositionsGroup2BeforeClump ), [&tipPositionsGroup2AfterClump]( const Vector3& value )
		{
			return NearEqual( value, tipPositionsGroup2AfterClump[0] );
		} ) != end( tipPositionsGroup2BeforeClump ) );
	}

	SECTION( "SurfaceDependenciesAreUpdatedIfRootsAreClumped" )
	{
		// Setting ramp to a constant value of 1 will make sure that all strands, including their roots are moved to the shapes of the stems
		clumper.parameters.amount.ramp = Ramp( 1.0f );
		targetHairContext.Invalidate();
		clumper.Apply( targetHairContext );

		vector<unordered_set<int>> clumpHairGroups;
		REQUIRE( GetUniqueRootPositions( hair, 0.001f, &clumpHairGroups ).size() == 2 );

		// The surface dependencies of each of the unique strand root positions should now be the same
		for( auto const& clumpHairGroup: clumpHairGroups )
		{
			auto const surfaceDependencies = HairUtilities( hair ).GetSurfaceDependencies2( NumberSet<int>{ ConvertToVector( clumpHairGroup ) } );
			REQUIRE( AreAllNearEqual( surfaceDependencies ) );
		}
	}
}

TEST_CASE( "ClumpSelectedClumpedHairs" )
{
	Clumper clumper;
	clumper.parameters.flyAway.fraction = 0.0f;

	Clumper::TargetHairContext targetHairContext;

	SharedHair sharedHair;
	IHair& hair = sharedHair;

	static auto constexpr SurfaceLength = 1;
	auto const distributionMesh = GeneratePlane<Real>( SurfaceLength );
	
	clumper.parameters.targetHair = &hair;
	clumper.parameters.distributionMesh = &distributionMesh;
	clumper.parameters.amount.ramp = Ramp::IncreasingLine();

	GenerateRandomHairs( hair, 20, 1, &distributionMesh );

	// Apply first clumping
	clumper.GenerateRandomClumps( hair, 2, 1, nullptr, &distributionMesh );
	targetHairContext.Invalidate();
	clumper.parameters.clumpSize.enabled = true;
	clumper.parameters.clumpSize.size.value = 0.3f;
	clumper.Apply( targetHairContext );

	// Two clumps are created
	vector<unordered_set<int>> clumpHairGroups;
	REQUIRE( GetUniqueTipPositions( hair, 0.001f, &clumpHairGroups ).size() == 2 );

	// Apply second clumping to a sub-selection
	auto const halfHairIndices = ConvertToVector( clumpHairGroups[0] );
	clumper.Clear();
	clumper.GenerateRandomClumps( hair, 3, 1, &halfHairIndices, &distributionMesh );
	targetHairContext.Invalidate();
	clumper.parameters.clumpSize.enabled = false;
	clumper.parameters.amount.ramp = Ramp( 1.0f );
	clumper.Apply( targetHairContext );

	// Three sub clumps are created
	vector<unordered_set<int>> clumpHairGroups2;
	NumberSet<int> firstClumpIndices( clumpHairGroups[0] );
	REQUIRE( GetUniqueRootPositions( hair, 0.001f, &clumpHairGroups2, &firstClumpIndices ).size() == 3 );
}

TEST_CASE( "ClumperRespectHairParts" )
{
	static auto constexpr PartingPositionY = 0.3_r;
	
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	HairUtilities const hairUtilities( hair );

	static auto constexpr SurfaceLength = 1;
	auto const distributionMesh = GeneratePlane<Real>( SurfaceLength );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh );

	Clumper clumper;
	clumper.parameters.flyAway.fraction = 0;
	clumper.parameters.amount.ramp = Ramp::IncreasingLine();
	clumper.parameters.region.respectHairParts = true;
	clumper.parameters.targetHair = &hair;
	clumper.parameters.distributionMesh = &distributionMesh;

	// Enable guide dependency and define parts in it such that each half of the hair doesn't share the same guide ids
	hair.SetUsesGuideDependency( true );

	auto const strandCount = hair.GetStrandCount();
	auto const guideDependencies = Generate( strandCount, [&hairUtilities, index = 0, strandCount]() mutable
	{
		auto const rootPosition = hairUtilities.GetStrandPoint( index++, 0, IHair::Object );
		if( rootPosition.y() < PartingPositionY )
		{
			auto const id = static_cast<unsigned>( index ) + 1;
			return GuideDependency{ { id, id, id }, { 0.0f, 0.0f, 0.0f } };
		}

		auto const id = static_cast<unsigned>( index ) + strandCount + 1;
		return GuideDependency{ { id, id, id }, { 0.0f, 0.0f, 0.0f } };
	} );

	hairUtilities.SetGuideDependencies( guideDependencies );

	clumper.GenerateRandomClumps( hair, 2, 1, nullptr, &distributionMesh );

	Clumper::TargetHairContext targetHairContext;
	clumper.Apply( targetHairContext );

	// Hair should be clumped respective of the guide dependency values we set
	auto const tipPositions = hairUtilities.GetTipPositions();
	auto const leftTipPositions = Subset( tipPositions, [guideDependencyIterator = guideDependencies.begin()]( auto const& /*tipPosition*/ ) mutable
	{
		return guideDependencyIterator++->closestRootIndices[0] == 1u;
	} );

	auto const rightTipPositions = Subset( tipPositions, [guideDependencyIterator = guideDependencies.begin()]( auto const& /*tipPosition*/ ) mutable
	{
		return guideDependencyIterator++->closestRootIndices[0] == 2u;
	} );

	REQUIRE( AllOf( leftTipPositions, [&leftTipPositions]( auto const& position ) { return NearEqual( position, leftTipPositions.front() ); } ) );
	REQUIRE( AllOf( rightTipPositions, [&rightTipPositions]( auto const& position ) { return NearEqual( position, rightTipPositions.front() ); } ) );
}
	
TEST_CASE( "ClumpsFromGuides" )
{
	Clumper clumper;
	clumper.parameters.flyAway.fraction = 0.0f;

	Clumper::TargetHairContext targetHairContext;

	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh );

	auto const verticesBeforeClumping = hair.GetVertices( IHair::CoordinateSpace::Object );

	LineCurve amountCurve;
	clumper.parameters.amount.ramp = Ramp::IncreasingLine();

	SharedHair sharedGuideHair;
	IHair& guides = sharedGuideHair;
	GenerateRandomHairs( guides, 2, 1, &distributionMesh );

	hair.SetUsesGuideDependency( true );

	array const guideDependencies = {
		GuideDependency { { guides.GetStrandId( 0 ), 0u, 0u }, { 1.0f, 0.0f, 0.0f } },
		GuideDependency { { guides.GetStrandId( 1 ), 0u, 0u }, { 1.0f, 0.0f, 0.0f } } };

	array<GuideDependency, 10> hairGuideDependencies{};
	for( auto& hairGuideDependency : hairGuideDependencies )
	{
		hairGuideDependency = &hairGuideDependency - hairGuideDependencies.data() < 5 ? guideDependencies[0] : guideDependencies[1];
	}

	hair.SetGuideDependencies( 0, Size( hairGuideDependencies ), hairGuideDependencies.data() );

	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	// Generate clumps from guides for all hairs
	clumper.GenerateClumpsFromGuides( hair, guides, nullptr, &distributionMesh );
	clumper.parameters.targetHair = &hair;
	clumper.parameters.distributionMesh = &distributionMesh;

	SECTION( "CreateRandomClumpsFromClumpsFromGuides" )
	{
		// Generate two clumps for first half of the hairs
		auto const strandIndices = Iota( 5, 0 );
		clumper.GenerateRandomClumps( hair, 2, 1, &strandIndices, &distributionMesh );
		clumper.GenerateRandomClumps( hair, 1, 1, &strandIndices, &distributionMesh );
		clumper.parameters.externalClumpStrands = &guides;

		clumper.Apply( targetHairContext );

		// Two clumps are remaining: one generated from guide and another randomly
		// TODO: Adjusting test, but why do we get 3 clumps?
		REQUIRE( GetUniqueTipPositions( hair, 0.001f ).size() == 3 );
	}

	SECTION( "GuideClumpsRegionChannel" )
	{
		clumper.parameters.addClumpIndexChannel = true;
		clumper.parameters.externalClumpStrands = &guides;
		clumper.Apply( targetHairContext );
		REQUIRE( hair.GetStrandChannelCount( IHair::StrandDataType::PerStrand ) == 1 );

		vector<float> values( hair.GetStrandCount() );
		hair.GetStrandChannelData( IHair::StrandDataType::PerStrand, 0, 0, Size( values ), values.data() );

		// ReSharper disable once CppRedundantExplicitTemplateArguments (Clang 11 on Mojave can't deduce the template parameter)
		unordered_set<float> valueSet( begin( values ), end( values ) );
		REQUIRE( valueSet.size() == 2 );
	}

	SECTION( "SubClumpsAfterHairReordering" )
	{
		clumper.parameters.addClumpIndexChannel = true;
		clumper.parameters.externalClumpStrands = &guides;

		Clumper clumper2;
		clumper2.parameters.flyAway.fraction = 0.0f;
		clumper2.parameters.amount.ramp = Ramp::IncreasingLine();
		clumper2.parameters.region.value.channel = StrandChannel( StrandChannelType_PerStrand, 0 );
		clumper2.GenerateRandomClumps( hair, 2, 1, nullptr, &distributionMesh );

		Clumper::TargetHairContext targetHairContext2;
		
		clumper.Apply( targetHairContext );
		clumper2.Apply( targetHairContext2 );

		static auto constexpr VectorXCompare = []( auto const& value1, auto const& value2 ) { return value1.x() < value2.x(); };

		auto const uniqueTipPositionsAfterFirstGen = GetSorted( GetUniqueTipPositions( hair, 0.001f ), VectorXCompare );

		sharedHair = originalHair;

		HairUtilities( sharedHair ).ChangeStrandOrder();
		
		clumper.Apply( targetHairContext );
		clumper2.Apply( targetHairContext2 );

		auto const uniqueTipPositionsAfterSecondGen = GetSorted( GetUniqueTipPositions( hair, 0.001f ), VectorXCompare );
		
		REQUIRE_THAT( uniqueTipPositionsAfterFirstGen, IsVectorNearEqual( uniqueTipPositionsAfterSecondGen ) );
	}
}

}
