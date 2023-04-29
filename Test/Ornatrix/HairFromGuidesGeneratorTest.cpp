#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshGenerator.h"
#include "Ephere/Ornatrix/Operators/HairFromGuidesGenerator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Geometry/Native/Image/PixelGray.h"
#include "Ornatrix/Private/TextureMapImage.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

void SetRandomRotations( IHair& hair, IPolygonMeshSA const* distributionMesh = nullptr, int seed = 1 )
{
	hair.SetUseStrandRotations( true );

	auto const strandCount = hair.GetStrandCount();
	vector<float> rotationAngles;
	rotationAngles.reserve( strandCount );

	UniformRandomGenerator randomGenerator( seed );
	auto const randomDistribution = randomGenerator.CreateUniformFloatDistribution( -Pif, Pif );
	for( auto index = 0; index < strandCount; ++index )
	{
		rotationAngles.push_back( randomDistribution.GetNextValue() );
	}

	hair.SetCoordinateSpace( IHair::CoordinateSpace::Object );
	hair.SetStrandRotations( 0, Size( rotationAngles ), rotationAngles.data() );
	CheckedStaticCast<HairBaseSA&>( hair ).InvalidateStrandToObjectTransformsCache();
	hair.ValidateStrandToObjectTransforms( distributionMesh, false );
}

TEST_CASE( "HairFromGuidesGenerator" )
{
	SharedHair sharedGuides;
	IHair& guides = sharedGuides;

	auto distributionMesh = GeneratePlane<Real>( 1, 2, 2, true );

	SharedHair sharedHair, sharedCacheHair;
	IHair& hair = sharedHair;

	HairFromGuidesGenerator generator;
	generator.rootParameters.distributionMesh = &distributionMesh;
	generator.parameters.generatedStrandFraction = 1;
	generator.parameters.inputGuides = &guides;
	generator.parameters.outputHair = &hair;
	generator.SetCacheHair( &sharedCacheHair );

	hair.SetUseSurfaceDependency2( true );
	hair.SetUsesGuideDependency( true );

	// Make sure we're using newer algorithm
	generator.SetChangedGuides( true, nullptr, 0, 0, 0 );

	SECTION( "AutoPartAndTextureChannels" )
	{
		GenerateRandomHairs( guides, 5, 1, &distributionMesh, 0, 2, -1, true );

		generator.parameters.parting.automatic.enabled = false;
		generator.GenRoots( false );
		generator.UpdateState();

		// Hair has texture coordinates by default
		REQUIRE( hair.GetTextureCoordinateChannelCount() > 0 );

		// Clear hair
		hair.SetStrandCount( 0 );
		hair.SetVertexCount( 0 );
		hair.SetTextureCoordinateChannelCount( 0 );
		sharedCacheHair.SetStrandCount( 0 );
		sharedCacheHair.SetVertexCount( 0 );
		sharedCacheHair.SetTextureCoordinateChannelCount( 0 );

		generator.parameters.parting.automatic.enabled = true;
		generator.GenRoots( false );
		generator.UpdateState();

		REQUIRE( hair.GetTextureCoordinateChannelCount() > 0 );
	}

	SECTION( "GenerateRotations" )
	{
		hair.SetUseStrandToObjectTransforms( true );
		hair.SetUsesGuideDependency( true );

		GenerateTriangle<Real>( distributionMesh, 1, true );

		guides.SetUseSurfaceDependency( true );
		guides.SetUsesStrandTopology( true );
		guides.SetUseStrandToObjectTransforms( true );

		GuidesFromMeshGenerator guidesFromMesh;
		guidesFromMesh.rootParameters.distributionMesh = &distributionMesh;
		guidesFromMesh.rootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
		guidesFromMesh.parameters.guideLength = 1.0f;
		guidesFromMesh.parameters.outputGuides = &guides;
		REQUIRE( guidesFromMesh.GenRoots() );
		REQUIRE( guidesFromMesh.UpdateState() );

		// Generate one hair at face center
		generator.parameters.generateRotations = true;
		generator.rootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::FaceCenter;

		REQUIRE( generator.GenRoots( false ) );
		generator.UpdateState();

		// By default we don't have a rotation because guides don't have rotations
		REQUIRE( !hair.HasStrandRotations() );

		// Set guide strand rotations
		SetRandomRotations( guides );

		REQUIRE( generator.GenRoots( false ) );
		generator.UpdateState();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		auto const guideDependency = hair.GetGuideDependency( 0 );

		auto const hairVector = HairUtilities( hair ).GetStrandToObjectTransform( 0 ).yVector();
		auto const guidesVector = ( HairUtilities( guides ).GetStrandToObjectTransform( static_cast<int>( guideDependency.closestRootIndices[0] ) ).yVector() * static_cast<Real>( guideDependency.closestRootDistances[0] ) +
			HairUtilities( guides ).GetStrandToObjectTransform( static_cast<int>( guideDependency.closestRootIndices[1] ) ).yVector() * static_cast<Real>( guideDependency.closestRootDistances[1] ) +
			HairUtilities( guides ).GetStrandToObjectTransform( static_cast<int>( guideDependency.closestRootIndices[2] ) ).yVector() * static_cast<Real>( guideDependency.closestRootDistances[2] ) ).normalized();

		// The hair should now have an interpolated rotation
		REQUIRE( hair.HasStrandRotations() );
		REQUIRE( NearEqual( guidesVector, hairVector ) );
	}

	SECTION( "HairShapeDoesntChangeWhenSettingRotations" )
	{
		hair.SetUseStrandToObjectTransforms( true );
		hair.SetUsesGuideDependency( true );

		GenerateRandomHairs( guides, 3, 1, &distributionMesh, 10.0f, 2 );

		generator.rootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::FaceCenter;
		//generator.SetParameterValueT( HairFromGuidesGenerator::Parameter::RenderCount, 2 );
		//generator.SetParameterValueT( HairFromGuidesGenerator::Parameter::ViewportCount, 2 );

		// First, no rotations
		generator.GenRoots( false );
		generator.UpdateState();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		auto const verticesWithoutGuideRotations = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		// Then with random rotations
		SetRandomRotations( guides, &distributionMesh );
		generator.GenRoots( false );
		generator.UpdateState();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		auto const verticesWithGuideRotations = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		// Shouldn't see a difference in hair shapes
		REQUIRE_THAT( verticesWithoutGuideRotations, IsVectorNearEqual( verticesWithGuideRotations ) );
	}

	SECTION( "StrandChannelDistribution" )
	{
		GenerateVertexHairs( guides, 1, &distributionMesh );
		guides.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 2 );

		array<float, 4> const channelData = { 1.0f, 1.0f, 0.5f, 0.0f };
		guides.SetStrandChannelData( IHair::StrandDataType::PerStrand, 1, 0, Size( channelData ), channelData.data() );

		guides.SetUseStrandRotations( true );
		guides.SetStrandRotations( vector<Real>( 4, 0.0f ) );

		generator.rootParameters.rootCount = 10;

		generator.GenRoots( false );
		generator.UpdateState();

		auto const rootCountWithoutDistribution = hair.GetStrandCount();

		generator.parameters.distributionChannel = StrandChannel( StrandChannelType_PerStrand, 1 );
		generator.GenRoots( false );
		generator.UpdateState();

		auto const rootCountWithDistribution = hair.GetStrandCount();

		REQUIRE( rootCountWithDistribution < rootCountWithoutDistribution );
	}

	SECTION( "StrandGroup" )
	{
		GenerateVertexHairs( guides, 1, &distributionMesh );
		MoveTipsRandomly( guides );

		generator.rootParameters.rootCount = 10;
		generator.GenRoots( false );
		generator.UpdateState();

		// Without strand groups we have interpolated strands
		auto const directionsWithoutGroups = HairUtilities( hair ).GetRootToTipDirections();
		auto const firstDirection = directionsWithoutGroups.front();
		REQUIRE( any_of( begin( directionsWithoutGroups ) + 1, end( directionsWithoutGroups ), [&firstDirection]( const auto& direction ) { return !NearEqual( firstDirection, direction ); } ) );

		// Enable strand groups such that only one guide will participate
		guides.SetUsesStrandGroups( true );
		guides.SetStrandGroup( 0, 1 );
		generator.parameters.strandGroup.pattern = "1";
		generator.parameters.autoGuideProximity = false;

		generator.GenRoots( false );
		generator.UpdateState();

		// All resulting hairs should have the same shape since only one guide is used for interpolation
		auto const directionsWithoutGroups2 = HairUtilities( hair ).GetRootToTipDirections();
		auto const firstDirection2 = directionsWithoutGroups2.front();
		REQUIRE( all_of( begin( directionsWithoutGroups2 ) + 1, end( directionsWithoutGroups2 ), [&firstDirection2]( const auto& direction ) { return NearEqual( firstDirection2, direction ); } ) );
	}

	SECTION( "LargePlaneCircleAreaSingleStrandAutoGuideProximity" )
	{
		distributionMesh = GeneratePlane<Real>( 10, 2, 2, true );
		GenerateVertexHairs( guides, 1, &distributionMesh );

		generator.rootParameters.rootCount = 5;
		generator.parameters.guideAreaCalculation.method = HairFromGuidesParameters::GuideAreaCalculationMethodType::Circle;
		generator.parameters.guideCount = 1;
		generator.parameters.autoGuideProximity = true;
		
		REQUIRE( generator.GenRoots( false ) );
		generator.UpdateState();

		REQUIRE( hair.GetStrandCount() > 0 );
	}

	SECTION( "UpdatePartingsOnStrandGroupChanges" )
	{
		GenerateVertexHairs( guides, 1, &distributionMesh );
		MoveTipsRandomly( guides );
		guides.SetUsesStrandGroups( true );
		guides.SetStrandGroups( array<int, 4> { 1, 1, 1, 1 } );
		generator.parameters.parting.useStrandGroupsAsParts = true;

		generator.Evaluate();
		auto const verticesBeforeStrandGroupChange = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE( !verticesBeforeStrandGroupChange.empty() );

		guides.SetStrandGroups( array<int, 4> { 1, 1, 2, 2 } );
		generator.Evaluate();
		auto const verticesAfterStrandGroupChange = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( verticesBeforeStrandGroupChange, !IsVectorNearEqual( verticesAfterStrandGroupChange ) );
	}

	SECTION( "GuidesAsHairWithDistributionMap" )
	{
		GenerateVertexHairs( guides, 1, &distributionMesh );
		generator.rootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::GuidesAsHair;

		vector<Image::PixelGray16> pixels{ 65534, 65533, 0, 0 };
		generator.parameters.distributionMaps.first = { make_shared<TextureMapImage>( Image::ImageGray16{ pixels } ), 0, true };

		generator.Evaluate();

		// If distribution map was applied correctly we should have fewer hairs than input guides
		REQUIRE( sharedHair.GetStrandCount() < guides.GetStrandCount() );
	}

	// Checks that removing a distribution mesh is handled correctly. This can happen if a ground operator before HFG is turned off.
	SECTION( "NoDistributionMesh" )
	{
		GenerateVertexHairs( guides, 1, &distributionMesh );
		generator.rootParameters.rootCount = 5;

		generator.GenRoots( false );
		generator.UpdateState();

		REQUIRE( sharedHair.GetStrandCount() == generator.rootParameters.rootCount() );

		generator.rootParameters.distributionMesh = nullptr;
		generator.GenRoots( false );
		generator.UpdateState();

		REQUIRE( sharedHair.GetStrandCount() == 0 );
	}

	// #6177
	SECTION( "SegmentNoInterpolationVaryingGuidePointCount" )
	{
		guides.SetUsesStrandTopology( true );
		guides.SetUseSurfaceDependency2( true );

		static auto constexpr FirstStrandPointCount = 5, SecondStrandPointCount = 3;

		guides.SetStrandCount( 2 );
		CHECK( guides.SetStrandTopologies( array{ StrandTopology { 0u, FirstStrandPointCount }, StrandTopology { FirstStrandPointCount, SecondStrandPointCount } } ) );
		CHECK( guides.SetSurfaceDependencies2( array{ SurfacePosition { 0u, { 0.0f, 0.0f } }, SurfacePosition { 0u, { 1.0f, 1.0f } } } ) );
		guides.SetVertexCount( FirstStrandPointCount + SecondStrandPointCount );
		HairUtilities( guides ).SetVertices( array {
			Vector3{ 0, 0, 0 }, Vector3{ 0,0,0.2_r }, Vector3{ 0,0,0.4_r }, Vector3{ 0,0,0.8_r }, Vector3{ 0,0,1 },
			Vector3{ 0, 0, 0 }, Vector3{ 0,0,0.5_r }, Vector3{ 0,0,1 }
			
		} );

		// No interpolation, using segment method
		generator.rootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
		generator.parameters.interpolationMethod = HairFromGuidesParameters::InterpolationMethodType::Segment;
		generator.parameters.guideCount = 1;

		CHECK( generator.GenRoots( false ) );
		generator.UpdateState();

		REQUIRE( sharedHair.GetStrandCount() == 4 );
	}
}
}
