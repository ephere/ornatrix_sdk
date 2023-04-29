#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixToXMath.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/GuidesEditor.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Private/GuideControlPointsData.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "ControlPoints Move Roots" )
{
	auto const surface = GeneratePlane( 1_r );
	GuidesEditorImplementation guidesEditor( false );
	auto& hair = guidesEditor.GetGuidesInternal();

	GenerateHairs( hair, array{ SurfacePosition{ 0, { 0.5_r, 0.5_r } } }, 1, &surface );

	// Call BeginTrackingChanges to make sure guides we just initialized get copied into "initial" guides internally
	guidesEditor.BeginTrackingChanges( {} );

	GuideControlPointsData controlPoints;

	controlPoints.InitFromGuides( hair, guidesEditor );

	controlPoints.SetPositionOnStrand( 0_r );
	controlPoints.Update( hair, guidesEditor );

	auto const tipBeforeMove = hair.GetVertex( 1, IHair::CoordinateSpace::Object );

	auto& hairVertices = hair.WriteVertices();
	auto const weights = guidesEditor.ReadVertexWeights();

	auto const firstVertexIndex = hair.GetStrandFirstVertexIndex( 0 );
	auto const verticesCount = hair.GetStrandPointCount( 0 );

	auto const strandVertices = Span{ &hairVertices[firstVertexIndex], verticesCount };
	auto const strandWeights = Span{ &weights[ firstVertexIndex ], verticesCount };

	// Move the root of the strand by 0.5 units in X direction
	controlPoints.BeginTransform( surface );
	controlPoints.Move( hair, 0, strandVertices, strandWeights, guidesEditor , ToXM( Vector3::XAxis() / 2_r ) );
	controlPoints.EndTransform();

	auto const rootSurfaceDependency = hair.GetSurfaceDependency2( 0 );

	REQUIRE_THAT( rootSurfaceDependency, IsNearEqual( SurfacePosition { 0, { 1_r, 0.5f } } ) );
	REQUIRE_THAT( tipBeforeMove, IsNearEqual( hair.GetVertex( 1, IHair::CoordinateSpace::Object ) ) );
}
}
