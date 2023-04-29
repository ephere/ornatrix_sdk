// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppTemplateArgumentsCanBeDeduced
// ReSharper disable CppClangTidyModernizeReturnBracedInitList
// ReSharper disable CppVariableCanBeMadeConstexpr
#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/NativeTools/SmartPointers.h"
#include "Ephere/NativeTools/StlExtensions.h"
#include "Ephere/Ornatrix/IHair.h"

namespace Ephere
{
namespace Ornatrix
{
namespace Groom
{

struct IOperator;

typedef UniquePtr<IOperator>( *OperatorFactory )( );

struct OperatorDescriptor
{
	Parameters::String typeName;
	OperatorFactory operatorFactory;
	Parameters::Array<Parameters::ISetDescriptor const*> parameterSetDescriptors;

	bool operator==( OperatorDescriptor const& other ) const
	{
		return typeName == other.typeName;
	}

	bool operator!=( OperatorDescriptor const& other ) const
	{
		return !( *this == other );
	}

	bool operator<( OperatorDescriptor const& other ) const
	{
		return typeName < other.typeName;
	}
};

inline bool operator<( OperatorDescriptor const& desc, Parameters::String const& text )
{
	return desc.typeName < text;
}

inline bool operator<( Parameters::String const& text, OperatorDescriptor const& desc )
{
	return text < desc.typeName;
}


struct IOperatorRegistry
{
	virtual ~IOperatorRegistry()
	{
	}

	virtual OperatorDescriptor const* Add( OperatorDescriptor descriptor ) = 0;

	EPHERE_NODISCARD virtual OperatorDescriptor const* Find( StringView name ) const = 0;
};

template <class TOperator>
OperatorFactory MakeOperatorFactory()
{
	return []() -> UniquePtr<IOperator>
	{
		return UniquePtr<IOperator>( new TOperator() );
	};
}

template <class TOperator>
OperatorDescriptor MakeDescriptor( Parameters::String name )
{
	return OperatorDescriptor
	{
		std::move( name ),
		MakeOperatorFactory<TOperator>(),
		{}
	};
}

template <class TOperator>
OperatorDescriptor MakeDescriptor( Parameters::String name, Parameters::ISetDescriptor const* setDescriptor )
{
	return OperatorDescriptor
	{
		std::move( name ),
		MakeOperatorFactory<TOperator>(),
		{ setDescriptor }
	};
}


struct EvaluationContext
{
	typedef UniquePtr<IHair>( *HairFactoryFunctionType )( bool initAsGuides );

	HairFactoryFunctionType hairFactoryFunction;
};


// Common interface to groom operators
struct IOperator
{
	struct ParameterRef;
	struct ParameterIterator;
	template <bool AddConst> struct ParameterSetIterator;

	virtual ~IOperator()
	{
	}

	//virtual const OperatorDescriptor& GetDescriptor() const = 0;

	EPHERE_NODISCARD virtual int GetParameterSetCount() const
	{
		return 1;
	}

	EPHERE_NODISCARD virtual Parameters::IContainer const& GetParameterSet( int /*index*/ ) const = 0;

	virtual void* GetImplementation()
	{
		return nullptr;
	}

	Parameters::IContainer& GetParameterSet( int index )
	{
		return const_cast<Parameters::IContainer&>( const_cast<IOperator const*>( this )->GetParameterSet( index ) );
	}

	virtual bool Apply( EvaluationContext& ) = 0;

	// If you know the operator won't need any context
	bool Apply()
	{
		EvaluationContext context = { nullptr };
		return Apply( context );
	}


	EPHERE_NODISCARD Iterable<ParameterSetIterator<true>> EnumerateParameterSets() const;

	Iterable<ParameterSetIterator<false>> EnumerateParameterSets();

	Iterable<ParameterIterator> EnumerateParameters();


	EPHERE_NODISCARD bool IsGenerator() const
	{
		if( GetParameterSetCount() != 1 )
		{
			return false;
		}

		auto const& descriptor = GetParameterSet( 0 ).GetDescriptor();
		auto const count = descriptor.GetParameterCount();
		// The generated value is expected to be in the first parameter, at index 0
		if( count < 1 || descriptor.GetParameterByIndex( 0 )->GetDirection() != Parameters::Direction::Out )
		{
			return false;
		}

		return count == 1
			// TODO: Handle other combinations of simple parameters
			// A single Real input at index 1
			|| count == 2 && descriptor.GetParameterByIndex( 1 )->GetDirection() == Parameters::Direction::In && descriptor.GetParameterByIndex( 1 )->GetTypeId() == Parameters::GetTypeId<Real>();
	}

	template <typename T>
	void SetGeneratorValue( T value )
	{
		ASSERT( IsGenerator() );
		ASSERT( GetParameterSet( 0 ).GetParameterByIndex( 0 )->MoveValueImpl( Parameters::GetTypeId<T>(), &value ) );
	}

	template <typename T>
	T const* GetGeneratorValue() const
	{
		ASSERT( IsGenerator() );
		return static_cast<T const*>( GetParameterSet( 0 ).GetParameterByIndex( 0 )->GetValueImpl( Parameters::GetTypeId<T>() ) );
	}

	template <typename T>
	T const* GetOutputValue();

	template <typename T>
	T const* GetFirstInputValue();

	template <typename T>
	ParameterRef GetPort( std::string_view name, Span<Parameters::Direction const> acceptableDirections );

	template <typename T>
	ParameterRef GetOutput( std::string_view name = {} );

	ParameterRef GetFirstOutput( std::string_view name = {} );

	template <typename T>
	ParameterRef GetInput( std::string_view name = {} );

	int GetParameterSetIndex( std::string_view setName )
	{
		for( auto index = 0, count = GetParameterSetCount(); index < count; ++index )
		{
			if( GetParameterSet( index ).GetDescriptor().GetName() == setName )
			{
				return index;
			}
		}

		return -1;
	}

	Parameters::IContainer* GetParameterSet( std::string_view setName )
	{
		auto const index = GetParameterSetIndex( setName );
		return index >= 0 ? &GetParameterSet( index ) : nullptr;
	}

	template <class TParameters>
	typename TParameters::Container* GetParameterSet()
	{
		using ContainerType = typename TParameters::Container;
		return static_cast<ContainerType*>( GetParameterSet( TParameters::Descriptor::GetInstance().GetName() ) );
	}

	template <class TParameters>
	typename TParameters::Container const* GetParameterSet() const
	{
		return const_cast<IOperator*>( this )->GetParameterSet<TParameters>();
	}

	template <class TParameters>
	int GetParameterSetIndex()
	{
		return GetParameterSetIndex( TParameters::Descriptor::GetInstance().GetName() );
	}
};


struct IOperator::ParameterRef
{
	IOperator* owner;
	int setIndex;
	Parameters::ParameterId parameterId;
	Parameters::IContainer* container;
	Parameters::ISetDescriptor const* setDescriptor;
	Parameters::IParameterDescriptor const* descriptor;

	ParameterRef()
		: owner( nullptr ),
		setIndex( -1 ),
		parameterId( Parameters::ParameterId::Invalid ),
		container( nullptr ),
		setDescriptor( nullptr ),
		descriptor( nullptr )
	{
	}

	ParameterRef( IOperator& owner, int setIndex, int parameterIndex )
		: owner( &owner ),
		setIndex( setIndex ),
		container( setIndex >= 0 ? &owner.GetParameterSet( setIndex ) : nullptr ),
		setDescriptor( container != nullptr ? &container->GetDescriptor() : nullptr )
	{
		if( setDescriptor != nullptr && parameterIndex >= 0 )
		{
			descriptor = setDescriptor->GetParameterByIndex( parameterIndex );
			parameterId = descriptor->GetId();
		}
		else
		{
			descriptor = nullptr;
			parameterId = Parameters::ParameterId::Invalid;
		}
	}

	bool operator==( ParameterRef const& other ) const
	{
		return descriptor == other.descriptor;
	}

	bool operator!=( ParameterRef const& other ) const
	{
		return !( *this == other );
	}

	EPHERE_NODISCARD Parameters::IParameter* Get() const
	{
		return container != nullptr ? container->GetParameterById( parameterId ) : nullptr;
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return Get() == nullptr;
	}

	//! ASSERTs if Get() returns nullptr
	Parameters::IParameter* operator->() const
	{
		auto* result = Get();
		ASSERT( result );
		return result;
	}

	//! ASSERTs if Get() returns nullptr
	Parameters::IParameter& operator*() const
	{
		return *operator->();
	}
};

struct IOperator::ParameterIterator
{
	typedef ParameterRef value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::ptrdiff_t difference_type;
	typedef std::forward_iterator_tag iterator_category;

	ParameterRef parameter;
	int parameterIndex;
	int setCount;
	int parameterCount;

	ParameterIterator()
		: parameterIndex( -1 ),
		setCount( -1 ),
		parameterCount( -1 )
	{
	}

	ParameterIterator( IOperator& owner, int setIndex, int parameterIndex )
		: parameter( owner, setIndex, parameterIndex ),
		parameterIndex( parameterIndex ),
		setCount( owner.GetParameterSetCount() )
	{
		ASSERT( setIndex < setCount );
		parameterCount = parameter.setDescriptor->GetParameterCount();
		ASSERT( parameterIndex < parameterCount );
	}

	ParameterIterator& operator++()
	{
		if( parameter.parameterId == Parameters::ParameterId::Invalid )
		{
			return *this;
		}

		++parameterIndex;
		if( parameterIndex < parameterCount )
		{
			parameter = { *parameter.owner, parameter.setIndex, parameterIndex };
		}
		else
		{
			auto const nextSet = parameter.setIndex + 1;
			if( nextSet == setCount )
			{
				parameter = {};
			}
			else
			{
				parameter = { *parameter.owner, nextSet, parameterIndex = 0 };
				parameterCount = parameter.setDescriptor->GetParameterCount();
			}
		}

		return *this;
	}

	bool operator==( ParameterIterator const& other ) const
	{
		return parameter == other.parameter;
	}

	bool operator!=( ParameterIterator const& other ) const
	{
		return !( *this == other );
	}

	ParameterRef const& operator*() const
	{
		return parameter;
	}
};

inline Iterable<IOperator::ParameterIterator> IOperator::EnumerateParameters()
{
	auto const setCount = GetParameterSetCount();
	return Iterable<ParameterIterator>( setCount == 0 ? ParameterIterator{} : ParameterIterator{ *this, 0, 0 }, ParameterIterator{} );
}

template <typename T>
T const* IOperator::GetFirstInputValue()
{
	auto inputs = Subset( EnumerateParameters(), []( ParameterRef const& parameter )
	{
		return parameter.descriptor->GetDirection() != Parameters::Direction::Out;
	} );

	for( auto const& input : inputs )
	{
		if( input.Get() == nullptr )
		{
			continue;
		}

		if( auto result = static_cast<T const*>( input->GetValueImpl( Parameters::GetTypeId<T>() ) ) )
		{
			return result;
		}
	}

	return nullptr;
}

template <typename T>
T const* IOperator::GetOutputValue()
{
	auto outputs = Subset( EnumerateParameters(), []( ParameterRef const& parameter )
	{
		return parameter.descriptor->GetDirection() != Parameters::Direction::In;
	} );

	return outputs.size() == 1 && !outputs[0].IsEmpty() ? static_cast<T const*>( outputs[0]->GetValueImpl( Parameters::GetTypeId<T>() ) ) : nullptr;
}

template <typename T>
IOperator::ParameterRef IOperator::GetPort( std::string_view name, Span<Parameters::Direction const> acceptableDirections )
{
	auto outputs = Subset( EnumerateParameters(), [name, acceptableDirections]( ParameterRef const& parameter )
	{
		auto const direction = parameter.descriptor->GetDirection();
		auto const typeId = parameter.descriptor->GetTypeId();
		return Contains( acceptableDirections, direction ) && typeId == Parameters::GetTypeId<T>() && ( name.empty() || parameter.descriptor->GetName() == name );
	} );

	return outputs.empty() ? ParameterRef{} : outputs[0];
}

template <typename T>
IOperator::ParameterRef IOperator::GetOutput( std::string_view name )
{
	static Parameters::Direction directions[] = { Parameters::Direction::InOut, Parameters::Direction::Out };
	return GetPort<T>( name, directions );
}

inline IOperator::ParameterRef IOperator::GetFirstOutput( std::string_view name )
{
	static Parameters::Direction const AcceptableDirections[] = { Parameters::Direction::InOut, Parameters::Direction::Out };
	auto const outputs = Subset( EnumerateParameters(), [&]( ParameterRef const& parameter )
	{
		auto const direction = parameter.descriptor->GetDirection();
		return Contains( AcceptableDirections, direction ) && ( name.empty() || parameter.descriptor->GetName() == name );
	} );

	return outputs.empty() ? ParameterRef{} : outputs[0];
}

template <typename T>
IOperator::ParameterRef IOperator::GetInput( std::string_view name )
{
	static Parameters::Direction const directions[] = { Parameters::Direction::InOut, Parameters::Direction::In };
	return GetPort<T>( name, directions );
}

template <bool AddConst>
struct IOperator::ParameterSetIterator : IndexedIterator<IOperator, Parameters::IContainer, AddConst>
{
	typedef IndexedIterator<IOperator, Parameters::IContainer, AddConst> BaseType;

	// Can't inherit the constructor, needs to compile with VC 2012
	//using IndexedIterator<IContainer, IParameter, AddConst>::IndexedIterator;
	ParameterSetIterator( typename BaseType::OwnerType& newOwner, int newIndex )
		: BaseType( newOwner, newIndex )
	{
	}

	typename BaseType::reference operator*() const
	{
		return this->owner->GetParameterSet( this->index );
	}
};

inline Iterable<IOperator::ParameterSetIterator<true>> IOperator::EnumerateParameterSets() const
{
	return Iterable<ParameterSetIterator<true>>( ParameterSetIterator<true>{ *this, 0 }, ParameterSetIterator<true>{ *this, GetParameterSetCount() } );
}

inline Iterable<IOperator::ParameterSetIterator<false>> IOperator::EnumerateParameterSets()
{
	return Iterable<ParameterSetIterator<false>>( ParameterSetIterator<false>{ *this, 0 }, ParameterSetIterator<false>{ *this, GetParameterSetCount() } );
}


struct OperatorName
{
	static char const* Mesh() { return "Mesh"; }

	static char const* Curves() { return "Curves"; }

	static char const* Hair() { return "Hair"; }

	static char const* Sphere() { return "Sphere"; }

	static char const* CurrentTime() { return "CurrentTime"; }

	static char const* TextureImage() { return "TextureImage"; }


	static char const* Curl() { return "CurlOperator"; }

	static char const* GuidesFromMesh() { return "GuidesFromMeshOperator"; }

	static char const* GuidesFromCurves() { return "GuidesFromCurvesOperator"; }

	static char const* ChangeWidth() { return "ChangeWidthOperator"; }
};

}
}
}
