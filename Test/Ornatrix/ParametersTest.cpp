// ReSharper disable CppClangTidyReadabilityContainerSizeEmpty
#include "PrecompiledHeaders.h"

#include "TestOperator.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Groom/Operators.h"

#include <sstream>
#include <vector>

struct Counted
{
	static unsigned Count;

	// Size big enough to not fit in CustomValue
	std::array<void*, 3> data = {};

	Counted()
	{
		++Count;
	}

	Counted( Counted const& other )
		: data( other.data )
	{
		++Count;
	}

	Counted( Counted&& other ) noexcept
		: data( other.data )
	{
		++Count;
	}

	~Counted()
	{
		--Count;
	}

	Counted& operator=( Counted&& ) = default;

	Counted& operator=( Counted const& ) = default;

	bool operator==( Counted const& ) const
	{
		return false;
	}
};

unsigned Counted::Count = 0;

namespace Ephere::Parameters
{

using BaseTypes = TypeList<bool, int, std::int64_t, float, String>;

template <typename T>
struct RemoveExtentImpl
{
	using Type = std::remove_extent_t<T>;
};

template <typename T>
struct RemoveExtentImpl<Array<T>>
{
	using Type = T;
};

template <typename T>
using RemoveExtent = typename RemoveExtentImpl<T>::Type;

// VC 2017 can't handle the recursive definition (2019 can)
//template <typename T>
//constexpr bool IsDefined = TypeListContains<BaseTypes, T> || std::is_enum_v<T>
//	|| IsSpecialization<T, Parameters::Array> && IsDefined<RemoveExtent<T>>;

template <typename T>
constexpr bool IsElementTypeDefined = TypeListContains<BaseTypes, T> || std::is_enum_v<T>;

template <typename T>
constexpr bool IsDefined = IsSpecialization<T, Array> ? IsElementTypeDefined<RemoveExtent<T>> : IsElementTypeDefined<T>;

static_assert( IsDefined<int> );
static_assert( IsDefined<Ornatrix::TestParameters::WaveformType> );
static_assert( !IsDefined<int[]> );
static_assert( !IsDefined<char[]> );
static_assert( IsDefined<Array<int>> );
static_assert( !IsDefined<Array<char>> );

template <>
struct TypeImpl<Counted> : TypeBase<Counted, Counted, NoSerialization<Counted>, NoStringConversion<Counted>>
{
};

struct NotTriviallyRelocatable
{
	int data;

	NotTriviallyRelocatable()
		: data( ++Counter )
	{
		Registry.insert( this );
	}

	NotTriviallyRelocatable( NotTriviallyRelocatable const& other )
		: data( other.data )
	{
		Registry.insert( this );
	}

	NotTriviallyRelocatable( NotTriviallyRelocatable&& other ) noexcept
		: data( other.data )
	{
		Registry.insert( this );
	}

	NotTriviallyRelocatable& operator=( NotTriviallyRelocatable&& other ) = default;
	NotTriviallyRelocatable& operator=( NotTriviallyRelocatable const& ) = delete;

	~NotTriviallyRelocatable()
	{
		Unregister();
	}

	void Unregister() const
	{
		if( auto const iter = Registry.find( this ); iter != Registry.end() )
		{
			Registry.erase( iter );
		}
		else
		{
			++Missing;
		}
	}

	static int Counter;
	static std::set<NotTriviallyRelocatable const*> Registry;
	static int Missing;

	static bool IsEmpty()
	{
		Counter = 0;
		return Registry.empty() && Missing == 0;
	}
};

int NotTriviallyRelocatable::Counter;
std::set<NotTriviallyRelocatable const*> NotTriviallyRelocatable::Registry;
int NotTriviallyRelocatable::Missing;


template <>
struct IsTriviallyRelocatable<NotTriviallyRelocatable> : std::false_type
{
};

}

namespace Ephere::Ornatrix
{

namespace Groom::Operators
{
OperatorDescriptor const* Test;
}


TEST_CASE( "Parameters_Types" )
{
	using namespace Parameters;

	REQUIRE( GetType<int>().Is( GetType<int>() ) );
	REQUIRE_FALSE( GetType<int>().Is( GetType<int[]>() ) );
	REQUIRE_FALSE( GetType<TestParameters::WaveformType>().Is( GetType<int>() ) );

	REQUIRE( GetType<TestParameters::WaveformType>().IsCompatibleWith<int>() );
	REQUIRE( GetType<int>().IsCompatibleWith<TestParameters::WaveformType>() );
	REQUIRE_FALSE( GetType<TestParameters::WaveformType[]>().IsCompatibleWith<int>() );
	REQUIRE_FALSE( GetType<TestParameters::WaveformType>().IsCompatibleWith<int[]>() );
	REQUIRE( GetType<TestParameters::WaveformType[]>().IsCompatibleWith<int[]>() );

	SECTION( "SharedPtr" )
	{
		SharedPtr<int> pointer;
		auto str = GetType<SharedPtr<int>>().ToString( &pointer );
		REQUIRE( std::equal( str.begin(), str.end(), "\0" ) );
		pointer = SharedPtr( new int( 4 ) );
		str = GetType<SharedPtr<int>>().ToString( &pointer );
		REQUIRE( std::equal( str.begin(), str.end(), "\x1\x4\0\0\0" ) );
	}

	SECTION( "IType_AreEqual" )
	{
		auto int1 = 1, intOne = 1, int2 = 2;
		REQUIRE( GetType<int>().AreEqual( &int1, &intOne ) );
		REQUIRE_FALSE( GetType<int>().AreEqual( &int1, &int2 ) );
	}
}

TEST_CASE( "Parameters_Private" )
{
	SECTION( "Private header contents" )
	{
		RequireFileCrc( "../../Ephere/Ornatrix/Test/TestParametersImpl.g.h", 0xc920683f );
	}

	SECTION( "Enumerate Descriptor" )
	{
		std::stringstream text;

		TestParameters::Descriptor::ForEach( [&text]( auto const& desc )
		{
			using DescriptorType = std::decay_t<decltype( desc )>;
			text << desc.Name << '\n';
			if constexpr( DescriptorType::Traits.template Has< Parameters::RangeCheckTrait<float> >() )
			{
				if( desc.Traits.rangeEnabled )
				{
					text << "\tValue Range: " << desc.Traits.low << " - " << desc.Traits.high << '\n';
				}

				if( desc.Traits.uiRangeEnabled )
				{
					text << "\tUI Range: " << desc.Traits.uiLow << " - " << desc.Traits.uiHigh << '\n';
				}
			}

			if constexpr( DescriptorType::Id == TestParameters::Examples::AuxExample::Id )
			{
				REQUIRE( ( Parameters::HasAuxFieldType<DescriptorType, StrandTopology>() ) );
				REQUIRE( ( Parameters::HasAuxFieldType<DescriptorType, double>() ) );
				REQUIRE_FALSE( Parameters::HasAuxFieldType<DescriptorType, int>() );
			}
			else
			{
				REQUIRE_FALSE( Parameters::HasAuxFieldType<DescriptorType, int>() );
			}
		} );

		//std::cout << text.str() << '\n';
		RequireCrc( text.str(), 0xaf34c778 );
	}

	SECTION( "CustomValue" )
	{
		Parameters::CustomValue v{ 13 };
		REQUIRE( v.As<int>() == 13 );
		Parameters::CustomValue a = Parameters::Array{ 13, 14 };
		REQUIRE( a.As<int[]>()[1] == 14 );

		Parameters::CustomValue b = a;
		REQUIRE( b.As<int[]>()[1] == 14 );

		// Large values are copied, not shared
		b.As<int[]>()[1] = 15;
		REQUIRE( a.As<int[]>()[1] == 14 );

		REQUIRE( Parameters::CustomValue( Ephere::Parameters::GetType<int>() ).As<int>() == 0 );

		Parameters::CustomValue shared{ SharedPtr( new int( 13 ) ) };
		auto sharedCopy = shared;
		REQUIRE( *sharedCopy.As<SharedPtr<int>>() == 13 );

		// CustomValue holding a SharedPtr will clone it on copy
		*sharedCopy.As<SharedPtr<int>>() = 17;
		REQUIRE( *shared.As<SharedPtr<int>>() == 13 );

		// Small type (fits into CustomValue) with non-trivial destructor
		{
			Parameters::CustomValue value = SharedPtr<Counted>::DefaultConstruct();
			REQUIRE( value.IsTypeSmall() );
		}
		REQUIRE( Counted::Count == 0 );

		// Large type (doesn't fit into CustomValue) with non-trivial destructor
		{
			Parameters::CustomValue value = Counted();
			REQUIRE_FALSE( value.IsTypeSmall() );
		}
		REQUIRE( Counted::Count == 0 );
	}

	SECTION( "Usage" )
	{
		TestParameters::Container parameters;

		SECTION( "General" )
		{
			static_assert( TestParameters::Descriptor::Examples::BoolExample::IsAnimatable );
			static_assert( TestParameters::Descriptor::Examples::TransientExample::IsTransient );
			REQUIRE( TestParameters::Descriptor::GetInstance().GetParameterById( TestParameters::Examples::TransientExample::Id )->GetIsTransient() );
			REQUIRE( TestParameters::Descriptor::GetIndex( TestParameters::Magnitude::Channel::Id ) == 2 );
			REQUIRE( TestParameters::Descriptor::FindIdAndIndex( "Magnitude.Channel" ) == std::pair( Parameters::ParameterId{ 4 }, 2 ) );
			REQUIRE( TestParameters::Descriptor::GetInstance().GetParameterById( TestParameters::Magnitude::Channel::Id )->GetName() == std::string_view( "Magnitude.Channel" ) );

			REQUIRE( TestParameters::Descriptor::Waveform::Traits.enumeratedValues[int( TestParameters::WaveformType::Helix )] == std::wstring( L"Helix" ) );
			REQUIRE( TestParameters::Descriptor::Examples::ExternalEnumParameter::Traits.enumeratedValues[int( Ephere::Ornatrix::AiPhysics::AlgorithmType::LinearWithBasePosition )] == std::wstring( L"LinearWithBasePosition" ) );
			REQUIRE( parameters.waveform() == TestParameters::Descriptor::Waveform::DefaultValue() );
			REQUIRE( parameters.examples.externalEnumParameter() == Ephere::Ornatrix::AiPhysics::AlgorithmType::LinearWithBasePosition );

			REQUIRE( parameters.magnitude.value() == 1.0f );
			REQUIRE( parameters.examples.auxExample() == 0.0f );

			parameters.magnitude.value = 3;
			REQUIRE( parameters.magnitude.value() == 3 );

			parameters.magnitude.channel = StrandChannel( StrandChannelType_PerStrand, 13 );
			REQUIRE( parameters.magnitude.channel().index == 13 );

			parameters.phase =
			{
				3.f,
				StrandChannel{ StrandChannelType_PerStrand, 13 },
				TextureMapParameter{}
			};

			REQUIRE( parameters.phase.value.GetUnits() == Parameters::DistanceUnits::Centimeters );
			parameters.phase.value.ChangeUnits( Parameters::DistanceUnits::Millimeters );
			REQUIRE( parameters.phase.value() == 30 );

			{
				auto positionsEditor = parameters.magnitude.sinks.positions.Edit();
				positionsEditor->resize( 3 );
			}
			REQUIRE( parameters.magnitude.sinks.positions.GetArrayLength() == 3 );

			REQUIRE( parameters.examples.meshExample().IsEmpty() );
			parameters.examples.meshExample = std::make_shared<SharedPolygonMesh>();

			REQUIRE( parameters.arrayExamples.meshesExample.GetArrayLength() == 0 );
			parameters.arrayExamples.meshesExample.PushBack( std::make_shared<SharedPolygonMesh>() );

			static_assert( TestParameters::Descriptor::Examples::TargetHairExample::IsTransient );
			static_assert( TestParameters::Descriptor::Examples::TargetHairExample::IsHidden );
			static_assert( TestParameters::Descriptor::Examples::TargetHairExample::Direction == Parameters::Direction::InOut );
			REQUIRE( parameters.examples.targetHairExample().IsEmpty() );
			parameters.examples.targetHairExample = { std::make_shared<SharedHair>(), Xform3::Identity() };

			// Vector3 is initialized to 0,0,0 or 0,0,1 for directions
			REQUIRE( parameters.examples.vectorDirectionExample() == Vector3::ZAxis() );

			REQUIRE( parameters.examples.vectorDistanceExample() == Vector3( 0.3_r, 0.3_r, 0_r ) );
		}

		SECTION( "Arrays and change event" )
		{
			auto changeCount = 0;
			auto lastChangedId = Parameters::ParameterId::Invalid;
			parameters.magnitude.sinks.positions.valueChangedEvent += [&changeCount]( Parameters::IParameter& )
			{
				changeCount++;
			};
			parameters.parameterValueChangedEvent += [&lastChangedId]( Parameters::IContainer&, Parameters::IParameter const& p )
			{
				lastChangedId = p.GetDescriptor().GetId();
			};

			REQUIRE( parameters.magnitude.sinks.ramps().empty() );
			parameters.magnitude.sinks.positions = Parameters::Array<Geometry::SurfacePosition>::Repeat( 5, Geometry::SurfacePosition{} );
			parameters.magnitude.sinks.positions.SetElement( 1, Geometry::SurfacePosition{ 1u, Geometry::Vector2f::XAxis() } );
			parameters.magnitude.sinks.positions.Resize( 4 );
			parameters.magnitude.sinks.positions.Erase( 2, 2 );
			REQUIRE( parameters.magnitude.sinks.positions.GetArrayLength() == 2 );
			REQUIRE( changeCount == 4 );
			auto parametersCopy = parameters;
			parametersCopy.magnitude.sinks.positions.Clear();
			REQUIRE( changeCount == 4 );

			REQUIRE( lastChangedId == Parameters::ParameterId::Invalid );
			parameters.EnableParameterChangedEvents();
			parameters.magnitude.sinks.positions.Resize( 1 );
			REQUIRE( lastChangedId == Parameters::ParameterId{ 7 } );

			// Array parameters of primitive types are resized with 0
			parameters.arrayExamples.enumsExample.Resize( 1 );
			REQUIRE( parameters.arrayExamples.enumsExample()[0] == TestParameters::WaveformType::Sine );
			parameters.arrayExamples.intsExample.Resize( 1 );
			REQUIRE( parameters.arrayExamples.intsExample()[0] == 0 );

			parameters.strandGroup.pattern = "test";
			parameters.examples.auxExample.topology = { 1, 2 };
			parameters.examples.auxExample.accumulator = 1.0;

			REQUIRE( parameters.arrayOfStructExample().IsEmpty() );
			parameters.arrayOfStructExample().PushBack( 1.0f, 2 );
			parameters.arrayOfStructExample().Insert( 0, 2.0f, 3 );
			REQUIRE( parameters.arrayOfStructExample().GetLength() == 2 );
			REQUIRE( parameters.arrayOfStructExample.real()[0] == 2.0f );
			REQUIRE( std::get<0>( parameters.arrayOfStructExample[0].asTuple() ) == 2.0f );
			REQUIRE( parameters.arrayOfStructExample[0].real == 2.0f );
			for( auto value : parameters.arrayOfStructExample() )
			{
				REQUIRE( value.real > 0 );
			}
			parameters.arrayOfStructExample().Resize( 3 );
			REQUIRE( parameters.arrayOfStructExample.real()[2] == 0.0f );
			parameters.arrayOfStructExample().SetElement( 2, 3.0f, 3 );
			REQUIRE( parameters.arrayOfStructExample.real()[2] == 3.0f );
			parameters.arrayOfStructExample().Erase( 2 );
			REQUIRE( parameters.arrayOfStructExample().GetLength() == 2 );
			parameters.arrayOfStructExample().Clear();
			REQUIRE( parameters.arrayOfStructExample().GetLength() == 0 );

			REQUIRE( parameters.arrayOfStructExample().begin() == parameters.arrayOfStructExample().begin() );
			REQUIRE( parameters.arrayOfStructExample().end() == parameters.arrayOfStructExample().end() );
		}

		SECTION( "CustomData" )
		{
			auto changes = 0;
			parameters.examples.customTypeExample.valueChangedEvent += [&changes]( Parameters::IParameter& )
			{
				changes++;
			};

			using Groom::Operators::SomeData;
			REQUIRE( parameters.examples.customTypeExample().x == 0 );
			parameters.examples.customTypeExample = SomeData{ 13, { 14, 15, 16 } };
			REQUIRE( parameters.examples.customTypeExample().x == 13 );
			REQUIRE( parameters.examples.customTypeExample().v[2] == 16 );
			REQUIRE( changes == 1 );
			parameters.examples.customTypeExample.Edit()->x = 17;
			REQUIRE( changes == 2 );
			REQUIRE( parameters.examples.customTypeExample().x == 17 );

			REQUIRE( parameters.examples.customSharedExample()->x == 0 );
		}

		SECTION( "ForEach" )
		{
			parameters.ForEach( []( auto& parameter )
			{
				using ParameterType = std::decay_t<decltype( parameter )>;
				using DescriptorType = typename ParameterType::DescriptorType;

				if constexpr( Parameters::HasAuxFieldType<DescriptorType, StrandTopology>() )
				{
					parameter.topology.startingVertexIndex = 14;
				}
			} );

			REQUIRE( parameters.examples.auxExample.topology.startingVertexIndex == 14 );
		}

		SECTION( "CopyTextureMap" )
		{
			parameters.phase.map = TextureMapParameter{ nullptr, 13 };
			REQUIRE( &parameters.phase.map.evaluator.GetParameter() == &parameters.phase.map() );
			REQUIRE( parameters.phase.map.evaluator.GetParameter().GetTextureChannel() == 13 );
			TestParameters::Container copy;
			REQUIRE( &copy.phase.map.evaluator.GetParameter() == &copy.phase.map() );
			copy = parameters;
			REQUIRE( &copy.phase.map.evaluator.GetParameter() == &copy.phase.map() );
			REQUIRE( copy.phase.map.evaluator.GetParameter().GetTextureChannel() == 13 );
		}

		SECTION( "Copy" )
		{
			parameters.magnitude.sinks.positions = Parameters::Array<Geometry::SurfacePosition>::Repeat( 2, Geometry::SurfacePosition{} );
			auto copy = std::move( parameters );
			REQUIRE( copy.magnitude.sinks.positions.GetArrayLength() == 2 );
		}

		SECTION( "Dynamic" )
		{
			Parameters::DynamicParameterDescriptor dynDescriptor{ Parameters::ParameterId::_1, "dynIntArray", Parameters::GetType<int[]>() };
			Parameters::DynamicParameter dynParameter{ dynDescriptor };
			auto& parameter = *dynParameter.AsType<int[]>();
			parameter.AppendValue( 15 );
			auto dynParameter2 = dynParameter;
			auto& parameter2 = *dynParameter2.AsType<int[]>();
			REQUIRE( parameter2.GetArrayLength() == 1 );
			parameter2.AppendValue( 17 );
			REQUIRE( parameter.GetArrayLength() == 1 );
		}

		SECTION( "Map" )
		{
			auto mapView = parameters.mapExample();  // NOLINT(bugprone-use-after-move)
			REQUIRE( mapView.IsEmpty() );
			REQUIRE( mapView.FindIndex( 1 ) == -1 );

			mapView.InsertOrReplace( 2, 2.0f );
			REQUIRE( mapView.FindIndex( 2 ) == 0 );

			mapView.InsertOrReplace( 1, 1.0f );
			REQUIRE( mapView.FindIndex( 2 ) == 1 );
			REQUIRE( mapView.GetLength() == 2 );
			REQUIRE( mapView.Find( 0 ) == mapView.end() );
			REQUIRE( mapView.Find( 1 ) != mapView.end() );
			REQUIRE( mapView.Find( 1 ).GetKey() == 1 );
			REQUIRE( mapView.Find( 1 ).GetValue() == 1.0f );

			mapView.EraseKey( 1 );
			REQUIRE( mapView.GetLength() == 1 );
			for( auto value : mapView )
			{
				REQUIRE( value.stripId > 0 );
			}

			mapView.InsertOrReplace( 2, 2.5f );
			REQUIRE( mapView[0].volume == 2.5f );

			REQUIRE( parameters.mapExample().begin() == parameters.mapExample().begin() );
			REQUIRE( parameters.mapExample().end() == parameters.mapExample().end() );
		}
	}
}

TEST_CASE( "Parameters_Public" )
{
	Parameters::ISetDescriptor const& globalDescriptor = TestParameters::Descriptor::GetInstance();
	auto const* waveformDescriptor = globalDescriptor.Get<TestParameters::Waveform>();

	SECTION( "Public header contents" )
	{
		RequireFileCrc( "../../Ephere/Ornatrix/Test/TestParameters.g.h", 0x8675ab37 );
	}

	SECTION( "Descriptors" )
	{
		REQUIRE( globalDescriptor.GetVersion() == TestParameters::Version );
		REQUIRE( globalDescriptor.GetParameterCount() == TestParameters::Descriptor::ParameterCount );
		REQUIRE( globalDescriptor.GetName() == std::string( "TestParameters" ) );
		static_assert( TestParameters::Descriptor::ParameterCount + TestParameters::Descriptor::ObsoleteCount == TestParameters::HighestId );
		static_assert( TestParameters::FirstUnusedId == TestParameters::HighestId + 1 );

		REQUIRE( waveformDescriptor->GetId() == TestParameters::Waveform::Id );
		REQUIRE( &waveformDescriptor->GetOwner() == &globalDescriptor );
		REQUIRE( waveformDescriptor->GetName() == std::string( "Waveform" ) );
		static_assert( GetEnumInfo<TestParameters::WaveformType>::Count == 2, "GetEnumInfo::Count mismatch" );
		REQUIRE( GetEnumInfo<TestParameters::WaveformType>::GetValueName( TestParameters::WaveformType::Sine ) == std::string( "Sine" ) );

		REQUIRE( globalDescriptor.GetParameterByIndex( 0 )->GetId() == TestParameters::Waveform::Id );
		REQUIRE( globalDescriptor.GetParameterById( TestParameters::Waveform::Id )->GetId() == TestParameters::Waveform::Id );

		REQUIRE( waveformDescriptor->GetDefaultValueAs<TestParameters::WaveformType>() == TestParameters::WaveformType::Helix );

		REQUIRE( waveformDescriptor->GetValueRange() == std::pair<void const*, void const*>{} );
		auto const magnitudeValueRange = globalDescriptor.Get<TestParameters::Magnitude::Value>()->GetValueRangeAs<float>();
		auto const magnitudeUiRange = globalDescriptor.Get<TestParameters::Magnitude::Value>()->GetUiValueRangeAs<float>();
		REQUIRE( *magnitudeValueRange.first == -1000.0f );
		REQUIRE( *magnitudeValueRange.second == 1000.0f );
		REQUIRE( *magnitudeUiRange.first == 0.0f );
		REQUIRE( *magnitudeUiRange.second == 10.0f );

		auto const phaseValueRange = globalDescriptor.Get<TestParameters::Phase::Value>()->GetValueRangeAs<float>();
		auto const phaseUiRange = globalDescriptor.Get<TestParameters::Phase::Value>()->GetUiValueRangeAs<float>();
		REQUIRE( phaseValueRange.first == nullptr );
		REQUIRE( phaseValueRange.second == nullptr );
		REQUIRE( *phaseUiRange.first == -5.f );
		REQUIRE( *phaseUiRange.second == 5.f );
	}

	SECTION( "Enumerate Descriptor" )
	{
		std::stringstream text;

		for( auto const& parameter : globalDescriptor )
		{
			// Can't include parameter->GetTypeId()->name() in the CRC check as it is platform-dependent
			text << parameter.GetId() << ", " << parameter.GetName() << ", IsArray: " << parameter.GetIsArray() << '\n';
		}

		//std::cout << '\n' << text.str() << '\n';
		RequireCrc( text.str(), 0x2e46c31a );
	}

	auto parameters = std::unique_ptr<Parameters::IContainer>( globalDescriptor.CreateContainer() );

	SECTION( "Basic container operations" )
	{
		SECTION( "Static" )
		{
			for( auto const& parameter : *parameters )
			{
				UNUSED_VALUE( parameter );
			}

			REQUIRE( parameters->Get<TestParameters::Waveform>()->GetValue() == TestParameters::Waveform::DefaultValue() );
			REQUIRE( parameters->Get<TestParameters::Magnitude::Value>()->GetValue() == TestParameters::Magnitude::Value::DefaultValue() );

			parameters->Set<TestParameters::Magnitude::Value>( 3 );
			REQUIRE( parameters->Get<TestParameters::Magnitude::Value>()->GetValue() == 3.0f );

			StrandChannel channel = { StrandChannelType_PerStrand, 13 };
			parameters->Set<TestParameters::Magnitude::Channel>( channel );
			REQUIRE( parameters->Get<TestParameters::Magnitude::Channel>()->GetValue().index == 13 );
		}

		SECTION( "Dynamic" )
		{
			auto dynDescriptor = std::unique_ptr<Parameters::ISetDescriptor>( globalDescriptor.Clone( "Dynamic" ) );
			REQUIRE( dynDescriptor->GetName() == std::string( "Dynamic" ) );
			REQUIRE( dynDescriptor->AddDynamicParameterDescriptor( Parameters::ParameterId::_1, "Cannot overwrite existing id", Parameters::GetType<int[]>() ) == nullptr );
			auto const* dynIntDesc = dynDescriptor->AddDynamicParameterDescriptor( Parameters::ParameterId{ 101 }, "DynamicInt", Parameters::GetType<int>() );
			REQUIRE( dynIntDesc != nullptr );
			REQUIRE( dynDescriptor->AddDynamicParameterDescriptor( Parameters::ParameterId{ 102 }, "DynamicIntArray", Parameters::GetType<int[]>() ) != nullptr );
			REQUIRE( dynDescriptor->GetParameterCount() == TestParameters::Descriptor::ParameterCount + 2 );

			REQUIRE( dynIntDesc->GetId() == Parameters::ParameterId{ 101 } );
			REQUIRE( &dynIntDesc->GetOwner() == dynDescriptor.get() );
			REQUIRE( dynIntDesc->GetName() == std::string( "DynamicInt" ) );
			REQUIRE( Parameters::IsSame<int>( dynIntDesc->GetTypeId() ) );
			REQUIRE_FALSE( dynIntDesc->GetIsArray() );

			REQUIRE( dynDescriptor->GetParameterByIndex( TestParameters::Descriptor::ParameterCount + 1 )->GetIsArray() );

			auto dynParameters = std::unique_ptr<Parameters::IContainer>( dynDescriptor->CreateContainer() );
			REQUIRE( dynParameters->GetParameterCount() == TestParameters::Descriptor::ParameterCount + 2 );

			auto& dynInt = *dynParameters->GetParameterById( Parameters::ParameterId{ 101 } )->AsType<int>();
			auto& dynIntArray = *dynParameters->GetParameterById( Parameters::ParameterId{ 102 } )->AsType<int[]>();
			REQUIRE( dynInt() == 0 );
			dynInt = 13;
			REQUIRE( dynInt() == 13 );

			REQUIRE( dynIntArray.GetArrayLength() == 0 );
			dynIntArray.AppendValue( 17 );
			REQUIRE( dynIntArray.GetArrayLength() == 1 );
			REQUIRE( dynIntArray[0] == 17 );

			auto dynParametersCopy = std::unique_ptr<Parameters::IContainer>( dynParameters->Clone() );
			auto& dynIntCopy = *dynParametersCopy->GetParameterById( Parameters::ParameterId{ 101 } )->AsType<int>();
			REQUIRE( dynIntCopy() == 13 );

			auto& dynIntArrayCopy = *dynParametersCopy->GetParameterById( Parameters::ParameterId{ 102 } )->AsType<int[]>();
			REQUIRE( dynIntArrayCopy.GetArrayLength() == 1 );
			dynIntArrayCopy.AppendValue( 21 );
			// Large values are copied, not shared
			REQUIRE( dynIntArray.GetArrayLength() == 1 );
		}
	}

	SECTION( "Arrays" )
	{
		REQUIRE_FALSE( waveformDescriptor->GetIsArray() );
		REQUIRE( globalDescriptor.Get<TestParameters::Magnitude::Sinks::Positions>()->GetIsArray() );

		REQUIRE( parameters->Get<TestParameters::Waveform>()->GetArrayLength() == 1 );

		REQUIRE( parameters->GetParameterById( TestParameters::ArrayExamples::BoolsExample::Id )->GetValueImpl( Parameters::GetTypeId<bool[]>() ) != nullptr );
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::BoolsExample>()->GetValue().empty() );

		auto waveformType = TestParameters::WaveformType::Sine;
		parameters->GetParameterById( TestParameters::Waveform::Id )->SetValue( waveformType );
		auto waveforms = parameters->GetParameterById( TestParameters::Waveform::Id )->GetValuesImpl( Parameters::GetTypeId<TestParameters::WaveformType>() );
		REQUIRE( waveforms.count == 1 );
		REQUIRE( *static_cast<TestParameters::WaveformType const*>( waveforms.data ) == TestParameters::WaveformType::Sine );

		auto& sinkPositions = *parameters->Get<TestParameters::Magnitude::Sinks::Positions>();
		REQUIRE( sinkPositions.GetArrayLength() == 0 );
		auto sinkPositionValues = parameters->Get<TestParameters::Magnitude::Sinks::Positions>()->GetValues();
		REQUIRE( sinkPositionValues.empty() );

		sinkPositions = Parameters::Array<Geometry::SurfacePosition>::Repeat( 2, Geometry::SurfacePosition() );
		Geometry::SurfacePosition surfacePos = { 2, { 0, 1 } };
		sinkPositions.SetElement( 1, surfacePos );
		REQUIRE( sinkPositions.GetArrayLength() == 2 );
		sinkPositionValues = parameters->Get<TestParameters::Magnitude::Sinks::Positions>()->GetValues();
		REQUIRE( sinkPositionValues.size() == 2 );
		REQUIRE( sinkPositionValues[1].faceIndex == 2 );

		*parameters->Get<TestParameters::ArrayExamples::EnumsExample>() = { TestParameters::WaveformType::Sine };
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::EnumsExample>()->GetArrayLength() == 1 );

		*parameters->GetParameterById( TestParameters::ArrayExamples::BoolsExample::Id )->AsType<bool[]>() = std::array{ true };
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::BoolsExample>()->GetArrayLength() == 1 );

		parameters->Set<TestParameters::ArrayExamples::BoolsExample>( { true, true, true } );
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::BoolsExample>()->GetArrayLength() == 3 );

		Parameters::Array boolArray{ true, false };
		REQUIRE( parameters->GetParameterById( TestParameters::ArrayExamples::BoolsExample::Id )->SetValueImpl( Parameters::GetTypeId<bool[]>(), &boolArray ) );
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::BoolsExample>()->GetArrayLength() == 2 );

		// Arrays of enums can be assigned arrays of ints through the public interface
		Parameters::Array intArray{ 1, 0 };
		REQUIRE( parameters->GetParameterById( TestParameters::ArrayExamples::EnumsExample::Id )->SetValueImpl( Parameters::GetTypeId<int[]>(), &intArray ) );
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::EnumsExample>()->GetArrayLength() == 2 );

		intArray.push_back( 1 );
		REQUIRE( parameters->GetParameterById( TestParameters::ArrayExamples::EnumsExample::Id )->MoveValueImpl( Parameters::GetTypeId<int[]>(), &intArray ) );
		REQUIRE( parameters->Get<TestParameters::ArrayExamples::EnumsExample>()->GetArrayLength() == 3 );
		REQUIRE( intArray.empty() );

		REQUIRE(
			parameters->Get<TestParameters::ArrayExamples::EnumsExample>()->GetValue()
			== Parameters::Array<TestParameters::WaveformType>{ TestParameters::WaveformType::Helix, TestParameters::WaveformType::Sine, TestParameters::WaveformType::Helix } );
	}

	SECTION( "Array of Ramps" )
	{
		auto& magnitudeRamps = *parameters->Get<TestParameters::Magnitude::Sinks::Ramps>();
		REQUIRE( magnitudeRamps.GetValues().empty() );
		Parameters::Array<Ramp> newRamps;
		newRamps.push_back( Ramp( 1 ) );
		auto copy = newRamps;
		magnitudeRamps = std::move( newRamps );
		REQUIRE( magnitudeRamps.GetArrayLength() == 1 );
		auto ramps = magnitudeRamps.GetValues();
		REQUIRE( ramps.size() == 1 );
		REQUIRE( ramps[0] == copy[0] );
		//ramps[0].reset(); // won't compile, ramps[0] is const
	}

	SECTION( "ToReadableString" )
	{
		*parameters->Get<TestParameters::StrandGroup::Pattern>() = "pattern";
		*parameters->Get<TestParameters::Examples::StringExample>() = "asd";
		*parameters->Get<TestParameters::ArrayExamples::StringsExample>() = { "a", "b" };
		auto mesh = Geometry::GeneratePlane<Real>( 1 );
		*parameters->Get<TestParameters::Examples::MeshExample>() = { &mesh, Xform3::Translation( Vector3( 1, 2, 3 ) ) };
		auto result = ToReadableString( *parameters );
		auto expected = R"(Waveform	1
Magnitude.Value	1
Magnitude.Channel	[ -1 0 ]
Magnitude.Sinks.Ramps	0 [ ]
Magnitude.Sinks.Positions	0 [ ]
Phase.Value	2
Phase.Channel	[ -1 0 ]
Phase.Map	[ <null> 0 false ]
StrandGroup.Pattern	pattern
StrandGroup.BlendDistance	0
Examples.AuxExample	0
Examples.TransientExample	0
Examples.FractionExample	0.1
Examples.VectorDistanceExample	[ 0.3 0.3 0 ]
Examples.VectorDirectionExample	[ 0 0 1 ]
Examples.XformExample	[ 1 0 0 | 0 1 0 | 0 0 1 | 0 0 0 ]
Examples.ExternalEnumParameter	1
Examples.CustomTypeExample	?
Examples.CustomSharedExample	?
Examples.BoolExample	1
Examples.IntExample	1
Examples.StringExample	asd
Examples.RampExample	[ Spline 0 0 Spline 1 1 ]
Examples.MeshExample	[ 4 1 / [ 1 0 0 | 0 1 0 | 0 0 1 | 1 2 3 ] ]
Examples.TargetHairExample	<null>
ArrayExamples.VectorsExample	0 [ ]
ArrayExamples.XformsExample	0 [ ]
ArrayExamples.BoolsExample	0 [ ]
ArrayExamples.EnumsExample	0 [ ]
ArrayExamples.IntsExample	0 [ ]
ArrayExamples.MeshesExample	0 [ ]
ArrayExamples.StringsExample	2 [ a b ]
ArrayOfStructExample.Real	0 [ ]
ArrayOfStructExample.Integer	0 [ ]
MapExample.StripId	0 [ ]
MapExample.Volume	0 [ ]
)";
		REQUIRE( result == expected );
	}
}

TEST_CASE( "String" )
{
	using std::string;
	using Parameters::String;
	using Parameters::WString;

#ifndef __RESHARPER__
	static_assert( sizeof( String ) == 24 );
#endif

	SECTION( "ConstructDefault" )
	{
		String t;
		char const* s = t.c_str();
		REQUIRE( s[0] == 0 );
		REQUIRE( t.empty() );

		REQUIRE( String().c_str() );
		REQUIRE( String().c_str()[0] == 0 );

		String longText( 129, ' ' );
		REQUIRE( !longText.IsShort() );
	}

	SECTION( "ConstructCharStr" )
	{
		String t( "asd" );
		REQUIRE( t == "asd" );
		REQUIRE( t.IsShort() );

		WString wt( L"asd" );
		REQUIRE( wt == L"asd" );
		REQUIRE( wt.IsShort() );

		String t1( 3, '*' );
		REQUIRE( t1 == "***" );

		String t2( "asd", 2 );
		REQUIRE( t2 == "as" );

		String t3( "asd", 4 );
		REQUIRE( t3.c_str() == string( "asd" ) );

		String t4( "asd", 0 );
		REQUIRE( t4.c_str() == string( "" ) );
	}

	SECTION( "ConstructCopy" )
	{
		String source( "asd" );
		auto const t = source;  // NOLINT(performance-unnecessary-copy-initialization)
		REQUIRE( t == "asd" );

		String empty;
		auto const t0 = empty;  // NOLINT(performance-unnecessary-copy-initialization)
		REQUIRE( t0.empty() );

		String t1( "asd", 2 );
		REQUIRE( t1 == "as" );

		source = "very long 123456789 123456789 123456789 123456789 ";
		REQUIRE( source.length() == 50 );
		REQUIRE_FALSE( source.IsShort() );
		auto const t2 = source;
		REQUIRE( t2.length() == 50 );
		REQUIRE_FALSE( t2.IsShort() );
	}

	SECTION( "ConstructMove" )
	{
		String longText = "very long 123456789 123456789 123456789 123456789 ";
		REQUIRE( String( std::move( longText ) ) == "very long 123456789 123456789 123456789 123456789 " );  // NOLINT(bugprone-use-after-move)
		REQUIRE( longText.empty() ); // NOLINT(bugprone-use-after-move)
		REQUIRE( longText.length() == 0 );

		WString wideLong = L"very long 123456789 123456789 123456789 123456789 ";
		REQUIRE( WString( std::move( wideLong ) ) == L"very long 123456789 123456789 123456789 123456789 " );  // NOLINT(bugprone-use-after-move)
		REQUIRE( wideLong.empty() ); // NOLINT(bugprone-use-after-move)
		REQUIRE( wideLong.length() == 0 );

		String shortText = "asd";
		REQUIRE( String( std::move( shortText ) ) == "asd" );  // NOLINT(bugprone-use-after-move)
		REQUIRE( shortText == "asd" ); // NOLINT(bugprone-use-after-move)

		WString wideShortText = L"asd";
		REQUIRE( wideShortText.IsShort() );
		REQUIRE( WString( std::move( wideShortText ) ) == L"asd" );  // NOLINT(bugprone-use-after-move)
		REQUIRE( wideShortText == L"asd" ); // NOLINT(bugprone-use-after-move)
	}

	SECTION( "ConstructStdStr" )
	{
		string s( "asd" );
		String t( s );
		REQUIRE( t == "asd" );

		string s0;
		String t0( s0 );
		REQUIRE( t0.empty() );

		String s1( ( std::string_view() ) );
		REQUIRE( s1.data() );
	}

	SECTION( "Long" )
	{
		int constexpr len = 1024 * 1024 * 64 - 1;
		std::string orig( len, ' ' );
		String s( orig );
		REQUIRE_FALSE( s.IsShort() );

		std::wstring worig( len, L' ' );
		WString ws( worig );
		REQUIRE_FALSE( ws.IsShort() );
	}

	SECTION( "Swap" )
	{
		String t;
		String t2( "asd" );

		t.swap( t2 );
		REQUIRE( t == "asd" );
		REQUIRE( t2.empty() );

		t2 = String( "qwe" );
		t2.swap( t );
		REQUIRE( t2 == "asd" );
		REQUIRE( t == "qwe" );

		WString wt;
		WString wt2( L"." );
		wt.swap( wt2 );
		REQUIRE( wt == L"." );
	}

	SECTION( "operatorAssignShort" )
	{
		String t;
		String tSrc( "asd" );
		t = tSrc;
		REQUIRE( t == "asd" );

		String t0;
		t = t0;
		REQUIRE( t.empty() );

		t = "qwe";
		REQUIRE( t == "qwe" );

		std::string_view p = "zxc";
		t = p;
		REQUIRE( t == "zxc" );

		auto const& reference = t;
		t = reference;
		REQUIRE( t == "zxc" );
	}

	SECTION( "operatorAssignLong" )
	{
		String t;
		String tSrc( "very long 123456789 123456789 123456789 123456789 " );
		REQUIRE( tSrc.length() == 50 );
		t = tSrc;
		REQUIRE( t == "very long 123456789 123456789 123456789 123456789 " );

		String t0;
		t = t0;
		REQUIRE( t.empty() );

		t = "another long one 123456789 123456789 123456789";
		REQUIRE( t == "another long one 123456789 123456789 123456789" );

		std::string_view p = "third long one 123456789 123456789 123456789";
		t = p;
		REQUIRE( t == "third long one 123456789 123456789 123456789" );

		auto const& reference = t;
		t = reference;
		REQUIRE( t == "third long one 123456789 123456789 123456789" );
	}

	SECTION( "operatorAssignStr" )
	{
		String t = "asd";
		REQUIRE( t == "asd" );

		string s = t;
		REQUIRE( s == "asd" );

		t = "";
		REQUIRE( t.empty() );
	}

	SECTION( "operatorAssignStdStr" )
	{
		String t = string( "asd" );
		REQUIRE( t == "asd" );

		t = string();
		REQUIRE( t.empty() );
	}

	SECTION( "operatorMoveAssign" )
	{
		String longText = "very long 123456789 123456789 123456789 123456789 ";
		String t;
		t = std::move( longText );
		REQUIRE( t == "very long 123456789 123456789 123456789 123456789 " );
		REQUIRE( longText.empty() ); // NOLINT(bugprone-use-after-move)
		REQUIRE( longText.length() == 0 );

		auto& reference = t;
		t = std::move( reference );
		REQUIRE( t == "very long 123456789 123456789 123456789 123456789 " );

		String tShort = "asd";
		t = std::move( tShort );
		REQUIRE( t == "asd" );
		REQUIRE( tShort == "asd" ); // NOLINT(bugprone-use-after-move)
	}

	SECTION( "StreamOutput" )
	{
		String t = "asd";
		std::ostringstream s;
		s << t;
		REQUIRE( s.str() == "asd" );

		t.clear();
		s << t;
		REQUIRE( s.str() == "asd" );

		String e;
		s << e;
		REQUIRE( s.str() == "asd" );
	}

	SECTION( "Clear" )
	{
		String t = "asd";
		REQUIRE_FALSE( t.empty() );
		t.clear();
		REQUIRE( t.empty() );
	}

	SECTION( "To_string" )
	{
		String t;
		string s = t;
		REQUIRE( s.empty() );

		REQUIRE( t + string( "asd" ) == "asd" );
		REQUIRE( string( "asd" ) + t == "asd" );
	}

	SECTION( "Compare" )
	{
		String t = "asd";
		REQUIRE( t == "asd" );
		REQUIRE( t != "asde" );
		REQUIRE( t != "" );
		REQUIRE( t == String( "asd" ) );
		REQUIRE( t != String( "asde" ) );
		REQUIRE( t != String() );

		REQUIRE( t[0] == 'a' );
		REQUIRE( t[2] == 'd' );
		t[0] = 'b';
		REQUIRE( t[0] == 'b' );

		t = "";
		REQUIRE( t == "" );
		REQUIRE( t != "asd" );
		REQUIRE( t == String() );
		REQUIRE( t != String( "asd" ) );

		char zero[] = { 'a', 's', 0, 'd' };
		String strZero( zero, Size( zero ) );
		String strZero2( strZero );
		REQUIRE( strZero2.length() == Size( zero ) );
		REQUIRE( strZero == strZero2 );
		strZero2[3] = 'e';
		REQUIRE( strZero != strZero2 );
	}

	SECTION( "SelfAssign" )
	{
		String s;
		String e;
		REQUIRE( s.empty() );
		REQUIRE( e.empty() );
		s = String( e.Release() );
		REQUIRE( s.empty() );
		REQUIRE( e.empty() );

		s = "asd";

#ifdef _MSC_VER // Clang doesn't allow assigning to self
		// ReSharper disable once CppIdenticalOperandsInBinaryExpression
		s = s;  // NOLINT(clang-diagnostic-self-assign-overloaded)
		REQUIRE( s == "asd" );
#endif

		auto cs = String::MakeView( s.begin() );
		s = cs;
		REQUIRE( s == "asd" );

		s = s.c_str();
		REQUIRE( s == "asd" );

		auto p( s.Release() );
		UniquePtr<char> pweak( p.get(), nullptr );
		s = String( std::move( p ) );
		REQUIRE( s == "asd" );
		REQUIRE( pweak.get() == s.c_str() );

		s = String( std::move( pweak ) );
		REQUIRE( s == "asd" );
	}

	SECTION( "FromToUniquePtr" )
	{
		String s( UniquePtr<char>( new char[5], FunctionDeleter<char>::Free ), 5 );
		REQUIRE( s.length() == 5 );

		auto unique = s.Release();
		REQUIRE( s.empty() );
	}

	SECTION( "View" )
	{
		char text[] = "test";
		auto view = String::MakeView( text );
	}
}

struct TypeWithDestructor
{
	static unsigned Errors;
	static unsigned Count;

	int data;

	TypeWithDestructor() :
		data( 13 )
	{
		++Count;
	}

	TypeWithDestructor( TypeWithDestructor const& other ) :
		data( other.data )
	{
		++Count;
	}

	TypeWithDestructor( TypeWithDestructor&& other ) noexcept :
		data( other.data )
	{
		++Count;
	}

	~TypeWithDestructor()
	{
		Errors += data != 13;
		--Count;
	}

	TypeWithDestructor& operator=( TypeWithDestructor&& ) = delete;
	TypeWithDestructor& operator=( TypeWithDestructor const& ) = delete;
};

unsigned TypeWithDestructor::Errors = 0;
unsigned TypeWithDestructor::Count = 0;


TEST_CASE( "Array" )
{
	using Parameters::Array;
	using Parameters::String;

	SECTION( "Empty" )
	{
		Array<int> array;

		REQUIRE( array.empty() );
		REQUIRE( array.size() == 0 );
		REQUIRE( array.capacity() == 0 );
		REQUIRE( ( array.begin() == nullptr ) );
		REQUIRE( ( array.end() == nullptr ) );

		REQUIRE( array == Array<int>() );
		REQUIRE( array == Span<const int>() );
	}

	SECTION( "Erase" )
	{
		Array<String> array;
		unsigned constexpr n = 10;
		for( unsigned i = 0; i < n; ++i )
		{
			array.push_back( String( 1, static_cast<char>( '1' + i ) ) );
		}

		array.erase( array.begin() + 2, 2 );
		REQUIRE( array.size() == n - 2 );
		REQUIRE( array[6] == "9" );

		array.erase( array.end() - 2, 2 );
		REQUIRE( array.size() == n - 4 );

		array.erase( array.begin(), array.begin() );
		REQUIRE( array.size() == n - 4 );

		array.clear();
		array.erase( array.begin(), array.begin() );
		REQUIRE( array.size() == 0 );
	}

	SECTION( "ConstructFromSpan" )
	{
		auto x = 3;
		Array<int> array( Span( &x, &x + 1 ) );
		REQUIRE( array.size() == 1 );
		REQUIRE( array[0] == 3 );
	}

	SECTION( "ConstructBoolFromChar" )
	{
		std::vector<char> vectorOfChars = { 0, 1, 1, 0 };
		Array<bool> arrayOfBool{ vectorOfChars };
		REQUIRE( arrayOfBool.size() == 4 );
		REQUIRE_FALSE( arrayOfBool[0] );
		REQUIRE( arrayOfBool[1] );
		REQUIRE( arrayOfBool[2] );
		REQUIRE_FALSE( arrayOfBool[3] );

		// From bool also works, of course
		std::vector vectorOfBool = { true, false };
		arrayOfBool = vectorOfBool;
		REQUIRE( arrayOfBool.size() == 2 );
		REQUIRE( arrayOfBool[0] );
		REQUIRE_FALSE( arrayOfBool[1] );
	}

	SECTION( "Misc" )
	{
		Array<int> array;

		REQUIRE( array.push_back( 1 ) );
		REQUIRE( array.push_back( 2 ) );
		REQUIRE( array.push_back( 3 ) );
		REQUIRE( !array.empty() );
		REQUIRE( array.size() == 3 );
		REQUIRE( array.capacity() == 3 );
		REQUIRE( array.begin() );
		REQUIRE( array.end() );
		REQUIRE( array[0] == 1 );
		REQUIRE( array[2] == 3 );

		REQUIRE( array.insert( array.begin() + 2, 4 ) );
		REQUIRE( array.size() == 4 );
		REQUIRE( array[2] == 4 );
		REQUIRE( array[3] == 3 );
		REQUIRE_FALSE( std::is_sorted( array.begin(), array.end() ) );

		array.erase( array.begin() + 1 );
		REQUIRE( array.size() == 3 );
		REQUIRE( array[1] == 4 );
		REQUIRE( array[2] == 3 );

		Array array2( array );
		REQUIRE( array2.size() == 3 );
		REQUIRE( array2[0] == 1 );
		REQUIRE( array2[2] == 3 );
		REQUIRE( array == array2 );

		array2.clear();
		REQUIRE( array2.empty() );
		REQUIRE( array2.capacity() > 0 );

		array2.swap( array );
		REQUIRE( array.empty() );
		REQUIRE( array.capacity() > 0 );
		REQUIRE( array2.size() == 3 );
		REQUIRE( array2.capacity() == 4 );

		REQUIRE( array2.resize( 5, 7 ) );
		REQUIRE( array2.size() == 5 );
		REQUIRE( array2.capacity() == 6 );
		REQUIRE( array2[3] == 7 );
		REQUIRE( array2[4] == 7 );
		array2[3] = 6;
		array2.push_back( 8 );
		array2.insert( array2.begin() + 2, 13 );
		REQUIRE( array2[4] == 6 );
		REQUIRE( array2[5] == 7 );
		REQUIRE( array2[6] == 8 );
		REQUIRE( array2.resize( 3, 7 ) );

		std::vector<int> v;
		Array<int> arr3( array2.begin(), array2.end() );
		REQUIRE( arr3 == array2 );

		array2 = array;
		REQUIRE( array2.size() == 0 );
		REQUIRE( array2.capacity() > 0 );

		arr3.assign( array2.begin(), array2.end() );
		REQUIRE( arr3.size() == 0 );

		arr3.clear();
		arr3.assign( array2.begin(), array2.end() );
		REQUIRE( arr3.size() == 0 );

		int c[] = { 1, 2, 3 };
		REQUIRE( array.insert( array.begin(), c, c + std::size( c ) ) );
		REQUIRE( array.size() == 3 );
		REQUIRE( array.insert( array.begin() + 2, c, c + 1 ) );
		REQUIRE( array.size() == 4 );
		REQUIRE( array[2] == 1 );

		REQUIRE( *array.rbegin() == 3 );
		REQUIRE( array.rend()[-1] == 1 );

		REQUIRE( Array<int>::Repeat( 3, 5 )[1] == 5 );
	}

	SECTION( "Moveable" )
	{
		Array<std::unique_ptr<int>> moveable;
		moveable.push_back( std::make_unique<int>( 1 ) );
		moveable.push_back( std::make_unique<int>( 3 ) );
		moveable.insert( moveable.begin() + 1, std::make_unique<int>( 2 ) );
		REQUIRE( *moveable[1] == 2 );
		REQUIRE( *moveable[2] == 3 );
		moveable.resize( 4 );
		REQUIRE( moveable[3].get() == nullptr );

		auto moveable2( std::move( moveable ) );
		REQUIRE( moveable2.size() == 4 );
		REQUIRE( moveable.empty() ); // NOLINT(bugprone-use-after-move)
	}

	SECTION( "TypeWithDestructor" )
	{
		Array<TypeWithDestructor> a;
		a.resize( 1 );
		a.push_back( TypeWithDestructor() );
		a.erase( a.begin() );
		a.resize( 5, TypeWithDestructor() );
		a.clear();
		REQUIRE( TypeWithDestructor::Errors == 0 );
		REQUIRE( TypeWithDestructor::Count == 0 );

		Array< std::shared_ptr<TypeWithDestructor> > ap;
		ap.push_back( std::make_shared<TypeWithDestructor>() );
		ap.insert( ap.begin(), std::make_shared<TypeWithDestructor>() );
		ap.clear();
		REQUIRE( TypeWithDestructor::Errors == 0 );
		REQUIRE( TypeWithDestructor::Count == 0 );
	}

	using Parameters::NotTriviallyRelocatable;

	SECTION( "NotTriviallyRelocatable" )
	{
		{
			Array<NotTriviallyRelocatable> a;
			a.push_back( NotTriviallyRelocatable() );
			a.resize( 5 );
		}
		REQUIRE( NotTriviallyRelocatable::IsEmpty() );

		{
			Array<NotTriviallyRelocatable> a;
			a.resize( 5 );
			a.erase( a.begin() + 2 );
		}
		REQUIRE( NotTriviallyRelocatable::IsEmpty() );

		{
			Array<NotTriviallyRelocatable> a;
			a.resize( 5 );
			a.insert( a.begin() + 2, NotTriviallyRelocatable() );
			REQUIRE( a[2].data == 6 );
			REQUIRE( a[5].data == 5 );
		}
		REQUIRE( NotTriviallyRelocatable::IsEmpty() );

		{
			Array<NotTriviallyRelocatable> a;
			a.resize( 5 );
			NotTriviallyRelocatable b[2];
			a.insert( a.begin() + 2, b, b + 2 );
			REQUIRE( a[2].data == 6 );
			REQUIRE( a[3].data == 7 );
			REQUIRE( a[6].data == 5 );
		}
		REQUIRE( NotTriviallyRelocatable::IsEmpty() );
	}

	SECTION( "ResizeDefault" )
	{
		static Array<int>::Allocator debugAllocator =
		{ 
			[]( size_t size )
			{
				auto const result = std::malloc( size );
				std::fill_n( static_cast<std::uint8_t*>( result ), size, static_cast<uint8_t>( 0x55 ) );
				return result;
			},
			[]( void* pointer )
			{
				std::free( pointer );
			}
		};

		Array<int> array;
		array.SetAllocator( debugAllocator );
		array.resize( 1 );
		REQUIRE( array[0] == 0 );
	}
}

}
