// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
#pragma once

#include "Ephere/Core/Parameters/Types.h"
#include "Ephere/NativeTools/StlExtensions.h"
#include "Ephere/Ornatrix/ParameterComponents.h"
#include "Ephere/Ornatrix/Groom/IOperator.h"

namespace Ephere { namespace Ornatrix { namespace Groom
{

struct INode;
class Node;
struct IGraph;
struct OperatorDescriptor;

enum class UpAxisType : int
{
	Undefined,
	Y,
	Z,
};

enum class HandednessType : int
{
	Undefined,
	Right,
	Left,
};

struct CoordinateSystemProperties
{
	UpAxisType upAxis;

	HandednessType handedness;


	explicit CoordinateSystemProperties( UpAxisType upAxis = UpAxisType::Undefined, HandednessType handedness = HandednessType::Undefined )
		: upAxis( upAxis ),
		handedness( handedness )
	{
	}

	bool operator==( CoordinateSystemProperties const& other ) const
	{
		return upAxis == other.upAxis && handedness == other.handedness;
	}

	bool operator!=( CoordinateSystemProperties const& other ) const
	{
		return !( *this == other );
	}

	EPHERE_NODISCARD bool IsUndefined() const
	{
		return upAxis == UpAxisType::Undefined || handedness == HandednessType::Undefined;
	}

	EPHERE_NODISCARD Xform3 GetTransformTo( CoordinateSystemProperties const& other ) const;
};

inline CoordinateSystemProperties const& CoordinateSystem3dsMax()
{
	static CoordinateSystemProperties result{ UpAxisType::Z, HandednessType::Right };
	return result;
}

inline CoordinateSystemProperties const& CoordinateSystemMaya()
{
	static CoordinateSystemProperties result{ UpAxisType::Y, HandednessType::Right };
	return result;
}

inline CoordinateSystemProperties const& CoordinateSystemC4D()
{
	static CoordinateSystemProperties result{ UpAxisType::Y, HandednessType::Left };
	return result;
}

inline CoordinateSystemProperties const& CoordinateSystemUnreal()
{
	static CoordinateSystemProperties result{ UpAxisType::Z, HandednessType::Left };
	return result;
}

inline CoordinateSystemProperties const& CoordinateSystemDefault()
{
	return CoordinateSystem3dsMax();
}


struct NodeRef
{
	union
	{
		INode* node;
		char const* nameData;
	} pointer{};

	int nameLength = -1;


	EPHERE_NODISCARD std::string_view AsView() const
	{
		DEBUG_ONLY( ASSERT( nameLength >= 0 ) );
		return { pointer.nameData, static_cast<std::string_view::size_type>( nameLength ) };
	}

	NodeRef()
	{
		pointer.node = nullptr;
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	NodeRef( INode const& node )
	{
		pointer.node = const_cast<INode*>( &node );
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	NodeRef( Node const& node );

	template <typename T>
	// ReSharper disable once CppNonExplicitConvertingConstructor
	NodeRef( T nodeName/*, std::enable_if_t<std::is_convertible_v<T, std::string_view>, int> = 0*/ )
	{
		std::string_view const nodeNameView = nodeName;
		if( !nodeNameView.empty() )
		{
			pointer.nameData = nodeNameView.data();
			nameLength = static_cast<int>( nodeNameView.length() );
		}
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return nameLength < 0 && pointer.node == nullptr;
	}

	EPHERE_NODISCARD bool IsResolved() const
	{
		return nameLength < 0;
	}

	bool TryResolve( IGraph const& );

	NodeRef& Resolve( IGraph const& graph )
	{
		TryResolve( graph );
		return *this;
	}

	EPHERE_NODISCARD NodeRef ToResolved( IGraph const& graph ) const
	{
		NodeRef result( *this );
		result.TryResolve( graph );
		return result;
	}

	EPHERE_NODISCARD INode* Get() const
	{
		return IsResolved() ? pointer.node : nullptr;
	}

	EPHERE_NODISCARD Node& NodeImpl() const;

	INode& operator*() const
	{
		DEBUG_ONLY( ASSERT( IsResolved() ) );
		return *pointer.node;
	}

	INode* operator->() const
	{
		DEBUG_ONLY( ASSERT( IsResolved() ) );
		return pointer.node;
	}

	friend bool operator==( NodeRef const& left, NodeRef const& right )
	{
		return left.Get() == right.Get();
	}

	friend bool operator==( INode const* left, NodeRef const& right )
	{
		return left == right.Get();
	}

	friend bool operator==( NodeRef const& left, INode const* right )
	{
		return left.Get() == right;
	}

	friend bool operator!=( NodeRef const& left, NodeRef const& right )
	{
		return !( left == right );
	}

	friend bool operator!=( INode const* left, NodeRef const& right )
	{
		return !( left == right );
	}

	friend bool operator!=( NodeRef const& left, INode const* right )
	{
		return !( left == right );
	}
};

struct ParameterRef
{
	NodeRef node;
	Parameters::ParameterId parameterId;
	int setIndex;

	// ReSharper disable once CppNonExplicitConvertingConstructor
	ParameterRef( Parameters::ParameterId parameterId = Parameters::ParameterId::Invalid, int setIndex = 0 )
		: parameterId( parameterId ),
		setIndex( setIndex )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	ParameterRef( IOperator::ParameterRef parameterRef )
		: parameterId( parameterRef.parameterId ),
		setIndex( parameterRef.setIndex )
	{
	}

	ParameterRef( std::string_view nodeName, Parameters::ParameterId parameterId, int setIndex = 0 )
		: node( nodeName ),
		parameterId( parameterId ),
		setIndex( setIndex )
	{
	}

	ParameterRef( INode const& node, Parameters::ParameterId parameterId, int setIndex = 0 )
		: node( node ),
		parameterId( parameterId ),
		setIndex( setIndex )
	{
	}

	ParameterRef( INode const& node, IOperator::ParameterRef const& other )
		: node( node ),
		parameterId( other.parameterId ),
		setIndex( other.setIndex )
	{
	}

	template <class TParameterDescriptor>
	ParameterRef( INode const&, TParameterDescriptor );

	bool TryResolve( IGraph const& graph )
	{
		return node.TryResolve( graph );
	}

	ParameterRef& Resolve( IGraph const& graph )
	{
		node.Resolve( graph );
		return *this;
	}

	EPHERE_NODISCARD ParameterRef ToResolved( IGraph const& graph ) const
	{
		ParameterRef result( *this );
		result.Resolve( graph );
		return result;
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return parameterId == Parameters::ParameterId::Invalid;
	}

	EPHERE_NODISCARD bool IsResolved() const
	{
		return node.IsResolved();
	}

	bool operator==( ParameterRef const& other ) const
	{
		return ( node.IsEmpty() || other.node.IsEmpty() || node == other.node ) && setIndex == other.setIndex && parameterId == other.parameterId;
	}

	bool operator!=( ParameterRef const& other ) const
	{
		return !( *this == other );
	}

	bool operator<( ParameterRef const& other ) const
	{
		DEBUG_ONLY( ASSERT( node.IsEmpty() || other.node.IsEmpty() || node == other.node ) );
		return setIndex < other.setIndex || setIndex == other.setIndex && static_cast<int>( parameterId ) < static_cast<int>( other.parameterId );
	}

	EPHERE_NODISCARD Node& NodeImpl() const;

	EPHERE_NODISCARD Parameters::IParameter* Get() const;

	//! ASSERTs if Get() returns nullptr
	Parameters::IParameter* operator->() const
	{
		auto* result = Get();
		DEBUG_ONLY( ASSERT( result ) );
		return result;
	}

	//! ASSERTs if Get() returns nullptr
	Parameters::IParameter& operator*() const
	{
		return *operator->();
	}

	EPHERE_NODISCARD std::string ToString() const;
};

class Connection
{
public:

	Connection( ParameterRef source, ParameterRef target )
		: source_( source ),
		target_( target )
	{
	}

	Connection( Span<ParameterRef const> sources, ParameterRef target )
		: target_( target )
	{
		if( sources.size() == 1 )
		{
			source_ = sources[0];
		}
		else
		{
			arraySources_.assign( sources.begin(), sources.end() );
		}
	}

	EPHERE_NODISCARD Span<ParameterRef const> GetSources() const
	{
		return arraySources_.empty() ? Span<ParameterRef const>( &source_, 1 ) : Span<ParameterRef const>( arraySources_ );
	}

	EPHERE_NODISCARD ParameterRef const& GetTarget() const
	{
		return target_;
	}

	bool operator==( Connection const& other ) const
	{
		return source_ == other.source_ && arraySources_ == other.arraySources_ && target_ == other.target_;
	}

	bool operator!=( Connection const& other ) const
	{
		return !( *this == other );
	}

	bool operator<( Connection const& other ) const
	{
		return source_ < other.source_;
	}

	struct OrderByTarget
	{
		typedef int is_transparent;

		bool operator()( Connection const& left, Connection const& right ) const
		{
			return left.target_ < right.target_;
		}

		bool operator()( Connection const& left, ParameterRef const& right ) const
		{
			return left.target_ < right;
		}

		bool operator()( ParameterRef const& left, Connection const& right ) const
		{
			return left < right.target_;
		}
	};

private:

	ParameterRef source_;
	ParameterRef target_;
	Parameters::Array<ParameterRef> arraySources_{};
};

struct INode
{
	virtual ~INode()
	{
	}

	EPHERE_NODISCARD virtual Parameters::String GetName() const = 0;

	EPHERE_NODISCARD virtual OperatorDescriptor const& GetDescriptor() const = 0;

	EPHERE_NODISCARD virtual IOperator& GetOperator() const = 0;

	EPHERE_NODISCARD virtual bool IsEnabled() const = 0;

	virtual void SetEnabled( bool enabled ) = 0;

	EPHERE_NODISCARD virtual bool IsDirty() const = 0;

	// Enables the dirty flag and optionally propagates it to all dependent nodes (wired to Out parameters of this one)
	virtual void SetDirty( bool isDirty = true, bool propagate = true ) = 0;

	EPHERE_NODISCARD virtual int GetConnectionCount() const = 0;

	EPHERE_NODISCARD virtual int GetInputConnectionCount() const = 0;

	EPHERE_NODISCARD virtual int GetOutputConnectionCount() const = 0;

	EPHERE_NODISCARD bool HasOutputsOnly() const
	{
		return GetConnectionCount() > 0 && GetInputConnectionCount() == 0;
	}

	EPHERE_NODISCARD virtual Parameters::Array<Connection> GetInputConnections() const = 0;

	EPHERE_NODISCARD virtual Parameters::Array<Connection> GetOutputConnections() const = 0;

	EPHERE_NODISCARD virtual bool HasInputConnection( ParameterRef const& target ) const = 0;

	EPHERE_NODISCARD virtual bool HasOutputConnection( ParameterRef const& source ) const = 0;

	EPHERE_NODISCARD virtual Parameters::Array<ParameterRef> GetInputConnection( ParameterRef const& target ) const = 0;

	EPHERE_NODISCARD virtual Parameters::Array<Connection> GetOutputConnections( ParameterRef const& source ) const = 0;


	template <typename T>
	Parameters::Parameter<T>& GetParameterByIndex( int index, int setIndex = 0 ) const
	{
		auto const parameter = GetOperator().GetParameterSet( setIndex ).GetParameterByIndex( index );
		auto const typed = parameter != nullptr ? parameter->AsType<T>() : nullptr;
		ASSERT( typed != nullptr, "Parameter does not have expected type" );
		return *typed;
	}

	template <typename T>
	Parameters::Parameter<T>& GetParameterById( Parameters::ParameterId id, int setIndex = 0 ) const
	{
		auto const parameter = GetOperator().GetParameterSet( setIndex ).GetParameterById( id );
		auto const typed = parameter != nullptr ? parameter->AsType<T>() : nullptr;
		ASSERT( typed != nullptr, "Parameter does not have expected type" );
		return *typed;
	}

	template <typename TParameterDescriptor>
	Parameters::Parameter<typename TParameterDescriptor::ValueType>& GetParameter() const
	{
		auto& op = GetOperator();
		auto const setIndex = op.GetParameterSetIndex<typename TParameterDescriptor::SetType>();
		ASSERT( setIndex >= 0 );
		auto const parameter = op.GetParameterSet( setIndex ).GetParameterById( TParameterDescriptor::Id );
		auto const typed = parameter != nullptr ? parameter->template AsType<typename TParameterDescriptor::ValueType>() : nullptr;
		ASSERT( typed != nullptr, "Parameter does not have expected type" );
		return *typed;
	}

	template <typename T>
	void SetParameterValueByIndex( int index, T value ) const
	{
		GetParameterByIndex<T>( index ) = std::move( value );
	}

	template <typename T>
	void SetParameterValueById( Parameters::ParameterId id, T value ) const
	{
		GetParameterById<T>( id ) = std::move( value );
	}

	template <typename T>
	void SetParameterValueByIndex( int index, int setIndex, T value ) const
	{
		GetParameterByIndex<T>( index, setIndex ) = std::move( value );
	}

	template <typename T>
	void SetParameterValueById( Parameters::ParameterId id, int setIndex, T value ) const
	{
		GetParameterById<T>( id, setIndex ) = std::move( value );
	}

	template <typename T>
	T const& GetParameterValueByIndex( int index, int setIndex = 0 ) const
	{
		return GetParameterByIndex<T>( index, setIndex ).GetValue();
	}

	template <typename T>
	T const& GetParameterValueById( Parameters::ParameterId id, int setIndex = 0 ) const
	{
		return GetParameterById<T>( id, setIndex ).GetValue();
	}
};


inline char const* CurrentTimeNodeName()
{
	return "currentTime";
}


template <typename TGraph, typename TNode>
class GraphIterator;

struct IGraph
{
	typedef GraphIterator<IGraph, INode> iterator;
	typedef GraphIterator<IGraph, INode> const_iterator;

	virtual ~IGraph()
	{
	}

	EPHERE_NODISCARD virtual CoordinateSystemProperties GetCoordinateSystemProperties() const = 0;

	virtual void SetCoordinateSystemProperties( CoordinateSystemProperties ) = 0;

	EPHERE_NODISCARD virtual Parameters::String GetParametersScript() const = 0;

	virtual void SetParametersScript( StringView newScript ) = 0;

	EPHERE_NODISCARD virtual int GetNodeCount() const = 0;

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return GetNodeCount() < 2;
	}

	EPHERE_NODISCARD virtual INode& GetNode( int nodeIndex ) const = 0;

	EPHERE_NODISCARD virtual Parameters::Array<INode*> GetNodes() const = 0;

	EPHERE_NODISCARD int size() const
	{
		return GetNodeCount();
	}

	// ReSharper disable once CppHiddenFunction
	EPHERE_NODISCARD const_iterator begin() const;

	// ReSharper disable once CppHiddenFunction
	EPHERE_NODISCARD const_iterator end() const;

	EPHERE_NODISCARD virtual INode* FindNode( StringView nodeName ) const = 0;

	EPHERE_NODISCARD INode const& GetCurrentTimeNode() const
	{
		return *FindNode( CurrentTimeNodeName() );
	}

	// The actual name assigned to the node may have a trailing number, to maintain uniqueness of the node names within the graph
	virtual INode& AddNode( StringView name, OperatorDescriptor const&, int index = -1 ) = 0;

	virtual bool RemoveNode( NodeRef ) = 0;

	// Returns the renamed node, if it was found
	virtual INode* RenameNode( NodeRef, StringView newName ) = 0;

	virtual bool Connect( ParameterRef outputRef, ParameterRef inputRef ) = 0;

	virtual bool Connect( Span<ParameterRef const> outputRefsUnresolved, ParameterRef inputRef ) = 0;

	virtual void AutoConnect( NodeRef sourceRef, NodeRef destinationRef ) = 0;
};

template <typename TGraph, typename TNode>
class GraphIterator
{
public:

	typedef TNode value_type;
	typedef value_type& reference;
	typedef value_type* pointer;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef int difference_type;

	GraphIterator( TGraph const& owner, int index )
		: owner_( &owner ),
		index_( index )
	{
	}

	reference operator*() const
	{
		DEBUG_ONLY( ASSERT( index_ >= 0 ) );
		DEBUG_ONLY( ASSERT( index_ < owner_->GetNodeCount() ) );
		return owner_->GetNode( index_ );
	}

	pointer operator->() const
	{
		return &operator*();
	}

	GraphIterator& operator++()
	{
		DEBUG_ONLY( ASSERT( index_ < owner_->GetNodeCount() ) );
		if( index_ < owner_->GetNodeCount() )
		{
			++index_;
		}

		return *this;
	}

	GraphIterator& operator--()
	{
		DEBUG_ONLY( ASSERT( index_ > 0 ) );
		if( index_ > 0 )
		{
			--index_;
		}

		return *this;
	}

	GraphIterator operator+( int const value ) const
	{
		DEBUG_ONLY( ASSERT( ( index_ + value ) < owner_->GetNodeCount() ) );
		return { *owner_, index_ + value };
	}

	bool operator==( GraphIterator const& other ) const
	{
		return owner_ == other.owner_ && index_ == other.index_;
	}

	bool operator!=( GraphIterator const& other ) const
	{
		return !( *this == other );
	}

private:

	TGraph const* owner_;
	int index_;
};


inline bool IsStartPointHairNode( INode const& node )
{
	// There is no connected In parameter of type Hair
	return !AnyOf( node.GetInputConnections(), []( Connection const& connection )
	{
		return connection.GetTarget()->GetDescriptor().template IsOfType<HairParameter>();
	} );
}

inline bool IsDistributionMesh( INode const& node )
{
	return node.GetDescriptor().typeName == OperatorName::Mesh()
		&& AnyOf( node.GetOutputConnections(), []( Connection const& connection )
	{
		return connection.GetTarget()->GetDescriptor().GetPurpose() == Parameters::Purpose::DistributionMesh;
	} );
}

inline INode* FindDistributionMeshNode( IGraph const& groom )
{
	return FindFirstOrNull( groom, IsDistributionMesh );
}

inline bool IsBaseObject( INode const& node )
{
	auto const& typeName = node.GetDescriptor().typeName;
	return typeName == OperatorName::Mesh() || typeName == OperatorName::Sphere() || typeName == OperatorName::Curves();
};


// Implementations

inline IGraph::const_iterator IGraph::begin() const
{
	return { *this, 0 };
}

inline IGraph::const_iterator IGraph::end() const
{
	return { *this, GetNodeCount() };
}


inline bool NodeRef::TryResolve( IGraph const& graph )
{
	if( nameLength > 0 )
	{
		if( auto const result = graph.FindNode( AsView() ) )
		{
			pointer.node = result;
			nameLength = -1;
		}
	}

	return IsResolved();
}

template <class TParameterDescriptor>
ParameterRef::ParameterRef( INode const& node, TParameterDescriptor )
	: ParameterRef( node, TParameterDescriptor::Id, node.GetOperator().GetParameterSetIndex<typename TParameterDescriptor::SetType>() )
{
}

inline Parameters::IParameter* ParameterRef::Get() const
{
	auto* op = node != nullptr ? &node->GetOperator() : nullptr;
	return op != nullptr ? op->GetParameterSet( setIndex ).GetParameterById( parameterId ) : nullptr;
}

inline std::string ParameterRef::ToString() const
{
	ASSERT( IsResolved() && !IsEmpty() );
	auto const setDescriptor = node->GetDescriptor().parameterSetDescriptors[setIndex];
	auto const parameterDescriptor = setDescriptor->GetParameterById( parameterId );
	return std::string( node->GetName() ) + '.' + setDescriptor->GetName() + '.' + parameterDescriptor->GetName();
}

} } }


namespace Ephere { namespace Parameters
{

template <>
struct Type<SharedPtr<Ephere::Ornatrix::Groom::IGraph>> final : TypeBase<SharedPtr<Ornatrix::Groom::IGraph>>
{
	enum : int
	{
		SerializeId = 102
	};

	EPHERE_NODISCARD int GetSerializeId() const override
	{
		return SerializeId;
	}

	void Construct( void* uninitializedMemory ) const override;

	void CopyValue( void const* source, void* target ) const override
	{
		Cast( target ) = Cast( source );
	}

	void ShareValue( void const* source, void* target ) const override
	{
		Cast( target ) = Cast( source );
	}

	bool WriteImpl( IOutputStream&, SharedPtr<Ornatrix::Groom::IGraph> const& ) const override;

	bool ReadImpl( IInputStream&, SharedPtr<Ornatrix::Groom::IGraph>& ) const override;
};

} }
