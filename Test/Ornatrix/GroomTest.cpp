#include "PrecompiledHeaders.h"

#include "TestOperator.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedNurbsCurves.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/LibZip.h"
#include "Ephere/NativeTools/Profile/Stopwatch.h"
#include "Ephere/NativeTools/Test/TestLogger.h"
#include "Ephere/Ornatrix/Ornatrix.h"
#include "Ephere/Ornatrix/Groom/Evaluator.h"
#include "Ephere/Ornatrix/Groom/Operators.h"
#include "Ephere/Ornatrix/Operators/AdoptExternalGuidesParameters.g.h"
#include "Ephere/Ornatrix/Operators/BakedHairParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/CurlParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/GuidesEditorParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshGenerator.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/HairFromGuidesParameters.g.h"
#include "Ephere/Ornatrix/Operators/NoiseParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/RootGeneratorParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/SurfaceComber.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Ephere/Ornatrix/Private/TextureMapImage.h"
#include "Ephere/Ornatrix/Test/Utilities.h"

using namespace Ephere;
using namespace Ornatrix;
using namespace Groom;
using namespace Geometry;
using namespace Image;
using namespace Parameters;
using namespace std;

static auto const TestFilename = string( "TestGroom" ) + IGrooms::BaseFileExtension();
static auto const TestFilenameNoDefaults = string( "TestGroom-NoDefaults" ) + IGrooms::BaseFileExtension();

shared_ptr<Graph> DeserializeGroomFromFile( string_view filePath, double time = TimeUndefined )
{
	return static_pointer_cast<Graph>( TheOrnatrixLibrary.grooms->DeserializeGroomFromFile( filePath, time ).ToShared() );
}

shared_ptr<Graph> DeserializeGroomFromFileContents( string_view fileContents, double time = TimeUndefined )
{
	return static_pointer_cast<Graph>( TheOrnatrixLibrary.grooms->DeserializeGroomFromContentBuffer( fileContents, time ).ToShared() );
}

TEST_CASE( "ParameterRef" )
{
	REQUIRE( ParameterRef{}.IsEmpty() );
	REQUIRE( ParameterRef{ ParameterId::Invalid }.IsEmpty() );
	REQUIRE( ParameterRef{ *reinterpret_cast<Node*>( 1 ), ParameterId::Invalid }.IsEmpty() );
	REQUIRE_FALSE( ParameterRef{ ParameterId::_1 }.IsEmpty() );
}

TEST_CASE( "Groom Empty" )
{
	Graph groom;

	// There is a default 'currentTime' node
	REQUIRE( groom.GetNodeCount() == 1 );

	REQUIRE( Operators::AdoptExternalGuides.parameterSetDescriptors.size() == 1 );
	REQUIRE( Operators::Mesh.parameterSetDescriptors.size() == 1 );
	REQUIRE( Operators::TextureImage.parameterSetDescriptors.size() == 1 );
	REQUIRE( Operators::HairFromGuides.parameterSetDescriptors.size() == 2 );
}

TEST_CASE( "Groom AutoConnect Dont Connect Auxiliary Inputs" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ AdoptExternalGuidesNodeName, Operators::AdoptExternalGuides }
	};

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, AdoptExternalGuidesNodeName } );

	// The main hair input is connected
	REQUIRE( groom.AreConnected( { groom[GuidesFromMeshNodeName], GuidesFromMeshParameters::OutputGuides() }, { AdoptExternalGuidesNodeName, AdoptExternalGuidesParameters::TargetHair::Id } ) );
	REQUIRE( groom[GuidesFromMeshNodeName].GetConnectionCount() == 2 );
	REQUIRE( groom[GuidesFromMeshNodeName].GetInputConnectionCount() == 1 );

	// But auxiliary hair inputs are not connected
	REQUIRE( !groom.AreConnected( { groom[GuidesFromMeshNodeName], GuidesFromMeshParameters::OutputGuides() }, { AdoptExternalGuidesNodeName, AdoptExternalGuidesParameters::Objects::Control::Id } ) );
	REQUIRE( !groom.AreConnected( { groom[GuidesFromMeshNodeName], GuidesFromMeshParameters::OutputGuides() }, { AdoptExternalGuidesNodeName, AdoptExternalGuidesParameters::Objects::Reference::Id } ) );
}

TEST_CASE( "Groom Generate Sample" )
{
	Graph groom
	{
		CoordinateSystem3dsMax(),
		{
			{ StartNodeName, Operators::Mesh },
			{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
			{ Curl1NodeName, Operators::Curl }
		}
	};

	auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 10, 5, 5 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Translation( { 1, 2, 3 } ) } );

	auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
	gfmParameters.guideLength = 15.0f;
	auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	gfmRootParameters.rootCount = 30;
	gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Even;

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, Curl1NodeName } );

	auto const [extension, expectedCrc] = GENERATE( make_pair( ".usda"s, 0x84c3fcf7 ), make_pair( ".yaml"s, 0x9147d1d0 ) );
	auto const sampleFileName = "SampleGroom" + string( IGrooms::BaseFileExtension() ) + extension;

	FileRemove( sampleFileName );
	REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, sampleFileName, 0 ) == sampleFileName );

	gfmParameters.guideLength = 20.0f;
	REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, sampleFileName, 1 ) == sampleFileName );

	RequireTextFileCrc( sampleFileName, expectedCrc );

	auto readGroom = static_unique_cast<Graph>( TheOrnatrixLibrary.grooms->DeserializeGroomFromFile( sampleFileName, 0 ) );
	REQUIRE( readGroom != nullptr );
	auto readMesh = dynamic_cast<SharedPolygonMesh const*>( (*readGroom)[StartNodeName].GetOperator().GetGeneratorValue<PolygonMeshParameter>()->Get() );
	REQUIRE( readMesh != nullptr );
	REQUIRE( *planeMesh == *readMesh );
}

TEST_CASE( "Groom Convert", "[.]" )
{
	auto const inputFilePath = GetEnvironmentVariable( "CONVERT_GROOM" );
	GroomInfo info;
	REQUIRE( TheOrnatrixLibrary.grooms->GetGroomInfoFromFile( inputFilePath, info ) );
	REQUIRE( info.IsValid() );
	if( info.timeSamples.empty() )
	{
		info.timeSamples.push_back( TimeUndefined );
	}

	String outputFilePath = GetEnvironmentVariable( "CONVERT_GROOM_OUTPUT" );
	if( outputFilePath.empty() )
	{
		outputFilePath = GetExtension( inputFilePath ) == IUsdSerializer::ExtensionUsd() ? ChangeExtension( inputFilePath, ".yaml" ) : ChangeExtension( inputFilePath, ".zip" );
	}

	SerializedGroom serializedInput;
	GroomSerializeSettings settings{ TimeUndefined, false };
	REQUIRE( TheOrnatrixLibrary.grooms->LoadGroomFromFile( inputFilePath, serializedInput ) );
	for( auto const time : info.timeSamples )
	{
		auto groom = TheOrnatrixLibrary.grooms->DeserializeGroom( "", serializedInput, time );
		REQUIRE( groom != nullptr );
		auto result = serializedInput;
		result.contentBuffer.clear();
		settings.time = time;
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroom( *groom, outputFilePath, settings, result ) );
	}
}

TEST_CASE( "Groom Serialize to Zip with Absolute Path" )
{
	Graph const groom
	{
		{ Curl1NodeName, Operators::Curl },
	};
	auto const extension = GENERATE( pair( ".usdz", ".usda" ), pair( ".zip", ".yaml" ) );
	auto const fileName = PathCombine( GetCurrentPath<char>(), TestFilename + extension.first );
	FileRemove( fileName );
	auto const resultPath = TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, fileName, TimeUndefined );
	REQUIRE( FileSize( resultPath ) > 0 );
	REQUIRE( GetExtension( Zip::Archive::Read( resultPath ).first.GetEntry( 0 ).Name ) == extension.second );

	auto const groomPtr = DeserializeGroomFromFile( resultPath );
	REQUIRE( groomPtr != nullptr );
	REQUIRE( groomPtr->GetNodeCount() == 2 );

	FileRemove( resultPath );
}

TEST_CASE( "Groom serialize empty mesh array" )
{
	Graph const groom
	{
		{ Test1NodeName, Operators::StrandDataGenerator },
	};
	auto const extension = GENERATE( ".usda", ".yaml" );
	auto const fileName = PathCombine( GetCurrentPath<char>(), TestFilename + extension );
	FileRemove( fileName );
	auto const resultPath = TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, fileName, TimeUndefined );
	REQUIRE( FileSize( resultPath ) > 0 );
	auto const groomPtr = DeserializeGroomFromFile( resultPath );
	REQUIRE( groomPtr != nullptr );
	REQUIRE( groomPtr->GetNodeCount() == 2 );
	FileRemove( resultPath );
}

TEST_CASE( "Groom Read from In-memory Text" )
{
	Graph groom
	{
		{ Curl1NodeName, Operators::Curl },
	};

	auto& curlParameters = *groom[Curl1NodeName].GetOperator().GetParameterSet<CurlParameters>();
	curlParameters.magnitude.value = 0.125;

	auto const extension = GENERATE( ".usda", ".yaml" );
	FileRemove( TestFilename + extension );
	auto const resultPath = TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension, TimeUndefined );
	REQUIRE( FileSize( resultPath ) > 0 );

	auto const buffer = ReadAllBytes( resultPath );
	FileRemove( resultPath );

	auto groomPtr = DeserializeGroomFromFileContents( buffer );
	REQUIRE( groomPtr );
	REQUIRE( groomPtr->GetNodeCount() == 2 );
	REQUIRE( groomPtr->FindNode( Curl1NodeName )->GetOperator().GetParameterSet<CurlParameters>()->magnitude.value() == 0.125f );
}

TEST_CASE( "Groom with texture" )
{
	Graph groom
	{
		{ Curl1NodeName, Operators::Curl },
	};

	auto& curlParameters = *groom[Curl1NodeName].GetOperator().GetParameterSet<CurlParameters>();
	array<PixelRgba16, 4> pixels{ 65534, { 65533, 0, 0 }, { 0, 65532, 0 }, { 0, 0, 65531 } };
	curlParameters.magnitude.map = TextureMapParameter{ make_shared<TextureMapImage>( ImageRgba16{ pixels } ) };

	SECTION( "Read from In-memory Zip" )
	{
		auto const extension = GENERATE( ".usda", ".yaml" );
		FileRemove( TestFilename + extension );

		auto const resultPath = TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension );
		REQUIRE( FileSize( resultPath ) > 0 );

		// Auto-convert to zip
		REQUIRE( GetExtension( resultPath ) == TheOrnatrixLibrary.grooms->GetGroomSerializerByFileExtension( extension )->GroomZippedFormatExtension().ToString() );

		auto const buffer = ReadAllBytes( resultPath );
		FileRemove( resultPath );

		auto groomPtr = DeserializeGroomFromFileContents( buffer );
		REQUIRE( groomPtr );
		auto textureImage = dynamic_cast<TextureMapImage const*>( groomPtr->FindNode( Curl1NodeName )->GetOperator().GetParameterSet<CurlParameters>()->magnitude.map().GetMap() );
		REQUIRE( textureImage != nullptr );
		REQUIRE( textureImage->GetWidth() == 2 );
		REQUIRE( textureImage->GetHeight() == 2 );
	}

	SECTION( "Read from separate files" )
	{
		static string const TextureName = "TestTexture";
		auto const extension = GENERATE( ".usda"s, ".yaml"s );
		FileRemove( TestFilename + extension );

		auto& textureNode = groom.AddNode( "texture1", Operators::TextureImage );
		textureNode.SetParameterValueById( ParameterId::_1, TextureMapParameter{ make_shared<TextureMapImage>( ImageRgba16{ pixels } ) } );
		textureNode.SetParameterValueById( ParameterId::_2, String( TextureName ) );
		REQUIRE( groom.Connect( { textureNode, ParameterId::_1 }, { Curl1NodeName, CurlParameters::Phase::Map::Id } ) );

		auto const resultPath = TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension, TimeUndefined, nullptr, true, false, false );
		auto groomPtr = DeserializeGroomFromFile( resultPath );
		REQUIRE( groomPtr );
		auto textureImage = dynamic_cast<TextureMapImage const*>( groomPtr->FindNode( Curl1NodeName )->GetParameterValueById<TextureMapParameter>( CurlParameters::Magnitude::Map::Id ).GetMap() );
		REQUIRE( textureImage != nullptr );
		REQUIRE( textureImage->GetWidth() == 2 );

		REQUIRE( FileExists( "curl1.CurlParameters.Magnitude.Map.png" ) );
		FileRemove( "curl1.CurlParameters.Magnitude.Map.png" );

		REQUIRE( FileExists( TextureName + ".png" ) );
		FileRemove( TextureName + ".png" );
	}

	SECTION( "Animation" )
	{
		auto const extension = GENERATE( ".usdz", ".zip" );
		FileRemove( TestFilename + extension );

		curlParameters.magnitude.value = 0.5;
		auto const resultPath = TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension, 0 );
		REQUIRE( FileSize( resultPath ) > 0 );

		curlParameters.magnitude.value = 0.25;
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension, 1 ) == resultPath );

		auto groomPtr = DeserializeGroomFromFile( resultPath, 0 );
		REQUIRE( groomPtr->FindNode( Curl1NodeName )->GetOperator().GetParameterSet<CurlParameters>()->magnitude.value() == 0.5f );
		groomPtr = DeserializeGroomFromFile( resultPath, 1 );
		REQUIRE( groomPtr->FindNode( Curl1NodeName )->GetOperator().GetParameterSet<CurlParameters>()->magnitude.value() == 0.25f );

		FileRemove( resultPath );
	}
}

TEST_CASE( "Groom Serialize Overwrite Same Time" )
{
	Graph groom
	{
		CoordinateSystemDefault(),
		{
			{ Curl1NodeName, Operators::Curl },
		}
	};

	String filePath;
	GroomSerializeSettings const settings( 0, true, true );
	for( auto const serializer : TheOrnatrixLibrary.grooms->GetGroomSerializers() )
	{
		SerializedGroom result;
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroom( groom, filePath, settings, result, serializer ) );
		auto const first = result.contentBuffer;
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroom( groom, filePath, settings, result, serializer ) );
		REQUIRE( first == result.contentBuffer );
	}
}

TEST_CASE( "Groom Serialize Complex" )
{
	static char constexpr TestRamp[] = "St 0 0.1 Li 0.2 0.3 Sm 0.4 0.5 Sp 1 0.9";
	static constexpr char ImageFilename[] = "TestTextureImage.png";
	static constexpr char ImageNodeName[] = "TestTextureImage";
	String filePath;

	auto const [extension, extensionZip, expectedCrcNoDefaults, expectedCrcDefaults] =
		GENERATE( make_tuple( ".usda"s, ".usdz"s, 0xe3327bb, 0xd85b3a2c ), make_tuple( ".yaml"s, ".zip"s, 0xa034ee23, 0xde111c7e ) );

	// Serialize
	{
		Graph groom
		{
			CoordinateSystemDefault(),
			{
				{ StartNodeName, Operators::Mesh },
				{ ColliderNodeName, Operators::Mesh },
				{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
				{ EditGuidesNodeName, Operators::EditGuides },
				{ Curl1NodeName, Operators::Curl },
				{ Test1NodeName, *Operators::Test },
				{ HairFromGuidesNodeName, Operators::HairFromGuides },
				{ ImageNodeName, Operators::TextureImage }
			}
		};

		auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
		planeMesh->ValidatePolygonVertexNormals();
		//planeMesh->SetVertexNormal( 0, Vector3::YAxis() );
		auto cylinderMesh = make_shared<SharedPolygonMesh>( GenerateCylinder10<Real>( 1 ) );
		groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Translation( Vector3( 2, 0, 0 ) ) } );
		groom[ColliderNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ cylinderMesh, Xform3::Translation( Vector3( 5, 0, 0 ) ) } );

		auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
		gfmParameters.guideLength = 1.0f;
		auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
		gfmRootParameters.rootCount = 17;
		gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
		gfmRootParameters.useFaceInclude = true;
		gfmRootParameters.faceInclude = { true };
		array<PixelRgba16, 4> pixels{ 65534, { 65533, 0, 0 }, { 0, 65532, 0 }, { 0, 0, 65531 } };
		gfmParameters.distributionMap = { make_shared<TextureMapImage>( ImageRgba16{ pixels } ), 13, true };

		auto& curl1Parameters = *groom[Curl1NodeName].GetOperator().GetParameterSet<CurlParameters>();
		curl1Parameters.magnitude.value = 3;
		curl1Parameters.magnitude.ramp = Ramp( TestRamp );
		curl1Parameters.strandGroup.pattern = "1";

		auto& test1Parameters = *groom[Test1NodeName].GetOperator().GetParameterSet<TestParameters>();
		test1Parameters.magnitude.sinks.ramps = { Ramp{}, Ramp{ { 0, 0 }, { 1, 1 } } };
		test1Parameters.examples.vectorDistanceExample = { 1, 2, 3 };
		test1Parameters.examples.xformExample = Xform3::Translation( { 1, 2, 3 } );
		test1Parameters.arrayExamples.intsExample = { 3, 2, 1 };
		test1Parameters.arrayExamples.enumsExample = { TestParameters::WaveformType::Helix, TestParameters::WaveformType::Sine, TestParameters::WaveformType::Helix };
		test1Parameters.arrayExamples.vectorsExample = { { 1, 3, 5 }, { 2, 4, 6 } };
		test1Parameters.arrayExamples.xformsExample = { Xform3::Translation( { 1, 2, 3 } ), Xform3::Scale( { 4, 5, 6 } ) };
		test1Parameters.arrayExamples.stringsExample = { "one", "two" };
		test1Parameters.magnitude.sinks.positions = { { 1, { 3, 5 } }, { 2, { 4, 6 } } };
		test1Parameters.examples.customTypeExample = Operators::SomeData{ 13, { 14, 15, 16 } };
		*test1Parameters.examples.customSharedExample() = Operators::SomeData{ 13, { 14, 15, 16 } };

		std::array<PixelGray16, 4> pixels2{ 32768, 32768, 32768, 32768 };
		REQUIRE( TextureMapImage{ ImageGray16{ pixels2 } }.ToPng( FileNameToWideChar( ImageFilename ) ) );
		groom[ImageNodeName].SetParameterValueById( ParameterId::_2, String( ImageFilename ) );

		auto& hfgRootParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
		hfgRootParameters.rootCount = 1017;
		auto& hfgParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<HairFromGuidesParameters>();
		hfgParameters.generatedStrandFraction = 1.0f;

		REQUIRE( groom.Connect( { ImageNodeName, ParameterId::_1 }, { HairFromGuidesNodeName, HairFromGuidesParameters::DistributionMaps::First::Id, 1 } ) );
		REQUIRE( groom.Connect( { ImageNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::Phase::Map::Id } ) );

		groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, EditGuidesNodeName, Curl1NodeName, Test1NodeName, HairFromGuidesNodeName } );
		REQUIRE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { groom[Curl1NodeName], CurlParameters::DistributionMesh() } ) );
		REQUIRE( groom.AreConnected( { groom[GuidesFromMeshNodeName], GuidesFromMeshParameters::OutputGuides() }, { EditGuidesNodeName, GuidesEditorParameters::TargetHair::Id } ) );
		REQUIRE( groom.AreConnected( { EditGuidesNodeName, GuidesEditorParameters::TargetHair::Id }, { Curl1NodeName, CurlParameters::TargetHair::Id } ) );
		REQUIRE( groom.AreConnected( { Curl1NodeName, CurlParameters::TargetHair::Id }, { Test1NodeName, TestParameters::Examples::TargetHairExample::Id } ) );

		REQUIRE( groom.Connect( { Curl1NodeName, CurlParameters::Magnitude::Value::Id }, { Test1NodeName, TestParameters::Examples::AuxExample::Id } ) );
		REQUIRE_FALSE( groom[Test1NodeName].GetInputConnection( TestParameters::Examples::AuxExample::Id ).empty() );
		REQUIRE( groom.AreConnected( { Curl1NodeName, CurlParameters::Magnitude::Value::Id }, { Test1NodeName, TestParameters::Examples::AuxExample::Id } ) );

		REQUIRE( groom.Connect( { StartNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::Examples::MeshExample::Id } ) );
		vector<ParameterRef> inputs{ { StartNodeName, ParameterId::_1 }, { ColliderNodeName, ParameterId{ 1 } } };
		REQUIRE( groom.Connect( inputs, { Test1NodeName, TestParameters::ArrayExamples::MeshesExample::Id } ) );
		REQUIRE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::ArrayExamples::MeshesExample::Id } ) );
		REQUIRE( groom.AreConnected( { ColliderNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::ArrayExamples::MeshesExample::Id } ) );
		// Just connecting the array won't change its size, only evaluation will do it
		REQUIRE( test1Parameters.arrayExamples.meshesExample().empty() );

		SerializedGroom serializedGroom;
		filePath = TestFilenameNoDefaults + extension;
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroom( groom, filePath, GroomSerializeSettings{ TimeUndefined, false, true, false }, serializedGroom ) );
		REQUIRE( filePath == TestFilenameNoDefaults + extension );
		RequireCrc( serializedGroom.contentBuffer, expectedCrcNoDefaults );

		serializedGroom.Clear();
		filePath = TestFilename + extensionZip;
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroom( groom, filePath, GroomSerializeSettings{ TimeUndefined, true, false, false }, serializedGroom ) );
		RequireCrc( serializedGroom.contentBuffer, expectedCrcDefaults );

		groom.DisconnectAll();
		REQUIRE_FALSE( groom.AreConnected( { Curl1NodeName, CurlParameters::Magnitude::Value::Id }, { Test1NodeName, TestParameters::Examples::AuxExample::Id } ) );
	}

	// Deserialize
	{
		auto groomPtr = DeserializeGroomFromFile( filePath );
		REQUIRE( groomPtr != nullptr );
		auto contents = TheOrnatrixLibrary.grooms->SerializeGroomToBuffer(
			*groomPtr, TimeUndefined, TheOrnatrixLibrary.grooms->GetGroomSerializerByFileExtension( extension ), false, true );
		RequireCrc( contents, expectedCrcNoDefaults );
	}

	// Deserialize and evaluate
	{
		auto groomPtr = DeserializeGroomFromFile( filePath );
		REQUIRE( groomPtr != nullptr );
		auto& groom = *groomPtr;
		REQUIRE( groom.GetNodeCount() == 9 );
		REQUIRE( groom.GetCoordinateSystemProperties() == CoordinateSystemDefault() );

		auto contents = TheOrnatrixLibrary.grooms->SerializeGroomToBuffer(
			groom, TimeUndefined, TheOrnatrixLibrary.grooms->GetGroomSerializerByFileExtension( extension ), true, true );
		RequireCrc( contents, expectedCrcDefaults );

		auto startMesh = groom[StartNodeName].GetOperator().GetGeneratorValue<PolygonMeshParameter>();
		REQUIRE( startMesh->Get()->GetTextureChannelCount() == 1 );
		//REQUIRE( startMesh->Get()->GetTriangleVertexNormal( 0, 0 ) == Vector3::YAxis() );
		auto colliderMesh = groom[ColliderNodeName].GetOperator().GetGeneratorValue<PolygonMeshParameter>();
		REQUIRE( colliderMesh->GetObjectToWorldTransform() == Xform3::Translation( Vector3( 5, 0, 0 ) ) );
		array<int, 4> indices{};
		colliderMesh->Get()->GetPolygonTextureCoordinateIndices( 0, 0, 0, indices );
		REQUIRE( indices == array{ 10, 11, 22, 21 } );
		colliderMesh->Get()->GetPolygonTextureCoordinateIndices( 0, 29, 0, Span( indices.data(), 3 ) );
		REQUIRE( indices == array{ 41, 40, 43, 21 } );
		REQUIRE( groom[Curl1NodeName].GetParameter<CurlParameters::Magnitude::Value>() == 3_r );
		REQUIRE( groom[Curl1NodeName].GetOperator().GetParameterSet<CurlParameters>()->strandGroup.pattern() == "1" );
		REQUIRE( groom[Curl1NodeName].GetOperator().GetParameterSet<CurlParameters>()->magnitude.ramp() == Ramp( TestRamp ) );

		auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
		REQUIRE( gfmParameters.guideLength() == 1_r );
		auto const& distributionMap = gfmParameters.distributionMap();
		REQUIRE( !distributionMap.IsEmpty() );
		auto const distributionMapImage = dynamic_cast<TextureMapImage const*>( distributionMap.GetMap() );
		REQUIRE( distributionMapImage != nullptr );
		REQUIRE( distributionMapImage->GetWidth() == 2 );
		REQUIRE( distributionMapImage->GetHeight() == 2 );
		REQUIRE( distributionMapImage->GetImage().GetRgba16( 1, 0 ) == PixelRgba16{ 65533, 0, 0, 65535 } );
		REQUIRE( distributionMapImage->GetImage().GetRgba16( 1, 1 ) == PixelRgba16{ 0, 0, 65531, 65535 } );
		REQUIRE( distributionMap.GetTextureChannel() == 13 );
		REQUIRE( distributionMap.GetInvertValues() );

		auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
		REQUIRE( gfmRootParameters.rootCount() == 17 );
		REQUIRE( gfmRootParameters.generationMethod() == RootGeneratorParameters::GenerationMethodType::Vertex );
		REQUIRE( gfmRootParameters.useFaceInclude() );
		REQUIRE( gfmRootParameters.faceInclude().size() == 1 );

		REQUIRE( groom[Test1NodeName].GetOperator().GetParameterSet( 0 ).GetParameterById( TestParameters::Waveform::Id ) != nullptr );
		auto& test1Parameters = *groom[Test1NodeName].GetOperator().GetParameterSet<TestParameters>();
		REQUIRE( test1Parameters.magnitude.sinks.ramps().size() == 2 );
		REQUIRE( test1Parameters.magnitude.sinks.ramps()[1] == Ramp{ { 0, 0 }, { 1, 1 } } );
		REQUIRE( test1Parameters.examples.auxExample() == 0 );
		REQUIRE( test1Parameters.examples.vectorDistanceExample() == Vector3{ 1, 2, 3 } );
		REQUIRE( test1Parameters.examples.xformExample() == Xform3::Translation( { 1, 2, 3 } ) );
		REQUIRE( test1Parameters.arrayExamples.enumsExample() == vector<TestParameters::WaveformType>{
			{
				TestParameters::WaveformType::Helix, TestParameters::WaveformType::Sine, TestParameters::WaveformType::Helix
			} } );
		REQUIRE( test1Parameters.arrayExamples.intsExample() == vector<int>{ 3, 2, 1 } );
		REQUIRE( test1Parameters.arrayExamples.vectorsExample() == vector<Vector3>{
			{
				1, 3, 5
			}, { 2, 4, 6 } } );
		REQUIRE( test1Parameters.arrayExamples.xformsExample() == vector<Xform3>{ Xform3::Translation( { 1, 2, 3 } ), Xform3::Scale( { 4, 5, 6 } ) } );
		REQUIRE( test1Parameters.arrayExamples.stringsExample() == vector<String>{ "one", "two" } );
		REQUIRE( test1Parameters.magnitude.sinks.positions() == vector<Geometry::SurfacePosition>{
			{
				1, { 3, 5 }
			}, { 2, { 4, 6 } } } );
		REQUIRE( test1Parameters.examples.customTypeExample().v == vector<int>{ 14, 15, 16 } );
		REQUIRE( test1Parameters.examples.customSharedExample()->v == vector<int>{ 14, 15, 16 } );

		auto& hfgRootParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
		REQUIRE( hfgRootParameters.rootCount() == 1017 );
		groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<HairFromGuidesParameters>()->previewStrandFraction = 1;

		REQUIRE( groom.AreConnected( { GuidesFromMeshNodeName, GuidesFromMeshParameters::OutputGuides::Id, 1 }, { EditGuidesNodeName, GuidesEditorParameters::TargetHair::Id } ) );
		REQUIRE( groom.AreConnected( { EditGuidesNodeName, GuidesEditorParameters::TargetHair::Id }, { Curl1NodeName, CurlParameters::TargetHair::Id } ) );
		REQUIRE( groom.AreConnected( { Curl1NodeName, CurlParameters::TargetHair::Id }, { Test1NodeName, TestParameters::Examples::TargetHairExample::Id } ) );

		REQUIRE( groom.AreConnected( { Curl1NodeName, CurlParameters::Magnitude::Value::Id }, { Test1NodeName, TestParameters::Examples::AuxExample::Id } ) );
		REQUIRE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::Examples::MeshExample::Id } ) );
		REQUIRE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::ArrayExamples::MeshesExample::Id } ) );
		REQUIRE( groom.AreConnected( { ColliderNodeName, ParameterId::_1 }, { Test1NodeName, TestParameters::ArrayExamples::MeshesExample::Id } ) );
		// Just connecting the array won't change its size, only evaluation will do it
		REQUIRE( test1Parameters.arrayExamples.meshesExample().empty() );

		REQUIRE( Evaluator{ groom }.Evaluate() );
		auto const resultParameter = groom[HairFromGuidesNodeName].GetParameter<HairFromGuidesParameters::OutputHair>().GetValue();
		REQUIRE( !resultParameter.IsEmpty() );
		REQUIRE( GetEndPointHair( groom ).Get() == resultParameter.Get() );
		REQUIRE( resultParameter.GetObjectToWorldTransform() == startMesh->GetObjectToWorldTransform() );

		auto const* result = resultParameter.Get();
		INFO( result->GetStrandCount() )
		REQUIRE( InRange( result->GetStrandCount(), 490, 495 ) );

		// Make sure that generated strands are spread apart in object space
		auto const rootPositions = HairUtilities( *result ).GetRootPositions( 0, -1 );
		Real averageDistanceFromOrigin( 0 );
		for( auto const& rootPosition : rootPositions )
		{
			averageDistanceFromOrigin += rootPosition.length();
		}

		REQUIRE( averageDistanceFromOrigin > 0.5f );
		REQUIRE( test1Parameters.examples.auxExample() == 3 );
		// The connected array gets assigned on evaluation
		REQUIRE( test1Parameters.arrayExamples.meshesExample().size() == 2 );
		REQUIRE_FALSE( test1Parameters.arrayExamples.meshesExample()[1].IsEmpty() );

		REQUIRE_FALSE( groom[ImageNodeName].GetParameterValueByIndex<TextureMapParameter>( 0 ).IsEmpty() );
	}

	FileRemove( FileNameToWideChar( ImageFilename ) );
}

TEST_CASE( "Groom Serialize EditGuidesShape Changes" )
{
	auto const extension = GENERATE( ".usda", ".yaml" );

	{
		Graph groom
		{
			{ StartNodeName, Operators::Mesh },
			{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
			{ EditGuidesNodeName, Operators::EditGuides }
		};

		// Base mesh
		{
			auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
			groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );
		}

		// GuidesFromMesh settings
		{
			auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
			gfmParameters.guideLength = 1.0f;
			gfmParameters.pointCount = 2;
			auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
			gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
		}

		// Guides delta shape changes
		{
			auto const& parameters = *groom[EditGuidesNodeName].GetOperator().GetParameterSet<GuidesEditorParameters>();
			auto const& delta = *parameters.delta();
			delta.ChangeGuideShapeSurface( { 0, Vector2f::Zero() }, vector{ Vector3::Zero(), Vector3::XAxis() } );
		}

		groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, EditGuidesNodeName } );

		FileRemove( TestFilename + extension );
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension ) == TestFilename + extension );
	}

	{
		auto const groomPtr = DeserializeGroomFromFile( TestFilename + extension );
		REQUIRE( groomPtr != nullptr );
		auto const& groom = *groomPtr;

		auto const& parameters = *groom[EditGuidesNodeName].GetOperator().GetParameterSet<GuidesEditorParameters>();
		auto const& delta = *parameters.delta();
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceShapeChanges ) == 1 );
	}
}

TEST_CASE( "Groom Serialize SurfaceComb" )
{
	auto const extension = GENERATE( ".usda", ".yaml" );

	vector<Vector3> originalVertices, loadedVertices;

	{
		Graph groom
		{
			{ StartNodeName, Operators::Mesh },
			{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
			{ SurfaceCombNodeName, Operators::SurfaceComb }
		};

		// Base mesh
		{
			auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
			groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );
		}

		// GuidesFromMesh settings
		{
			auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
			gfmParameters.guideLength = 1.0f;
			gfmParameters.pointCount = 2;
			auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
			gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
		}

		// Surface comb parameters
		{
			auto& op = *static_cast<SurfaceComber*>( groom[SurfaceCombNodeName].GetOperator().GetImplementation() );
			auto const sinkIndex = op.AddSink();
			op.SetSinkCoordinate( sinkIndex, { 0, { 0.5f, 0.5f } } );
			op.SetSinkDirection( sinkIndex, Vector3::XAxis() );
		}

		groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, SurfaceCombNodeName } );

		REQUIRE( Evaluator{ groom }.Evaluate() );
		originalVertices = GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object );

		FileRemove( TestFilename + extension );
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension ) == TestFilename + extension );
	}

	{
		auto groomPtr = DeserializeGroomFromFile( TestFilename + extension );
		REQUIRE( groomPtr != nullptr );
		auto& groom = *groomPtr;

		REQUIRE( Evaluator{ groom }.Evaluate() );
		loadedVertices = GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object );
	}

	REQUIRE_THAT( originalVertices, IsVectorNearEqual( loadedVertices ) );
}

TEST_CASE( "Groom Serialize Clump" )
{
	auto const extension = GENERATE( ".usda", ".yaml" );

	vector<Vector3> originalVertices, loadedVertices;

	{
		Graph groom
		{
			{ StartNodeName, Operators::Mesh },
			{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
			{ ClumpNodeName, Operators::Clump }
		};

		// Base mesh
		auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
		groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );

		// GuidesFromMesh settings
		{
			auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
			gfmParameters.guideLength = 1.0f;
			gfmParameters.pointCount = 2;
			auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
			gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
		}

		// Clump parameters
		{
			SharedHair hair;
			GenerateVertexHairs( hair, 1, planeMesh.get() );

			auto& op = *static_cast<Clumper*>( groom[ClumpNodeName].GetOperator().GetImplementation() );
			op.GenerateRandomClumps( hair, 1, 1, nullptr, planeMesh.get() );
		}

		groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, ClumpNodeName } );

		REQUIRE( Evaluator{ groom }.Evaluate() );
		originalVertices = GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object );

		FileRemove( TestFilename + extension );
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension ) == TestFilename + extension );
	}

	{
		auto groomPtr = DeserializeGroomFromFile( TestFilename + extension );
		REQUIRE( groomPtr != nullptr );
		auto& groom = *groomPtr;

		REQUIRE( Evaluator{ groom }.Evaluate() );
		loadedVertices = GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object );
	}

	REQUIRE_THAT( originalVertices, IsVectorNearEqual( loadedVertices ) );
}

TEST_CASE( "Groom Serialize CurrentTime Parameter" )
{
	String contents;
	for( auto const serializer : TheOrnatrixLibrary.grooms->GetGroomSerializers() )
	{
		{
			Graph groom
			{
				{ StartNodeName, Operators::Mesh },
				{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
				{ Noise1NodeName, Operators::Noise },
			};

			auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
			groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );
			groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, Noise1NodeName } );

			REQUIRE( groom.GetNodeCount() == 4 );
			REQUIRE( groom.AreConnected( { groom.GetCurrentTimeNode(), ParameterId::_1 }, { Noise1NodeName, NoiseParameters::CurrentTime::Id } ) );
			contents = TheOrnatrixLibrary.grooms->SerializeGroomToBuffer( groom, TimeUndefined, serializer );
			REQUIRE_FALSE( contents.empty() );
		}

		{
			auto groomPtr = DeserializeGroomFromFileContents( contents );
			REQUIRE( groomPtr != nullptr );
			REQUIRE( groomPtr->GetNodeCount() == 4 );
			REQUIRE( groomPtr->AreConnected( { groomPtr->GetCurrentTimeNode(), ParameterId::_1 }, { Noise1NodeName, NoiseParameters::CurrentTime::Id } ) );
		}
	}
}

bool AreEqual( NurbsCurve<3, Real> const& left, NurbsCurve<3, Real> const& right )
{
	return left.degree() == right.degree() && left.ctrlPnts() == right.ctrlPnts() && left.knot() == right.knot();
}

TEST_CASE( "Groom Serialize Curves" )
{
	auto const [extension, expectedCrc] = GENERATE( make_pair( ".usda", 0xc3a667d ), make_pair( ".yaml", 0x43cd8cb4 ) );

	auto originalCurves = make_shared<SharedNurbsCurves>();
	originalCurves->AddCurve( GenerateCurve( -0.5_r ) );
	originalCurves->AddCurve( GenerateCurve( 0.5_r ) );
	{
		Graph groom
		{
			{ Test1NodeName, Operators::Curves },
		};

		groom[Test1NodeName].GetOperator().SetGeneratorValue( CurvesParameter{ originalCurves, Xform3::Translation( Vector3( 3, 0, 0 ) ) } );

		FileRemove( TestFilename + extension );
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension ) == TestFilename + extension );
		RequireTextFileCrc( TestFilename + extension, expectedCrc );
	}

	{
		auto groomPtr = DeserializeGroomFromFile( TestFilename + extension );
		REQUIRE( groomPtr != nullptr );
		auto& groom = *groomPtr;
		REQUIRE( groom.GetNodeCount() == 2 );
		auto curves = groom[Test1NodeName].GetOperator().GetGeneratorValue<CurvesParameter>();
		REQUIRE( curves->GetObjectToWorldTransform().translation() == Vector3( 3, 0, 0 ) );
		REQUIRE( curves->IsModifiable() );
		auto nurbs = SharedNurbsCurves::Create( **curves );
		REQUIRE( nurbs.GetSplineCount() == 2 );
		REQUIRE( AreEqual( nurbs[0], ( *originalCurves )[0] ) );
		REQUIRE( AreEqual( nurbs[1], ( *originalCurves )[1] ) );
	}
}

TEST_CASE( "Groom serialize sphere and move generators to the front" )
{
	auto const extension = GENERATE( ".usda", ".yaml" );

	{
		Graph groom
		{
			{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
			{ Test1NodeName, Operators::Sphere },
		};

		groom[Test1NodeName].SetParameterValueById( ParameterId::_2, 13.0f );

		groom.AutoConnect( { Test1NodeName, GuidesFromMeshNodeName } );

		FileRemove( TestFilename + extension );
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension ) == TestFilename + extension );
	}

	{
		auto const groomPtr = DeserializeGroomFromFile( TestFilename + extension );
		REQUIRE( groomPtr != nullptr );
		auto const& groom = *groomPtr;

		// The generator node was moved to the front. 0 is currentTime, 1 should be the sphere.
		REQUIRE( groom.GetNodes()[1]->GetName() == Test1NodeName );
		REQUIRE( groom.GetNodes()[1]->GetDescriptor() == Operators::Sphere );

		REQUIRE( groom[Test1NodeName].GetParameterValueByIndex<Real>( 1 ) == 13.0f );
	}
}

TEST_CASE( "Groom Serialize Unconnected Mesh Array" )
{
	Graph const groom
	{
		{ Test1NodeName, *Operators::Test },
	};

	auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
	planeMesh->ValidatePolygonVertexNormals();
	groom[Test1NodeName].GetOperator().GetParameterSet<TestParameters>()->arrayExamples.meshesExample = { PolygonMeshParameter{ planeMesh, Xform3::Translation( Vector3( 5, 0, 0 ) ) } };
	REQUIRE_LOG_WARNING_BLOCK( L"Parameter must be wired to a generator node to be converted into USD, parameter TestParameters:ArrayExamples.MeshesExample" )
	{
		FileRemove( TestFilename + ".usda" );
		TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + ".usda" );
	}
}

TEST_CASE( "Groom Serialize BakedHair" )
{
	auto const extension = GENERATE( ".usda", ".yaml" );

	{
		Graph groom
		{
			{ StartNodeName, Operators::Mesh },
			{ BakedHairNodeName, Operators::BakedHair },
		};

		{
			auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 10, 2, 2 ) );
			groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );

			auto& parameters = *groom[BakedHairNodeName].GetOperator().GetParameterSet<BakedHairParameters>();
			auto hair = make_shared<SharedHair>();
			GenerateVertexHairs( *hair, 4, planeMesh.get() );
			parameters.inputHair = hair;
			groom.AutoConnect( { StartNodeName, BakedHairNodeName } );
		}

		FileRemove( TestFilename + extension );
		REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, TestFilename + extension ) == TestFilename + extension );
	}

	{
		auto groomPtr = DeserializeGroomFromFile( TestFilename + extension );
		REQUIRE( groomPtr != nullptr );
		auto& groom = *groomPtr;

		REQUIRE( Evaluator{ groom }.Evaluate() );
		REQUIRE( GetEndPointHair( groom ).IsModifiable() );
	}
}

TEST_CASE( "Groom Evaluation" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ Curl1NodeName, Operators::Curl },
		{ Noise1NodeName, Operators::Noise },
	};

	auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 4 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Translation( { 1, 2, 3 } ) } );

	auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
	auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	auto& curlParameters = *groom[Curl1NodeName].GetOperator().GetParameterSet<CurlParameters>();
	auto& noiseParameters = *groom[Noise1NodeName].GetOperator().GetParameterSet<NoiseParameters>();

	gfmRootParameters.rootCount = 3;
	gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::RandomFace;
	gfmParameters.pointCount = 2;
	gfmParameters.lengthRandomness = 0;
	noiseParameters.amount.value = 0;
	curlParameters.magnitude.value = 0;
	curlParameters.noise.amount = 0;

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, Curl1NodeName, Noise1NodeName } );

	Array<String> evaluatedNodes;
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
	auto resultHair = GetEndPointHair( groom );
	REQUIRE_FALSE( resultHair.IsEmpty() );
	REQUIRE( resultHair->GetStrandCount() == 3 );
	REQUIRE( evaluatedNodes == Array{ "start", "guidesFromMesh", "curl1", "currentTime", "noise1" } );

	{
		// It's OK to reuse the evaluator
		Evaluator evaluator{ groom };

		// Repeated evaluation does nothing...
		REQUIRE( evaluator.Evaluate( &evaluatedNodes, 1 ) );
		REQUIRE( evaluatedNodes.empty() );

		// ...unless a node was dirtied
		groom[StartNodeName].SetDirty();
		REQUIRE( evaluator.Evaluate( &evaluatedNodes, 1 ) );
		REQUIRE( evaluatedNodes == Array{ "start", "guidesFromMesh", "curl1", "noise1" } );

		// Disabled nodes are not evaluated, just cleaned
		// Enabling/disabling a node triggers evaluation only for its dependent nodes
		groom[Curl1NodeName].SetEnabled( false );
		REQUIRE( groom[Curl1NodeName].IsDirty() );
		REQUIRE( evaluator.Evaluate( &evaluatedNodes, 1 ) );
		REQUIRE( evaluatedNodes == Array{ "noise1" } );
		REQUIRE_FALSE( groom[Curl1NodeName].IsDirty() );

		groom[Curl1NodeName].SetEnabled( true );
		REQUIRE( evaluator.Evaluate( &evaluatedNodes, 1 ) );
		REQUIRE( evaluatedNodes == Array{ "curl1", "noise1" } );

		// It's OK to reuse the evaluator, as long as the graph topology doesn't change
		groom.RemoveNode( Curl1NodeName );
		REQUIRE_LOG_WARNING_BLOCK( L"Graph node count has changed, expected 5, actual 4" )
		{
			REQUIRE_FALSE( evaluator.Evaluate() );
		}
	}

	// Removing a node does not automatically rewire the connections around it, need to do it explicitly
	groom[GuidesFromMeshNodeName].SetDirty();
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
	REQUIRE( evaluatedNodes == Array{ "guidesFromMesh" } );
	groom.AutoConnect( { GuidesFromMeshNodeName, Noise1NodeName } );
	groom[GuidesFromMeshNodeName].SetDirty();
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
	REQUIRE( evaluatedNodes == Array{ "guidesFromMesh", "noise1" } );

	// Changing a parameter dirties its node
	REQUIRE_FALSE( groom[GuidesFromMeshNodeName].IsDirty() );
	gfmRootParameters.rootCount = 4;
	REQUIRE( groom[GuidesFromMeshNodeName].IsDirty() );
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
	REQUIRE( evaluatedNodes == Array{ "guidesFromMesh", "noise1" } );
	REQUIRE( GetEndPointHair( groom )->GetStrandCount() == 4 );
}

TEST_CASE( "Groom Reevaluate GuidesFromMesh" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
	};

	groom.AutoConnect( StartNodeName, GuidesFromMeshNodeName );

	auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );

	auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
	gfmParameters.guideLength = 1.0f;
	auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Even;

	Evaluator evaluator{ groom };
	REQUIRE( evaluator.Evaluate() );
	auto resultHair = GetEndPointHair( groom );
	REQUIRE( resultHair->GetStrandCount() == 300 );

	gfmParameters.useTriangularRootGeneration = true;
	for( auto iteration = 0; iteration < 2; ++iteration )
	{
		REQUIRE( evaluator.Evaluate() );
		REQUIRE( resultHair->GetStrandCount() == 300 );
	}

	gfmParameters.useTriangularRootGeneration = false;
	for( auto iteration = 0; iteration < 2; ++iteration )
	{
		REQUIRE( evaluator.Evaluate() );
		REQUIRE( resultHair->GetStrandCount() == 300 );
	}
}

TEST_CASE( "Groom unique node names" )
{
	Graph groom;
	auto node = &groom.AddNode( Curl1NodeName, Operators::Curl );
	REQUIRE( node->GetName() == Curl1NodeName );
	node = &groom.AddNode( Curl1NodeName, Operators::Curl );
	REQUIRE( node->GetName() == string( "curl2" ) );

	REQUIRE( groom.RenameNode( *node, "curl1" )->GetName() == "curl2" );

	groom.AddNode( "test", Operators::Curl );
	REQUIRE( groom.RenameNode( *node, "test" )->GetName() == "test1" );
}

TEST_CASE( "Groom Disabled Node" )
{
	for ( auto const serializer : TheOrnatrixLibrary.grooms->GetGroomSerializers() )
	{
		String contents;
		vector<Vector3> verticesNoCurl;
		{
			Graph groom
			{
				{ StartNodeName, Operators::Mesh },
				{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
			};

			groom.AutoConnect( StartNodeName, GuidesFromMeshNodeName );

			auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 1 ) );
			groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh } );

			auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
			gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;
			auto& gfmParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<GuidesFromMeshParameters>();
			gfmParameters.pointCount = 5;

			REQUIRE( Evaluator{ groom }.Evaluate() );
			verticesNoCurl = GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object );

			groom.AddNode( Curl1NodeName, Operators::Curl );
			groom.AutoConnect( GuidesFromMeshNodeName, Curl1NodeName );
			Evaluator evaluator{ groom };
			REQUIRE( evaluator.Evaluate() );
			auto verticesCurl = GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object );
			REQUIRE_THAT( verticesCurl, !IsVectorNearEqual( verticesNoCurl ) );

			groom[Curl1NodeName].SetEnabled( false );
			REQUIRE( evaluator.Evaluate() );
			REQUIRE( GetEndPointHair( groom )->GetVertices( IHair::CoordinateSpace::Object ) == verticesNoCurl );

			contents = TheOrnatrixLibrary.grooms->SerializeGroomToBuffer( groom, TimeUndefined, serializer );
			REQUIRE_FALSE( contents.empty() );
		}

		{
			auto groomPtr = DeserializeGroomFromFileContents( contents );
			REQUIRE( groomPtr != nullptr );
			REQUIRE( groomPtr->FindNode( Curl1NodeName ) != nullptr );
			REQUIRE( Evaluator{ *groomPtr }.Evaluate() );
			auto resultHair = GetEndPointHair( *groomPtr );
			REQUIRE_THAT( resultHair->GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( verticesNoCurl ) );
		}
	}
}

TEST_CASE( "CoordinateSystemProperties::GetTransformTo is bidirectional" )
{
	REQUIRE( ( CoordinateSystemMaya().GetTransformTo( CoordinateSystemUnreal() ) * CoordinateSystemUnreal().GetTransformTo( CoordinateSystemMaya() ) ).IsIdentity() );
	REQUIRE( ( CoordinateSystemMaya().GetTransformTo( CoordinateSystemC4D() ) * CoordinateSystemC4D().GetTransformTo( CoordinateSystemMaya() ) ).IsIdentity() );
	REQUIRE( ( CoordinateSystemMaya().GetTransformTo( CoordinateSystem3dsMax() ) * CoordinateSystem3dsMax().GetTransformTo( CoordinateSystemMaya() ) ).IsIdentity() );
}

TEST_CASE( "CoordinateSystemProperties::GetTransformTo Unreal to Maya" )
{
	auto const unrealToMayaTransform = CoordinateSystemUnreal().GetTransformTo( CoordinateSystemMaya() );
	auto const xVector = unrealToMayaTransform * -Vector3::XAxis();

	REQUIRE_THAT( xVector, IsNearEqual( Vector3::XAxis() ) );
}

TEST_CASE( "Groom Change Coordinate System" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
	};

	auto planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 4 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Translation( { 1, 2, 3 } ) } );

	groom.AutoConnect( StartNodeName, GuidesFromMeshNodeName );

	REQUIRE( groom.GetCoordinateSystemProperties() == Groom::CoordinateSystemProperties{} );
	groom.SetCoordinateSystemProperties( CoordinateSystem3dsMax() );
	REQUIRE( groom.GetCoordinateSystemProperties() == Groom::CoordinateSystem3dsMax() );
	auto const& transform = groom[StartNodeName].GetOperator().GetGeneratorValue<PolygonMeshParameter>()->GetObjectToWorldTransform();
	//REQUIRE( Usd::SerializeGroom( groom, L"CoordSystem_3dsMax.usda" ) );

	groom.SetCoordinateSystemProperties( CoordinateSystemMaya() );
	REQUIRE( groom.GetCoordinateSystemProperties() == Groom::CoordinateSystemMaya() );
	REQUIRE( transform == Xform3{ 1, 0, 0, 0, 0, -1, 0, 1, 0, 1, 3, -2 } );
	//REQUIRE( Usd::SerializeGroom( groom, L"CoordSystem_Maya.usda" ) );

	groom.SetCoordinateSystemProperties( CoordinateSystem3dsMax() );
	REQUIRE( groom.GetCoordinateSystemProperties() == Groom::CoordinateSystem3dsMax() );
	REQUIRE( transform == Xform3::Translation( { 1, 2, 3 } ) );

	groom.SetCoordinateSystemProperties( CoordinateSystemUnreal() );
	REQUIRE( groom.GetCoordinateSystemProperties() == Groom::CoordinateSystemUnreal() );
	REQUIRE( transform == Xform3{ -1, 0, 0, 0, -1, 0, 0, 0, 1, -1, -2, 3 } );

	auto const extension = GENERATE( ".usda"s, ".yaml"s );
	REQUIRE( TheOrnatrixLibrary.grooms->SerializeGroomToFile( groom, "CoordSystem_Unreal" + extension ) == "CoordSystem_Unreal" + extension );
}

void SetPlaneSurface( Graph const& groomGraph, char const* surfaceNodeName = StartNodeName, Real planeSize = 1_r, Xform3 const& objectToWorldTransform = Xform3::Identity() )
{
	auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( planeSize ) );
	groomGraph[surfaceNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, objectToWorldTransform } );
}

TEST_CASE( "Groom Serialize Parameter Script" )
{
	static auto const ScriptString = fmt::format( R"(Parameters = (
		( 'PointCount', 'int', lambda gm, v : gm.set_value( '{0}', 'PointCount', v ) ),
		( 'PointCount2', 'int', lambda gm, v : gm.set_value( '{0}', 'PointCount', v ) )
))", GuidesFromMeshNodeName );

	String contents;
	{
		Graph groom
		{
			{ StartNodeName, Operators::Mesh },
			{ GuidesFromMeshNodeName, Operators::GuidesFromMesh }
		};

		SetPlaneSurface( groom );
		groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName } );

		groom.SetParametersScript( ScriptString.c_str() );
		contents = TheOrnatrixLibrary.grooms->SerializeGroomToBuffer( groom );
		REQUIRE_FALSE( contents.empty() );
	}

	{
		auto const groomPtr = DeserializeGroomFromFileContents( contents );
		REQUIRE( groomPtr != nullptr );
		REQUIRE( ScriptString == groomPtr->GetParametersScript() );
	}
}

TEST_CASE( "Groom Graph Iteration" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
	};

	auto count = 0;
	for( auto const& node : groom )
	{
		REQUIRE( !node.GetName().empty() );
		++count;
	}

	REQUIRE( count == 3 );
	REQUIRE( groom.begin() != groom.end() );
	REQUIRE( ++ ++ ++groom.begin() == groom.end() );
	REQUIRE( --groom.end() == ++ ++groom.begin() );
}

TEST_CASE( "Groom Connections" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ Curl1NodeName, Operators::Curl },
		{ Noise1NodeName, Operators::Noise },
	};

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, Curl1NodeName, Noise1NodeName } );
	REQUIRE( groom[Curl1NodeName].GetInputConnections().size() == 2 );
	REQUIRE( groom.AreConnected( { groom[GuidesFromMeshNodeName], GuidesFromMeshParameters::OutputGuides() }, { Curl1NodeName, CurlParameters::TargetHair::Id } ) );
	REQUIRE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Curl1NodeName, CurlParameters::DistributionMesh::Id } ) );
	REQUIRE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Noise1NodeName, NoiseParameters::DistributionMesh::Id } ) );

	groom[Curl1NodeName].BreakInputConnection( { groom[Curl1NodeName], CurlParameters::TargetHair() } );
	REQUIRE( groom[Curl1NodeName].GetInputConnections().size() == 1 );
	REQUIRE_FALSE( groom.AreConnected( { groom[GuidesFromMeshNodeName], GuidesFromMeshParameters::OutputGuides() }, { Curl1NodeName, CurlParameters::TargetHair::Id } ) );

	groom[Curl1NodeName].DisconnectInputs();
	REQUIRE( groom[Curl1NodeName].GetInputConnectionCount() == 0 );
	REQUIRE_FALSE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Curl1NodeName, CurlParameters::DistributionMesh::Id } ) );

	groom[StartNodeName].DisconnectOutputs();
	REQUIRE( groom[StartNodeName].GetOutputConnectionCount() == 0 );
	REQUIRE_FALSE( groom.AreConnected( { StartNodeName, ParameterId::_1 }, { Noise1NodeName, NoiseParameters::DistributionMesh::Id } ) );
}

TEST_CASE( "Groom Remove Node" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ HairFromGuidesNodeName, Operators::HairFromGuides },
		{ Curl1NodeName, Operators::Curl },
		{ Noise1NodeName, Operators::Noise },
	};

	SetPlaneSurface( groom, StartNodeName, 4, Xform3::Translation( { 1, 2, 3 } ) );

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, HairFromGuidesNodeName, Curl1NodeName, Noise1NodeName } );

	REQUIRE( groom.GetNodeCount() == 6 );
	REQUIRE( FindEndPointHairNode( groom ) == &groom[Noise1NodeName] );

	REQUIRE( groom.RemoveNode( Noise1NodeName ) );

	REQUIRE( groom.GetNodeCount() == 5 );
	REQUIRE( FindEndPointHairNode( groom ) == &groom[Curl1NodeName] );
}

TEST_CASE( "Groom Guides Output" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ Curl1NodeName, Operators::Curl },
	};

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, Curl1NodeName } );

	SetPlaneSurface( groom );

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto const resultHair = GetEndPointHair( groom );
	REQUIRE( CheckedStaticCast<CommonHairSA const&>( *resultHair ).isGuides );
}

TEST_CASE( "Groom Hair Output" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ HairFromGuidesNodeName, Operators::HairFromGuides }
	};

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, HairFromGuidesNodeName } );

	SetPlaneSurface( groom );
	auto& hfgRootParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	hfgRootParameters.rootCount = 3;

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto const resultHair = GetEndPointHair( groom );
	REQUIRE( !CheckedStaticCast<CommonHairSA const&>( *resultHair ).isGuides );
}

TEST_CASE( "Groom EditGuides on top of Mesh" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ EditGuidesNodeName, Operators::EditGuides }
	};

	auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 4 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Translation( { 1, 2, 3 } ) } );
	groom.AutoConnect( { StartNodeName, EditGuidesNodeName } );

	Array<String> evaluatedNodes;
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
}

TEST_CASE( "Groom EG on top of Mesh multiple evaluations" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ EditGuidesNodeName, Operators::EditGuides }
	};

	auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 4 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Translation( { 1, 2, 3 } ) } );
	groom.AutoConnect( { StartNodeName, EditGuidesNodeName } );

	auto& egParameters = *groom[EditGuidesNodeName].GetOperator().GetParameterSet<GuidesEditorParameters>();

	SharedHair hair2;
	GenerateHairs( hair2, array{ SurfacePosition { 0, { 0.0f, 0.0f } } } );
	egParameters.delta = make_shared<GuidesDelta>( SharedHair(), hair2 );

	Array<String> evaluatedNodes;

	// One strand added after first evaluation
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
	REQUIRE( GetEndPointHair( groom )->GetStrandCount() == 1 );

	groom[EditGuidesNodeName].SetDirty();

	// One strand added after second evaluation
	REQUIRE( Evaluator{ groom }.Evaluate( &evaluatedNodes, 1 ) );
	REQUIRE( GetEndPointHair( groom )->GetStrandCount() == 1 );
}

TEST_CASE( "Groom bake GFM" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh }
	};

	auto const planeMesh = make_shared<SharedPolygonMesh>( GeneratePlane<Real>( 4 ) );
	groom[StartNodeName].GetOperator().SetGeneratorValue( PolygonMeshParameter{ planeMesh, Xform3::Identity() } );
	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName } );
	auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::FaceCenter;

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto const strandCountBeforeBaking = GetEndPointHair( groom )->GetStrandCount();
	REQUIRE( strandCountBeforeBaking == 1 );

	REQUIRE( groom.BakeNodes( 2, 1 ) );

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto const strandCountAfterBaking = GetEndPointHair( groom )->GetStrandCount();
	REQUIRE( strandCountAfterBaking == 1 );
}

TEST_CASE( "Groom Implicit Guides Connection" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ HairFromGuidesNodeName, Operators::HairFromGuides },
		{ ClumpNodeName, Operators::Clump },
	};

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, HairFromGuidesNodeName, ClumpNodeName } );

	SetPlaneSurface( groom );

	auto& gfmRootParameters = *groom[GuidesFromMeshNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	gfmRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Vertex;

	auto& hfgRootParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	hfgRootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::RandomArea;
	hfgRootParameters.rootCount = 40;

	auto& hfgParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<HairFromGuidesParameters>();
	hfgParameters.generatedStrandFraction = 1.0f;
	hfgParameters.previewStrandFraction = 1.0f;

	auto& clumpParameters = *groom[ClumpNodeName].GetOperator().GetParameterSet<ClumpParameters>();
	clumpParameters.clumpCreateMethod = ClumpParameters::ClumpCreateMethodType::Guides;
	clumpParameters.flyAway.fraction = 0.0f;

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto resultHair = GetEndPointHair( groom );

	auto& clumper = dynamic_cast<Operators::ClumpOperator&>( groom[ClumpNodeName].GetOperator() ).op;
	clumper.GenerateClumps( *resultHair, 1 );

	REQUIRE( Evaluator{ groom }.Evaluate() );
	resultHair = GetEndPointHair( groom );

	// If the hair is clumped to guides we will have 4 unique clumps since we have 4 guides (one per each mesh vertex)
	REQUIRE( resultHair->GetStrandCount() > 6 );
	REQUIRE( GetUniquePoints( HairUtilities( *resultHair ).GetTipPositions(), 0.001f ).size() == 4 );
}

TEST_CASE( "Groom distribution map evaluation" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
		{ HairFromGuidesNodeName, Operators::HairFromGuides },
	};

	groom.AutoConnect( { StartNodeName, GuidesFromMeshNodeName, HairFromGuidesNodeName } );
	SetPlaneSurface( groom );

	auto& hfgRootParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<RootGeneratorParameters>();
	hfgRootParameters.rootCount = 600;

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto const strandCountBeforeMap = GetEndPointHair( groom )->GetStrandCount();
	REQUIRE( strandCountBeforeMap > 0 );

	auto& hfgParameters = *groom[HairFromGuidesNodeName].GetOperator().GetParameterSet<HairFromGuidesParameters>();
	std::vector<PixelGray16> pixels{ 65534, 65533, 0, 0 };
	hfgParameters.distributionMaps.first = { make_shared<TextureMapImage>( ImageGray16{ pixels } ), 0, true };

	REQUIRE( Evaluator{ groom }.Evaluate() );
	auto const strandCountAfterMap = GetEndPointHair( groom )->GetStrandCount();
	REQUIRE( strandCountBeforeMap != strandCountAfterMap );
}

TEST_CASE( "TextureMapImage To/From Png" )
{
	static constexpr wchar_t ImageFilename[] = L"ToFromPng.png";

	PixelGray16 constexpr pixelValue = 32768;
	REQUIRE( TextureMapImage{ ImageGray16{ pixelValue, pixelValue, pixelValue, pixelValue } }.ToPng( ImageFilename ) );

	auto const loadedImage = TextureMapImage::FromPng( ImageFilename );
	auto const firstRow = loadedImage.GetImage().As<PixelGray16>().GetRowPixels( 0 );

	REQUIRE( firstRow[0] == pixelValue );

	auto const sample = loadedImage.Sample( TextureCoordinate( 0.0f, 0.0f, 0.0f ) );
	REQUIRE( sample.Red() == pixelValue );
}

TEST_CASE( "Groom Performance", "[.Performance]" )
{
	auto test = []( std::string_view name )
	{
		Profile::Stopwatch timer;
		auto const readGroom = static_unique_cast<Graph>( TheOrnatrixLibrary.grooms->DeserializeGroomFromFile( name ) );
		INFO( name );
		REQUIRE( readGroom != nullptr );
		std::cout << name << "  read: " << timer.ElapsedMilliseconds() << '\n';

#if 0
		{
			timer.Start();
			GroomSerializeSettings const settings{ TimeUndefined, false,true, false };
			String outputFile = "test.oxg.yaml";
			SerializedGroom groom;
			TheOrnatrixLibrary.grooms->SerializeGroom( *readGroom, outputFile, settings, groom );
			std::cout << name << " write: " << timer.ElapsedMilliseconds() << '\n';
		}
#endif
	};

	for( auto i = 0; i < 3; ++i )
	{
		test( "../../External/Grooms/1/MediumHair.oxg.zip" );
		//test( "SampleGroom.oxg.yaml" );
		//test( "../../External/Grooms/1/MediumHair.oxg.usdz" );
	}
}
