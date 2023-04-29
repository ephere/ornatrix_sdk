#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "Ephere/Geometry/Native/PolygonMeshUtilities.h"
#include "Ephere/Geometry/Native/SharedNurbsCurves.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/IHair.h"
#include "Ephere/Ornatrix/Ornatrix.h"
#include "Ephere/Ornatrix/ParameterComponents.h"
#include "Ephere/Ornatrix/Operators/GroomerParameters.g.h"
#include "Ephere/Ornatrix/Operators/GuidesFromCurvesParameters.g.h"

#include <iostream>

using namespace Ephere;
using namespace Geometry;
using namespace Ornatrix;
using namespace std;

namespace Ephere::Ornatrix
{
OrnatrixLibrary TheOrnatrixLibrary;
}

TEST_CASE( "ParameterTypes" )
{
	SECTION( "Ramp" )
	{
		REQUIRE( Ramp( 1 ).IsNearEqual( Ramp{ { 0, 1 }, { 0.6_r, 1 }, { 1, 1 } } ) );
		REQUIRE( Ramp( 1 ).Evaluate( 0.5f ) == 1 );
	}
}

TEST_CASE( "TypeId_CrossModule_Compatibility" )
{
	auto const descriptor = TheOrnatrixLibrary.grooms->GetOperatorRegistry().Find( "GroomOperator" );
	{
		auto const parameterDescriptor = descriptor->parameterSetDescriptors[0]->GetParameterById( GroomerParameters::ScaleFactor::Id );
		REQUIRE( AreSameType( parameterDescriptor->GetTypeId(), Parameters::GetTypeId<float>() ) );
	}
	{
		auto const parameterDescriptor = descriptor->parameterSetDescriptors[0]->GetParameterById( GroomerParameters::GroomFile::Id );
		REQUIRE( AreSameType( parameterDescriptor->GetTypeId(), Parameters::GetTypeId<Parameters::String>() ) );
	}
	{
		auto const parameterDescriptor = descriptor->parameterSetDescriptors[0]->GetParameterById( GroomerParameters::GraftSurfaceVertexPositions::Id );
		REQUIRE( AreSameType( parameterDescriptor->GetTypeId(), Parameters::GetTypeId<SurfacePosition[]>() ) );
	}
}

TEST_CASE( "GetGroomInfo" )
{
	auto const extension = GENERATE( ".usda"s, ".yaml"s );

	GroomInfo info;
	REQUIRE( TheOrnatrixLibrary.grooms->GetGroomInfoFromFile( SampleGroomFilename + extension, info ) );
	REQUIRE( info.version == IGrooms::GroomFormatVersion );
	REQUIRE( info.timeSamples == Parameters::Array{ 0.0, 1.0 } );
}

TEST_CASE( "DeserializeGroom" )
{
	auto const extension = GENERATE( ".usda"s, ".yaml"s );

	auto groom = TheOrnatrixLibrary.grooms->DeserializeGroomFromFile( SampleGroomFilename + extension );
	REQUIRE( groom != nullptr );

	auto hairAndMesh = TheOrnatrixLibrary.grooms->EvaluateGroom( *groom );
	REQUIRE( hairAndMesh.first );
	REQUIRE( hairAndMesh.second );

	REQUIRE( hairAndMesh.first->GetStrandCount() == 30 );
	REQUIRE( hairAndMesh.second->GetVertexCount() == 25 );
}

UniquePtr<Groom::IOperator> CreateOperator( char const* name )
{
	auto const descriptor = TheOrnatrixLibrary.grooms->GetOperatorRegistry().Find( name );
	REQUIRE( descriptor );
	auto op = descriptor->operatorFactory();
	REQUIRE( op != nullptr );
	return op;
}

TEST_CASE( "Groomer" )
{
	auto const extension = GENERATE( ".usda"s, ".yaml"s );

	// TODO: Constants for the operator names in the public API
	auto const groomOperator = CreateOperator( "GroomOperator" );
	auto& groomParameters = groomOperator->GetParameterSet( 0 );
	auto const groomFileParameter = groomParameters.Get<GroomerParameters::GroomFile>();
	REQUIRE( groomFileParameter );
	*groomFileParameter = SampleGroomFilename + extension;

	auto const hair = TheOrnatrixLibrary.library->CreateHair( true ).ToShared();
	REQUIRE( hair );
	REQUIRE( hair->IsEmpty() );
	*groomParameters.Get<GroomerParameters::TargetHair>() = hair;

	REQUIRE( groomOperator->Apply() );
	REQUIRE_FALSE( hair->IsEmpty() );
}

TEST_CASE( "AdoptExternalGuides" )
{
	auto const op = CreateOperator( "AdoptExternalGuidesOperator" );

	SECTION( "No input" )
	{
		REQUIRE_FALSE( op->Apply() );
	}
}

TEST_CASE( "ChangeWidth" )
{
	auto const op = CreateOperator( Groom::OperatorName::ChangeWidth() );

	SECTION( "No input" )
	{
		// Doesn't crash
		op->Apply();
	}
}

TEST_CASE( "GuidesFromCurves" )
{
	constexpr auto Count = 1000;

	auto const op = CreateOperator( Groom::OperatorName::GuidesFromCurves() );
	auto& parameters = op->GetParameterSet( 0 );

	SECTION( "ParallelEvaluate" )
	{
		SECTION( "Single curve" )
		{
			auto const curves = SharedPtr<SharedNurbsCurves>::DefaultConstruct();
			for( auto i = 0; i < Count; ++i )
			{
				curves->AddCurve( GenerateCurve( static_cast<Real>( i ) ) );
			}

			parameters.Set<GuidesFromCurvesParameters::InputCurves>( { curves } );
		}

		SECTION( "Multiple curves" )
		{
			Parameters::Array<CurvesParameter> curves;
			for( auto i = 0; i < Count; ++i )
			{
				auto const curve = SharedPtr<SharedNurbsCurves>::DefaultConstruct();
				curve->AddCurve( GenerateCurve( static_cast<Real>( i ) ) );
				curves.push_back( curve );
			}

			parameters.Set<GuidesFromCurvesParameters::InputCurves>( curves );
		}

		Groom::EvaluationContext evaluationContext =
		{
			[]( bool const initAsGuides ) -> UniquePtr<IHair>
			{
				return TheOrnatrixLibrary.library->CreateHair( initAsGuides );
			}
		};
		op->Apply( evaluationContext );

		auto const hair = parameters.Get<GuidesFromCurvesParameters::OutputGuides>()->GetValue().Get();
		REQUIRE( hair != nullptr );
		REQUIRE( hair->GetStrandCount() == Count );
		auto const pointCount = parameters.Get<GuidesFromCurvesParameters::PointCount>()->GetValue();
		for( auto i = 0; i < Count; ++i )
		{
			INFO( i );
			REQUIRE( hair->GetStrandPointCount( i ) == pointCount );
		}
	}
}

TEST_CASE( "UsdHairExport" )
{
	auto const hairOwner = TheOrnatrixLibrary.library->CreateHair( false );
	auto& hair = *hairOwner;

	auto const meshOwner = TheOrnatrixLibrary.library->CreateMesh();
	auto& distributionMesh = *meshOwner;

	PolygonMeshUtilities( distributionMesh ).GeneratePlane( 1 );
	GenerateVertexHairs( hair, 1, &distributionMesh, 0.0f, 2, -1, true, true );

	array const widths = { 0.1f, 0.2f, 0.1f, 0.2f, 0.1f, 0.2f, 0.1f, 0.2f };
	REQUIRE( hair.GetVertexCount() == Size( widths ) );
	hair.SetWidths( 0, Size( widths ), widths.data() );

	AddStrandChannel( hair, IHair::PerStrand, { 0.1f, 0.2f, 0.3f, 0.4f }, L"PerStrand0" );
	AddStrandChannel( hair, IHair::PerVertex, { 0.1f, 0.2f, 0.1f, 0.2f, 0.1f, 0.2f, 0.1f, 0.2f }, L"PerVertex0" );

	static string const TestStaticFileName = "HairCurvesStatic.usda";
	static string const TestAnimatedFileName = "HairCurvesAnimated.usda";

	auto evaluator = []( void* hostNode, double const time, double const /*completedFraction*/ )
	{
		IUsdSerializer::ExportHairOptions::EvaluateResult result;
		result.hair = static_cast<IHair*>( hostNode );
		result.transform = Xform3f::Translation( Vector3f( 1, 2, 3 + static_cast<float>( time ) ) );
		auto const width = 0.1f + static_cast<float>( time ) / 10;
		result.hair->SetWidths( 0, 1, &width );
		result.hair->SetStrandChannelData( 0, 0, width );
		return result;
	};
	IUsdSerializer::ExportHairOptions options{ evaluator, true, true, true, true, 2 };

	auto const usdSerializer = TheOrnatrixLibrary.library->GetUsdSerializer();
	REQUIRE( usdSerializer != nullptr );
	REQUIRE( usdSerializer->ExportHair( &hair, options, TestStaticFileName ) );
	RequireFileCrc( TestStaticFileName, 0x87a9ae02 );
	//FileRemove( TestStaticFileName );

	options.timeEnd = 2;
	REQUIRE( usdSerializer->ExportHair( &hair, options, TestAnimatedFileName ) );
	RequireFileCrc( TestAnimatedFileName, 0x3947655a );
	//FileRemove( TestAnimatedFileName );
}

using namespace Ephere;
using namespace Ornatrix;

int main( int argc, char* argv[] )
{
	string loadError;
	TheOrnatrixLibrary = LoadOrnatrixLibrary( Log::LogDispatcher, nullptr, ".", &loadError );
	if( TheOrnatrixLibrary.IsEmpty() )
	{
		cout << "Failed to load Ornatrix shared library: " << loadError << '\n';
		return 1;
	}

	if( TheOrnatrixLibrary.library->LoadUsdSerializer() == nullptr )
	{
		cout << "Failed to load USD plugin: " << GetLoadDynamicLibraryError() << '\n';
		return 1;
	}

	return RunCatchTests( argc, argv );
}
