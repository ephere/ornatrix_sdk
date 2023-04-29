#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/MeshFromHairGenerator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

using namespace std;
using namespace Ephere::Geometry;
using namespace Ephere::Ornatrix;

TEST_CASE( "MeshFromHairGenerator" )
{
	MeshFromHairGenerator generator;

	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 10, 1, &distributionMesh, 1.0f );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	generator.parameters.inputHair = &originalHair;
	generator.parameters.distributionMesh = &distributionMesh;
	generator.Initialize();

	SECTION( "ConformToSurface" )
	{
		// Offset the vertices of the distribution mesh vertically
		{
			auto vertices = distributionMesh.GetVertices();

			auto index = 0;
			for( auto& vertex : vertices )
			{
				vertex.z() += 2.0f * static_cast<Real>( index++ % 2 );
			}

			distributionMesh.SetVertices( vertices );
		}

		generator.GenerateMesh();
		auto const verticesWithoutConformToSurface = generator.parameters.outputMesh()->GetVertices();

		generator.parameters.outputMesh = make_shared<SharedPolygonMesh>();
		generator.parameters.conformToSurface = MeshFromHairParameters::ConformToSurfaceType::SurfaceNormal;
		generator.GenerateMesh();
		auto const verticesWithConformToSurface = generator.parameters.outputMesh()->GetVertices();

		REQUIRE_THAT(
			verticesWithoutConformToSurface,
			!IsVectorNearEqual( verticesWithConformToSurface )
		);
	}

	SECTION( "ProxyMeshWithInvalidChannel" )
	{
		generator.parameters.proxy.meshes = { &distributionMesh };
		generator.parameters.proxy.inheritTextureCoordinates = false;
		generator.parameters.mappingChannelCount = 1;
		generator.parameters.mappingChannels.usePerStrandCoordinates = { true };
		generator.parameters.mappingChannels.baseChannel = { 2 };
		generator.GenerateProxyMesh();

		// We expect no texture channels to be generated because incoming base channel is invalid
		REQUIRE( generator.parameters.outputMesh()->GetTextureChannelCount() == 0 );
	}

	SECTION( "ProxyMeshWithNoVolume" )
	{
		auto const meshWithNoVolume = GeneratePlane<Real>( 0 );

		// Make sure hair has widths
		originalHair.SetUseWidths( true );
		
		generator.parameters.proxy.meshes = { &meshWithNoVolume };
		generator.GenerateProxyMesh();

		// There should be no crash for the test to pass
	}
}
