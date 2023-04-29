// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Geometry {
struct SurfacePosition;
} }

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct HairFromGuidesParameters
{
	enum { Version = 1 };

	enum { HighestId = 49, FirstUnusedId = 50 };

	static char const* GetName() { return "HairFromGuidesParameters"; }

	enum class InterpolationMethodType
	{
		Polar,
		Affine,
		Segment,
	};

	enum class TextureGenerationMethodType
	{
		None,
		InheritFromBase,
		FlatMap,
	};

	enum class GuideAreaCalculationMethodType
	{
		NClosestGuides,
		Barycentric,
		Circle,
	};

	enum class GuideAreaBarycentricTriangulationMethodType
	{
		DistributionMesh,
		NearestNeighbourTriangulation,
		ManualTriangulation,
	};

	enum class HiddenGuideBehaviorType
	{
		Use,
		Hide,
		Ignore,
	};

	struct InputGuides : Parameters::ParameterDescriptor<HairFromGuidesParameters, 36, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputGuides"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<HairFromGuidesParameters, 37, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<HairFromGuidesParameters, 38, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct InterpolationMethod : Parameters::ParameterDescriptor<HairFromGuidesParameters, 1, InterpolationMethodType>
	{
		static char const* Name() { return "InterpolationMethod"; }

		static InterpolationMethodType DefaultValue() { return InterpolationMethodType::Affine; }
	};

	// Number of guides between which to interpolate each hair
	struct GuideCount : Parameters::ParameterDescriptor<HairFromGuidesParameters, 21, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "GuideCount"; }

		static int DefaultValue() { return 3; }
	};

	// Provides a fraction of hair in range [0,1] which will be generated. Other hairs will be skipped.
	struct GeneratedStrandFraction : Parameters::ParameterDescriptor<HairFromGuidesParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "GeneratedStrandFraction"; }

		static float DefaultValue() { return 0.1f; }
	};

	// Provides a fraction of hair in range [0,1] which will be generated when "is rendering" parameter is off (e.g. inside a viewport).
	struct PreviewStrandFraction : Parameters::ParameterDescriptor<HairFromGuidesParameters, 40, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "PreviewStrandFraction"; }

		static float DefaultValue() { return 0.2f; }
	};

	// When enabled, hairs will only be generated on those faces where guides are present
	struct UseGuideFaces : Parameters::ParameterDescriptor<HairFromGuidesParameters, 3, bool>
	{
		static char const* Name() { return "UseGuideFaces"; }

		static bool DefaultValue() { return false; }
	};

	struct GuideFaceIterationCount : Parameters::ParameterDescriptor<HairFromGuidesParameters, 4, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "GuideFaceIterationCount"; }

		static int DefaultValue() { return 1; }
	};

	// When enabled, strand length will be gradually interpolated for each generated hair
	struct InterpolateStrandLength : Parameters::ParameterDescriptor<HairFromGuidesParameters, 22, bool>
	{
		static char const* Name() { return "InterpolateStrandLength"; }

		static bool DefaultValue() { return false; }
	};

	// Determines whether the guide matching should be done in UV space or in world space. When world space is used matching map will not work.
	struct InterpolateGuidesInUvSpace : Parameters::ParameterDescriptor<HairFromGuidesParameters, 5, bool>
	{
		static char const* Name() { return "InterpolateGuidesInUvSpace"; }

		static bool DefaultValue() { return false; }
	};

	struct DistributionChannel : Parameters::ParameterDescriptor<HairFromGuidesParameters, 23, Ephere::Ornatrix::StrandChannel>
	{
		static char const* Name() { return "DistributionChannel"; }
	};

	struct DistributionMaps
	{
		struct First : Parameters::ParameterDescriptor<HairFromGuidesParameters, 29, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "DistributionMaps.First"; }
		};

		// Weight amount of the first distribution map
		struct FirstAmount : Parameters::ParameterDescriptor<HairFromGuidesParameters, 45, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "DistributionMaps.FirstAmount"; }

			static float DefaultValue() { return 1; }
		};

		struct Second : Parameters::ParameterDescriptor<HairFromGuidesParameters, 46, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "DistributionMaps.Second"; }
		};

		// Weight amount of the second distribution map
		struct SecondAmount : Parameters::ParameterDescriptor<HairFromGuidesParameters, 47, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "DistributionMaps.SecondAmount"; }

			static float DefaultValue() { return 1; }
		};

		struct Third : Parameters::ParameterDescriptor<HairFromGuidesParameters, 48, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "DistributionMaps.Third"; }
		};

		// Weight amount of the third distribution map
		struct ThirdAmount : Parameters::ParameterDescriptor<HairFromGuidesParameters, 49, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "DistributionMaps.ThirdAmount"; }

			static float DefaultValue() { return 1; }
		};
	};

	// Specifies the value below which all distribution map values will be considered zero. This is needed sometimes when using bitmaps which do not evaluate to true black pixels.
	struct DistributionMapZeroValue : Parameters::ParameterDescriptor<HairFromGuidesParameters, 43, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "DistributionMapZeroValue"; }

		static float DefaultValue() { return 0.0f; }
	};

	struct DisplacementMap : Parameters::ParameterDescriptor<HairFromGuidesParameters, 31, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "DisplacementMap"; }
	};

	struct GuideWeightsChannel : Parameters::ParameterDescriptor<HairFromGuidesParameters, 24, Ephere::Ornatrix::StrandChannel>
	{
		static char const* Name() { return "GuideWeightsChannel"; }
	};

	// When enabled, any per-strand data present in guides will be interpolated into resulting hairs
	struct GeneratePerStrandData : Parameters::ParameterDescriptor<HairFromGuidesParameters, 6, bool>
	{
		static char const* Name() { return "GeneratePerStrandData"; }

		static bool DefaultValue() { return true; }
	};

	// When enabled, any per-vertex data present in guides will be interpolated into resulting hairs
	struct GeneratePerVertexData : Parameters::ParameterDescriptor<HairFromGuidesParameters, 7, bool>
	{
		static char const* Name() { return "GeneratePerVertexData"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, rotations will be interpolated from guides to generated hairs
	struct GenerateRotations : Parameters::ParameterDescriptor<HairFromGuidesParameters, 8, bool>
	{
		static char const* Name() { return "GenerateRotations"; }

		static bool DefaultValue() { return true; }
	};

	/* Specifies the displacement value for black values of the displacement map.
	0 by default, if this is -1 then displacement will be negative, if it is 0.5 then displacement will happen on both sides of the surface. */
	struct DisplacementMapMinimumValue : Parameters::ParameterDescriptor<HairFromGuidesParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "DisplacementMapMinimumValue"; }

		static float DefaultValue() { return 0; }
	};

	struct TextureGenerationMethod : Parameters::ParameterDescriptor<HairFromGuidesParameters, 10, TextureGenerationMethodType>
	{
		static char const* Name() { return "TextureGenerationMethod"; }

		static TextureGenerationMethodType DefaultValue() { return TextureGenerationMethodType::InheritFromBase; }
	};

	struct UseGuideProximity : Parameters::ParameterDescriptor<HairFromGuidesParameters, 11, bool>
	{
		static char const* Name() { return "UseGuideProximity"; }

		static bool DefaultValue() { return true; }
	};

	// When enabled along with UseGuideProximity, the distance to guides is automatically calculated based on input guides spacing
	struct AutoGuideProximity : Parameters::ParameterDescriptor<HairFromGuidesParameters, 12, bool>
	{
		static char const* Name() { return "AutoGuideProximity"; }

		static bool DefaultValue() { return false; }
	};

	struct GuideProximityDistance : Parameters::ParameterDescriptor<HairFromGuidesParameters, 13, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "GuideProximityDistance"; }

		static float DefaultValue() { return 50; }
	};

	struct UseInstancedStrands : Parameters::ParameterDescriptor<HairFromGuidesParameters, 14, bool>
	{
		static char const* Name() { return "UseInstancedStrands"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, roots will be generated on mesh triangles instead of polygons. This is used for backward compatibility.
	struct UseTriangularRootGeneration : Parameters::ParameterDescriptor<HairFromGuidesParameters, 15, bool>
	{
		static char const* Name() { return "UseTriangularRootGeneration"; }

		static bool DefaultValue() { return false; }
	};

	struct GuideAreaCalculation
	{
		// Method used for generating surface dependence
		struct Method : Parameters::ParameterDescriptor<HairFromGuidesParameters, 16, GuideAreaCalculationMethodType>
		{
			static char const* Name() { return "GuideAreaCalculation.Method"; }

			static GuideAreaCalculationMethodType DefaultValue() { return GuideAreaCalculationMethodType::Barycentric; }
		};

		struct BarycentricTriangulationMethod : Parameters::ParameterDescriptor<HairFromGuidesParameters, 17, GuideAreaBarycentricTriangulationMethodType>
		{
			static char const* Name() { return "GuideAreaCalculation.BarycentricTriangulationMethod"; }

			static GuideAreaBarycentricTriangulationMethodType DefaultValue() { return GuideAreaBarycentricTriangulationMethodType::NearestNeighbourTriangulation; }
		};

		struct Circle
		{
			struct ThroughNearest : Parameters::ParameterDescriptor<HairFromGuidesParameters, 18, bool>
			{
				static char const* Name() { return "GuideAreaCalculation.Circle.ThroughNearest"; }

				static bool DefaultValue() { return true; }
			};

			struct Radius : Parameters::ParameterDescriptor<HairFromGuidesParameters, 19, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "GuideAreaCalculation.Circle.Radius"; }

				static float DefaultValue() { return 5; }
			};
		};
	};

	struct Parting
	{
		struct Automatic
		{
			struct Enabled : Parameters::ParameterDescriptor<HairFromGuidesParameters, 20, bool>
			{
				static char const* Name() { return "Parting.Automatic.Enabled"; }

				static bool DefaultValue() { return false; }
			};

			struct AngleThreshold : Parameters::ParameterDescriptor<HairFromGuidesParameters, 25, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Parting.Automatic.AngleThreshold"; }

				static float DefaultValue() { return 0.7f; }
			};

			struct DistanceThreshold : Parameters::ParameterDescriptor<HairFromGuidesParameters, 26, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Parting.Automatic.DistanceThreshold"; }

				static float DefaultValue() { return 999.f; }
			};
		};

		struct UseStrandGroupsAsParts : Parameters::ParameterDescriptor<HairFromGuidesParameters, 27, bool>
		{
			static char const* Name() { return "Parting.UseStrandGroupsAsParts"; }

			static bool DefaultValue() { return false; }
		};

		struct Channel : Parameters::ParameterDescriptor<HairFromGuidesParameters, 28, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Parting.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<HairFromGuidesParameters, 30, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Parting.Map"; }
		};

		struct Custom
		{
			struct Point1 : Parameters::ParameterDescriptor<HairFromGuidesParameters, 32, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::Distance>
			{
				static char const* Name() { return "Parting.Custom.Point1"; }

				static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
			};

			struct Point2 : Parameters::ParameterDescriptor<HairFromGuidesParameters, 33, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::Distance>
			{
				static char const* Name() { return "Parting.Custom.Point2"; }

				static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
			};

			struct Normal : Parameters::ParameterDescriptor<HairFromGuidesParameters, 34, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::UnitDirection>
			{
				static char const* Name() { return "Parting.Custom.Normal"; }
			};

			struct Threshold : Parameters::ParameterDescriptor<HairFromGuidesParameters, 35, float[]>
			{
				static char const* Name() { return "Parting.Custom.Threshold"; }
			};
		};

		struct Strips
		{
			// Holds a flattened array of vertices defining parting plane strips on the surface of the mesh
			struct Vertices : Parameters::ParameterDescriptor<HairFromGuidesParameters, 41, Ephere::Geometry::SurfacePosition[]>
			{
				static char const* Name() { return "Parting.Strips.Vertices"; }
			};

			// Array of indices into the Partings.Strips.Vertices array specifying the starting index of each parting plane strip
			struct StripStartIndices : Parameters::ParameterDescriptor<HairFromGuidesParameters, 44, int[]>
			{
				static char const* Name() { return "Parting.Strips.StripStartIndices"; }
			};
		};
	};

	struct IsRendering : Parameters::ParameterDescriptor<HairFromGuidesParameters, 39, bool, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "IsRendering"; }

		static bool DefaultValue() { return false; }
	};

	// Specifying the behavior of hidden guides for generating hair
	struct HiddenGuideBehavior : Parameters::ParameterDescriptor<HairFromGuidesParameters, 42, HiddenGuideBehaviorType>
	{
		static char const* Name() { return "HiddenGuideBehavior"; }

		static HiddenGuideBehaviorType DefaultValue() { return HiddenGuideBehaviorType::Use; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::HairFromGuidesParameters::InterpolationMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::HairFromGuidesParameters::InterpolationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case HairFromGuidesParameters::InterpolationMethodType::Polar: return "Polar";
			case HairFromGuidesParameters::InterpolationMethodType::Affine: return "Affine";
			case HairFromGuidesParameters::InterpolationMethodType::Segment: return "Segment";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::HairFromGuidesParameters::TextureGenerationMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::HairFromGuidesParameters::TextureGenerationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case HairFromGuidesParameters::TextureGenerationMethodType::None: return "None";
			case HairFromGuidesParameters::TextureGenerationMethodType::InheritFromBase: return "InheritFromBase";
			case HairFromGuidesParameters::TextureGenerationMethodType::FlatMap: return "FlatMap";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::HairFromGuidesParameters::GuideAreaCalculationMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::HairFromGuidesParameters::GuideAreaCalculationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case HairFromGuidesParameters::GuideAreaCalculationMethodType::NClosestGuides: return "NClosestGuides";
			case HairFromGuidesParameters::GuideAreaCalculationMethodType::Barycentric: return "Barycentric";
			case HairFromGuidesParameters::GuideAreaCalculationMethodType::Circle: return "Circle";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::HairFromGuidesParameters::GuideAreaBarycentricTriangulationMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::HairFromGuidesParameters::GuideAreaBarycentricTriangulationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case HairFromGuidesParameters::GuideAreaBarycentricTriangulationMethodType::DistributionMesh: return "DistributionMesh";
			case HairFromGuidesParameters::GuideAreaBarycentricTriangulationMethodType::NearestNeighbourTriangulation: return "NearestNeighbourTriangulation";
			case HairFromGuidesParameters::GuideAreaBarycentricTriangulationMethodType::ManualTriangulation: return "ManualTriangulation";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::HairFromGuidesParameters::HiddenGuideBehaviorType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::HairFromGuidesParameters::HiddenGuideBehaviorType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case HairFromGuidesParameters::HiddenGuideBehaviorType::Use: return "Use";
			case HairFromGuidesParameters::HiddenGuideBehaviorType::Hide: return "Hide";
			case HairFromGuidesParameters::HiddenGuideBehaviorType::Ignore: return "Ignore";
			default: return "";
		}
	}
};
}
