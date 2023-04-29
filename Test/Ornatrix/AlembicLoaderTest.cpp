#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/SystemTools.h"
#include "Ephere/Ornatrix/Operators/HairAnimationCache.h"
#include "Ephere/Ornatrix/Private/AlembicExport.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

#include <filesystem>

using namespace std;
using namespace Ephere;
using namespace Geometry;
using namespace Ornatrix;

TEST_CASE( "AlembicLoader" )
{
	AlembicExporter exporter;

	SharedHair sharedHair;
	IHair& hair = sharedHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh, 1.0f, 2, -1, true );

	AlembicExporter::AlembicNode node;
	node.name = L"Test";

	exporter.filePath = L"AlembicLoader.abc";

	exporter.SetEvaluator( [&hair]( void* /*hostNode*/, double const /*time*/, float const /*completedFraction*/, bool& /*wasCancelled*/ )
	{
		AlembicExporter::AlembicNodeState result = {};
		result.hair = &hair;
		result.renderingPropertiesContainer = nullptr;
		result.xform = Xform3::Identity();
		return result;
	} );

	SECTION( "VertexChannelData" )
	{
		// Add some vertex channel data to hair
		static auto constexpr ChannelCount = 2;
		auto const elementCount = hair.GetVertexCount();
		vector<wstring> const channelNames { L"ChannelOne", L"ChannelTwo" };
		vector<Real> const values[ChannelCount] = { vector<Real>( elementCount, 1 ), vector<Real>( elementCount, 3 ) };
		for( auto index = 0; index < ChannelCount; ++index )
		{
			AddStrandChannel( hair, IHair::StrandDataType::PerVertex, values[index], channelNames[index] );
		}

		REQUIRE( exporter.Export( vector<AlembicExporter::AlembicNode>( 1, node ), { 0.0 } ) );
		REQUIRE( FileExists( exporter.filePath ) );

		SharedHair loadedSharedHair;
		REQUIRE( loadedSharedHair.CanLoadFromFile( exporter.filePath.c_str() ) );
		REQUIRE( loadedSharedHair.LoadFromFile( exporter.filePath.c_str(), HairBaseSA::HairComponentSelection{} ) );

		IHair const& loadedHair = loadedSharedHair;

		// Import the file back into new hair
		REQUIRE( loadedHair.GetStrandChannelCount( IHair::StrandDataType::PerVertex ) == 2 );

		// Compare channel names
		vector<StrandChannelName> loadedChannelNames( 2 );
		loadedHair.GetStrandChannelNames( IHair::StrandDataType::PerVertex, 0, Size( loadedChannelNames ), loadedChannelNames.data() );
		for( auto index = 0; index < ChannelCount; ++index )
		{
			REQUIRE( wstring( loadedChannelNames[index].name ) == channelNames[index] );
		}

		// Compare channel data
		for( auto index = 0; index < ChannelCount; ++index )
		{
			vector<Real> loadedValues( elementCount );
			REQUIRE( hair.GetStrandChannelData( IHair::StrandDataType::PerVertex, index, 0, int( loadedValues.size() ), loadedValues.data() ) );

			REQUIRE_THAT(
				values[index],
				IsVectorNearEqual( loadedValues )
			);
		}
	}

	SECTION( "VaryingPointCount" )
	{
		// Generate hairs with varying point counts per strand
		GenerateRandomHairs( hair, 10, 1, &distributionMesh, 1.0f, 2, 5 );

		auto const strandPointsBeforeExport = GetStrandPoints( hair, IHair::CoordinateSpace::Object );

		// Save and reload
		SharedHair loadedSharedHair;
		REQUIRE( exporter.Export( vector<AlembicExporter::AlembicNode>( 1, node ), { 0.0 } ) );
		REQUIRE( loadedSharedHair.LoadFromFile( exporter.filePath.c_str(), HairBaseSA::HairComponentSelection{} ) );
		IHair const& loadedHair = loadedSharedHair;

		auto const strandPointsAfterLoading = GetStrandPoints( loadedHair, IHair::CoordinateSpace::Object );

		REQUIRE( strandPointsBeforeExport.size() == strandPointsAfterLoading.size() );
		for( auto strandPointsBeforeExportIterator = strandPointsBeforeExport.begin(), strandPointsAfterLoadingIterator = strandPointsAfterLoading.begin();
			 strandPointsBeforeExportIterator != strandPointsBeforeExport.end() && strandPointsAfterLoadingIterator != strandPointsAfterLoading.end();
			 ++strandPointsBeforeExportIterator, ++strandPointsAfterLoadingIterator )
		{
			REQUIRE_THAT(
				*strandPointsBeforeExportIterator,
				IsVectorNearEqual( *strandPointsAfterLoadingIterator )
			);
		}
	}

	SECTION( "Fraction hair" )
	{
		SharedHair loadedHair;
		REQUIRE( exporter.Export( vector<AlembicExporter::AlembicNode>( 1, node ), { 0.0 } ) );
		REQUIRE( loadedHair.LoadFromFile( exporter.filePath.c_str(), HairBaseSA::HairComponentSelection{}, 0.25f ) );
		REQUIRE( loadedHair.GetStrandCount() == 3 );
	}

	FileRemove( exporter.filePath );
}

wstring GetRootPath()
{
	if( auto const envP = GetCurrentPath<wchar_t>();
		!envP.empty() )
	{
		return ToWideChar( filesystem::path( envP ).parent_path().string() );
	}

	return ToWideChar( filesystem::current_path().string() );
}

wstring GetRepositoryRootPath()
{
	auto repositoryRootPath = GetRootPath();

	// NOTE: Assuming that Local.runsettings only exists in the root directory. Replace with some other file name if that's no longer the case.
	while( !repositoryRootPath.empty() && !FileExists( repositoryRootPath + L"/Local.runsettings" ) )
	{
		repositoryRootPath = ToWideChar( filesystem::path( repositoryRootPath ).parent_path().string() );
	}

	return repositoryRootPath;
}

wstring FindTestFile( wchar_t const* const filename )
{
	auto fullPath = ToWideChar( filesystem::path( filesystem::current_path() ).parent_path().string() ) + DirectorySeparator() + filename;
	if( FileExists( fullPath ) )
	{
		return fullPath;
	}

	fullPath = GetRepositoryRootPath() + L"/External/TestData/" + filename;
	if( FileExists( fullPath ) )
	{
		return fullPath;
	}

	return {};
}

TEST_CASE( "AlembicImportSpecificCurve" )
{
	auto const testFilePath = FindTestFile( L"MultipleCurveObjects.abc" );
	REQUIRE( !testFilePath.empty() );

	SharedHair loadedHair;

	SECTION( "LoadFirstCurve" )
	{
		REQUIRE( loadedHair.LoadFromFile( testFilePath.data(), {}, -1, array{ string { "/Plane1/Line1" } } ) );

		REQUIRE( loadedHair.GetStrandCount() == 1 );
		REQUIRE_THAT(
			loadedHair.GetVertex( loadedHair.GetVertexCount() - 1, IHair::CoordinateSpace::Object ),
			IsNearEqual( Vector3( 0, 0, 10 ) )
		);
	}

	SECTION( "LoadSecondCurve" )
	{
		REQUIRE( loadedHair.LoadFromFile( testFilePath.data(), {}, -1, array{ string { "/Plane2/Line2" } } ) );
		REQUIRE( loadedHair.GetStrandCount() == 1 );
	}

	SECTION( "LoadNonExistentCurve" )
	{
		REQUIRE( !loadedHair.LoadFromFile( testFilePath.data(), {}, -1, array{ string { "/Plane2/Line3" } } ) );
	}
}