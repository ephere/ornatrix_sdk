// Parameters system public interface

// Must compile with VC 2012 / GCC 4.8 (could go back to VC 2010 if needed, only VC 2012 feature is 'enum class')
// Must be "reasonably binary-compatible", i.e. modules compiled with different versions of the same compiler or in Debug vs. Release configuration should work together

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppClangTidyModernizeReturnBracedInitList
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedCopyWithUserProvidedDtor
#pragma once

#include "Ephere/Core/Parameters/Types.h"
#include "Ephere/NativeTools/StlExtensions.h"
#include "Ephere/NativeTools/StringToolsBase.h"

#include <ostream>
#include <utility>

namespace Ephere { namespace Parameters
{

enum class ParameterId : int
{
	Invalid = 0,

	// Constants useful in dynamic descriptors
	_1,
	_2,
	_3,
	_4,
};

struct IContainer;

enum class Direction
{
	In,
	Out,
	InOut
};

// Purpose and units

enum class Purpose
{
	Undefined,

	// String
	FilePath,
	DirectoryPath,
	Url,

	// Int
	RandomSeed,

	// Float
	Fraction,
	Angle,
	Time,
	CurrentTime,
	Distance,
	Velocity,

	// Vector2/3
	Direction,
	UnitDirection,

	// PolygonMeshParameter
	DistributionMesh,
};

enum class AngleUnits
{
	Undefined,
	Degrees,
	Radians,
};

enum class TimeUnits
{
	Undefined,
	Milliseconds,
	Seconds,
};

enum class DistanceUnits
{
	Undefined,

	Millimeters,
	Centimeters,
	Meters,
	Kilometers,

	Inches,
	Feet,
	Miles,
};

// Virtual interfaces

struct IGroupDescriptor
{
	virtual ~IGroupDescriptor()
	{
	}

	EPHERE_NODISCARD virtual char const* GetName() const = 0;

	EPHERE_NODISCARD virtual IGroupDescriptor const* GetOwner() const = 0;

protected:

	IGroupDescriptor()
	{
	}

	IGroupDescriptor( IGroupDescriptor const& )
	{
	}
};

struct IParameterDescriptor
{
	virtual ~IParameterDescriptor()
	{
	}

	EPHERE_NODISCARD virtual ParameterId GetId() const = 0;

	EPHERE_NODISCARD virtual IGroupDescriptor const& GetOwner() const = 0;

	EPHERE_NODISCARD virtual char const* GetName() const = 0;

	EPHERE_NODISCARD virtual IType const& GetType() const = 0;

	EPHERE_NODISCARD TypeId GetTypeId() const
	{
		return GetType().GetTypeId();
	}

	template <typename T>
	EPHERE_NODISCARD bool IsOfType() const
	{
		return IsSame<T>( GetTypeId() );
	}

	EPHERE_NODISCARD virtual bool GetIsArray() const = 0;

	EPHERE_NODISCARD virtual bool GetIsTransient() const = 0;

	EPHERE_NODISCARD virtual bool GetIsAnimatable() const = 0;

	EPHERE_NODISCARD virtual Direction GetDirection() const = 0;

	EPHERE_NODISCARD virtual Purpose GetPurpose() const = 0;

	EPHERE_NODISCARD virtual void const* GetDefaultValue() const = 0;

	template <typename T>
	EPHERE_NODISCARD T const& GetDefaultValueAs() const
	{
		DEBUG_ONLY( ASSERT( IsOfType<T>() ) );
		return *static_cast<T const*>( GetDefaultValue() );
	}

	EPHERE_NODISCARD virtual std::pair<void const*, void const*> GetValueRange() const = 0;

	EPHERE_NODISCARD virtual std::pair<void const*, void const*> GetUiValueRange() const = 0;

	template <typename T>
	EPHERE_NODISCARD std::pair<T const*, T const*> GetValueRangeAs() const
	{
		DEBUG_ONLY( ASSERT( IsOfType<T>() ) );
		auto const range = GetValueRange();
		return std::make_pair( static_cast<T const*>( range.first ), static_cast<T const*>( range.second ) );
	}

	template <typename T>
	EPHERE_NODISCARD std::pair<T const*, T const*> GetUiValueRangeAs() const
	{
		DEBUG_ONLY( ASSERT( IsOfType<T>() ) );
		auto const range = GetUiValueRange();
		return std::make_pair( static_cast<T const*>( range.first ), static_cast<T const*>( range.second ) );
	}

protected:

	IParameterDescriptor()
	{
	}

	IParameterDescriptor( IParameterDescriptor const& )
	{
	}
};

struct ObsoleteParameter
{
	ParameterId id;
	TypeId elementType;
	bool isArray;

	ObsoleteParameter()
		: id( ParameterId::Invalid )
		, isArray( false )
	{
	}

	ObsoleteParameter( ParameterId id, TypeId elementType, bool isArray )
		: id( id )
		, elementType( elementType )
		, isArray( isArray )
	{
	}
};

struct ParameterDescriptorIterator;

struct ISetDescriptor : IGroupDescriptor
{
	EPHERE_NODISCARD virtual int GetVersion() const = 0;

	EPHERE_NODISCARD virtual int GetParameterCount() const = 0;

	EPHERE_NODISCARD virtual IParameterDescriptor const* GetParameterByIndex( int index ) const = 0;

	EPHERE_NODISCARD virtual IParameterDescriptor const* GetParameterById( ParameterId ) const = 0;

	EPHERE_NODISCARD virtual int GetParameterIndex( ParameterId ) const = 0;

	EPHERE_NODISCARD virtual IParameterDescriptor const* FindParameter( char const* name ) const = 0;

	template <typename TDescriptor>
	EPHERE_NODISCARD IParameterDescriptor const* Get() const
	{
		return GetParameterById( TDescriptor::Id );
	}

	virtual IParameterDescriptor* AddDynamicParameterDescriptor( ParameterId, char const* name, IType const&, Direction = Direction::In, Purpose = Purpose::Undefined ) = 0;

	EPHERE_NODISCARD virtual IContainer* CreateContainer() const = 0;

	EPHERE_NODISCARD IGroupDescriptor const* GetOwner() const override
	{
		return nullptr;
	}

	EPHERE_NODISCARD virtual Span<ObsoleteParameter const> GetObsoleteParameters() const = 0;

	EPHERE_NODISCARD ObsoleteParameter GetObsoleteParameterById( ParameterId id ) const
	{
		auto const obsolete = GetObsoleteParameters();
		auto const iterator = std::find_if( obsolete.begin(), obsolete.end(), [id]( ObsoleteParameter const& p )
		{
			return p.id == id;
		} );
		return iterator != obsolete.end() ? *iterator : ObsoleteParameter();
	}

	virtual void UpdateFromOldVersion( IContainer&, int loadedVersion, Span<std::pair<ParameterId, void*> const> obsoleteValues ) const = 0;

	EPHERE_NODISCARD virtual ISetDescriptor* Clone( char const* newName = "" ) const = 0;

	EPHERE_NODISCARD ParameterDescriptorIterator begin() const;

	EPHERE_NODISCARD ParameterDescriptorIterator end() const;
};


template <typename TValueType>
struct Parameter;

struct IParameter
{
	virtual ~IParameter()
	{
	}

	EPHERE_NODISCARD virtual IParameterDescriptor const& GetDescriptor() const = 0;

	// Returns true if the parameter type is compatible with T
	template <typename T>
	EPHERE_NODISCARD bool IsCompatibleWith() const
	{
		return GetDescriptor().GetType().IsCompatibleWith<T>();
	}

	// Returns true if the parameter holds a scalar type or an array of a type that is compatible with TElement
	template <typename TElement>
	EPHERE_NODISCARD bool IsCompatibleWithElement() const
	{
		auto const& type = GetDescriptor().GetType();
		return !type.IsArray() ? type.IsCompatibleWith<TElement>() : type.GetElementType().IsCompatibleWith<TElement>();
	}


	EPHERE_NODISCARD virtual void const* GetValueImpl( TypeId ) const = 0;

	// Sets newValue into this parameter through the assignment operator (IType::AssignValue), which might either do a full or shallow copy (sharing)
	virtual bool SetValueImpl( TypeId, void const* newValue ) = 0;

	// newValue gets force-copied into this parameter via IType::CopyValue, which may be different from just assigning it (IType::CopyValue vs AssignValue/ShareValue),
	virtual bool CopyValueImpl( TypeId, void const* newValue ) = 0;

	// newValue gets shallow-copied (shared) into this parameter via IType::ShareValue, if the type supports it
	virtual bool ShareValueImpl( TypeId, void const* newValue ) = 0;

	// newValue gets moved into this parameter via IType::MoveValue, if the type supports it (otherwise it's copied)
	virtual bool MoveValueImpl( TypeId, void* newValue ) = 0;


	EPHERE_NODISCARD virtual int GetArrayLength() const = 0;

	/*! If the type does not match, ArrayView::data is nullptr and stride is 0.
	Notice that data is also nullptr when the array is empty, so check if stride is positive to see if the element type matches.
	Works for scalar parameters too, returns an ArrayView with count=1.
	*/
	EPHERE_NODISCARD virtual ArrayView GetValuesImpl( TypeId elementType ) const = 0;

	//! Replaces a range of values within the current array with new ones via IArrayType::SetRange, which uses the element's copy assignment operator
	virtual bool SetRangeImpl( TypeId elementType, std::pair<void const*, int> sourceRange, int destinationIndex = 0 ) = 0;

	//! Replaces a range of values within the current array with new ones via IArrayType::MoveRange, which uses the element's move assignment operator
	virtual bool MoveRangeImpl( TypeId elementType, std::pair<void*, int> sourceRange, int destinationIndex = 0 ) = 0;

	virtual void Resize( int newSize ) = 0;

	virtual bool ResizeImpl( int newSize, TypeId elementType, void const* newValue ) = 0;

	virtual void Erase( int startIndex, int count = 1 ) = 0;

	void Clear()
	{
		Resize( 0 );
	}

	template <typename T, typename TEnable = typename Type<T>::ElementType>
	EPHERE_NODISCARD Parameter<T> const* AsType() const
	{
		return GetDescriptor().IsOfType<T>() ? static_cast<Parameter<T> const*>( this ) : nullptr;
	}

	template <typename T, typename TEnable = typename Type<T>::ElementType>
	Parameter<T>* AsType()
	{
		return GetDescriptor().IsOfType<T>() ? static_cast<Parameter<T>*>( this ) : nullptr;
	}

	EPHERE_NODISCARD bool IsDefaultValue() const
	{
		auto const& descriptor = GetDescriptor();
		return descriptor.GetType().AreEqual( descriptor.GetDefaultValue(), GetValueImpl( descriptor.GetTypeId() ) );
	}

	template<typename T, typename TEnable = typename Type<T>::ElementType>
	bool SetValue( T const& newValue )
	{
		return SetValueImpl( GetTypeId<T>(), &newValue );
	}

protected:

	IParameter()
	{
	}

	IParameter( IParameter const& )
	{
	}
};


// Helpers

template <typename TValue>
struct Parameter : IParameter
{
	typedef TValue ValueType;

	EPHERE_NODISCARD ValueType const& GetValue() const
	{
		return *static_cast<ValueType const*>( GetValueImpl( GetTypeId<ValueType>() ) );
	}

	ValueType const& operator()() const
	{
		return GetValue();
	}

	Parameter& operator=( ValueType const& value )
	{
		ASSERT( SetValueImpl( GetTypeId<ValueType>(), &value ) );
		return *this;
	}

	Parameter& operator=( ValueType&& value )
	{
		ASSERT( MoveValueImpl( GetTypeId<ValueType>(), &value ) );
		return *this;
	}

	EPHERE_NODISCARD bool operator==( ValueType const& value ) const
	{
		return GetValue() == value;
	}

	EPHERE_NODISCARD bool operator!=( ValueType const& value ) const
	{
		return *this != value;
	}
};

template <typename TElement>
struct Parameter<TElement[]> : IParameter
{
	typedef TElement ElementType;
	typedef Array<ElementType> ValueType;

	EPHERE_NODISCARD ValueType const& GetValue() const
	{
		return *static_cast<ValueType const*>( GetValueImpl( GetTypeId<TElement[]>() ) );
	}

	EPHERE_NODISCARD Span<ElementType const> GetValues() const
	{
		// ReSharper disable once CppRedundantTemplateKeyword - Clang won't compile without 'template'
		return this->GetValuesImpl( GetTypeId<ElementType>() ).template AsSpan<ElementType>();
	}

	Parameter& operator=( ValueType const& value )
	{
		SetValueImpl( GetTypeId<ElementType[]>(), &value );
		return *this;
	}

	Parameter& operator=( ValueType&& value )
	{
		MoveValueImpl( GetTypeId<ElementType[]>(), &value );
		return *this;
	}

	ElementType const& operator[]( int destinationIndex ) const
	{
		return GetValues()[ destinationIndex];
	}

	void SetRange( Span<ElementType const> values, int destinationIndex = 0 )
	{
		SetRangeImpl( GetTypeId<ElementType>(), std::make_pair( values.data(), values.size() ), destinationIndex );
	}

	void MoveRange( Span<ElementType> values, int destinationIndex = 0 )
	{
		MoveRangeImpl( GetTypeId<ElementType>(), std::make_pair( values.data(), values.size() ), destinationIndex );
	}

	void SetElement( int destinationIndex, ElementType const& value )
	{
		SetRange( Span<ElementType const>( &value, &value + 1 ), destinationIndex );
	}

	void SetElement( int destinationIndex, ElementType&& value )
	{
		MoveRange( Span<ElementType>( &value, &value + 1 ), destinationIndex );
	}

	void AppendValue( ElementType value )
	{
		auto length = this->GetArrayLength();
		Resize( length + 1 );
		SetElement( length, std::move( value ) );
	}
};

struct IContainer
{
	virtual ~IContainer()
	{
	}

	EPHERE_NODISCARD virtual ISetDescriptor const& GetDescriptor() const = 0;

	EPHERE_NODISCARD virtual int GetParameterCount() const = 0;

	EPHERE_NODISCARD virtual IParameter const* GetParameterByIndex( int index ) const = 0;

	EPHERE_NODISCARD virtual IParameter const* GetParameterById( ParameterId ) const = 0;

	IParameter* GetParameterByIndex( int index )
	{
		return const_cast<IParameter*>( const_cast<IContainer const*>( this )->GetParameterByIndex( index ) );
	}

	IParameter* GetParameterById( ParameterId id )
	{
		return const_cast<IParameter*>( const_cast<IContainer const*>( this )->GetParameterById( id ) );
	}

	EPHERE_NODISCARD virtual IContainer* Clone() const = 0;

	template <typename TDescriptor>
	EPHERE_NODISCARD Parameter<typename TDescriptor::ValueType> const* Get() const
	{
		auto parameter = GetParameterById( TDescriptor::Id );
		return parameter != nullptr ? parameter->template AsType<typename TDescriptor::ValueType>() : nullptr;
	}

	template <typename TDescriptor>
	Parameter<typename TDescriptor::ValueType>* Get()
	{
		auto parameter = GetParameterById( TDescriptor::Id );
		return parameter != nullptr ? parameter->template AsType<typename TDescriptor::ValueType>() : nullptr;
	}

	template <typename TDescriptor>
	Parameter<typename TDescriptor::ValueType>& Set( typename TDescriptor::ValueType value )
	{
		auto parameter = GetParameterById( TDescriptor::Id )->template AsType<typename TDescriptor::ValueType>();
		DEBUG_ONLY( ASSERT( parameter != nullptr ) );
		*parameter = std::move( value );
		return *parameter;
	}

	template <typename TDescriptor>
	Parameter<typename TDescriptor::ValueType>& Set( Array<typename TDescriptor::ElementType> value )
	{
		static_assert( TDescriptor::IsArray, "Parameters must be of array type" );
		typedef typename TDescriptor::ElementType ElementType;
		auto parameter = GetParameterById( TDescriptor::Id )->template AsType<ElementType[]>();
		DEBUG_ONLY( ASSERT( parameter != nullptr ) );
		*parameter = std::move( value );
		return *parameter;
	}

	template <bool AddConst>
	struct ParameterIterator;

	EPHERE_NODISCARD ParameterIterator<true> begin() const;

	EPHERE_NODISCARD ParameterIterator<true> end() const;

	ParameterIterator<false> begin();

	ParameterIterator<false> end();

protected:

	IContainer()
	{
	}

	IContainer( IContainer const& )
	{
	}
};

template <class TType, class TFunction>
bool DoIfTypeIs( IParameter const& parameter, IType const& type, TFunction&& function )
{
	if( !type.Is<TType>() )
	{
		return false;
	}

	std::forward<TFunction>( function )( parameter, parameter.AsType<TType>()->GetValue() );
	return true;
}


// Public descriptor helper base

template <class TParameterSet, int NId, typename TValue, Direction NDirection = Direction::In, Purpose NPurpose = Purpose::Undefined, bool NIsTransient = false, bool NIsAnimatable = false>
struct ParameterDescriptor
{
#ifdef EPHERE_HAVE_CPP11
	static ParameterId constexpr Id = static_cast<ParameterId>( NId );
#else
	static ParameterId const Id = static_cast<ParameterId>( NId );
#endif

	typedef TParameterSet SetType;

	typedef TValue ValueType;

	static bool const IsArray = std::is_array<ValueType>::value;

	typedef typename std::conditional<IsArray, typename std::remove_extent<ValueType>::type, ValueType>::type ElementType;

	static Direction const ParameterDirection = NDirection;

	static Purpose const ParameterPurpose = NPurpose;

	static bool const IsTransient = NIsTransient;

	static bool const IsAnimatable = NIsAnimatable;


	typedef typename std::conditional<IsArray, Span<ElementType const>, ValueType>::type DefaultValueType;

	static DefaultValueType DefaultValue()
	{
		return DefaultValueType();
	}
};


inline bool operator<( ParameterId left, ParameterId right )
{
	return static_cast<int>( left ) < static_cast<int>( right );
}

inline std::ostream& operator<<( std::ostream& stream, ParameterId id )
{
	return stream << static_cast<int>( id );
}


struct ParameterDescriptorIterator : IndexedIterator<ISetDescriptor, IParameterDescriptor, true>
{
	// Can't inherit the constructor, needs to compile with VC 2012
	//using IndexedIterator<ISetDescriptor, IParameterDescriptor, true>::IndexedIterator;
	ParameterDescriptorIterator( ISetDescriptor const& owner_, int index_ )
		: IndexedIterator<ISetDescriptor, IParameterDescriptor, true>( owner_, index_ )
	{
	}
	
	reference operator*() const
	{
		return *owner->GetParameterByIndex( index );
	}
};

inline ParameterDescriptorIterator ISetDescriptor::begin() const
{
	return ParameterDescriptorIterator( *this, 0 );
}

inline ParameterDescriptorIterator ISetDescriptor::end() const
{
	return ParameterDescriptorIterator( *this, GetParameterCount() );
}

template <bool AddConst>
struct IContainer::ParameterIterator : IndexedIterator<IContainer, IParameter, AddConst>
{
	typedef IndexedIterator<IContainer, IParameter, AddConst> BaseType;

	// Can't inherit the constructor, needs to compile with VC 2012
	//using IndexedIterator<IContainer, IParameter, AddConst>::IndexedIterator;
	ParameterIterator( typename BaseType::OwnerType& owner_, int index_ )
		: BaseType( owner_, index_ )
	{
	}

	typename BaseType::reference operator*() const
	{
		return *this->owner->GetParameterByIndex( this->index );
	}
};

inline IContainer::ParameterIterator<true> IContainer::begin() const
{
	return ParameterIterator<true>( *this, 0 );
}

inline IContainer::ParameterIterator<true> IContainer::end() const
{
	return ParameterIterator<true>( *this, GetParameterCount() );
}

inline IContainer::ParameterIterator<false> IContainer::begin()
{
	return ParameterIterator<false>( *this, 0 );
}

inline IContainer::ParameterIterator<false> IContainer::end()
{
	return ParameterIterator<false>( *this, GetParameterCount() );
}

inline std::string ToReadableString( IContainer const& parameters )
{
	std::string result;
	for( auto const& parameter : parameters )
	{
		auto const& descriptor = parameter.GetDescriptor();
		result += descriptor.GetName();
		result += '\t';
		result += descriptor.GetType().ToReadableString( parameter.GetValueImpl( descriptor.GetTypeId() ) );
		result += '\n';
	}

	return result;
}


inline std::string GetParameterIdentifierName( std::string_view parameterName )
{
	auto identifier = ReplaceAll( parameterName, ".", "" );
	identifier[0] = static_cast<char>( std::tolower( identifier[0] ) );
	return identifier;
}

} }

namespace Ephere
{

template <typename T>
struct GetEnumInfo;

}
