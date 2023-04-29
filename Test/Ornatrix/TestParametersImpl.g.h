// Private parameter definition - generated automatically

#pragma once

#include "Ephere/Core/Parameters/ParametersImpl.h"
#include "TestParameters.g.h"

#include "Ephere/Ornatrix/ParameterComponents.h"
#include "Ephere/Ornatrix/Private/StrandChannelAccessor.h"
#include "Ephere/Ornatrix/Private/StrandGroupApplicationTester.h"
#include "Ephere/Ornatrix/Private/TextureMapEvaluator.h"
#include "Ephere/Ornatrix/StrandTopology.h"

namespace Ephere::Ornatrix
{

struct TestParameters::Descriptor : Parameters::SetDescriptor
{
	static constexpr int Version = 1;

	static constexpr int ParameterCount = 36;

	// Enum traits

	static constexpr auto WaveformTypeEnumerableTrait = Parameters::MakeEnumerableTrait( L"Sine", L"Helix" );

	static constexpr auto AlgorithmTypeEnumerableTrait = Parameters::MakeEnumerableTrait( L"Linear", L"LinearWithBasePosition", L"LinearWithBasePositionAndVelocity" );
	// For now external enum values must be 0, 1, 2, ...
	static_assert( int( Ephere::Ornatrix::AiPhysics::AlgorithmType::Linear ) == 0 );
	static_assert( int( Ephere::Ornatrix::AiPhysics::AlgorithmType::LinearWithBasePosition ) == 1 );
	static_assert( int( Ephere::Ornatrix::AiPhysics::AlgorithmType::LinearWithBasePositionAndVelocity ) == 2 );

	// Descriptors

	struct Waveform : Parameters::ParameterDescriptorImpl<TestParameters::Waveform, Descriptor>
	{
		static constexpr std::string_view Name = "Waveform";

		static constexpr auto Traits = MakeComposite(
			WaveformTypeEnumerableTrait
		);

		static Waveform const& GetInstance()
		{
			static Waveform descriptor;
			return descriptor;
		}
	};

	struct Magnitude : Parameters::IGroupDescriptor
	{
		static Magnitude const& GetInstance()
		{
			static Magnitude descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "Magnitude";
		}

		struct Value : Parameters::ParameterDescriptorImpl<TestParameters::Magnitude::Value, Magnitude>
		{
			static constexpr std::string_view Name = "Magnitude.Value";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite(
				Parameters::RangeCheckTrait<float>( -1000.0f, 1000.0f, 0.0f, 10.0f )
			);

			[[nodiscard]] std::pair<void const*, void const*> GetValueRange() const override
			{
				return { &Traits.low, &Traits.high };
			}

			[[nodiscard]] std::pair<void const*, void const*> GetUiValueRange() const override
			{
				return { &Traits.uiLow, &Traits.uiHigh };
			}

			static Value const& GetInstance()
			{
				static Value descriptor;
				return descriptor;
			}
		};

		struct Channel : Parameters::ParameterDescriptorImpl<TestParameters::Magnitude::Channel, Magnitude, StrandChannelAccessor>
		{
			static constexpr std::string_view Name = "Magnitude.Channel";

			static constexpr auto Traits = MakeComposite();

			static Channel const& GetInstance()
			{
				static Channel descriptor;
				return descriptor;
			}
		};

		struct Sinks : Parameters::IGroupDescriptor
		{
			static Sinks const& GetInstance()
			{
				static Sinks descriptor;
				return descriptor;
			}

			Parameters::IGroupDescriptor const* GetOwner() const override
			{
				return &Magnitude::GetInstance();
			}

			char const* GetName() const override
			{
				return "Sinks";
			}

			struct Ramps : Parameters::ParameterDescriptorImpl<TestParameters::Magnitude::Sinks::Ramps, Sinks>
			{
				static constexpr std::string_view Name = "Magnitude.Sinks.Ramps";

				static constexpr auto Traits = MakeComposite();

				static Ramps const& GetInstance()
				{
					static Ramps descriptor;
					return descriptor;
				}
			};

			struct Positions : Parameters::ParameterDescriptorImpl<TestParameters::Magnitude::Sinks::Positions, Sinks>
			{
				static constexpr std::string_view Name = "Magnitude.Sinks.Positions";

				static constexpr auto Traits = MakeComposite();

				static Positions const& GetInstance()
				{
					static Positions descriptor;
					return descriptor;
				}
			};
		};
	};

	struct Phase : Parameters::IGroupDescriptor
	{
		static Phase const& GetInstance()
		{
			static Phase descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "Phase";
		}

		struct Value : Parameters::ParameterDescriptorImpl<TestParameters::Phase::Value, Phase>
		{
			static constexpr std::string_view Name = "Phase.Value";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite(
				Parameters::UiLimitsTrait<float>( -5.0f, 5.0f )
			);

			[[nodiscard]] std::pair<void const*, void const*> GetUiValueRange() const override
			{
				return { &Traits.uiLow, &Traits.uiHigh };
			}

			static Value const& GetInstance()
			{
				static Value descriptor;
				return descriptor;
			}
		};

		struct Channel : Parameters::ParameterDescriptorImpl<TestParameters::Phase::Channel, Phase, StrandChannelAccessor>
		{
			static constexpr std::string_view Name = "Phase.Channel";

			static constexpr auto Traits = MakeComposite();

			static Channel const& GetInstance()
			{
				static Channel descriptor;
				return descriptor;
			}
		};

		struct Map : Parameters::ParameterDescriptorImpl<TestParameters::Phase::Map, Phase, Ephere::Ornatrix::TextureMapEvaluator>
		{
			static constexpr std::string_view Name = "Phase.Map";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static Map const& GetInstance()
			{
				static Map descriptor;
				return descriptor;
			}
		};
	};

	struct StrandGroup : Parameters::IGroupDescriptor
	{
		static StrandGroup const& GetInstance()
		{
			static StrandGroup descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "StrandGroup";
		}

		struct Pattern : Parameters::ParameterDescriptorImpl<TestParameters::StrandGroup::Pattern, StrandGroup, StrandGroupApplicationTester>
		{
			static constexpr std::string_view Name = "StrandGroup.Pattern";

			static constexpr auto Traits = MakeComposite();

			static Pattern const& GetInstance()
			{
				static Pattern descriptor;
				return descriptor;
			}
		};

		struct BlendDistance : Parameters::ParameterDescriptorImpl<TestParameters::StrandGroup::BlendDistance, StrandGroup>
		{
			static constexpr std::string_view Name = "StrandGroup.BlendDistance";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite(
				Parameters::UiLimitsTrait<float>( -100.0f, 100.0f )
			);

			[[nodiscard]] std::pair<void const*, void const*> GetUiValueRange() const override
			{
				return { &Traits.uiLow, &Traits.uiHigh };
			}

			static BlendDistance const& GetInstance()
			{
				static BlendDistance descriptor;
				return descriptor;
			}
		};
	};

	struct Examples : Parameters::IGroupDescriptor
	{
		static Examples const& GetInstance()
		{
			static Examples descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "Examples";
		}

		struct AuxExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::AuxExample, Examples, Ephere::Ornatrix::StrandTopology, double>
		{
			static constexpr std::string_view Name = "Examples.AuxExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static AuxExample const& GetInstance()
			{
				static AuxExample descriptor;
				return descriptor;
			}
		};

		struct TransientExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::TransientExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.TransientExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static TransientExample const& GetInstance()
			{
				static TransientExample descriptor;
				return descriptor;
			}
		};

		struct FractionExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::FractionExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.FractionExample";

			static constexpr bool IsAnimatable = true;

			static constexpr bool IsHidden = true;

			static constexpr auto Traits = MakeComposite(
				Parameters::RangeCheckTrait<float>( 0.0f, 1.0f )
			);

			[[nodiscard]] std::pair<void const*, void const*> GetValueRange() const override
			{
				return { &Traits.low, &Traits.high };
			}

			static FractionExample const& GetInstance()
			{
				static FractionExample descriptor;
				return descriptor;
			}
		};

		struct VectorDistanceExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::VectorDistanceExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.VectorDistanceExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static Ephere::Ornatrix::Vector3 DefaultValue()
			{
				return { 0.3f, 0.3f, 0.f };
			}

			static VectorDistanceExample const& GetInstance()
			{
				static VectorDistanceExample descriptor;
				return descriptor;
			}
		};

		struct VectorDirectionExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::VectorDirectionExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.VectorDirectionExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static Ephere::Ornatrix::Vector3 DefaultValue()
			{
				return Ephere::Ornatrix::Vector3( 0, 0, 1 );
			}

			static VectorDirectionExample const& GetInstance()
			{
				static VectorDirectionExample descriptor;
				return descriptor;
			}
		};

		struct XformExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::XformExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.XformExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static Ephere::Ornatrix::Xform3 DefaultValue()
			{
				return Xform3::Identity();
			}

			static XformExample const& GetInstance()
			{
				static XformExample descriptor;
				return descriptor;
			}
		};

		struct ExternalEnumParameter : Parameters::ParameterDescriptorImpl<TestParameters::Examples::ExternalEnumParameter, Examples>
		{
			static constexpr std::string_view Name = "Examples.ExternalEnumParameter";

			static constexpr auto Traits = MakeComposite(
				AlgorithmTypeEnumerableTrait
			);

			static Ephere::Ornatrix::AiPhysics::AlgorithmType DefaultValue()
			{
				return Ephere::Ornatrix::AiPhysics::AlgorithmType::LinearWithBasePosition;
			}

			static ExternalEnumParameter const& GetInstance()
			{
				static ExternalEnumParameter descriptor;
				return descriptor;
			}
		};

		struct CustomTypeExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::CustomTypeExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.CustomTypeExample";

			static constexpr auto Traits = MakeComposite();

			static CustomTypeExample const& GetInstance()
			{
				static CustomTypeExample descriptor;
				return descriptor;
			}
		};

		struct CustomSharedExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::CustomSharedExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.CustomSharedExample";

			static constexpr auto Traits = MakeComposite();

			static CustomSharedExample const& GetInstance()
			{
				static CustomSharedExample descriptor;
				return descriptor;
			}
		};

		struct BoolExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::BoolExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.BoolExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static BoolExample const& GetInstance()
			{
				static BoolExample descriptor;
				return descriptor;
			}
		};

		struct IntExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::IntExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.IntExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static IntExample const& GetInstance()
			{
				static IntExample descriptor;
				return descriptor;
			}
		};

		struct StringExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::StringExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.StringExample";

			static constexpr auto Traits = MakeComposite();

			static StringExample const& GetInstance()
			{
				static StringExample descriptor;
				return descriptor;
			}
		};

		struct RampExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::RampExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.RampExample";

			static constexpr bool IsAnimatable = true;

			static constexpr auto Traits = MakeComposite();

			static RampExample const& GetInstance()
			{
				static RampExample descriptor;
				return descriptor;
			}
		};

		struct MeshExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::MeshExample, Examples>
		{
			static constexpr std::string_view Name = "Examples.MeshExample";

			static constexpr auto Traits = MakeComposite();

			static MeshExample const& GetInstance()
			{
				static MeshExample descriptor;
				return descriptor;
			}
		};

		struct TargetHairExample : Parameters::ParameterDescriptorImpl<TestParameters::Examples::TargetHairExample, Descriptor>
		{
			static constexpr std::string_view Name = "Examples.TargetHairExample";

			static constexpr bool IsHidden = true;

			static constexpr bool IsGuides = false;

			static constexpr auto Traits = MakeComposite();

			static TargetHairExample const& GetInstance()
			{
				static TargetHairExample descriptor;
				return descriptor;
			}
		};
	};

	struct ArrayExamples : Parameters::IGroupDescriptor
	{
		static ArrayExamples const& GetInstance()
		{
			static ArrayExamples descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "ArrayExamples";
		}

		struct VectorsExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::VectorsExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.VectorsExample";

			static constexpr auto Traits = MakeComposite();

			static VectorsExample const& GetInstance()
			{
				static VectorsExample descriptor;
				return descriptor;
			}
		};

		struct XformsExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::XformsExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.XformsExample";

			static constexpr auto Traits = MakeComposite();

			static XformsExample const& GetInstance()
			{
				static XformsExample descriptor;
				return descriptor;
			}
		};

		struct BoolsExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::BoolsExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.BoolsExample";

			static constexpr auto Traits = MakeComposite();

			static BoolsExample const& GetInstance()
			{
				static BoolsExample descriptor;
				return descriptor;
			}
		};

		struct EnumsExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::EnumsExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.EnumsExample";

			static constexpr auto Traits = MakeComposite(
				WaveformTypeEnumerableTrait
			);

			static EnumsExample const& GetInstance()
			{
				static EnumsExample descriptor;
				return descriptor;
			}
		};

		struct IntsExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::IntsExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.IntsExample";

			static constexpr auto Traits = MakeComposite();

			static IntsExample const& GetInstance()
			{
				static IntsExample descriptor;
				return descriptor;
			}
		};

		struct MeshesExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::MeshesExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.MeshesExample";

			static constexpr auto Traits = MakeComposite();

			static MeshesExample const& GetInstance()
			{
				static MeshesExample descriptor;
				return descriptor;
			}
		};

		struct StringsExample : Parameters::ParameterDescriptorImpl<TestParameters::ArrayExamples::StringsExample, ArrayExamples>
		{
			static constexpr std::string_view Name = "ArrayExamples.StringsExample";

			static constexpr auto Traits = MakeComposite();

			static StringsExample const& GetInstance()
			{
				static StringsExample descriptor;
				return descriptor;
			}
		};
	};

	struct ArrayOfStructExample : Parameters::IGroupDescriptor
	{
		static ArrayOfStructExample const& GetInstance()
		{
			static ArrayOfStructExample descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "ArrayOfStructExample";
		}

		struct Real : Parameters::ParameterDescriptorImpl<TestParameters::ArrayOfStructExample::Real, ArrayOfStructExample>
		{
			static constexpr std::string_view Name = "ArrayOfStructExample.Real";

			static constexpr auto Traits = MakeComposite();

			static Real const& GetInstance()
			{
				static Real descriptor;
				return descriptor;
			}
		};

		struct Integer : Parameters::ParameterDescriptorImpl<TestParameters::ArrayOfStructExample::Integer, ArrayOfStructExample>
		{
			static constexpr std::string_view Name = "ArrayOfStructExample.Integer";

			static constexpr auto Traits = MakeComposite();

			static Integer const& GetInstance()
			{
				static Integer descriptor;
				return descriptor;
			}
		};
	};

	struct MapExample : Parameters::IGroupDescriptor
	{
		static MapExample const& GetInstance()
		{
			static MapExample descriptor;
			return descriptor;
		}

		Parameters::IGroupDescriptor const* GetOwner() const override
		{
			return &Descriptor::GetInstance();
		}

		char const* GetName() const override
		{
			return "MapExample";
		}

		struct StripId : Parameters::ParameterDescriptorImpl<TestParameters::MapExample::StripId, MapExample>
		{
			static constexpr std::string_view Name = "MapExample.StripId";

			static constexpr auto Traits = MakeComposite();

			static StripId const& GetInstance()
			{
				static StripId descriptor;
				return descriptor;
			}
		};

		struct Volume : Parameters::ParameterDescriptorImpl<TestParameters::MapExample::Volume, MapExample>
		{
			static constexpr std::string_view Name = "MapExample.Volume";

			static constexpr auto Traits = MakeComposite();

			static Volume const& GetInstance()
			{
				static Volume descriptor;
				return descriptor;
			}
		};
	};


	using ParameterDescriptors = Parameters::ParameterDescriptorsHelper<
		Waveform,
		Magnitude::Value,
		Magnitude::Channel,
		Magnitude::Sinks::Ramps,
		Magnitude::Sinks::Positions,
		Phase::Value,
		Phase::Channel,
		Phase::Map,
		StrandGroup::Pattern,
		StrandGroup::BlendDistance,
		Examples::AuxExample,
		Examples::TransientExample,
		Examples::FractionExample,
		Examples::VectorDistanceExample,
		Examples::VectorDirectionExample,
		Examples::XformExample,
		Examples::ExternalEnumParameter,
		Examples::CustomTypeExample,
		Examples::CustomSharedExample,
		Examples::BoolExample,
		Examples::IntExample,
		Examples::StringExample,
		Examples::RampExample,
		Examples::MeshExample,
		Examples::TargetHairExample,
		ArrayExamples::VectorsExample,
		ArrayExamples::XformsExample,
		ArrayExamples::BoolsExample,
		ArrayExamples::EnumsExample,
		ArrayExamples::IntsExample,
		ArrayExamples::MeshesExample,
		ArrayExamples::StringsExample,
		ArrayOfStructExample::Real,
		ArrayOfStructExample::Integer,
		MapExample::StripId,
		MapExample::Volume
	>;

	static constexpr int ObsoleteCount = 1;

	struct ObsoleteIds
	{
		static constexpr int Examples_ObsoleteExample = 19;
	};

	// ISetDescriptor implementation

	Descriptor()
		: SetDescriptor( "TestParameters" )
	{
	}

	int GetParameterCount() const override
	{
		return SetDescriptor::GetParameterCount() + ParameterCount;
	}

	int GetVersion() const override
	{
		return Version;
	}

	Parameters::IParameterDescriptor const* GetParameterByIndex( int index ) const override
	{
		return index < ParameterCount ? ParameterDescriptors::GetParameterByIndex( index ) : SetDescriptor::GetParameterByIndex( index - ParameterCount );
	}

	Parameters::IParameterDescriptor const* GetParameterById( Parameters::ParameterId id ) const override
	{
		auto result = ParameterDescriptors::GetParameterById( id );
		return result != nullptr ? result : SetDescriptor::GetParameterById( id );
	}

	int GetParameterIndex( Parameters::ParameterId id ) const override
	{
		auto result = ParameterDescriptors::GetParameterIndex( id );
		return result >= 0 ? result : SetDescriptor::GetParameterIndex( id );
	}

	Parameters::IParameterDescriptor const* FindParameter( char const* name ) const override
	{
		auto idAndIndex = FindIdAndIndex( name );
		return idAndIndex.second >= 0 ? GetParameterByIndex( idAndIndex.second ) : nullptr;
	}

	Parameters::IContainer* CreateContainer() const override;

	Span<Parameters::ObsoleteParameter const> GetObsoleteParameters() const override
	{
		static std::array<Parameters::ObsoleteParameter, 1> obsolete = { {
			{ Parameters::ParameterId{ 19 }, Parameters::GetTypeId<Ephere::Parameters::String>(), false },
		} };
		return obsolete;
	}

	[[nodiscard]] ISetDescriptor* Clone( char const* newName = "" ) const override
	{
		auto* result = new Descriptor( *this );
		if( !IsEmpty( newName ) )
		{
			result->SetName( newName );
		}

		return result;
	}

	static Descriptor const& GetInstance()
	{
		static Descriptor descriptor;
		return descriptor;
	}

	template <typename F>
	static constexpr void ForEach( F&& function )
	{
		ParameterDescriptors::ForEach( std::forward<F>( function ) );
	}

	static int GetIndex( Parameters::ParameterId parameterId )
	{
		return ParameterDescriptors::GetParameterIndex( parameterId );
	}

	static std::pair<Parameters::ParameterId, int> FindIdAndIndex( std::string_view parameterName )
	{
		return ParameterDescriptors::FindIdAndIndex( parameterName );
	}
};


struct TestParameters::Container : Parameters::Container
{
	using OwnerType = TestParameters;

	using DescriptorType = Descriptor;

	Parameters::ParameterImpl<Descriptor::Waveform, WaveformType> waveform;

	struct MagnitudeGroup
	{
		Parameters::ParameterImpl<Descriptor::Magnitude::Value, float> value;

		struct ChannelParameter : Parameters::ParameterImpl<Descriptor::Magnitude::Channel, Ephere::Ornatrix::StrandChannel>
		{
			using BaseType::BaseType;
			using BaseType::operator=;

			StrandChannelAccessor accessor{};
		} channel;

		struct SinksGroup
		{
			Parameters::ParameterImpl<Descriptor::Magnitude::Sinks::Ramps, Ephere::Parameters::Array<Ephere::Ornatrix::Ramp>> ramps;

			Parameters::ParameterImpl<Descriptor::Magnitude::Sinks::Positions, Ephere::Parameters::Array<Ephere::Geometry::SurfacePosition>> positions;

			struct Element
			{
				Ephere::Ornatrix::Ramp const& ramps;
				Ephere::Geometry::SurfacePosition const& positions;

				auto asTuple() const
				{
					return MakeRefsTuple( ramps, positions );
				}
			};

			auto operator[]( int index ) const
			{
				return Element{ ramps()[index], positions()[index] };
			}

			auto operator()() const
			{
				return Parameters::MakeGroupArrayView<Element>( ramps, positions );
			}

			auto operator()()
			{
				return Parameters::MakeGroupArrayView<Element>( ramps, positions );
			}
		} sinks;
	} magnitude;

	struct PhaseGroup
	{
		Parameters::ParameterImpl<Descriptor::Phase::Value, float> value;

		struct ChannelParameter : Parameters::ParameterImpl<Descriptor::Phase::Channel, Ephere::Ornatrix::StrandChannel>
		{
			using BaseType::BaseType;
			using BaseType::operator=;

			StrandChannelAccessor accessor{};
		} channel;

		struct MapParameter : Parameters::ParameterImpl<Descriptor::Phase::Map, Ephere::Ornatrix::TextureMapParameter>
		{
			using BaseType::BaseType;
			using BaseType::operator=;

			Ephere::Ornatrix::TextureMapEvaluator evaluator{ &BaseType::operator()() };
		} map;
	} phase;

	struct StrandGroupGroup
	{
		struct PatternParameter : Parameters::ParameterImpl<Descriptor::StrandGroup::Pattern, Ephere::Parameters::String>
		{
			using BaseType::BaseType;
			using BaseType::operator=;

			StrandGroupApplicationTester tester{};
		} pattern;

		Parameters::ParameterImpl<Descriptor::StrandGroup::BlendDistance, float> blendDistance;
	} strandGroup;

	struct ExamplesGroup
	{
		struct AuxExampleParameter : Parameters::ParameterImpl<Descriptor::Examples::AuxExample, float>
		{
			using BaseType::BaseType;
			using BaseType::operator=;

			Ephere::Ornatrix::StrandTopology topology{};

			double accumulator{};
		} auxExample;

		Parameters::ParameterImpl<Descriptor::Examples::TransientExample, int> transientExample;

		Parameters::ParameterImpl<Descriptor::Examples::FractionExample, float> fractionExample;

		Parameters::ParameterImpl<Descriptor::Examples::VectorDistanceExample, Ephere::Ornatrix::Vector3> vectorDistanceExample;

		Parameters::ParameterImpl<Descriptor::Examples::VectorDirectionExample, Ephere::Ornatrix::Vector3> vectorDirectionExample;

		Parameters::ParameterImpl<Descriptor::Examples::XformExample, Ephere::Ornatrix::Xform3> xformExample;

		Parameters::ParameterImpl<Descriptor::Examples::ExternalEnumParameter, Ephere::Ornatrix::AiPhysics::AlgorithmType> externalEnumParameter;

		Parameters::ParameterImpl<Descriptor::Examples::CustomTypeExample, Groom::Operators::SomeData> customTypeExample;

		Parameters::ParameterImpl<Descriptor::Examples::CustomSharedExample, SharedPtr<Groom::Operators::SomeData>> customSharedExample;

		Parameters::ParameterImpl<Descriptor::Examples::BoolExample, bool> boolExample;

		Parameters::ParameterImpl<Descriptor::Examples::IntExample, int> intExample;

		Parameters::ParameterImpl<Descriptor::Examples::StringExample, Ephere::Parameters::String> stringExample;

		Parameters::ParameterImpl<Descriptor::Examples::RampExample, Ephere::Ornatrix::Ramp> rampExample;

		Parameters::ParameterImpl<Descriptor::Examples::MeshExample, Ephere::Ornatrix::PolygonMeshParameter> meshExample;

		Parameters::ParameterImpl<Descriptor::Examples::TargetHairExample, Ephere::Ornatrix::HairParameter> targetHairExample;
	} examples;

	struct ArrayExamplesGroup
	{
		Parameters::ParameterImpl<Descriptor::ArrayExamples::VectorsExample, Ephere::Parameters::Array<Ephere::Ornatrix::Vector3>> vectorsExample;

		Parameters::ParameterImpl<Descriptor::ArrayExamples::XformsExample, Ephere::Parameters::Array<Ephere::Ornatrix::Xform3>> xformsExample;

		Parameters::ParameterImpl<Descriptor::ArrayExamples::BoolsExample, Ephere::Parameters::Array<bool>> boolsExample;

		Parameters::ParameterImpl<Descriptor::ArrayExamples::EnumsExample, Ephere::Parameters::Array<WaveformType>> enumsExample;

		Parameters::ParameterImpl<Descriptor::ArrayExamples::IntsExample, Ephere::Parameters::Array<int>> intsExample;

		Parameters::ParameterImpl<Descriptor::ArrayExamples::MeshesExample, Ephere::Parameters::Array<Ephere::Ornatrix::PolygonMeshParameter>> meshesExample;

		Parameters::ParameterImpl<Descriptor::ArrayExamples::StringsExample, Ephere::Parameters::Array<Ephere::Parameters::String>> stringsExample;

		struct Element
		{
			Ephere::Ornatrix::Vector3 const& vectorsExample;
			Ephere::Ornatrix::Xform3 const& xformsExample;
			bool const& boolsExample;
			WaveformType const& enumsExample;
			int const& intsExample;
			Ephere::Ornatrix::PolygonMeshParameter const& meshesExample;
			Ephere::Parameters::String const& stringsExample;

			auto asTuple() const
			{
				return MakeRefsTuple( vectorsExample, xformsExample, boolsExample, enumsExample, intsExample, meshesExample, stringsExample );
			}
		};

		auto operator[]( int index ) const
		{
			return Element{ vectorsExample()[index], xformsExample()[index], boolsExample()[index], enumsExample()[index], intsExample()[index], meshesExample()[index], stringsExample()[index] };
		}

		auto operator()() const
		{
			return Parameters::MakeGroupArrayView<Element>( vectorsExample, xformsExample, boolsExample, enumsExample, intsExample, meshesExample, stringsExample );
		}

		auto operator()()
		{
			return Parameters::MakeGroupArrayView<Element>( vectorsExample, xformsExample, boolsExample, enumsExample, intsExample, meshesExample, stringsExample );
		}
	} arrayExamples;

	struct ArrayOfStructExampleGroup
	{
		Parameters::ParameterImpl<Descriptor::ArrayOfStructExample::Real, Ephere::Parameters::Array<float>> real;

		Parameters::ParameterImpl<Descriptor::ArrayOfStructExample::Integer, Ephere::Parameters::Array<int>> integer;

		struct Element
		{
			float const& real;
			int const& integer;

			auto asTuple() const
			{
				return MakeRefsTuple( real, integer );
			}
		};

		auto operator[]( int index ) const
		{
			return Element{ real()[index], integer()[index] };
		}

		auto operator()() const
		{
			return Parameters::MakeGroupArrayView<Element>( real, integer );
		}

		auto operator()()
		{
			return Parameters::MakeGroupArrayView<Element>( real, integer );
		}
	} arrayOfStructExample;

	struct MapExampleGroup
	{
		Parameters::ParameterImpl<Descriptor::MapExample::StripId, Ephere::Parameters::Array<std::int64_t>> stripId;

		Parameters::ParameterImpl<Descriptor::MapExample::Volume, Ephere::Parameters::Array<float>> volume;

		struct Element
		{
			std::int64_t const& stripId;
			float const& volume;

			auto asTuple() const
			{
				return MakeRefsTuple( stripId, volume );
			}
		};

		auto operator[]( int index ) const
		{
			return Element{ stripId()[index], volume()[index] };
		}

		auto operator()() const
		{
			return Parameters::MakeGroupSetView<Element>( stripId, volume );
		}

		auto operator()()
		{
			return Parameters::MakeGroupSetView<Element>( stripId, volume );
		}
	} mapExample;

	template <class TContainer>
	static auto AsTuple( TContainer& container )
	{
		return MakeRefsTuple( container.waveform, container.magnitude.value, container.magnitude.channel, container.magnitude.sinks.ramps, container.magnitude.sinks.positions, container.phase.value, container.phase.channel, container.phase.map, container.strandGroup.pattern, container.strandGroup.blendDistance, container.examples.auxExample, container.examples.transientExample, container.examples.fractionExample, container.examples.vectorDistanceExample, container.examples.vectorDirectionExample, container.examples.xformExample, container.examples.externalEnumParameter, container.examples.customTypeExample, container.examples.customSharedExample, container.examples.boolExample, container.examples.intExample, container.examples.stringExample, container.examples.rampExample, container.examples.meshExample, container.examples.targetHairExample, container.arrayExamples.vectorsExample, container.arrayExamples.xformsExample, container.arrayExamples.boolsExample, container.arrayExamples.enumsExample, container.arrayExamples.intsExample, container.arrayExamples.meshesExample, container.arrayExamples.stringsExample, container.arrayOfStructExample.real, container.arrayOfStructExample.integer, container.mapExample.stripId, container.mapExample.volume );
	}

	template <typename TFunction>
	void ForEach( TFunction&& function ) const
	{
		TupleForEach( AsTuple( *this ), std::forward<TFunction>( function ) );
	}

	template <typename TFunction>
	void ForEach( TFunction&& function )
	{
		TupleForEach( AsTuple( *this ), std::forward<TFunction>( function ) );
	}

	Container( Parameters::ISetDescriptor const& setDescriptor = TestParameters::Descriptor::GetInstance() )
		: Parameters::Container::Container( setDescriptor )
	{
	}

	// IContainer implementation

	using Parameters::IContainer::GetParameterByIndex;
	[[nodiscard]] Parameters::IParameter const* GetParameterByIndex( int index ) const override
	{
		switch( index )
		{
			case 0: return &waveform;
			case 1: return &magnitude.value;
			case 2: return &magnitude.channel;
			case 3: return &magnitude.sinks.ramps;
			case 4: return &magnitude.sinks.positions;
			case 5: return &phase.value;
			case 6: return &phase.channel;
			case 7: return &phase.map;
			case 8: return &strandGroup.pattern;
			case 9: return &strandGroup.blendDistance;
			case 10: return &examples.auxExample;
			case 11: return &examples.transientExample;
			case 12: return &examples.fractionExample;
			case 13: return &examples.vectorDistanceExample;
			case 14: return &examples.vectorDirectionExample;
			case 15: return &examples.xformExample;
			case 16: return &examples.externalEnumParameter;
			case 17: return &examples.customTypeExample;
			case 18: return &examples.customSharedExample;
			case 19: return &examples.boolExample;
			case 20: return &examples.intExample;
			case 21: return &examples.stringExample;
			case 22: return &examples.rampExample;
			case 23: return &examples.meshExample;
			case 24: return &examples.targetHairExample;
			case 25: return &arrayExamples.vectorsExample;
			case 26: return &arrayExamples.xformsExample;
			case 27: return &arrayExamples.boolsExample;
			case 28: return &arrayExamples.enumsExample;
			case 29: return &arrayExamples.intsExample;
			case 30: return &arrayExamples.meshesExample;
			case 31: return &arrayExamples.stringsExample;
			case 32: return &arrayOfStructExample.real;
			case 33: return &arrayOfStructExample.integer;
			case 34: return &mapExample.stripId;
			case 35: return &mapExample.volume;
			default: return Parameters::Container::GetParameterByIndex( index - Descriptor::ParameterCount );
		}
	}

	using Parameters::IContainer::GetParameterById;
	[[nodiscard]] Parameters::IParameter const* GetParameterById( Parameters::ParameterId id ) const override
	{
		auto index = Descriptor::GetIndex( id );
		return index >= 0 ? GetParameterByIndex( index ) : Parameters::Container::GetParameterById( id );
	}

	[[nodiscard]] IContainer* Clone() const override
	{
		return new Container( *this );
	}
};

inline Parameters::IContainer* TestParameters::Descriptor::CreateContainer() const
{
	return new TestParameters::Container( *this );
}

}
