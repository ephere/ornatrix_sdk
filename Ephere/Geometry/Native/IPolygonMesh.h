// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppVariableCanBeMadeConstexpr
// ReSharper disable CppClangTidyModernizeUseEqualsDefault
#pragma once

#include "Ephere/Geometry/Native/Box.h"
#include "Ephere/Geometry/Native/Matrix.h"
#include "Ephere/Geometry/Native/MeshSurfacePosition.h"
#include "Ephere/NativeTools/IInterfaceProvider.h"
#include "Ephere/NativeTools/MacroTools.h"

#ifndef EPHERE_NO_UTILITIES
#	include "Ephere/NativeTools/StlExtensions.h"
#endif

#include <array>
#include <vector>

namespace Ephere
{
namespace RayTracing
{
template<typename T>
class Scene;
}

namespace Geometry
{

template<typename T>
struct PolygonMeshQueryContextT;
template<typename T>
class IPolygonMesh;

InterfaceId const InterfaceId_PolygonMeshFloat = InterfaceId_Company_Ephere + 0x200;
InterfaceId const InterfaceId_PolygonMeshDouble = InterfaceId_Company_Ephere + 0x201;

template<typename T>
class IPolygonMeshT
{
protected:
	IPolygonMeshT( IPolygonMeshT const& ) {}

public:
	IPolygonMeshT() {}
	virtual ~IPolygonMeshT() {}

	static InterfaceId const IID = std::is_same<T, float>::value ? InterfaceId_PolygonMeshFloat : InterfaceId_PolygonMeshDouble;

	typedef Matrix<3, 1, T> Vector3;
	typedef Matrix<3, 1, T> TextureCoordinate;

	// Vertices:

	EPHERE_NODISCARD virtual int GetVertexCount() const = 0;

	virtual void SetVertexCount( int value ) = 0;

	virtual void GetVertices( int firstIndex, int count, Vector3* result ) const = 0;

	struct RawVerticesData
	{
		RawVerticesData() = default;
		RawVerticesData(int vertexByteStride_, int vertexCount_, char const* vertices_)
			: vertexByteStride{ vertexByteStride_ },vertexCount{ vertexCount_ }, vertices{ vertices_ }
		{}

		// The byte stride of each vertex
		int vertexByteStride;
		// Number of vertices
		int vertexCount;
		char const* vertices;
	};

	// Returns mesh vertex count and host's internal pointer to its vertices. avoid unnecessary copying data for performance reasons
	EPHERE_NODISCARD virtual RawVerticesData GetVerticesRaw() const = 0;

	virtual void SetVertices( int firstIndex, int count, Vector3 const* values ) = 0;

	EPHERE_NODISCARD virtual bool IsVertexSelected( int vertexIndex ) const = 0;

	virtual void SelectVertex( int vertexIndex, bool select ) = 0;

	// TODO: Make virtual, move to implementation
	void GetVerticesByIndex( int const* vertexIndices, int count, Vector3* result ) const
	{
		for( auto index = 0; index < count; ++index, ++result, ++vertexIndices )
		{
			GetVertices( *vertexIndices, 1, result );
		}
	}

	void GetVerticesByIndex( Span<int const> vertexIndices, Span<Vector3> result ) const
	{
		GetVerticesByIndex( vertexIndices.data(), vertexIndices.size(), result.data() );
	}

	// Triangles:

	// TODO: This call is used sometimes to initialize the mesh to use triangles. Need to find a better way to do that and uncomment below.
	/*EPHERE_NODISCARD */virtual int GetTriangleCount() const = 0;

	virtual void GetTriangleVerticesIndices( int firstTriangleIndex, int count, std::array<int, 3>* result ) const = 0;

	EPHERE_NODISCARD virtual std::array<int, 3> GetTriangleTextureCoordinateIndices( int channelIndex, int triangleIndex ) const = 0;

	/** Gets the indices of polygons on which specified triangles are located, including the indices of the triangles on said polygons
	 * If the mesh was not triangulated or the polygon is also a triangle then the index of the triangular polygon is returned */
	virtual void GetTrianglePolygonIndices( int firstTriangleIndex, int count, std::array<int, 2>* result ) const = 0;

	// Polygons:

	/** Allows this object to perform any necessary calculations before subsequent multi-threaded queries into its topology
	 *	This function is not thread safe.
	 *	@return true if everything went OK */
	virtual bool ValidatePolygonTables() const = 0;

	EPHERE_NODISCARD virtual int GetPolygonCount() const = 0;

	virtual void SetPolygonCount( int polygonCount, int verticesPerPolygon ) = 0;

	EPHERE_NODISCARD virtual int GetPolygonVertexCount( int polygonIndex ) const = 0;

	/** Sets the number of vertices per polygon
	 * @param polygonVertexCounts Polygon vertex count values to set per each polygon starting with firstPolygonIndex
	 * @param firstPolygonIndex First polygon for which to set vertex count
	 */
	virtual void SetPolygonVertexCounts( Span<int const> polygonVertexCounts, int firstPolygonIndex = 0 ) = 0;

	virtual void GetPolygonVertexIndices( int polygonIndex, int firstIndex, int count, int* result ) const = 0;

	void GetPolygonVertexIndices( int polygonIndex, int firstIndex, Span<int> result ) const
	{
		GetPolygonVertexIndices( polygonIndex, firstIndex, result.size(), result.data() );
	}

	virtual void SetPolygonVertexIndices( int polygonIndex, int firstIndex, int count, int const* vertexIndices ) = 0;

	EPHERE_NODISCARD virtual int GetPolygonTriangleCount( int polygonIndex ) const = 0;

	virtual void GetPolygonTriangleIndices( int polygonIndex, int firstIndex, int count, int* result ) const = 0;

	// Texture coordinates:

	EPHERE_NODISCARD virtual int GetTextureChannelCount() const = 0;

	virtual void SetTextureChannelCount( int value ) = 0;

	// TODO: We use this function to pre-initialize some stuff in Maya. Uncomment below when this is fixed.
	/*EPHERE_NODISCARD*/ virtual bool HasTextureChannel( int index ) const = 0;

	EPHERE_NODISCARD virtual int GetTextureCoordinateCount( int textureChannel ) const = 0;

	virtual void SetTextureCoordinateCount( int textureChannel, int count ) = 0;

	EPHERE_NODISCARD virtual int GetPolygonTextureCoordinateIndex( int channelIndex, int polygonIndex, int vertexIndex ) const = 0;

	virtual void GetPolygonTextureCoordinateIndices( int channelIndex, int polygonIndex, int firstPolygonVertexIndex, int count, int* result ) const final
	{
		for( auto index = firstPolygonVertexIndex, lastIndex = firstPolygonVertexIndex + count; index < lastIndex; ++index, ++result )
		{
			*result = GetPolygonTextureCoordinateIndex( channelIndex, polygonIndex, index );
		}
	}

	virtual void GetPolygonTextureCoordinateIndices( int channelIndex, int polygonIndex, int firstPolygonVertexIndex, Span<int> result ) const final
	{
		GetPolygonTextureCoordinateIndices( channelIndex, polygonIndex, firstPolygonVertexIndex, result.size(), result.data() );
	}

	virtual void SetPolygonTextureVertexIndices( int textureChannel, int polygonIndex, int firstPolygonVertexIndex, int count, int const* textureCoordinateIndices ) = 0;

	virtual void GetTextureCoordinates( int channelIndex, int firstIndex, int count, TextureCoordinate* result ) const = 0;

	virtual void SetTextureCoordinates( int channelIndex, int firstIndex, int count, TextureCoordinate const* values ) = 0;

	// Face selection:

	EPHERE_NODISCARD virtual int GetSelectedFaceCount() const = 0;

	EPHERE_NODISCARD virtual bool IsFaceSelected( int faceIndex ) const = 0;

	EPHERE_NODISCARD virtual bool IsTriangleSelected( int triangleIndex ) const = 0;

	EPHERE_NODISCARD virtual bool HasMaterialIds() const = 0;

	EPHERE_NODISCARD virtual int GetFaceMaterialId( int faceIndex ) const = 0;

	virtual void SetFaceMaterialId( int faceIndex, int value ) = 0;

	EPHERE_NODISCARD virtual int GetVertexColorChannelCount() const = 0;

	virtual void GetVertexColors( int polygonIndex, int channelIndex, int firstFaceVertexIndex, int count, Vector3* result ) const = 0;

	virtual void GetVertexNormals( int firstVertexIndex, int count, Vector3* result ) const = 0;

	virtual void PrepareVertexColorChannel( int channelIndex ) const = 0;

	EPHERE_NODISCARD virtual std::shared_ptr<void> CreateVertexNormalContext_Obsolete() const
	{
		return std::shared_ptr<void>();
	}

	// Normals:

	EPHERE_NODISCARD virtual Vector3 GetTriangleNormalDirection( int triangleIndex ) const = 0;

	virtual void SetVertexNormal( int vertexIndex, Vector3 const& normalVector ) = 0;

	//! Must be called before GetTriangleVertexNormals
	virtual void ValidateTriangleVertexNormals() const = 0;

	virtual void GetTriangleVertexNormals( int triangleIndex, int firstVertexIndex, int vertexCount, Vector3* result ) const = 0;

	//! Must be called before GetPolygonVertexNormals
	virtual void ValidatePolygonVertexNormals() const = 0;

	virtual void GetPolygonVertexNormals( int polygonIndex, int firstVertexIndex, int vertexCount, Vector3* result, PolygonMeshQueryContextT<T>& context ) const = 0;

	void GetPolygonVertexNormals( int polygonIndex, int firstVertexIndex, Span<Vector3> result, PolygonMeshQueryContextT<T>& context ) const
	{
		GetPolygonVertexNormals( polygonIndex, firstVertexIndex, result.size(), result.data(), context );
	}

	//! Allows inverting normals during calculation
	virtual void SetFlipNormals( bool value ) = 0;

	EPHERE_NODISCARD virtual bool AreNormalsFlipped() const = 0;

	// Bounding box:

	EPHERE_NODISCARD virtual Box<3, T> GetBoundingBox() const = 0;

	// Other:

	virtual bool ConvertToNativeMesh( IPolygonMesh<T>& result, bool includeGeometry = true, bool includeTopology = true, bool includeTextureCoordinates = true,
									  bool includeVertexNormals = false, bool forceTriangularFaces = false ) const = 0;

	virtual void GetSurfaceDependenciesFromPositions( std::vector<Vector3> const& positions, std::vector<MeshSurfacePosition>& result, void* meshIntersector = nullptr ) const = 0;

	virtual void CopyFacesFromMesh( IPolygonMeshT const& sourceMesh, int destinationFaceStartingIndex ) = 0;

	// Utility:

#ifndef EPHERE_NO_UTILITIES

	EPHERE_NODISCARD Vector3 GetVertex( int vertexIndex ) const
	{
		Vector3 result;
		GetVertices( vertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD std::vector<Vector3> GetVertices( int firstIndex = 0, int count = -1 ) const
	{
		std::vector<Vector3> result( count != -1 ? count : GetVertexCount() - firstIndex );
		GetVertices( firstIndex, Size( result ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<int> GetPolygonVertexIndices( int polygonIndex ) const
	{
		std::vector<int> result( GetPolygonVertexCount( polygonIndex ) );
		GetPolygonVertexIndices( polygonIndex, 0, result );
		return result;
	}

	void SetVertices( Span<Vector3 const> values, int firstIndex = 0, bool resizeVertices = false )
	{
		if( resizeVertices )
		{
			SetVertexCount( firstIndex + values.size() );
		}

		SetVertices( firstIndex, values.size(), values.data() );
	}

	void SetVertex( int vertexIndex, Vector3 const& value )
	{
		SetVertices( vertexIndex, 1, &value );
	}

	EPHERE_NODISCARD Vector3 GetTextureCoordinate( int channelIndex, int vertexIndex ) const
	{
		Vector3 result;
		GetTextureCoordinates( channelIndex, vertexIndex, 1, &result );
		return result;
	}

	void SetTextureCoordinate( int channelIndex, int index, TextureCoordinate const& value )
	{
		SetTextureCoordinates( channelIndex, index, 1, &value );
	}

	EPHERE_NODISCARD std::array<int, 3> GetTriangleVertexIndices( int triangleIndex ) const
	{
		std::array<int, 3> result;
		GetTriangleVerticesIndices( triangleIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD std::array<Vector3, 3> GetTriangleVertices( int triangleIndex ) const
	{
		std::array<Vector3, 3> result;
		auto const triangleVertexIndices = GetTriangleVertexIndices( triangleIndex );
		std::transform( triangleVertexIndices.begin(), triangleVertexIndices.end(), result.begin(), [this]( int value )
		{
			Vector3 resultVector;
			GetVertices( value, 1, &resultVector );
			return resultVector;
		} );

		return result;
	}

	EPHERE_NODISCARD std::vector<TextureCoordinate> GetTextureCoordinates( int channelIndex ) const
	{
		std::vector<TextureCoordinate> result( GetTextureCoordinateCount( channelIndex ) );
		GetTextureCoordinates( channelIndex, 0, Size( result ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::array<TextureCoordinate, 3> GetTriangleTextureCoordinates( int channelIndex, int triangleIndex ) const
	{
		auto const textureVertexIndices = GetTriangleTextureCoordinateIndices( channelIndex, triangleIndex );

		std::array<TextureCoordinate, 3> result;
		std::transform( textureVertexIndices.begin(), textureVertexIndices.end(), result.begin(), [this, channelIndex]( int value )
		{
			TextureCoordinate textureCoordinate;
			GetTextureCoordinates( channelIndex, value, 1, &textureCoordinate );
			return textureCoordinate;
		} );

		return result;
	}

	EPHERE_NODISCARD std::vector<int> GetPolygonTextureCoordinateIndices( int channelIndex, int polygonIndex ) const
	{
		std::vector<int> result( GetPolygonVertexCount( polygonIndex ) );
		GetPolygonTextureCoordinateIndices( channelIndex, polygonIndex, 0, Size( result ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<TextureCoordinate> GetPolygonTextureCoordinates( int uvChannel, int polygonIndex ) const
	{
		auto const faceVertexCount = GetPolygonVertexCount( polygonIndex );
		std::vector<int> polygonTextureCoordinateIndices( faceVertexCount );
		GetPolygonTextureCoordinateIndices( uvChannel, polygonIndex, 0, Size( polygonTextureCoordinateIndices ), polygonTextureCoordinateIndices.data() );

		std::vector<TextureCoordinate> polygonTextureCoordinates( faceVertexCount );
		std::transform( polygonTextureCoordinateIndices.begin(), polygonTextureCoordinateIndices.end(), polygonTextureCoordinates.begin(), [this, uvChannel]( int value )
		{
			TextureCoordinate result;
			GetTextureCoordinates( uvChannel, value, 1, &result );
			return result;
		} );

		return polygonTextureCoordinates;
	}

	EPHERE_NODISCARD Vector3 GetTriangleVertexColor( int const triangleIndex, Vector3 const& barycentricCoordinate, int const channelIndex ) const
	{
		// TODO: We're not getting colors correctly, need to get all polygon vertex colors and find the three belonging to this triangle
		std::array<Vector3, 3> colors;
		std::array<int, 2> trianglePolygonIndex = {};
		GetTrianglePolygonIndices( triangleIndex, 1, &trianglePolygonIndex );
		GetVertexColors( trianglePolygonIndex[0], channelIndex, 0, 3, colors.data() );

		return colors[0] * barycentricCoordinate.x() + colors[1] * barycentricCoordinate.y() + colors[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD Vector3 GetTriangleVertex( int triangleIndex, Vector3 const& barycentricCoordinate ) const
	{
		auto const vertices = GetTriangleVertices( triangleIndex );
		return vertices[0] * barycentricCoordinate.x() + vertices[1] * barycentricCoordinate.y() + vertices[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD TextureCoordinate GetTriangleTextureCoordinate( int channelIndex, int triangleIndex, Vector3 const& barycentricCoordinate ) const
	{
		auto const textureCoordinate = GetTriangleTextureCoordinates( channelIndex, triangleIndex );
		return textureCoordinate[0] * barycentricCoordinate.x() + textureCoordinate[1] * barycentricCoordinate.y() + textureCoordinate[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD Vector3 GetVertexNormal( int triangleIndex, Vector3 const& barycentricCoordinate, bool forceNormalized = false ) const
	{
		auto result = Vector3::Zero();
		auto const vertexIndices = GetTriangleVertexIndices( triangleIndex );
		for( auto index = 0; index < 3; ++index )
		{
			Vector3 vertexNormal;
			GetVertexNormals( vertexIndices[index], 1, &vertexNormal );
			if( forceNormalized )
			{
				vertexNormal.normalize();
			}

			result += vertexNormal * barycentricCoordinate[index];
		}

		return result;
	}

	EPHERE_NODISCARD Vector3 GetTrianglePoint( int triangleIndex, Vector3 const& barycentricCoordinate ) const
	{
		auto const vertices = GetTriangleVertices( triangleIndex );
		return vertices[0] * barycentricCoordinate.x() + vertices[1] * barycentricCoordinate.y() + vertices[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD Vector3 GetVertexColor( int channelIndex, int faceIndex, int faceVertexIndex ) const
	{
		Vector3 result;
		GetVertexColors( faceIndex, channelIndex, faceVertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD Vector3 GetVertexColor( int channelIndex, int faceIndex, Vector3 const& barycentricCoordinate ) const
	{
		std::array<Vector3, 3> vertexColors;
		GetVertexColors( faceIndex, channelIndex, 0, 3, vertexColors.data() );
		return Ephere::Geometry::GetPointFromBarycentric( vertexColors.data(), barycentricCoordinate );
	}

	EPHERE_NODISCARD Vector3 GetTriangleVertexNormal( int triangleIndex, int vertexIndex ) const
	{
		Vector3 result;
		GetTriangleVertexNormals( triangleIndex, vertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD std::array<Vector3, 3> GetTriangleVertexNormals( int triangleIndex ) const
	{
		std::array<Vector3, 3> result;
		GetTriangleVertexNormals( triangleIndex, 0, 3, result.data() );
		return result;
	}

#endif

	// new functionalities
	EPHERE_NODISCARD virtual bool IsBeingEdited() const = 0;
	EPHERE_NODISCARD virtual bool IsValid() const = 0;
	EPHERE_NODISCARD virtual RayTracing::Scene<T> const* GetRTScene() const = 0;	
};

typedef IPolygonMeshT<float> IPolygonMeshf;
typedef IPolygonMeshT<double> IPolygonMeshd;

} }
