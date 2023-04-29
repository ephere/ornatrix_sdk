// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct ClumpParameters
{
	enum { Version = 1 };

	enum { HighestId = 54, FirstUnusedId = 55 };

	static char const* GetName() { return "ClumpParameters"; }

	enum class ClumpCreateMethodType
	{
		Random,
		StrandGroups,
		Guides,
		ExternalClumpStrands,
	};

	struct TargetHair : Parameters::ParameterDescriptor<ClumpParameters, 48, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<ClumpParameters, 49, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct ExternalClumpStrands : Parameters::ParameterDescriptor<ClumpParameters, 50, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "ExternalClumpStrands"; }
	};

	struct InputHairGuides : Parameters::ParameterDescriptor<ClumpParameters, 54, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHairGuides"; }
	};

	// Specifies the strength of clumping. Hairs will be attracted to the center of their clump more or less based on this value.
	struct Amount
	{
		struct Value : Parameters::ParameterDescriptor<ClumpParameters, 1, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Amount.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<ClumpParameters, 4, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Amount.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<ClumpParameters, 2, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<ClumpParameters, 3, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Map"; }
		};
	};

	// Controls how much strands are twisted around the clump center. Negative values will twist counter-clockwise, otherwise twisting will be clockwise.
	struct Twist
	{
		struct Value : Parameters::ParameterDescriptor<ClumpParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Twist.Value"; }

			static float DefaultValue() { return 0; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Radians;
		};

		struct Channel : Parameters::ParameterDescriptor<ClumpParameters, 8, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Twist.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<ClumpParameters, 6, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Twist.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<ClumpParameters, 7, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Twist.Map"; }
		};
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<ClumpParameters, 9, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<ClumpParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<ClumpParameters, 11, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct FlyAway
	{
		// Controls the fraction of hairs which will randomly not be clumped
		struct Fraction : Parameters::ParameterDescriptor<ClumpParameters, 12, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "FlyAway.Fraction"; }

			static float DefaultValue() { return 0.05f; }
		};

		// Controls the amount of clumping for flyaway hairs
		struct Amount : Parameters::ParameterDescriptor<ClumpParameters, 13, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "FlyAway.Amount"; }

			static float DefaultValue() { return 0.1f; }
		};
	};

	// When this value is non-zero the strand shapes will be linearly blended from root to this position, between their original and clumped shapes. This is done to avoid roots becoming unglued from the distribution mesh when using an option like Round Clumps.
	struct RootBlendingPosition : Parameters::ParameterDescriptor<ClumpParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "RootBlendingPosition"; }

		static float DefaultValue() { return 0.1f; }
	};

	// If this option is on and this operator modifies the root positions of strands the surface dependency for said strands will be recalculated. It is advised to do this for other operators down the pipeline to work correctly.
	struct GroundDisplacedRoots : Parameters::ParameterDescriptor<ClumpParameters, 53, bool>
	{
		static char const* Name() { return "GroundDisplacedRoots"; }

		static bool DefaultValue() { return true; }
	};

	// When enabled, the clump profiles will be forced to be circular. This is useful when doing braiding to avoid intersections of individual clump stems.
	struct RoundClumps : Parameters::ParameterDescriptor<ClumpParameters, 15, bool>
	{
		static char const* Name() { return "RoundClumps"; }

		static bool DefaultValue() { return false; }
	};

	struct ClumpSize
	{
		// When enabled, the profile of clumps will be forced to be this size (at places where Amount value is 1). This allows you to precisely control how big clumps are in absolute values to each other and is useful for braid stems.
		struct Enabled : Parameters::ParameterDescriptor<ClumpParameters, 16, bool>
		{
			static char const* Name() { return "ClumpSize.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		// When Set Clump Size is enabled, this sets the absolute clump profile diameter.
		struct Size
		{
			struct Value : Parameters::ParameterDescriptor<ClumpParameters, 17, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
			{
				static char const* Name() { return "ClumpSize.Size.Value"; }

				static float DefaultValue() { return 10.0f; }

				static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
			};

			struct Ramp : Parameters::ParameterDescriptor<ClumpParameters, 24, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "ClumpSize.Size.Ramp"; }

				static char const* DefaultValue() { return "0 1 0.5 1 1 1"; }
			};

			struct Map : Parameters::ParameterDescriptor<ClumpParameters, 51, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "ClumpSize.Size.Map"; }
			};
		};
	};

	// Specifies how clumps will be created when pressing the "Create Clump(s)" button
	struct ClumpCreateMethod : Parameters::ParameterDescriptor<ClumpParameters, 18, ClumpCreateMethodType>
	{
		static char const* Name() { return "ClumpCreateMethod"; }

		static ClumpCreateMethodType DefaultValue() { return ClumpCreateMethodType::Random; }
	};

	// When enabled, a new per-strand channel containing hair clump indices will be added. Useful for sub-clumping.
	struct AddClumpIndexChannel : Parameters::ParameterDescriptor<ClumpParameters, 19, bool>
	{
		static char const* Name() { return "AddClumpIndexChannel"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, hairs will be assigned strand groups based on their clump association.
	struct SetClumpStrandGroups : Parameters::ParameterDescriptor<ClumpParameters, 52, bool>
	{
		static char const* Name() { return "SetClumpStrandGroups"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the input hair strand lengths will not be modified during clumping
	struct PreserveStrandLengths : Parameters::ParameterDescriptor<ClumpParameters, 20, bool>
	{
		static char const* Name() { return "PreserveStrandLengths"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the closest point on clump stem for each hair point will be used to calculate the clumping amount and other length-varying parameters
	struct AttractToClosestStemPoint : Parameters::ParameterDescriptor<ClumpParameters, 21, bool>
	{
		static char const* Name() { return "AttractToClosestStemPoint"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the guides shapes will be used to define the clump stems. Otherwise, guides will only be used to define clump roots.
	struct SetGuideClumpShapes : Parameters::ParameterDescriptor<ClumpParameters, 23, bool>
	{
		static char const* Name() { return "SetGuideClumpShapes"; }

		static bool DefaultValue() { return true; }
	};

	struct Region
	{
		// Controls regions between which no clumping happens. Can also be used to generate clumps based on a map.
		struct Value
		{
			struct Channel : Parameters::ParameterDescriptor<ClumpParameters, 26, Ephere::Ornatrix::StrandChannel>
			{
				static char const* Name() { return "Region.Value.Channel"; }
			};

			struct Map : Parameters::ParameterDescriptor<ClumpParameters, 25, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Region.Value.Map"; }
			};
		};

		// When using sub-clumping, specifies by how much region values of two separate hairs have to differ for them to be considered in separate clumps.
		struct DifferenceThreshold : Parameters::ParameterDescriptor<ClumpParameters, 27, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Region.DifferenceThreshold"; }

			static float DefaultValue() { return 0.05f; }
		};

		// When enabled, the clumping will be avoided between hairs and clump centers which belong to different hair parts
		struct RespectHairParts : Parameters::ParameterDescriptor<ClumpParameters, 28, bool>
		{
			static char const* Name() { return "Region.RespectHairParts"; }

			static bool DefaultValue() { return false; }
		};

		// When using sub-clumping, determines the limit of nearby clumps to look up to ensure that hairs are located in the same region. Smaller values yield better performance, larger values provide better accuracy.
		struct MaximumClosestRegionClumpCandidates : Parameters::ParameterDescriptor<ClumpParameters, 22, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Region.MaximumClosestRegionClumpCandidates"; }

			static int DefaultValue() { return 10; }
		};
	};

	// Specifies the rotation value around a clump's stem for clump patterns
	struct ClumpPatternRotation : Parameters::ParameterDescriptor<ClumpParameters, 29, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
	{
		static char const* Name() { return "ClumpPatternRotation"; }

		static float DefaultValue() { return 0; }

		static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Radians;
	};

	struct UvSpace
	{
		// When enabled, clump and hairs will be calculated using texture space. This prevents incorrect calculations when clumps on different faces are close to each other.
		struct Use : Parameters::ParameterDescriptor<ClumpParameters, 30, bool>
		{
			static char const* Name() { return "UvSpace.Use"; }

			static bool DefaultValue() { return false; }
		};

		// Specifies the index of the mapping channel to use when "Use UV Space" parameter is on
		struct MappingChannel : Parameters::ParameterDescriptor<ClumpParameters, 31, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "UvSpace.MappingChannel"; }

			static int DefaultValue() { return 0; }
		};
	};

	// When enabled, clumps will fill the volume of the circles around them.
	struct VolumeFillingClumps : Parameters::ParameterDescriptor<ClumpParameters, 32, bool>
	{
		static char const* Name() { return "VolumeFillingClumps"; }

		static bool DefaultValue() { return false; }
	};

	struct PerClump
	{
		// Maps indices of clumps to the per-clump data stored in perClumpParameters_
		struct Indices : Parameters::ParameterDescriptor<ClumpParameters, 36, int[]>
		{
			static char const* Name() { return "PerClump.Indices"; }
		};

		struct Amounts : Parameters::ParameterDescriptor<ClumpParameters, 39, float[]>
		{
			static char const* Name() { return "PerClump.Amounts"; }
		};

		struct AmountCurves : Parameters::ParameterDescriptor<ClumpParameters, 37, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "PerClump.AmountCurves"; }
		};

		struct Twists : Parameters::ParameterDescriptor<ClumpParameters, 40, float[]>
		{
			static char const* Name() { return "PerClump.Twists"; }
		};

		struct TwistCurves : Parameters::ParameterDescriptor<ClumpParameters, 38, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "PerClump.TwistCurves"; }
		};

		// Per-clump indices into the clumpPatterns_ array specifying the chosen pattern. Values of -1 denote a lack of pattern.
		struct PatternIndices : Parameters::ParameterDescriptor<ClumpParameters, 42, int[]>
		{
			static char const* Name() { return "PerClump.PatternIndices"; }
		};
	};

	struct ClumpGuides
	{
		// Stores index of the face containing the clump guide or AbsentSurfaceDependency if Coordinates defines object space position
		struct FaceIndices : Parameters::ParameterDescriptor<ClumpParameters, 34, int[]>
		{
			static char const* Name() { return "ClumpGuides.FaceIndices"; }
		};

		// Could store either a barycentric coordinate on the referenced face or object space position os the clump guide, depending on what FaceIndices specifies
		struct Coordinates : Parameters::ParameterDescriptor<ClumpParameters, 46, Ephere::Ornatrix::Vector3[]>
		{
			static char const* Name() { return "ClumpGuides.Coordinates"; }
		};

		// This array has the same size as clumpGuideSurfaceDependencies_ and stores the indices to the closest clump for each one (in clumpCenterSurfaceDependencies_)
		struct ClosestClumpIndices : Parameters::ParameterDescriptor<ClumpParameters, 35, int[]>
		{
			static char const* Name() { return "ClumpGuides.ClosestClumpIndices"; }
		};

		// For each clump specifies the id of the strand in target hair on which it was generated, if the clump was generated for propagated hairs. Otherwise it will be InvalidStrandId.
		struct BaseStrandIds : Parameters::ParameterDescriptor<ClumpParameters, 44, int[]>
		{
			static char const* Name() { return "ClumpGuides.BaseStrandIds"; }
		};
	};

	struct ClumpCenters
	{
		// Stores index of the face containing the clump center or AbsentSurfaceDependency if Coordinates defines object space position
		struct FaceIndices : Parameters::ParameterDescriptor<ClumpParameters, 33, int[]>
		{
			static char const* Name() { return "ClumpCenters.FaceIndices"; }
		};

		// Could store either a barycentric coordinate on the referenced face or object space position of the clump center, depending on what FaceIndices specifies
		struct Coordinates : Parameters::ParameterDescriptor<ClumpParameters, 47, Ephere::Ornatrix::Vector3[]>
		{
			static char const* Name() { return "ClumpCenters.Coordinates"; }
		};

		// If guide or external strands create method is used this array holds the per-clump strand ids into the guides object
		struct GuideIds : Parameters::ParameterDescriptor<ClumpParameters, 43, int[]>
		{
			static char const* Name() { return "ClumpCenters.GuideIds"; }
		};

		// For each clump specifies the id of the strand in target hair on which it was generated, if the clump was generated for propagated hairs. Otherwise it will be InvalidStrandId.
		struct BaseStrandIds : Parameters::ParameterDescriptor<ClumpParameters, 45, int[]>
		{
			static char const* Name() { return "ClumpCenters.BaseStrandIds"; }
		};

		struct PatternIndices : Parameters::ParameterDescriptor<ClumpParameters, 41, int[]>
		{
			static char const* Name() { return "ClumpCenters.PatternIndices"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::ClumpParameters::ClumpCreateMethodType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::ClumpParameters::ClumpCreateMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ClumpParameters::ClumpCreateMethodType::Random: return "Random";
			case ClumpParameters::ClumpCreateMethodType::StrandGroups: return "StrandGroups";
			case ClumpParameters::ClumpCreateMethodType::Guides: return "Guides";
			case ClumpParameters::ClumpCreateMethodType::ExternalClumpStrands: return "ExternalClumpStrands";
			default: return "";
		}
	}
};
}
