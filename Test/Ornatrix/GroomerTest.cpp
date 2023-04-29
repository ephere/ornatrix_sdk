#include "PrecompiledHeaders.h"

#include "Ephere/Geometry/Native/PolygonMeshUtilities.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Ornatrix.h"
#include "Ephere/Ornatrix/Groom/GraftManager.h"
#include "Ephere/Ornatrix/Groom/Graph.h"
#include "Ephere/Ornatrix/Groom/Operators.h"
#include "Ephere/Ornatrix/Operators/Groomer.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshParametersImpl.g.h"
#include "Ephere/Ornatrix/Test/Utilities.h"

using namespace Ephere;
using namespace Ornatrix;
using namespace Groom;
using namespace Geometry;
using namespace std;

TEST_CASE( "Groomer Implicit Parameters" )
{
	Groomer groomer;
	auto const groom = make_shared<Graph>( Graph
	{
		{ StartNodeName, Operators::Mesh },
		{ GuidesFromMeshNodeName, Operators::GuidesFromMesh },
	} );

	groomer.SetGroom( groom );

	static auto const ScriptString = string( fmt::format(R"(Parameters = (
		( 'PointCount', 'int', lambda gm, v : gm.set_value( '{0}', 'PointCount', v ) ),
		( 'PointCount2', 'int', lambda gm, v : gm.set_value( '{0}', 'PointCount', v ) )
))", GuidesFromMeshNodeName ) );
	groomer.SetParametersScript( ScriptString );
	REQUIRE( groomer.GetScriptParameters().size() == 2 );
	constexpr auto value = 5;
	REQUIRE( groomer.SetScriptParameterValue( "PointCount", value ) );
	REQUIRE( (*groom)[GuidesFromMeshNodeName].GetParameter<GuidesFromMeshParameters::PointCount>() == value );
}

TEST_CASE( "Groom NodeRef" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
	};

	REQUIRE( NodeRef().IsEmpty() );
	REQUIRE( NodeRef().IsResolved() );

	REQUIRE_FALSE( NodeRef( "asd" ).IsResolved() );
	REQUIRE_FALSE( NodeRef( "asd" ).TryResolve( groom ) );

	REQUIRE( NodeRef( StartNodeName ).TryResolve( groom ) );
	REQUIRE( NodeRef( StartNodeName ).ToResolved( groom ).IsResolved() );
	REQUIRE( &*NodeRef( groom[StartNodeName] ) == &groom[StartNodeName] );

	REQUIRE( NodeRef( StartNodeName ).ToResolved( groom ) == NodeRef( groom[StartNodeName] ) );
	REQUIRE( NodeRef() == NodeRef( "asd" ) );
}

TEST_CASE( "Groom ParameterRef" )
{
	Graph groom
	{
		{ StartNodeName, Operators::Mesh },
	};

	using ::ParameterRef;

	REQUIRE( ParameterRef().IsEmpty() );
	REQUIRE( ParameterRef().IsResolved() );

	REQUIRE_FALSE( ParameterRef( "asd", Parameters::ParameterId::Invalid ).IsResolved() );
	REQUIRE_FALSE( ParameterRef( "asd", Parameters::ParameterId::Invalid ).TryResolve( groom ) );

	REQUIRE( ParameterRef( StartNodeName, Parameters::ParameterId::Invalid ).TryResolve( groom ) );
	REQUIRE( ParameterRef( StartNodeName, Parameters::ParameterId::Invalid ).ToResolved( groom ).IsResolved() );
}

GraftManager& GetGraftManager()
{
	static GraftManager TheGraftManager;
	return TheGraftManager;
}

struct FitBaseMeshFixture
{
	Groomer groomer;
	shared_ptr<SharedHair> hair = make_shared<SharedHair>();
	shared_ptr<SharedPolygonMesh> baseMesh;

	void Initialize()
	{
		auto const groom = make_shared<Graph>( Graph
											   {
												   { StartNodeName, Operators::Mesh },
												   { GuidesFromMeshNodeName, Operators::GuidesFromMesh },
												   { EditGuidesNodeName, Operators::EditGuides },
											   } );

		groom->AutoConnect( { StartNodeName, GuidesFromMeshNodeName, EditGuidesNodeName } );

		auto const* const guidesFromMesh = groom->FindNode( GuidesFromMeshNodeName );
		auto& guidesFromMeshOp = guidesFromMesh->GetOperator();
		guidesFromMeshOp.GetParameterSet( 0 ).GetParameterById( RootGeneratorParameters::GenerationMethod::Id )->SetValue( RootGeneratorParameters::GenerationMethodType::Vertex );
		auto* pointCountParameter = guidesFromMeshOp.GetParameterSet( 1 ).GetParameterById( GuidesFromMeshParameters::PointCount::Id );
		pointCountParameter->SetValue( 2 );

		// Make some changes to the hair using EG
		auto const delta = SharedPtr<GuidesDelta>::DefaultConstruct();
		delta->ChangeGuideShapeSurface( { 0u, { 0.0f, 0.0f } }, array{ Vector3::Zero(), Vector3::XAxis() * 0.5_r } );
		delta->ChangeGuideShapeSurface( { 0u, { 1.0f, 0.0f } }, array{ Vector3::Zero(), Vector3::YAxis() * 0.5_r } );
		delta->ChangeGuideShapeSurface( { 0u, { 1.0f, 1.0f } }, array{ Vector3::Zero(), -Vector3::XAxis() * 0.5_r } );
		delta->ChangeGuideShapeSurface( { 0u, { 0.0f, 1.0f } }, array{ Vector3::Zero(), -Vector3::YAxis() * 0.5_r } );
		auto const* const editGuides = groom->FindNode( EditGuidesNodeName );
		editGuides->GetOperator().GetParameterSet( 0 ).GetParameterById( GuidesEditorParameters::Delta::Id )->SetValue( delta );

		auto const graftSurface = make_shared<SharedPolygonMesh>( GeneratePlane( 2_r ) );
		auto const* const meshNode = groom->FindNode( StartNodeName );
		meshNode->GetOperator().SetGeneratorValue( PolygonMeshParameter{ graftSurface, Xform3::Identity() } );

		groomer.SetGroom( groom );
		groomer.InitializeLoadedGraftSurfaceVertices();

		groomer.parameters.distributionMesh.SetValue( PolygonMeshParameter{ baseMesh, Xform3::Identity() } );
		groomer.parameters.targetHair.SetValue( HairParameter{ hair, Xform3::Identity() } );
	}
};

TEST_CASE( "Groomer FitGraftSurface" )
{
	FitBaseMeshFixture fixture;

	fixture.baseMesh = make_shared<SharedPolygonMesh>( GenerateCylinder10( 5.0_r ) );
	fixture.Initialize();

	array const fitHandles1 = { SurfacePosition { 0u, { 0.5f, 0.5f } }, SurfacePosition { 4u, { 0.5f, 0.5f } } };
	REQUIRE( fixture.groomer.FitGraftSurface( fitHandles1[0], fitHandles1[1], GetGraftManager() ) );
	REQUIRE( fixture.groomer.Compute() );
	auto const verticesAfterFit1 = fixture.hair->GetVertices( IHair::CoordinateSpace::Object );

	REQUIRE( Size( verticesAfterFit1 ) == 4 * 2 );

	// Fitting to different faces should produce different resulting hair vertices
	fixture.hair->Clear();
	array const fitHandles2 = { SurfacePosition { 5u, { 0.5f, 0.5f } }, SurfacePosition { 3u, { 0.5f, 0.5f } } };
	REQUIRE( fixture.groomer.FitGraftSurface( fitHandles2[0], fitHandles2[1], GetGraftManager() ) );
	REQUIRE( fixture.groomer.Compute() );
	auto const verticesAfterFit2 = fixture.hair->GetVertices( IHair::CoordinateSpace::Object );

	REQUIRE_THAT( verticesAfterFit2, !IsVectorNearEqual( verticesAfterFit1 ) );

	// Fitting to the same surface coordinates should produce the same hair vertices
	fixture.hair->Clear();
	REQUIRE( fixture.groomer.FitGraftSurface( fitHandles1[0], fitHandles1[1], GetGraftManager() ) );
	REQUIRE( fixture.groomer.Compute() );
	auto const verticesAfterFit3 = fixture.hair->GetVertices( IHair::CoordinateSpace::Object );

	REQUIRE_THAT( verticesAfterFit3, IsVectorNearEqual( verticesAfterFit1 ) );
}

TEST_CASE( "Groomer FitBaseMesh2" )
{
	FitBaseMeshFixture fixture;

	fixture.baseMesh = make_shared<SharedPolygonMesh>( GeneratePlane( 10.0_r ) );
	fixture.Initialize();

	array const targetFitHandles1 = { SurfacePosition { 0u, { .5f, .5f } }, SurfacePosition { 0u, { .5f, 0.75f } } };
	REQUIRE( fixture.groomer.FitGraftSurface( targetFitHandles1[0], targetFitHandles1[1], GetGraftManager() ) );
	REQUIRE( fixture.groomer.Compute() );
	auto const verticesAfterFit1 = fixture.hair->GetVertices( IHair::CoordinateSpace::Object );

	// Fitting to different faces should produce different resulting hair vertices
	fixture.hair->Clear();
	array const targetFitHandles2 = { SurfacePosition { 0u, { .5f, .5f } }, SurfacePosition { 0u, { .75f, .5f } } };
	REQUIRE( fixture.groomer.FitGraftSurface( targetFitHandles2[0], targetFitHandles2[1], GetGraftManager() ) );
	REQUIRE( fixture.groomer.Compute() );
	auto const verticesAfterFit2 = fixture.hair->GetVertices( IHair::CoordinateSpace::Object );

	PolygonMeshUtilities const baseMeshUtilities( *fixture.baseMesh );
	auto const surfaceHandleTransform1 = Xform3::LookAt( baseMeshUtilities.GetSurfacePosition( targetFitHandles1[0] ),
														 ( baseMeshUtilities.GetSurfacePosition( targetFitHandles1[1] ) - baseMeshUtilities.GetSurfacePosition( targetFitHandles1[0] ) ).normalized(), Vector3::ZAxis() );

	auto const surfaceHandleTransform2 = Xform3::LookAt( baseMeshUtilities.GetSurfacePosition( targetFitHandles2[0] ),
														 ( baseMeshUtilities.GetSurfacePosition( targetFitHandles2[1] ) - baseMeshUtilities.GetSurfacePosition( targetFitHandles2[0] ) ).normalized(), Vector3::ZAxis() );
	auto const expectedHandleDiffTransform = surfaceHandleTransform2 * Inverse( surfaceHandleTransform1 );

	auto const expectedVerticesAfterFit2 = Transform( verticesAfterFit1, expectedHandleDiffTransform.VectorMultiplyFunction() );

	REQUIRE_THAT( expectedVerticesAfterFit2, IsVectorNearEqual( verticesAfterFit2 ) );
}

TEST_CASE( "Groomer IsRendering" )
{
	Groomer groomer;
	auto const groom = make_shared<Graph>( Graph
										   {
											   { StartNodeName, Operators::Mesh },
											   { GuidesFromMeshNodeName, Operators::GuidesFromMesh },
												{ HairFromGuidesNodeName, Operators::HairFromGuides }
										   } );

	groom->AutoConnect( { StartNodeName, GuidesFromMeshNodeName, HairFromGuidesNodeName } );

	auto& gfmOp = groom->FindNode( GuidesFromMeshNodeName )->GetOperator();
	{
		auto& gfmRootParameters = gfmOp.GetParameterSet( 0 );
		gfmRootParameters.GetParameterById( RootGeneratorParameters::GenerationMethod::Id )->SetValue( RootGeneratorParameters::GenerationMethodType::Vertex );
	}
	
	auto& hfgOp = groom->FindNode( HairFromGuidesNodeName )->GetOperator();
	{
		auto& hfgRootParameters = hfgOp.GetParameterSet( 0 );
		hfgRootParameters.GetParameterById( RootGeneratorParameters::GenerationMethod::Id )->SetValue( RootGeneratorParameters::GenerationMethodType::RandomFace );
		hfgRootParameters.GetParameterById( RootGeneratorParameters::RootCount::Id )->SetValue( 10 );
	}

	{
		auto& hfgParameters = hfgOp.GetParameterSet( 1 );
		//hfgParameters.GetParameterById( HairFromGuidesParameters::GeneratedStrandFraction::Id )->SetValue( 0.5f );
		hfgParameters.GetParameterById( HairFromGuidesParameters::PreviewStrandFraction::Id )->SetValue( 0.5f );
	}

	groomer.SetGroom( groom );

	groomer.parameters.distributionMesh.SetValue( PolygonMeshParameter{ make_shared<SharedPolygonMesh>( GeneratePlane( 10.0_r ) ), Xform3::Identity() } );
	groomer.parameters.targetHair.SetValue( HairParameter{ make_shared<SharedHair>(), Xform3::Identity() } );

	groomer.parameters.isRendering = false;
	REQUIRE( groomer.Compute() );
	REQUIRE( groomer.parameters.targetHair()->GetStrandCount() == 5 );

	groomer.parameters.isRendering = true;
	REQUIRE( groomer.Compute() );
	REQUIRE( groomer.parameters.targetHair()->GetStrandCount() == 10 );
}