// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppVariableCanBeMadeConstexpr
// ReSharper disable CppClangTidyModernizeLoopConvert
// ReSharper disable CppClangTidyModernizeUseEqualsDefault
#pragma once

#include "Ephere/Geometry/Native/Box.h"
#include "Ephere/Geometry/Native/Matrix.h"
#include "Ephere/Geometry/Native/MeshSurfacePosition.h"
#include "Ephere/Geometry/Native/SurfaceTangentComputeMethod.h"
#include "Ephere/NativeTools/IInterfaceProvider.h"
#include "Ephere/NativeTools/MacroTools.h"
#include "Ephere/NativeTools/Span.h"
#include "Ephere/Ornatrix/GuideDependency.h"
#include "Ephere/Ornatrix/StrandChannelName.h"
#include "Ephere/Ornatrix/StrandTopology.h"
#include "Ephere/Ornatrix/Types.h"

// transform
#include <algorithm>
// intptr_t, uint64_t
#include <cstdint>
// inserter
#include <iterator>
// iota
#include <numeric>
// wstring
#include <string>
#include <unordered_set>
// wstringstream
#include <sstream>

namespace Ephere
{
struct IChunkOutputStream;
struct IChunkInputStream;

namespace Geometry
{
template<typename T>
class PolygonMeshRefiner;
}

}

namespace Ephere { namespace Ornatrix
{
	
InterfaceId const InterfaceId_Hair = InterfaceId_Company_Ephere + 0x100;

/** Ornatrix hair interface class. Contains data defining a hair structure consisting of vertices, strands, and methods for manipulating said data.

A typical implementation of this interface is structured in a similar way to a polygonal mesh. It contains a set of vertices defining the geometric information
about the hair. It also contains information about how these vertices are connected to form polylines, defining the topological information about hair. Each
polyline defines a hair strand.

The topology and geometry of hair can be represented in multiple ways. For example, all vertices can be specified in hair object's local coordinates or they can
be specified in local strand coordinates and a separate per-strand transformation can be defined to bring them into the hair object's coordinates. All strands
can have the same number of points defining their polylines or each strand can have a different number of points. In the latter case information is stored to specify
how many points each strand holds.
*/
class IHair1
{
public:

	/** The type of data which can be attached to elements of this class */
	enum StrandDataType
	{
		/** Each data element is attached to each strand */
		PerStrand,

		/** Each data element is attached to each vertex */
		PerVertex,

		/** There is a single global value for all strands */
		StrandDataType_Global,

		/** There is no data specified */
		StrandDataType_None
	};

	/** Coordinate space in which vertices of this class are defined */
	enum CoordinateSpace
	{
		/** Vertices are in local strand space. Per-strand transformations need to be applied to vertices to move them into object space */
		Strand,

		/** Vertices are in object space. Inverse of per-strand transformations need to be applied to vertices to move them into strand space */
		Object,
	};

	static InterfaceId const IID = InterfaceId_Hair + 3;

	virtual void CopyFrom( IHair1 const& source, bool copyGeometry = true, bool copyTopology = true, bool copySelection = true, bool copyTextureCoordinates = true, bool copyChannelData = true ) = 0;

	// Strands:

	/** Gets the total number of strands in this hair object
		@return Number of strands
	*/
	EPHERE_NODISCARD virtual int GetStrandCount() const = 0;

	virtual void SetStrandCount( int value ) = 0;

	// Vertices:

	EPHERE_NODISCARD virtual int GetVertexCount() const = 0;

	virtual void SetVertexCount( int value ) = 0;

	virtual bool GetVertices( int firstIndex, int count, Geometry::Vector3f* result, CoordinateSpace space ) const = 0;

	EPHERE_NODISCARD std::vector<Vector3> GetVertices( CoordinateSpace space = CoordinateSpace::Strand, int firstVertexIndex = 0, int count = -1 ) const
	{
		std::vector<Vector3> result( count == -1 ? GetVertexCount() - firstVertexIndex : count );
		GetVertices( firstVertexIndex, static_cast<int>( result.size() ), result.data(), space );
		return result;
	}

	virtual bool SetVertices( int firstIndex, int count, Geometry::Vector3f const* values, CoordinateSpace space ) = 0;

	/** Gets the indices of first strands that use specified vertices
	* @param firstVertexIndex Index of first vertex for which to get the result
	* @param count Total number of result entries to get
	* @param result Destination storage
	* @return true if successful
	*/
	virtual bool GetVertexStrandIndices( int firstVertexIndex, int count, int* result ) const = 0;

	/** This function should be called before one or more calls to GetVertexStrandIndices(...). It is not thread-safe.
	* @return true if successful. If false is returned using GetVertexStrandIndices(...) is not safe.
	*/
	virtual bool ValidateVertexStrandIndices() = 0;

	// Strand points:

	virtual bool GetStrandPointCounts( int firstStrandIndex, int count, int* result ) const = 0;

	EPHERE_NODISCARD std::vector<int> GetStrandPointCounts( int firstStrandIndex = 0, int count = -1 ) const
	{
		std::vector<int> result( count != -1 ? count : GetStrandCount() - firstStrandIndex );
		if( !GetStrandPointCounts( firstStrandIndex, static_cast<int>( result.size() ), result.data() ) )
		{
			return std::vector<int>();
		}

		return result;
	}

	virtual bool GetStrandFirstVertexIndices( int firstStrandIndex, int count, int* result ) const = 0;

	virtual bool GetStrandPoints( int strandIndex, int firstPointIndex, int pointCount, Geometry::Vector3f* result, CoordinateSpace resultSpace ) const = 0;

	virtual bool SetStrandPoints( int strandIndex, int firstPointIndex, int pointCount, Geometry::Vector3f const* values, CoordinateSpace sourceSpace ) = 0;

	// Strand Ids:

	/** Determines if this hair has unique per-strand ids
	 * @return true if unique ids are stored
	 */
	EPHERE_NODISCARD virtual bool HasStrandIds() const = 0;

	/** Sets whether unique per-strand ids are stored in this hair
	 * @param value true to store unique ids
	 */
	virtual void SetUseStrandIds( bool value ) = 0;

	virtual bool GetStrandIds( int firstStrandIndex, int count, StrandId* result ) const = 0;

	virtual bool SetStrandIds( int firstStrandIndex, int count, StrandId const* source ) = 0;

	bool SetStrandId( int const strandIndex, StrandId value )
	{
		return SetStrandIds( strandIndex, 1, &value );
	}

	template<typename T>
	bool SetStrandIds( Span<T const> values, int firstStrandIndex = 0 )
	{
		return SetStrandIds( firstStrandIndex, values.size(), reinterpret_cast<StrandId const*>( values.data() ) );
	}

	/** Gets indices into the strand arrays based on their unique ids */
	virtual bool GetStrandIndices( unsigned const* strandId, int count, int* result ) const = 0;

	// No need to call this one any longer
	virtual bool ValidateStrandIdsToIndices_obsolete() = 0;

	// Surface dependency:

	EPHERE_NODISCARD virtual bool HasSurfaceDependency() const = 0;

	virtual void SetUseSurfaceDependency( bool value ) = 0;

	virtual bool GetSurfaceDependencies( int firstIndex, int count, Geometry::MeshSurfacePosition* result ) const = 0;

	virtual bool SetSurfaceDependencies( int firstIndex, int count, Geometry::MeshSurfacePosition const* values ) = 0;

	//! This function will not validate the bounding box before getting it
	EPHERE_NODISCARD virtual Box3 GetBoundingBox() const = 0;

	// Guide dependency:

	EPHERE_NODISCARD virtual bool HasGuideDependency() const = 0;

	virtual void SetUsesGuideDependency( bool value ) = 0;

	virtual bool GetGuideDependencies( int firstStrandIndex, int count, GuideDependency* result ) const = 0;

	virtual bool SetGuideDependencies( int firstStrandIndex, int count, GuideDependency const* source ) = 0;

	// Strand topology:

	EPHERE_NODISCARD virtual bool HasStrandTopology() const = 0;

	virtual void SetUsesStrandTopology( bool value ) = 0;

	virtual bool GetStrandTopologies( int firstStrandIndex, int count, StrandTopology* result ) const = 0;

	virtual bool SetStrandTopologies( int firstStrandIndex, int count, StrandTopology const* source ) = 0;

	bool SetStrandTopologies( Span<StrandTopology const> source, int firstStrandIndex = 0 )
	{
		return SetStrandTopologies( firstStrandIndex, source.size(), source.data() );
	}

	EPHERE_NODISCARD virtual int GetGlobalStrandPointCount() const = 0;

	virtual void SetGlobalStrandPointCount( int value ) = 0;

	// Strand transformations:

	EPHERE_NODISCARD virtual bool HasStrandToObjectTransforms() const = 0;

	virtual void SetUseStrandToObjectTransforms( bool value ) = 0;

	virtual bool GetStrandToObjectTransforms( int firstStrandIndex, int count, Geometry::Xform3f* result ) const = 0;

	virtual bool SetStrandToObjectTransforms( int firstStrandIndex, int count, Geometry::Xform3f const* values ) = 0;

	virtual void ValidateStrandToObjectTransforms_deprecated( bool forceStrandCoordinates = true ) = 0;

	bool ValidateStrandToObjectTransforms( IPolygonMeshSA const* distributionMesh, bool forceStrandCoordinates = true )
	{
		return SetPropertyValues( static_cast<int>( CommandExtension::ValidateStrandToObjectTransforms ), forceStrandCoordinates ? 1 : 0, 0, distributionMesh );
	}

	// Strand rotations/twists:

	EPHERE_NODISCARD virtual bool HasStrandRotations() const = 0;

	virtual void SetUseStrandRotations( bool value ) = 0;

	// TODO: Deprecate in favor of function providing a data type
	virtual bool GetStrandRotations( int firstStrandIndex, int count, float* result ) const = 0;

	// TODO: Deprecate in favor of function providing a data type
	virtual bool SetStrandRotations( int firstStrandIndex, int count, float const* values ) = 0;

	/** Gets the current type of data storage for strand width values */
	EPHERE_NODISCARD StrandDataType GetRotationsDataType() const
	{
		int result;
		GetPropertyValues( static_cast<int>( CommandExtension::RotationsStrandDataType ), 0, 0, &result );
		return static_cast<StrandDataType>( result );
	}

	struct GetStrandRotationsResultStruct
	{
		Real* result;
		StrandDataType dataType;
	};

	// TODO: Add to IHair
	bool GetStrandRotations( int firstElementIndex, int count, Real* result, StrandDataType dataType ) const
	{
		GetStrandRotationsResultStruct resultStruct = { result, dataType };
		return GetPropertyValues( static_cast<int>( CommandExtension::Rotations ), firstElementIndex, count, &resultStruct );
	}

	EPHERE_NODISCARD std::vector<Real> GetStrandRotations( int firstElementIndex = 0, int count = -1, StrandDataType dataType = StrandDataType::PerStrand ) const
	{
		std::vector<Real> result( count != -1 ? count : ( dataType == StrandDataType::PerStrand ? GetStrandCount() : GetVertexCount() ) - firstElementIndex );
		GetStrandRotations( firstElementIndex, static_cast<int>( result.size() ), result.data(), dataType );
		return result;
	}

	struct SetStrandRotationsValuesStruct
	{
		Real const* values;
		StrandDataType dataType;
	};

	// TODO: Add to IHair
	bool SetStrandRotations( int firstElementIndex, int count, Real const* values, StrandDataType dataType )
	{
		SetStrandRotationsValuesStruct const valuesStruct = { values, dataType };
		return SetPropertyValues( static_cast<int>( CommandExtension::Rotations ), firstElementIndex, count, &valuesStruct );
	}

	// Vertex widths (strand diameters):

	EPHERE_NODISCARD virtual bool HasWidths() const = 0;

	virtual void SetUseWidths( bool value ) = 0;

	/** Gets the diameter of hair strand at each vertex position */
	virtual bool GetWidths( int firstVertexIndex, int count, float* result ) const = 0;

	/** Sets the diameter of hair strand at each vertex position */
	virtual bool SetWidths( int firstVertexIndex, int count, float const* values ) = 0;

	// Coordinate space:

	EPHERE_NODISCARD virtual CoordinateSpace GetCoordinateSpace() const = 0;

	virtual void SetCoordinateSpace( CoordinateSpace value ) const = 0;

	// Texture coordinates:

	EPHERE_NODISCARD virtual int GetTextureCoordinateChannelCount() const = 0;

	virtual void SetTextureCoordinateChannelCount( int value ) = 0;

	EPHERE_NODISCARD virtual StrandDataType GetTextureCoordinateDataType( int channelIndex ) const = 0;

	virtual bool GetTextureCoordinates( int channelIndex, int firstIndex, int count, Geometry::TextureCoordinatef* result, StrandDataType dataType ) const = 0;

	virtual bool SetTextureCoordinates( int channelIndex, int firstIndex, int count, Geometry::TextureCoordinatef const* values, StrandDataType dataType ) = 0;

	// Strand channels:

	EPHERE_NODISCARD virtual int GetStrandChannelCount( StrandDataType type ) const = 0;

	virtual bool GetStrandChannelNames( StrandDataType type, int channelIndex, int count, StrandChannelName* result ) const = 0;

	virtual bool GetStrandChannelData( StrandDataType type, int channelIndex, int firstElementIndex, int count, float* result ) const = 0;

	virtual void SetStrandChannelCount( StrandDataType type, int count ) = 0;

	virtual bool SetStrandChannelData( StrandDataType type, int channelIndex, int firstElementIndex, int count, float const* source ) = 0;

	virtual bool SetStrandChannelNames( StrandDataType type, int channelIndex, int count, StrandChannelName const* source ) = 0;

	virtual bool DeleteStrandChannels( StrandDataType type, int const* indices, int indexCount ) = 0;

	// Selection, hiding, and freezing:

	EPHERE_NODISCARD virtual int GetSelectedStrandCount() const = 0;

	virtual bool GetSelectedStrandIds( int startIndex, int count, StrandId* result ) const = 0;

	virtual bool SetSelectedStrandIds( StrandId const* values, int count ) = 0;

	bool SetSelectedStrandIds( Span<StrandId const> values )
	{
		return SetSelectedStrandIds( values.data(), values.size() );
	}

	EPHERE_NODISCARD virtual int GetHiddenStrandCount() const = 0;

	virtual bool GetHiddenStrandIds( int startIndex, int count, StrandId* result ) const = 0;

	virtual bool SetHiddenStrandIds( StrandId const* values, int count ) = 0;

	bool SetHiddenStrandIds( Span<StrandId const> values )
	{
		return SetHiddenStrandIds( values.data(), values.size() );
	}

	EPHERE_NODISCARD virtual int GetFrozenStrandCount() const = 0;

	virtual bool GetFrozenStrandIds( int startIndex, int count, StrandId* result ) const = 0;

	virtual bool SetFrozenStrandIds( StrandId const* values, int count ) = 0;

	bool SetFrozenStrandIds( Span<StrandId const> values )
	{
		return SetFrozenStrandIds( values.data(), values.size() );
	}

	// Caching:

	virtual void InvalidateGeometryCache() = 0;

	EPHERE_NODISCARD virtual std::uint64_t GetTopologyHash() const = 0;

	// Future expansion (these will be used to add new functionality without modifying the virtual signature of this interface):

	virtual bool Execute( int commandIndex ) = 0;

	EPHERE_NODISCARD virtual bool HasProperty( int propertyIndex ) const = 0;

	virtual void SetUsesProperty( int propertyIndex ) = 0;

	virtual bool GetPropertyValues( int propertyIndex, int firstElementIndex, int count, void* values ) const = 0;

	virtual bool SetPropertyValues( int propertyIndex, int firstElementIndex, int count, void const* values ) = 0;

	// Groups:

	EPHERE_NODISCARD virtual bool HasStrandGroups() const = 0;

	virtual void SetUsesStrandGroups( bool value ) = 0;

	virtual bool GetStrandGroups( int firstStrandIndex, int count, int* result ) const = 0;

	virtual bool SetStrandGroups( int firstStrandIndex, int count, int const* values ) = 0;

	bool SetStrandGroups( Span<int const> values, int firstStrandIndex = 0 )
	{
		return SetStrandGroups( firstStrandIndex, values.size(), values.data() );
	}

	bool SetStrandGroup( int const strandIndex, int value )
	{
		return SetStrandGroups( strandIndex, 1, &value );
	}

	// Deletion:

	virtual bool DeleteStrands( StrandId const* strandIds, int count ) = 0;

	bool DeleteStrands( Span<StrandId const> strandIds )
	{
		return DeleteStrands( strandIds.data(), strandIds.size() );
	}

	// Utility functions:
	// The following functions are inline and defined for providing common useful operations on this class.

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return GetStrandCount() == 0;
	}

	/** Removes all data from this object, returning it to initial state */
	void Clear()
	{
		SetStrandCount( 0 );
		SetVertexCount( 0 );
		SetStrandChannelCount( StrandDataType::PerStrand, 0 );
		SetStrandChannelCount( StrandDataType::PerVertex, 0 );
		SetTextureCoordinateChannelCount( 0 );
		SetCoordinateSpace( CoordinateSpace::Strand );
	}

	// Per-vertex transforms:

	bool ValidateVertexToObjectTransforms( IPolygonMeshSA const* distributionMesh, int const firstVertexIndex = 0, int const count = -1 )
	{
		return SetPropertyValues( static_cast<int>( CommandExtension::VertexToObjectTransforms ), firstVertexIndex, count, distributionMesh );
	}

	EPHERE_NODISCARD StrandId GetStrandId( int const strandIndex ) const
	{
		if( HasStrandIds() )
		{
			StrandId result;
			if( GetStrandIds( strandIndex, 1, &result ) )
			{
				return result;
			}
		}

		return static_cast<StrandId>( strandIndex );
	}

	void SetStrandToObjectTransform( int const strandIndex, Geometry::Xform3f const& value )
	{
		SetStrandToObjectTransforms( strandIndex, 1, &value );
	}

	int GetChannelIndex( StrandDataType const type, wchar_t const* channelName ) const
	{
		auto const channelCount = GetStrandChannelCount( type );
		if( channelCount == 0 )
		{
			return -1;
		}

		std::vector<StrandChannelName> channelNames( channelCount );
		GetStrandChannelNames( type, 0, channelCount, channelNames.data() );

		std::wstring const channelNameString( channelName );
		for( auto channelIndex = 0; channelIndex < channelCount; ++channelIndex )
		{
			if( channelNameString == channelNames[channelIndex].name )
			{
				return channelIndex;
			}
		}

		return -1;
	}

	EPHERE_NODISCARD Real GetStrandRotation( int const strandIndex ) const
	{
		Real result;
		if( GetStrandRotations( strandIndex, 1, &result, StrandDataType::PerStrand ) )
		{
			return result;
		}

		return 0;
	}

	bool SetStrandRotation( int const strandIndex, Real const value )
	{
		return SetStrandRotations( strandIndex, 1, &value, StrandDataType::PerStrand );
	}

	EPHERE_NODISCARD Geometry::MeshSurfacePosition GetSurfaceDependency( int const strandIndex ) const
	{
		Geometry::MeshSurfacePosition result;
		return GetSurfaceDependencies( strandIndex, 1, &result ) ? result : Geometry::MeshSurfacePosition( 0, Geometry::Vector3f::Zero() );
	}

	EPHERE_NODISCARD Geometry::SurfacePosition GetSurfaceDependency2( int const strandIndex ) const
	{
		Geometry::SurfacePosition result;
		return GetSurfaceDependencies2( strandIndex, 1, &result ) ? result : Geometry::SurfacePosition( 0, Geometry::Vector2f::Zero() );
	}

	void SetSurfaceDependency( int const strandIndex, Geometry::MeshSurfacePosition const& value )
	{
		SetSurfaceDependencies( strandIndex, 1, &value );
	}

	void SetSurfaceDependency2( int const strandIndex, Geometry::SurfacePosition const& value )
	{
		SetSurfaceDependencies2( strandIndex, 1, &value );
	}

	EPHERE_NODISCARD StrandTopology GetStrandTopology( int const strandIndex ) const
	{
		auto result = StrandTopology();
		GetStrandTopologies( strandIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD GuideDependency GetGuideDependency( int const strandIndex ) const
	{
		auto result = GuideDependency();
		GetGuideDependencies( strandIndex, 1, &result );
		return result;
	}

	void SetGuideDependency( int const strandIndex, GuideDependency const& value )
	{
		SetGuideDependencies( strandIndex, 1, &value );
	}

	EPHERE_NODISCARD int GetVertexStrandIndex( int vertexIndex ) const
	{
		int result;
		GetVertexStrandIndices( vertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD int GetStrandPointCount( int const strandIndex ) const
	{
		int result;
		if( GetStrandPointCounts( strandIndex, 1, &result ) )
		{
			return result;
		}

		return -1;
	}

	EPHERE_NODISCARD int GetStrandFirstVertexIndex( int const strandIndex ) const
	{
		int result;
		if( GetStrandFirstVertexIndices( strandIndex, 1, &result ) )
		{
			return result;
		}

		return -1;
	}

	EPHERE_NODISCARD float GetStrandChannelData( int const channelIndex, int const strandIndex ) const
	{
		Real result;
		return GetStrandChannelData( PerStrand, channelIndex, strandIndex, 1, &result ) ? result : 0;
	}

	void SetStrandChannelData( int const channelIndex, int const strandIndex, float const value )
	{
		SetStrandChannelData( PerStrand, channelIndex, strandIndex, 1, &value );
	}

	void SetVertexChannelData( int const channelIndex, int const vertexIndex, float const value )
	{
		SetStrandChannelData( PerVertex, channelIndex, vertexIndex, 1, &value );
	}

	void SetVertexChannelData( int const channelIndex, int const strandIndex, int const pointIndex, float const value )
	{
		SetVertexChannelData( channelIndex, GetStrandFirstVertexIndex( strandIndex ) + pointIndex, value );
	}

	EPHERE_NODISCARD float GetVertexChannelData( int const channelIndex, int const vertexIndex ) const
	{
		float result;
		return GetStrandChannelData( PerVertex, channelIndex, vertexIndex, 1, &result ) ? result : 0.0f;
	}

	EPHERE_NODISCARD float GetVertexChannelData( int const channelIndex, int const strandIndex, int const pointIndex ) const
	{
		return GetVertexChannelData( channelIndex, GetStrandFirstVertexIndex( strandIndex ) + pointIndex );
	}

	void SetStrandChannelAllData( StrandDataType const dataType, int const firstChannelIndex, int count, float const value )
	{
		auto const elementCount = dataType == PerStrand ? GetStrandCount() : GetVertexCount();

		if( count == -1 )
		{
			count = GetStrandChannelCount( dataType ) - firstChannelIndex;
		}

		std::vector<float> const channelData( elementCount, value );

		auto const lastChannelIndex = firstChannelIndex + count;
		for( auto channelIndex = firstChannelIndex; channelIndex < lastChannelIndex; ++channelIndex )
		{
			SetStrandChannelData( dataType, channelIndex, 0, static_cast<int>( channelData.size() ), channelData.data() );
		}
	}

	/** Get strand indices if they are present, or otherwise copy the strand ids as indices */
	bool GetStrandIndicesOrIds( StrandId const* strandIds, int const count, int* result ) const
	{
		if( HasStrandIds() )
		{
			return GetStrandIndices( strandIds, count, result );
		}

		std::transform( strandIds, strandIds + count, result, []( StrandId strandId )
		{
			return static_cast<int>( strandId );
		} );
		return true;
	}

	EPHERE_NODISCARD int GetStrandIndexOrId( StrandId strandId ) const
	{
		int result;
		return GetStrandIndicesOrIds( &strandId, 1, &result ) ? result : -1;
	}

	bool GetStrandIdsOrIndices( int const firstStrandIndex, int const count, StrandId* result ) const
	{
		if( HasStrandIds() )
		{
			return GetStrandIds( firstStrandIndex, count, result );
		}

		// Fill with incremented integers
		std::iota( result, result + count, firstStrandIndex );
		return true;
	}

	template<typename T>
	bool GetStrandIdsOrIndices( std::vector<T>& result, int firstStrandIndex = 0 ) const
	{
		result.resize( GetStrandCount() - firstStrandIndex );
		if( HasStrandIds() )
		{
			return GetStrandIds( firstStrandIndex, static_cast<int>( result.size() ), reinterpret_cast<StrandId*>( result.data() ) );
		}

		// Fill with incremented integers
		std::iota( result.begin(), result.end(), firstStrandIndex );
		return true;
	}

	template<typename T>
	std::vector<T> GetStrandIdsOrIndices( int firstStrandIndex = 0 ) const
	{
		std::vector<T> result;
		GetStrandIdsOrIndices( result, firstStrandIndex );
		return result;
	}

	EPHERE_NODISCARD Geometry::Vector3f GetVertex( int const vertexIndex, CoordinateSpace const space ) const
	{
		Geometry::Vector3f result;
		GetVertices( vertexIndex, 1, &result, space );
		return result;
	}

	void SetVertex( int const vertexIndex, Geometry::Vector3f const& value, CoordinateSpace const space )
	{
		SetVertices( vertexIndex, 1, &value, space );
	}

	EPHERE_NODISCARD std::vector<StrandId> GetStrandIds() const
	{
		std::vector<StrandId> result( GetStrandCount() );
		GetStrandIds( 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<int> GetStrandGroups() const
	{
		auto const strandCount = GetStrandCount();
		if( strandCount == 0 )
		{
			return std::vector<int>();
		}

		std::vector<int> result( strandCount );

		if( !GetStrandGroups( 0, strandCount, &result[0] ) )
		{
			result.clear();
		}

		return result;
	}

	EPHERE_NODISCARD int GetStrandGroup( int const strandIndex ) const
	{
		int result;
		GetStrandGroups( strandIndex, 1, &result );
		return result;
	}

	void GetSelectedStrandIdSet( std::vector<StrandId>& result ) const
	{
		result.resize( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, static_cast<int>( result.size() ), result.data() );
	}

	EPHERE_NODISCARD std::unordered_set<StrandId> GetSelectedStrandIdsSet() const
	{
		std::vector<StrandId> resultVector( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, static_cast<int>( resultVector.size() ), resultVector.data() );

		std::unordered_set<StrandId> result( begin( resultVector ), end( resultVector ) );
		return result;
	}

	void GetHiddenStrandIdSet( std::vector<StrandId>& result ) const
	{
		result.resize( GetHiddenStrandCount() );
		GetHiddenStrandIds( 0, static_cast<int>( result.size() ), result.data() );
	}

	void GetFrozenStrandIdSet( std::vector<StrandId>& result ) const
	{
		result.resize( GetFrozenStrandCount() );
		GetFrozenStrandIds( 0, static_cast<int>( result.size() ), result.data() );
	}

	template <class TContainer>
	void GetSelectedStrandIdSet( TContainer& result ) const
	{
		std::vector<StrandId> strandIds;
		GetSelectedStrandIdSet( strandIds );

		result.clear();
		result.insert( strandIds.begin(), strandIds.end() );
	}

	template <class TContainer>
	void SetSelectedStrandIdSet( TContainer const& values )
	{
		std::vector<StrandId> const strandIdArray( values.begin(), values.end() );
		SetSelectedStrandIds( strandIdArray.data(), static_cast<int>( strandIdArray.size() ) );
	}

	template <class TContainer>
	void GetHiddenStrandIdSet( TContainer& result, bool overwriteResult = true ) const
	{
		std::vector<StrandId> strandIds;
		GetHiddenStrandIdSet( strandIds );

		if( overwriteResult )
		{
			result.clear();
		}

		result.insert( strandIds.begin(), strandIds.end() );
	}

	template <class TContainer>
	void SetHiddenStrandIdSet( TContainer const& values )
	{
		std::vector<StrandId> const strandIdArray( values.begin(), values.end() );
		SetHiddenStrandIds( strandIdArray.data(), static_cast<int>( strandIdArray.size() ) );
	}

	template <class TContainer>
	void GetFrozenStrandIdSet( TContainer& result, bool overwriteResult = true ) const
	{
		std::vector<StrandId> strandIds;
		GetFrozenStrandIdSet( strandIds );

		if( overwriteResult )
		{
			result.clear();
		}

		result.insert( strandIds.begin(), strandIds.end() );
	}

	template <class TContainer>
	void SetFrozenStrandIdSet( TContainer const& values )
	{
		std::vector<StrandId> const strandIdArray( values.begin(), values.end() );
		SetFrozenStrandIds( strandIdArray.data(), static_cast<int>( strandIdArray.size() ) );
	}

	//! Utility method for legacy scripted commands. These are used mostly for unit testing so performance isn't very important.
	EPHERE_NODISCARD std::vector<bool> GetStrandSelection() const
	{
		std::vector<bool> result;
		std::unordered_set<StrandId> selectedStrandIds;
		GetSelectedStrandIdSet( selectedStrandIds );
		std::vector<StrandId> strandIds( GetStrandCount() );
		GetStrandIds( 0, static_cast<int>( strandIds.size() ), strandIds.data() );

		result.reserve( GetStrandCount() );
		for( auto& strandId : strandIds )
		{
			result.push_back( selectedStrandIds.find( strandId ) != selectedStrandIds.end() );
		}

		return result;
	}

	//! Utility method for legacy scripted commands. These are used mostly for unit testing so performance isn't very important.
	EPHERE_NODISCARD std::vector<bool> GetStrandVisibility() const
	{
		std::vector<bool> result;
		std::unordered_set<StrandId> selectedStrandIds;
		GetHiddenStrandIdSet( selectedStrandIds );
		std::vector<StrandId> strandIds( GetStrandCount() );
		GetStrandIds( 0, static_cast<int>( strandIds.size() ), strandIds.data() );

		result.reserve( GetStrandCount() );
		for( auto strandIdIter = strandIds.begin(); strandIdIter != strandIds.end(); ++strandIdIter )
		{
			result.push_back( selectedStrandIds.find( *strandIdIter ) == selectedStrandIds.end() );
		}

		return result;
	}

	EPHERE_NODISCARD Geometry::TextureCoordinatef GetStrandTextureCoordinate( int channelIndex, int strandIndex ) const
	{
		Geometry::TextureCoordinatef result;
		if( GetTextureCoordinates( channelIndex, strandIndex, 1, &result, PerStrand ) )
		{
			return result;
		}

		return Geometry::TextureCoordinatef::Zero();
	}

	void SetUniqueStrandChannelName( StrandDataType type, int channelIndex, wchar_t const* baseName )
	{
		auto const uniqueChannelNameString = GetUniqueStrandChannelName( type, baseName );
		// TODO: what if uniqueChannelNameString is longer than MaximumNameLength? The name won't be unique
		StrandChannelName const channelName = { uniqueChannelNameString };
		SetStrandChannelNames( type, channelIndex, 1, &channelName );
	}

	/** Given a root name, generates a name for a strand channel which is not currently present. Integers are added at the end of the name
	to make it unique */
	EPHERE_NODISCARD std::wstring GetUniqueStrandChannelName( StrandDataType type, wchar_t const* nameRoot ) const
	{
		auto const channelCount = GetStrandChannelCount( type );
		if( channelCount == 0 )
		{
			return nameRoot;
		}

		std::vector<StrandChannelName> channelNames( channelCount );
		GetStrandChannelNames( type, 0, channelCount, channelNames.data() );

		std::unordered_set<std::wstring> channelNamesSet;
		for( auto const& channelName : channelNames )
		{
			channelNamesSet.insert( std::wstring( channelName.name ) );
		}

		std::wstringstream stringStream;
		stringStream << nameRoot;

		auto index = 1;
		while( channelNamesSet.find( stringStream.str() ) != channelNamesSet.end() )
		{
			stringStream.str( std::wstring() );
			stringStream << nameRoot;
			stringStream << index++;
		}

		return stringStream.str();
	}

	EPHERE_NODISCARD std::vector<StrandId> GetSelectedStrandIds() const
	{
		std::vector<StrandId> result( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<int> GetSelectedStrandIndices() const
	{
		std::vector<StrandId> result( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, static_cast<int>( result.size() ), result.data() );

		std::vector<int> resultIndices( result.size() );
		GetStrandIndicesOrIds( result.data(), static_cast<int>( result.size() ), resultIndices.data() );
		return resultIndices;
	}

	/** Get the names of root channels
	*  @return List of root channel names
	*/
	EPHERE_NODISCARD std::vector<std::wstring> GetRootChannels() const
	{
		std::vector<StrandChannelName> channelNames( GetStrandChannelCount( IHair1::PerStrand ) );
		GetStrandChannelNames( IHair1::PerStrand, 0, static_cast<int>( channelNames.size() ), channelNames.data() );

		std::vector<std::wstring> result( channelNames.size() );
		std::transform( channelNames.begin(), channelNames.end(), result.begin(), []( StrandChannelName const& value )
		{
			return std::wstring( value.name );
		} );

		return result;
	}

	/** Get the names of vertex channels
	*  @return List of vertex channel names
	*/
	EPHERE_NODISCARD std::vector<std::wstring> GetVertexChannels() const
	{
		std::vector<StrandChannelName> channelNames( GetStrandChannelCount( IHair1::PerVertex ) );
		GetStrandChannelNames( IHair1::PerVertex, 0, static_cast<int>( channelNames.size() ), channelNames.data() );

		std::vector<std::wstring> result( channelNames.size() );
		std::transform( channelNames.begin(), channelNames.end(), result.begin(), []( StrandChannelName const& value )
		{
			return std::wstring( value.name );
		} );

		return result;
	}

	// Extension functions, move to IHair2 later:

	enum class CommandExtension
	{
		UseGlobalSegmentTransformOrientation,
		GetGuideDependencies2TotalCount,
		GetGuideDependencies2,
		SetGuideDependencies2,
		GetRootPositionsInObjectSpace,
		RotationsStrandDataType,
		Rotations,
		ValidateStrandToObjectTransforms,
		DeleteStrandsByIndices,
		SurfaceDependency2,
		SurfaceDependency2Off,
		VertexToObjectTransforms,
		SurfaceTangentComputeMethod
	};

	EPHERE_NODISCARD bool UseGlobalSegmentTransformOrientation() const
	{
		return HasProperty( static_cast<int>( CommandExtension::UseGlobalSegmentTransformOrientation ) );
	}

	// Guide dependency:

	/* New guide dependency functions with variable guide count per strand.

	These 3 new functions should be used to access guide dependencies in new applications instead
	of 4 old functions:
	* GetGuideDependencies and SetGuideDependencies
	* GetGuideDependency and SetGuideDependency

	Concurrency notes:

	It's not thread-safe to set strand guide dependencies because each strand can have arbitrary
	count of guide dependencies and change of this count may affect internal representation
	of other guide dependencies. See SetGuideDependencies2 function description for more details.

	Old SetGuideDependencies and SetGuideDependency functions can still be used concurrently with
	other old functions called for non-overlapping strand ranges but now this behavior is result
	of usage of internally created critical section.
	*/

	/** Gets total count of guide dependencies for specified strand range.

	Each strand can have arbitrary count of guide dependencies. */
	bool GetGuideDependencies2TotalCount( int firstStrandIndex, int count, unsigned& result ) const
	{
		return GetPropertyValues( static_cast<int>( CommandExtension::GetGuideDependencies2TotalCount ), firstStrandIndex, count, &result );
	}

	/** Gets guide dependencies with variable guide count per strand for specified strand range. */
	bool GetGuideDependencies2( int firstStrandIndex, int count, unsigned* indicesResult, int maxTotalGuideCount, GuideDependency2* guidesResult ) const
	{
		void* values[] = { indicesResult, reinterpret_cast<void*>( static_cast<std::intptr_t>( maxTotalGuideCount ) ), guidesResult };
		return GetPropertyValues( static_cast<int>( CommandExtension::GetGuideDependencies2 ), firstStrandIndex, count, values );
	}

	/** Sets guide dependencies with variable guide count per strand for specified strand range.

	Concurrency notes:

	In general this function is not thread-safe and can not be used concurrently with any guide dependency functions.
	The only specific case when it can be used concurrently is when total guide count for specified strands is not changed.
	In this case only guide dependencies for specified strand range are accessed and guide dependency functions can be
	concurrently called for non-overlapping strand ranges.
	*/
	bool SetGuideDependencies2( int firstStrandIndex, int count, unsigned const* indicesSource, int totalGuideCount, GuideDependency2 const* guidesSource )
	{
		void const* values[] = { indicesSource, reinterpret_cast<void*>( static_cast<std::intptr_t>( totalGuideCount ) ), guidesSource };
		return SetPropertyValues( static_cast<int>( CommandExtension::SetGuideDependencies2 ), firstStrandIndex, count, values );
	}

	/** Retrieves the positions of first points of each strand in object space
	*/

	struct GetRootPositionsResult
	{
		CoordinateSpace resultSpace;
		Vector3* values;
	};

	bool GetRootPositions( int firstStrandIndex, int count, Vector3* result, CoordinateSpace resultSpace ) const
	{
		GetRootPositionsResult resultStruct { resultSpace, result };
		return GetPropertyValues( static_cast<int>( CommandExtension::GetRootPositionsInObjectSpace ), firstStrandIndex, count, &resultStruct );
	}

	EPHERE_NODISCARD std::vector<Vector3> GetRootPositions( int firstStrandIndex = 0, int count = -1 ) const
	{
		std::vector<Vector3> result( count != -1 ? count : GetStrandCount() - firstStrandIndex );
		GetRootPositions( firstStrandIndex, static_cast<int>( result.size() ), result.data(), CoordinateSpace::Object );

		return result;
	}

	/** Deletes strands specified by their indices
	 * @param strandIndices Indices of strands to delete, sorted from smallest to largest
	 * @param count Number of strands to delete
	 * @return true if strands were deleted
	 */
	bool DeleteStrandsByIndices( int const* strandIndices, int count )
	{
		return SetPropertyValues( static_cast<int>( CommandExtension::DeleteStrandsByIndices ), 0, count, strandIndices );
	}

	bool DeleteStrandsByIndices( Span<int const> strandIndices )
	{
		return DeleteStrandsByIndices( strandIndices.data(), strandIndices.size() );
	}

	EPHERE_NODISCARD bool HasSurfaceDependency2() const
	{
		return HasProperty( static_cast<int>( CommandExtension::SurfaceDependency2 ) );
	}

	void SetUseSurfaceDependency2( bool value )
	{
		SetUsesProperty( static_cast<int>( value ? CommandExtension::SurfaceDependency2 : CommandExtension::SurfaceDependency2Off ) );
	}

	bool GetSurfaceDependencies2( int firstStrandIndex, int count, Geometry::SurfacePosition* result ) const
	{
		return GetPropertyValues( static_cast<int>( CommandExtension::SurfaceDependency2 ), firstStrandIndex, count, result );
	}

	EPHERE_NODISCARD std::vector<Geometry::SurfacePosition> GetSurfaceDependencies2( int firstStrandIndex = 0, int count = -1 ) const
	{
		std::vector<Geometry::SurfacePosition> result( count != -1 ? count : GetStrandCount() - firstStrandIndex );
		GetSurfaceDependencies2( firstStrandIndex, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	bool SetSurfaceDependencies2( int firstStrandIndex, int count, Geometry::SurfacePosition const* values )
	{
		return SetPropertyValues( static_cast<int>( CommandExtension::SurfaceDependency2 ), firstStrandIndex, count, values );
	}

	bool SetSurfaceDependencies2( Span<Geometry::SurfacePosition const> values, int firstStrandIndex = 0 )
	{
		return SetSurfaceDependencies2( firstStrandIndex, values.size(), values.data() );
	}

	EPHERE_NODISCARD Geometry::SurfaceTangentComputeMethod GetSurfaceTangentComputeMethod() const
	{
		Geometry::SurfaceTangentComputeMethod result;
		GetPropertyValues( static_cast<int>( CommandExtension::SurfaceTangentComputeMethod ), -1, -1, &result );
		return result;
	}

	void SetSurfaceTangentComputeMethod( Geometry::SurfaceTangentComputeMethod value )
	{
		SetPropertyValues( static_cast<int>( CommandExtension::SurfaceTangentComputeMethod ), -1, -1, &value );
	}

	IHair1& operator=( IHair1 const& other )
	{
		CopyFrom( other, true, true, true, true, true );
		return *this;
	}

	static bool CheckPointNaN( Vector3 const& value )
	{
		return !value.HasNaNElements();
	}

	static bool CheckPoints( Vector3 const* values, int const count )
	{
		for( auto index = 0; index < count; ++index )
		{
			if( !CheckPointNaN( values[index] ) )
			{
				return false;
			}
		}

		return true;
	}

	/** Get data values for a specified root channel
	*  @param channelIndex Channel index
	*  @return List of float data values
	*/
	EPHERE_NODISCARD std::vector<float> GetRootValuesForChannel( int const channelIndex ) const
	{
		std::vector<float> result( GetStrandCount() );
		GetStrandChannelData( IHair1::PerStrand, channelIndex, 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	/** Get data values for a specified vertex channel
	*  @param channelIndex Channel index
	*  @return List of float data values
	*/
	EPHERE_NODISCARD std::vector<float> GetVertexValuesForChannel( int const channelIndex ) const
	{
		std::vector<float> result( GetVertexCount() );
		GetStrandChannelData( IHair1::PerVertex, channelIndex, 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	//! This function is added to support Maya OxGetStrandIds command
	EPHERE_NODISCARD std::vector<StrandId> GetStrandIdsVector() const
	{
		std::vector<StrandId> result( GetStrandCount() );
		GetStrandIds( 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	//! This function is added to support Maya OxGetStrandGroups command
	EPHERE_NODISCARD std::vector<int> GetStrandGroupsVector() const
	{
		std::vector<int> result( GetStrandCount() );
		GetStrandGroups( 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

protected:

	/** Default constructor */
	IHair1()
	{
	}

	/** Default copy constructor */
	IHair1( IHair1 const& )
	{
	}

	/** Default move constructor */
	IHair1( IHair1&& ) EPHERE_NOEXCEPT
	{
	}

	/** Default destructor */
	~IHair1()
	{
	}

	/** Default move assignment operator */
	IHair1& operator=( IHair1&& ) EPHERE_NOEXCEPT
	{
		return *this;
	}
};

class IHair2 : public IHair1
{
public:

	IHair2& operator=( IHair2 const& other )
	{
		CopyFrom( other, true, true, true, true, true );
		return *this;
	}

	using IHair1::GetStrandPoints;
	virtual bool GetStrandPoints( int strandIndex, int firstPointIndex, int pointCount, Geometry::Vector3d* result, CoordinateSpace resultSpace ) const = 0;

	using IHair1::SetStrandPoints;
	virtual bool SetStrandPoints( int strandIndex, int firstPointIndex, int pointCount, Geometry::Vector3d const* values, CoordinateSpace sourceSpace ) = 0;

	using IHair1::GetVertices;
	virtual bool GetVertices( int firstIndex, int count, Geometry::Vector3d* result, CoordinateSpace space ) const = 0;

	using IHair1::SetVertices;
	virtual bool SetVertices( int firstIndex, int count, Geometry::Vector3d const* values, CoordinateSpace space ) = 0;

	using IHair1::GetStrandToObjectTransforms;
	virtual bool GetStrandToObjectTransforms( int firstStrandIndex, int count, Geometry::Xform3d* result ) const = 0;

	using IHair1::SetStrandToObjectTransforms;
	virtual bool SetStrandToObjectTransforms( int firstStrandIndex, int count, Geometry::Xform3d const* values ) = 0;

	using IHair1::GetStrandChannelData;
	virtual bool GetStrandChannelData( StrandDataType type, int channelIndex, int firstElementIndex, int count, double* result ) const = 0;

	using IHair1::SetStrandChannelData;
	virtual bool SetStrandChannelData( StrandDataType type, int channelIndex, int firstElementIndex, int count, double const* source ) = 0;

	using IHair1::GetTextureCoordinates;
	virtual bool GetTextureCoordinates( int channelIndex, int firstIndex, int count, Geometry::TextureCoordinated* result, StrandDataType dataType ) const = 0;

	using IHair1::SetTextureCoordinates;
	virtual bool SetTextureCoordinates( int channelIndex, int firstIndex, int count, Geometry::TextureCoordinated const* values, StrandDataType dataType ) = 0;

	using IHair1::GetStrandRotations;
	virtual bool GetStrandRotations( int firstElementIndex, int count, double* result, StrandDataType dataType ) const = 0;

	using IHair1::SetStrandRotations;
	virtual bool SetStrandRotations( int firstElementIndex, int count, double const* values, StrandDataType dataType ) = 0;

	using IHair1::GetWidths;
	virtual bool GetWidths( int firstVertexIndex, int count, double* result ) const = 0;

	using IHair1::SetWidths;
	virtual bool SetWidths( int firstVertexIndex, int count, double const* values ) = 0;

	// TODO: Move to HairUtilities

	bool SetStrandPoints( int const strandIndex, Span<Vector3 const> values, CoordinateSpace const space, int const firstPointIndex = 0 )
	{
		return SetStrandPoints( strandIndex, firstPointIndex, values.size(), values.data(), space );
	}

	void SetStrandPoint( int const strandIndex, int const pointIndex, Vector3 const& value, CoordinateSpace const space )
	{
		SetStrandPoints( strandIndex, pointIndex, 1, &value, space );
	}

	EPHERE_NODISCARD std::vector<Vector3> GetStrandPoints( int const strandIndex, CoordinateSpace const space ) const
	{
		std::vector<Vector3> result( GetStrandPointCount( strandIndex ) );
		GetStrandPoints( strandIndex, 0, static_cast<int>( result.size() ), result.data(), space );
		return result;
	}

	EPHERE_NODISCARD std::vector<Xform3> GetStrandToObjectTransforms( int firstStrandIndex = 0, int count = -1 ) const
	{
		std::vector<Xform3> result( count != -1 ? count : GetStrandCount() - firstStrandIndex );
		GetStrandToObjectTransforms( firstStrandIndex, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	bool SetStrandToObjectTransforms( Span<Xform3 const> values, int firstStrandIndex = 0 )
	{
		return SetStrandToObjectTransforms( firstStrandIndex, values.size(), values.data() );
	}

	EPHERE_NODISCARD Real GetStrandChannelData( int const channelIndex, int const strandIndex ) const
	{
		Real result;
		return GetStrandChannelData( PerStrand, channelIndex, strandIndex, 1, &result ) ? result : 0;
	}

	EPHERE_NODISCARD bool GetVertexToObjectTransforms( int const firstVertexIndex, Span<Xform3> result ) const
	{
		return GetPropertyValues( static_cast<int>( CommandExtension::VertexToObjectTransforms ), firstVertexIndex, result.size(), result.data() );
	}

	EPHERE_NODISCARD Real GetVertexChannelData( int const channelIndex, int const vertexIndex ) const
	{
		Real result;
		return GetStrandChannelData( PerVertex, channelIndex, vertexIndex, 1, &result ) ? result : 0;
	}

	EPHERE_NODISCARD Real GetVertexChannelData( int const channelIndex, int const strandIndex, int const pointIndex ) const
	{
		return GetVertexChannelData( channelIndex, GetStrandFirstVertexIndex( strandIndex ) + pointIndex );
	}

	void SetStrandChannelData( int const channelIndex, int const strandIndex, double const value )
	{
		SetStrandChannelData( PerStrand, channelIndex, strandIndex, 1, &value );
	}

	void SetVertexChannelData( int const channelIndex, int const vertexIndex, double const value )
	{
		SetStrandChannelData( PerVertex, channelIndex, vertexIndex, 1, &value );
	}

	void SetVertexChannelData( int const channelIndex, int const strandIndex, int const pointIndex, double const value )
	{
		SetVertexChannelData( channelIndex, GetStrandFirstVertexIndex( strandIndex ) + pointIndex, value );
	}

	EPHERE_NODISCARD std::vector<Real> GetWidthsVector() const
	{
		std::vector<Real> result( GetVertexCount() );
		GetWidths( 0, static_cast<int>( result.size() ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<Real> GetStrandWidths( int strandIndex ) const
	{
		std::vector<Real> result( GetStrandPointCount( strandIndex ) );
		GetWidths( GetStrandFirstVertexIndex( strandIndex ), static_cast<int>( result.size() ), result.data() );
		return result;
	}

	bool SetStrandRotations( Span<Real const> values, int firstStrandIndex = 0 )
	{
		return SetStrandRotations( firstStrandIndex, values.size(), values.data(), IHair1::PerStrand );
	}

	bool SetWidths( Span<Real const> values, int firstVertexIndex = 0 )
	{
		return SetWidths( firstVertexIndex, values.size(), values.data() );
	}

protected:

	/** Default constructor */
	IHair2()
	{
	}

	/** Default copy constructor */
	IHair2( IHair2 const& )
	{
	}

	/** Default move constructor */
	IHair2( IHair2&& ) EPHERE_NOEXCEPT
	{
	}

	/** Default destructor */
	~IHair2()
	{
	}

	/** Default move assignment operator */
	IHair2& operator=( IHair2&& ) EPHERE_NOEXCEPT
	{
		return *this;
	}
};

class IHair3 : public IHair2
{
public:
	virtual ~IHair3() = default;	

	virtual void SetDistributionMesh( std::shared_ptr<IPolygonMeshSA> const& value ) = 0;

	EPHERE_NODISCARD virtual std::shared_ptr<IPolygonMeshSA> const& GetDistributionMesh() const = 0;

	virtual void Clear()
	{
		IHair1::Clear();
	}	

	// New IHair functionalities to be able to access hair data directly

	EPHERE_NODISCARD virtual std::vector<Vector3> const& ReadVertices() const = 0;
	EPHERE_NODISCARD virtual std::vector<Vector3>& WriteVertices() = 0;
	EPHERE_NODISCARD virtual std::vector<StrandTopology> const& ReadStrandTopologies() const = 0;

	EPHERE_NODISCARD Span<Vector3 const> ReadStrandVertices( int strandIndex ) const
	{
		auto const vertexCount = GetStrandPointCount( strandIndex );
		auto const startIndex = GetStrandFirstVertexIndex( strandIndex );
		return Span<Vector3 const>{ ReadVertices().data() + startIndex, vertexCount };
	}

	EPHERE_NODISCARD virtual std::vector<StrandId> const& ReadStrandIds() const = 0;
	EPHERE_NODISCARD virtual Span<Xform3 const> ReadStrandToObjectTransforms() const = 0;

	EPHERE_NODISCARD virtual bool IsTransformationNeeded( IHair1::CoordinateSpace coordinateSpace ) const
	{
		if( GetCoordinateSpace() != coordinateSpace && HasStrandToObjectTransforms() )
		{
			return true;
		}

		return false;
	}
};

//typedef IHair3 IHair;

struct IHair_Extension1
{
	virtual bool UpdateStrandTransformationsFromDistributionMesh( IPolygonMeshSA const* distributionMesh, int startIndex, int count, bool updateBaseStrands, bool forceStrandCoordinates, 
																  Geometry::PolygonMeshRefiner<Real>* polygonMeshRefiner = nullptr, Span<Vector3 const> vertexTangents = {} ) = 0;

	EPHERE_NODISCARD virtual UniquePtr<IHair> CloneHair() const = 0;

protected:
	~IHair_Extension1()
	{
	}
};

struct IHair_ExtensionSaveLoad
{
	virtual bool Save( IChunkOutputStream& ) const = 0;

	virtual bool Load( IChunkInputStream& ) = 0;

protected:
	~IHair_ExtensionSaveLoad()
	{
	}
};

}
}
