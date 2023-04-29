#include "PrecompiledHeaders.h"

#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/PolygonMeshRefiner.h"
#include "Ephere/Geometry/Native/PolygonMeshUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/Random.h"
#include "Ephere/NativeTools/SystemTools.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshGenerator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

#include <iostream>

namespace Ephere::Ornatrix
{
using namespace Geometry;

#if 0
// Disabled because it contains no meaningful testing; use for visual inspection of refined hair root distributions
TEST_CASE( "DistributionMeshSubdivision" )
{
	const auto scale = 10_r;
	SharedPolygonMesh coarseMesh;
	GenerateCylinder10( coarseMesh, scale );
	PolygonMeshRefiner refiner( coarseMesh, 2 );

	SharedHair sharedHair;
	IHair& hair = sharedHair;
	GuidesFromMeshGenerator guidesFromMeshGenerator( &hair );
	guidesFromMeshGenerator.SetDistributionMesh( &coarseMesh );

	SECTION( "EvenDistribution" )
	{
		guidesFromMeshGenerator.rootParameters.rootCount = 3000;
		guidesFromMeshGenerator.rootParameters.generationMethod = RootGeneratorParameters::GenerationMethodType::Even;
		guidesFromMeshGenerator.GenRoots();
		//guidesFromMeshGenerator.UpdateState();

		std::vector<SurfacePosition> coarseSurfacePositions( hair.GetStrandCount() );
		hair.GetSurfaceDependencies2( 0, int( coarseSurfacePositions.size() ), coarseSurfacePositions.data() );

		std::vector<Vector3> coarsePoints( coarseSurfacePositions.size() );
		PolygonMeshUtilities coarseMeshUtilities( coarseMesh );
		std::transform( coarseSurfacePositions.begin(), coarseSurfacePositions.end(), coarsePoints.begin(), [&coarseMeshUtilities]( const SurfacePosition& position )
		{
			return coarseMeshUtilities.GetSurfacePosition( position );
		} );

		std::vector<Vector3> refinedPoints( coarseSurfacePositions.size() );
		std::vector<SurfacePosition> refinedSurfacePositions( coarseSurfacePositions.size() );
		refiner.GetRefinedPoints( coarseSurfacePositions, refinedPoints, refinedSurfacePositions );

		// Test if refined points correspond to interpolated surface positions in copied mesh
		SharedPolygonMesh refinedMesh;
		refiner.CopyToMesh( refinedMesh );
		const auto refinedMeshUtilities = PolygonMeshUtilities( refinedMesh );

		std::vector<Vector3> refinedPointsFromSurfacePositions( refinedSurfacePositions.size() );
		std::transform( refinedSurfacePositions.begin(),
						refinedSurfacePositions.end(),
						refinedPointsFromSurfacePositions.begin(),
						[&refinedMeshUtilities]( const SurfacePosition& surfacePosition )
		{
			return refinedMeshUtilities.GetSurfacePosition( surfacePosition );
		} );

		REQUIRE_THAT( refinedPoints, IsVectorNearEqual( refinedPointsFromSurfacePositions ) );

		PrintMayaParticles( std::cout, coarsePoints, "coarseEvenDistribution3000" );
		PrintMayaParticles( std::cout, refinedPoints, "refinedEvenDistribution3000" );
	}
}
#endif
}
