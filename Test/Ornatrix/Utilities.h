#pragma once

#include "Ephere/Core/Shared.h"
#include "Ephere/Geometry/Native/NurbsCurve.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/IFunction.h"
#include "Ephere/Ornatrix/IHair.h"

namespace Ephere::Ornatrix
{
class LineCurve final : public IFunction1
{
public:

	explicit LineCurve( Real value0Initial = 0, Real value1Initial = 1 ) :
		value0( value0Initial ),
		value1( value1Initial )
	{}

	EPHERE_NODISCARD Real Evaluate( Real position ) const override
	{
		return Interpolate( value0, value1, position );
	}

	Real value0;
	Real value1;
};

inline Geometry::NurbsCurve<3, Real> GenerateCurve( float z = 0 )
{
	std::vector const cp = { Vector4{ -1, 0, z, 1 }, Vector4{ 0, 1, z, 1 }, Vector4{ 1, 0, z, 1 } };
	std::vector<Real> const k = { 0, 0, 0, 1, 1, 1 };
	return { cp, k, 2 };
}

void GenerateHairs(
	IHair& hair, Span<Geometry::SurfacePosition const> surfaceDependencies, Real length = 1.0f, IPolygonMeshSA const* distributionMesh = nullptr, Real tipRandomness = 0.0f,
	int pointsPerStrandCountMin = 2, int pointsPerStrandCountMax = -1, bool generateTextureCoordinates = false, bool generateWidths = false, bool append = false, Span<int const> pointPerStrandCounts = Span<int const>() );

std::vector<Geometry::SurfacePosition> GenerateVertexSurfaceDependencies( IPolygonMeshSA const* distributionMesh );

/** Generates random hairs
 * @param pointsPerStrandCountMax If -1 the pointsPerStrandCountMin value will set global uniform point counts, otherwise value between pointsPerStrandCountMin and this parameter will be randomly chosen for each strand
 * @param generateTextureCoordinates When true a texture coordinate channel will be generates
 * @param randomSeed Random seed
 * @param append When true new strands will be appended to existing ones, otherwise hair is regenerated
 */
void GenerateRandomHairs(
	IHair&, int count, Real length = 1, IPolygonMeshSA const* distributionMesh = nullptr, Real tipRandomness = 0.0f,
	int pointsPerStrandCountMin = 2, int pointsPerStrandCountMax = -1, bool generateTextureCoordinates = false, bool generateWidths = false, int randomSeed = 1, bool append = false );

void GenerateVertexHairs(
	IHair&, Real length, IPolygonMeshSA const* distributionMesh = nullptr, Real tipRandomness = 0.0f,
	int pointsPerStrandCountMin = 2, int pointsPerStrandCountMax = -1, bool generateTextureCoordinates = false, bool generateWidths = false );

void GenerateZigzagHairs( IHair& hair, int strandCount, int strandPointCount, int period, Real length = 1, Real xDisplacement = 0.5f, Real xRandomness = 0.0f, IPolygonMeshSA const* distributionMesh = nullptr );

void MoveTipsRandomly( IHair& hair, Real amount = 1.0f, int start = 0, int stride = 1, int seed = 1 );

void MoveTips( IHair& hair, Vector3 const& direction, int start = 0, int end = -1, int stride = 1 );

std::vector<std::vector<Vector3>> GetStrandPoints( IHair const& hair, IHair::CoordinateSpace space );

std::vector<Vector3> GetVerticesSortedByStrandIds( IHair const& hair, IHair::CoordinateSpace space );

std::vector<std::vector<Real>> GetStrandWidths( IHair const& hair );

void AddStrandChannel( IHair&, IHair::StrandDataType, std::vector<Real> const& data, std::wstring const& name = L"" );

std::vector<Real> AddStrandChannelWithRandomData( IHair&, IHair::StrandDataType, Real minValue = 0, Real maxValue = 1 );

template<typename T>
Geometry::NearEqualMatcher<T, Real> IsNearEqual( T target, Real epsilon = 0 )
{
	return Geometry::IsNearEqual( target, epsilon );
}

template <class T>
Geometry::VectorNearEqualMatcher<typename T::value_type, Real> IsVectorNearEqual( T const& target, Real epsilon = 0 )
{
	return Geometry::IsVectorNearEqual( target, epsilon );
}

void TestMicroInputModificationsEffectOnResult(
	IHair& sharedHair, IHair& originalHair, IPolygonMeshSA const& distributionMesh, std::function<void( int )> const& iterationFunction,
	Real randomizeAmount = 0.0000001f, int iterationCount = 20, Real acceptableError = 0.001f );

std::vector<Vector3> GetUniquePoints( std::vector<Vector3> const& points, float epsilon = Geometry::Epsilon, std::vector<std::unordered_set<int>>* uniqueSets = nullptr );


static char constexpr StartNodeName[] = "start";
static char constexpr ColliderNodeName[] = "collider";
static char constexpr GuidesFromMeshNodeName[] = "guidesFromMesh";
static char constexpr EditGuidesNodeName[] = "editGuides";
static char constexpr HairFromGuidesNodeName[] = "hairFromGuides";
static char constexpr Curl1NodeName[] = "curl1";
static char constexpr Noise1NodeName[] = "noise1";
static char constexpr SurfaceCombNodeName[] = "surfaceComb";
static char constexpr Test1NodeName[] = "test1";
static char constexpr BakedHairNodeName[] = "bakedHair";
static char constexpr AdoptExternalGuidesNodeName[] = "adoptExternalGuides";
static char constexpr ClumpNodeName[] = "clump";

static char constexpr SampleGroomFilename[] = "SampleGroom.oxg";
}
