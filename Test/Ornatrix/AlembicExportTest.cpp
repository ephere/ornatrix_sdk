#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/SystemTools.h"
#include "Ephere/Ornatrix/Private/AlembicExport.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

using namespace std;
using namespace Ephere;
using namespace Geometry;
using namespace Ornatrix;

TEST_CASE( "AlembicExport" )
{
	AlembicExporter exporter;

	SharedHair sharedHair;
	IHair& hair = sharedHair;
	
	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh, 1.0f );

	AlembicExporter::AlembicNode node;
	node.name = L"Test";

	exporter.SetEvaluator( [&hair]( void* /*hostNode*/, double const /*time*/, float const /*completedFraction*/, bool& /*wasCancelled*/ )
	{
		AlembicExporter::AlembicNodeState result = {};
		result.hair = &hair;
		result.xform = Xform3::Identity();
		return result;
	} );

	exporter.filePath = L"AlembicExport.abc";

	SECTION( "SingleFile" )
	{
		exporter.filePath = L"SingleFile.abc";
		exporter.Export( vector{ node }, vector{ 0.0 } );

		REQUIRE( FileExists( exporter.filePath ) );

		FileRemove( exporter.filePath );
	}

	SECTION( "FileSequence" )
	{
		exporter.filePath = L"Sequence.###.abc";
		exporter.framesPerSecond = 24.0;

		static auto constexpr FramesInSequence = 3;

		vector<double> times;
		times.reserve( FramesInSequence );
		for( auto index = 0; index < FramesInSequence; ++index )
		{
			times.emplace_back( index / exporter.framesPerSecond );
		}

		exporter.Export( vector{ node }, times );

		// Make sure all files in sequence were written
		for( auto index = 0; index < FramesInSequence; ++index )
		{
			wstringstream expectedFileName;
			expectedFileName  << L"Sequence.00";
			expectedFileName << index;
			expectedFileName << L".abc";
			REQUIRE( FileExists( expectedFileName.str() ) );
			FileRemove( expectedFileName.str() );
		}
	}

	SECTION( "ObjectNameWithSpecialCharacters" )
	{
		exporter.filePath = L"ObjectNameWithSpecialCharacters.abc";
		node.name = L"One / two * three % ^&*(";
		exporter.Export( vector{ node }, vector{ 0.0 } );

		REQUIRE( FileExists( exporter.filePath ) );

		FileRemove( exporter.filePath );
	}

	SECTION( "ExportEachStrandAsSeparateObject" )
	{
		hair.SetUseStrandIds( true );
		hair.SetUsesStrandGroups( true );
		hair.SetUseWidths( true );
		
		exporter.exportEachStrandAsSeparateObject = true;
		exporter.exportNormals = exporter.exportStrandGroups = exporter.exportStrandIds = exporter.exportWidths = exporter.unrealEngine4Export = true;
		exporter.Export( vector{ node }, vector{ 0.0 } );

		REQUIRE( FileExists( exporter.filePath ) );

		// Import the hair back
		SharedHair loadedHair;
		REQUIRE( loadedHair.LoadFromFile( exporter.filePath.c_str(), {} ) );

		REQUIRE( loadedHair.GetVertexCount() == hair.GetVertexCount() );
		REQUIRE( loadedHair.GetStrandCount() == hair.GetStrandCount() );

		REQUIRE( FileRemove( exporter.filePath ) );
	}

	SECTION( "ExportVertexColorChannel" )
	{
		array const channelNames{ StrandChannelName { L"color_R" }, StrandChannelName { L"color_G" }, StrandChannelName { L"color_B" } };

		hair.SetStrandChannelCount( IHair::StrandDataType::PerVertex, Size( channelNames ) );
		REQUIRE( hair.SetStrandChannelNames( IHair::StrandDataType::PerVertex, 0, Size( channelNames ), channelNames.data() ) );

		vector<Real> strandData( hair.GetVertexCount() );
		for( auto index = 0; index < Size( channelNames ); ++index )
		{
			Fill( strandData, static_cast<Real>( index ) );
			REQUIRE( hair.SetStrandChannelData( IHair::StrandDataType::PerVertex, index, 0, Size( strandData ), strandData.data() ) );
		}
		
		exporter.exportStrandData = true;
		REQUIRE( exporter.Export( vector{ node }, vector{ 0.0 } ) );

		// Import the hair back
		SharedHair loadedHair;
		REQUIRE( loadedHair.LoadFromFile( exporter.filePath.c_str(), {} ) );

		REQUIRE( loadedHair.GetStrandChannelCount( IHair::StrandDataType::PerVertex ) == Size( channelNames ) );

		array<StrandChannelName, 3> loadedChannelNames;
		loadedHair.GetStrandChannelNames( IHair::StrandDataType::PerVertex, 0, Size( loadedChannelNames ), loadedChannelNames.data() );
		auto index = 0;
		for( auto const& channelName : channelNames )
		{
			REQUIRE( loadedChannelNames[index++].name == channelName.name );
		}

		for( index = 0; index < Size( channelNames ); ++index )
		{
			REQUIRE( hair.GetStrandChannelData( IHair::StrandDataType::PerVertex, index, 0, Size( strandData ), strandData.data() ) );

			REQUIRE( AllOf( strandData, [index]( auto const value )
			{
				return value == Real( index );
			} ) );
		}
	}
}
