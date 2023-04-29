#include "PrecompiledHeaders.h"

// Backward compatibility of new parameters with the .oxgroom format
// While converting operators to the new parameters, the new implementation of GetOperatorDefinition() should match the data in the following test

#include "Ephere/Core/OperatorDefinition.h"
#include "Ephere/Ornatrix/Private/Constants.h"
#include "Ephere/Ornatrix/Private/OperatorType.h"

#include "Ephere/Ornatrix/Operators/AdoptExternalGuides.h"
#include "Ephere/Ornatrix/Operators/Braider.h"
#include "Ephere/Ornatrix/Operators/Cacher.h"
#include "Ephere/Ornatrix/Operators/ChangeWidth.h"
#include "Ephere/Ornatrix/Operators/Clumper.h"
#include "Ephere/Ornatrix/Operators/Curler.h"
#include "Ephere/Ornatrix/Operators/Detailer.h"
#include "Ephere/Ornatrix/Operators/Frizzer.h"
#include "Ephere/Ornatrix/Operators/Gravity.h"
#include "Ephere/Ornatrix/Operators/Ground.h"
#include "Ephere/Ornatrix/Operators/GuidesEditor.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshGenerator.h"
#include "Ephere/Ornatrix/Operators/HairAnimationCache.h"
#include "Ephere/Ornatrix/Operators/HairFromGuidesGenerator.h"
#include "Ephere/Ornatrix/Operators/HairFromMeshStripsGenerator.h"
#include "Ephere/Ornatrix/Operators/Lengthener.h"
#include "Ephere/Ornatrix/Operators/MeshFromHairGenerator.h"
#include "Ephere/Ornatrix/Operators/Multiplier.h"
#include "Ephere/Ornatrix/Operators/Noise.h"
#include "Ephere/Ornatrix/Operators/Normalize.h"
#include "Ephere/Ornatrix/Operators/Oscillator.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Operators/PushAwayFromSurface.h"
#include "Ephere/Ornatrix/Operators/ResolveCollisions.h"
#include "Ephere/Ornatrix/Operators/Rotate.h"
#include "Ephere/Ornatrix/Operators/StrandDataGenerator.h"
#include "Ephere/Ornatrix/Operators/SurfaceComber.h"
#include "Ephere/Ornatrix/Operators/Symmetry.h"
#include "Ephere/Ornatrix/Operators/Weaver.h"

#include "Ephere/NativeTools/Test/Catch.h"

// TODO: Re-enable when parameters are converted
#ifndef DOUBLE_PRECISION

using namespace Ephere;
using namespace Core;
using namespace Ornatrix;

void Match( OperatorDefinition const& opDefOld, OperatorDefinition const& opDefNew )
{
	auto operatorName = std::string( opDefNew.GetName() );
	REQUIRE( opDefOld.GetName() == operatorName );
	REQUIRE( opDefOld.type == opDefNew.type );
	auto const& paramsOld = opDefOld.GetParameters();
	auto const& paramsNew = opDefNew.GetParameters();
	auto index = 0;
	for( auto const& paramOld : paramsOld )
	{
		std::string name = paramOld.name;
		auto iter = FindIf( paramsNew, [&name]( ParameterDefinition const& def )
		{
			return def.name == name;
		} );

		if( iter == paramsNew.end() )
		{
			// The only parameter that is allowed to be removed is 'strandGroup'
			if( name == "strandGroup" )
			{
				continue;
			}
		}

		INFO( operatorName + ": " + name )
		ASSERT( iter != paramsNew.end() );
		REQUIRE( iter != paramsNew.end() );
		auto const& paramNew = *iter;
		REQUIRE( paramOld.name == std::string( paramNew.name ) );
		REQUIRE( paramOld.type == paramNew.type );
		// Ignore ParameterUnits::Granularity for now
		if( paramOld.units != ParameterUnits::Granularity
			&& paramOld.units != ParameterUnits::Generic )
		{
			REQUIRE( paramOld.units == paramNew.units );
		}

		REQUIRE( paramOld.valueEnumerationEntries.size() <= paramNew.valueEnumerationEntries.size() );
		REQUIRE( paramOld.valueEnumerationEntries == std::vector<std::wstring>( paramNew.valueEnumerationEntries.begin(), paramNew.valueEnumerationEntries.begin() + paramOld.valueEnumerationEntries.size() ) );
		++index;
	}
}

TEST_CASE( "XmlGroomBackwardCompatibility" )
{
	SECTION( "AdoptExternalGuides" )
	{
		char const OperatorName[] = "AdoptExternalGuides";

		char const AmountParameterName[] = "amount";
		char const ControlPointParameterName[] = "controlPoint";
		char const MatchControlStrandsByDistanceParameterName[] = "matchControlStrandsByDistance";

		auto const AmountDefaultValue = 1.0f;
		auto const ControlPointDefaultValue = 0.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::AdoptExternalGuides,
				{
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ AmountParameterName, AmountDefaultValue, ParameterUnits::Fraction },
					{ ControlPointParameterName, ControlPointDefaultValue, ParameterUnits::Fraction },
					{ MatchControlStrandsByDistanceParameterName, false },
				} ),
				AdoptExternalGuides::GetOperatorDefinition() );
	}

	SECTION( "Braider" )
	{
		auto OperatorName = "Braider";

		char const LengthParameterName[] = "length";
		char const PointCountParameterName[] = "pointCount";
		char const FrequencyParameterName[] = "frequency";
		char const ScaleParameterName[] = "scale";
		char const VerticalScaleParameterName[] = "verticalScale";
		char const PhaseParameterName[] = "phase";

		auto const LengthDefaultValue = 50.0f;
		auto const FrequencyDefaultValue = 0.5f;
		auto const ScaleDefaultValue = 5.0f;
		auto const VerticalScaleDefaultValue = 0.5f;
		auto const PhaseDefaultValue = 0.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Braider,
				{
					{ LengthParameterName, LengthDefaultValue, ParameterUnits::Centimeters },
					{ PointCountParameterName, 50 },
					{ FrequencyParameterName, FrequencyDefaultValue },
					{ ScaleParameterName, ScaleDefaultValue },
					{ VerticalScaleParameterName, VerticalScaleDefaultValue },
					{ PhaseParameterName, PhaseDefaultValue },
				} ),
				Braider::GetOperatorDefinition() );
	}

	SECTION( "Cacher" )
	{
		auto const OperatorName = "Cache";
		auto const StrandCountParameterName = "strandCount";
		auto const PointsPerStrandCountsParameterName = "pointsPerStrandCounts";
		auto const VerticesParameterName = "vertices";

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Cache,
				{
					{ StrandCountParameterName, -1 },
					{ PointsPerStrandCountsParameterName, ParameterType::IntegerArray },
					{ VerticesParameterName, ParameterType::FloatArray },
				} ),
				Cacher::GetOperatorDefinition() );
	}

	SECTION( "Clumper" )
	{
		char const OperatorName[] = "Clump";
		char const AmountParameterName[] = "amount";
		char const TwistParameterName[] = "twist";
		char const ClumpBarycentricCoordinatesParameterName[] = "clumpBarycentricCoordinates";
		char const ClumpFaceIndicesParameterName[] = "clumpFaceIndices";
		char const RootBarycentricCoordinatesParameterName[] = "rootBarycentricCoordinates";
		char const RootFaceIndicesParameterName[] = "rootFaceIndices";
		char const RootClosestClumpIndicesParameterName[] = "rootClosestClumpIndices";
		char const PerClumpIndicesParameterName[] = "perClumpIndices";
		//const char ClumpAmountCurvesKnotCountsParameterName[] = "clumpAmountCurvesKnotCounts";
		//const char ClumpAmountCurvesPositionsParameterName[] = "clumpValueCurvesPositions";
		//const char ClumpAmountCurvesValuesParameterName[] = "clumpValueCurvesValues";
		//const char ClumpAmountCurvesInterpolationsParameterName[] = "clumpValueCurvesInterpolations";
		//const char ClumpTwistCurvesKnotCountsParameterName[] = "clumpTwistCurvesKnotCounts";
		//const char ClumpTwistCurvesPositionsParameterName[] = "clumpTwistCurvesPositions";
		//const char ClumpTwistCurvesValuesParameterName[] = "clumpTwistCurvesValues";
		//const char ClumpTwistCurvesInterpolationsParameterName[] = "clumpTwistCurvesInterpolations";
		char const ClumpPatternIndicesParameterName[] = "clumpPatternIndices";
		char const PerClumpAmountsParameterName[] = "perClumpAmounts";
		char const PerClumpTwistsParameterName[] = "perClumpTwists";
		char const PerClumpPatternIndicesParameterName[] = "perClumpPatternIndices";
		char const FlyawayFractionParameterName[] = "flyawayFraction";
		char const FlyawayAmountParameterName[] = "flyawayAmount";
		char const RootBlendingPositionParameterName[] = "rootBlendingPosition";
		char const RoundClumpsParameterName[] = "roundClumps";
		char const SetClumpSizeParameterName[] = "setClumpSize";
		char const ClumpSizeParameterName[] = "clumpSize";
		char const ClumpCreateMethodParameterName[] = "clumpCreateMethod";
		char const ClumpCenterGuideIdsParameterName[] = "clumpCenterGuideIds";
		char const AddClumpIndexChannelParameterName[] = "addClumpIndexChannel";
		char const PreserveStrandLengthsParameterName[] = "preserveStrandLengths";
		char const AttractToClosestStemPointParameterName[] = "attractToClosestStemPoint";
		char const MaximumClosestRegionClumpCandidatesParameterName[] = "maximumClosestRegionClumpCandidates";
		char const SetGuideClumpShapesParameterName[] = "setGuideClumpShapes";
		char const RegionDifferenceThresholdParameterName[] = "regionDifferenceThreshold";
		char const ClumpPatternRotationParameterName[] = "clumpPatternRotation";
		char const UseUvSpaceParameterName[] = "useUvSpace";
		char const UvMappingChannelParameterName[] = "uvMappingChannel";
		char const RespectHairPartsParameterName[] = "respectHairParts";
		char const VolumeFillingClumpsParameterName[] = "volumeFillingClumps";
		char const ClumpGuideBaseStrandIdsParameterName[] = "clumpGuideBaseStrandIds";
		char const ClumpCenterBaseStrandIdsParameterName[] = "clumpCenterBaseStrandIds";

		auto const AmountDefaultValue = 1.0f;
		auto const TwistDefaultValue = 0.0f;
		auto const FlyawayFractionDefaultValue = 0.05f;
		auto const FlyawayAmountDefaultValue = 0.1f;
		auto const RootBlendingPositionDefaultValue = 0.1f;
		auto const ClumpSizeDefaultValue = 10.0f;
		auto const RegionDifferenceThresholdDefaultValue = 0.05f;
		auto const ClumpPatternRotationDefaultValue = 0.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Clump,
				{
					{ AmountParameterName, AmountDefaultValue },
					{ TwistParameterName, TwistDefaultValue, ParameterUnits::Radians },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ ClumpBarycentricCoordinatesParameterName, ParameterType::Vector3Array },
					{ ClumpFaceIndicesParameterName, ParameterType::IntegerArray },
					{ RootBarycentricCoordinatesParameterName, ParameterType::Vector3Array },
					{ RootFaceIndicesParameterName, ParameterType::IntegerArray },
					{ RootClosestClumpIndicesParameterName, ParameterType::IntegerArray },
					{ PerClumpIndicesParameterName, ParameterType::IntegerArray },
					// No longer using these parameters, ramp arrays are stored explicitly
					/*{ ClumpAmountCurvesKnotCountsParameterName, ParameterType::IntegerArray },
					{ ClumpAmountCurvesPositionsParameterName, ParameterType::FloatArray },
					{ ClumpAmountCurvesValuesParameterName, ParameterType::FloatArray },
					{ ClumpAmountCurvesInterpolationsParameterName, ParameterType::IntegerArray },
					{ ClumpTwistCurvesKnotCountsParameterName, ParameterType::IntegerArray },
					{ ClumpTwistCurvesPositionsParameterName, ParameterType::FloatArray },
					{ ClumpTwistCurvesValuesParameterName, ParameterType::FloatArray },
					{ ClumpTwistCurvesInterpolationsParameterName, ParameterType::IntegerArray },*/
					{ PerClumpAmountsParameterName, ParameterType::FloatArray },
					{ PerClumpTwistsParameterName, ParameterType::FloatArray },
					{ ClumpPatternIndicesParameterName, ParameterType::IntegerArray },
					{ PerClumpPatternIndicesParameterName, ParameterType::IntegerArray },
					{ FlyawayFractionParameterName, FlyawayFractionDefaultValue, ParameterUnits::Fraction },
					{ FlyawayAmountParameterName, FlyawayAmountDefaultValue },
					{ RootBlendingPositionParameterName, RootBlendingPositionDefaultValue },
					{ RoundClumpsParameterName, false },
					{ SetClumpSizeParameterName, false },
					{ ClumpSizeParameterName, ClumpSizeDefaultValue },
					{ ClumpCreateMethodParameterName, 0 },
					{ ClumpCenterGuideIdsParameterName, ParameterType::IntegerArray },
					{ AddClumpIndexChannelParameterName, false },
					{ PreserveStrandLengthsParameterName, false },
					{ AttractToClosestStemPointParameterName, false },
					{ MaximumClosestRegionClumpCandidatesParameterName, 10 },
					{ SetGuideClumpShapesParameterName, true },
					{ RegionDifferenceThresholdParameterName, RegionDifferenceThresholdDefaultValue },
					{ ClumpPatternRotationParameterName, ClumpPatternRotationDefaultValue },
					{ UseUvSpaceParameterName, false },
					{ UvMappingChannelParameterName, 0 },
					{ RespectHairPartsParameterName, false },
					{ VolumeFillingClumpsParameterName, false },
					{ ClumpGuideBaseStrandIdsParameterName, ParameterType::IntegerArray },
					{ ClumpCenterBaseStrandIdsParameterName, ParameterType::IntegerArray },
					// Version 0 by default
					//{ Constants::VersionParameterName, 0 },
					{ Constants::GroupBlendDistanceParameterName, Constants::GroupBlendDistanceDefaultValue }
				} ),
			Clumper::GetOperatorDefinition() );
	}

	SECTION( "Curler" )
	{
		auto WaveformParameterName = "waveform";
		auto MagnitudeParameterName = "magnitude";
		auto MagnitudeCurveParameterName = "magnitudeCurve";
		auto PhaseParameterName = "phase";
		auto PhaseOffsetParameterName = "phaseOffset";
		auto StretchParameterName = "stretch";
		auto IsLengthDependentParameterName = "isLengthDependent";
		auto NoiseScaleParameterName = "noiseScale";
		auto NoiseAmountParameterName = "noiseAmount";

		auto DefaultWaveformValue = 0;
		auto DefaultMagnitudeValue = 1.0f;
		auto DefaultPhaseValue = 2.0f;
		auto DefaultPhaseOffsetValue = 0.0f;
		auto DefaultStretchValue = 0.0f;
		auto DefaultIsLengthDependentValue = false;
		auto DefaultNoiseScaleValue = 3.0f;
		auto DefaultNoiseAmountValue = 1.0f;

		Match(
			OperatorDefinition(
				"Curl", OperatorType::Curl,
				{
					{ WaveformParameterName, DefaultWaveformValue, { L"Sine", L"Helix" } },
					{ MagnitudeParameterName, DefaultMagnitudeValue, ParameterUnits::Centimeters },
					{ MagnitudeCurveParameterName, ParameterType::FloatArray },
					{ PhaseParameterName, DefaultPhaseValue },
					{ PhaseOffsetParameterName, DefaultPhaseOffsetValue },
					{ StretchParameterName, DefaultStretchValue },
					{ IsLengthDependentParameterName, DefaultIsLengthDependentValue },
					{ NoiseScaleParameterName, DefaultNoiseScaleValue/*, ParameterUnits::Centimeters*/ },
					{ NoiseAmountParameterName, DefaultNoiseAmountValue/*, ParameterUnits::Fraction*/ },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue }
				} ),
			Curler::GetOperatorDefinition() );
	}

	SECTION( "Detailer" )
	{
		char const OperatorName[] = "Detail";
		char const ViewCountParameterName[] = "viewCount";
		char const RenderCountParameterName[] = "renderCount";
		char const AngleThresholdParameterName[] = "angleThreshold";
		char const IsSamplingByThresholdParameterName[] = "sampleByThreshold";
		char const SampleSizeParameterName[] = "sampleSize";
		char const SmoothingParameterName[] = "smoothing";
		char const SmoothingPointCountParameterName[] = "smoothingPointCount";
		char const IsSamplingByLengthParameterName[] = "sampleByLength";
		char const SamplingByLengthDistanceParameterName[] = "distance";

		auto const DefaultSmoothingValue = 0.4f;
		auto const DefaultSampleSizeValue = 0.01f;
		auto const DefaultAngleThresholdValue = 0.1f;
		auto const DefaultSamplingByLengthDistanceValue = 1.f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Detail,
				{
					{ ViewCountParameterName, 10, ParameterUnits::Granularity },
					{ RenderCountParameterName, 10, ParameterUnits::Granularity },
					{ AngleThresholdParameterName, DefaultAngleThresholdValue },
					{ IsSamplingByThresholdParameterName, false },
					{ SampleSizeParameterName, DefaultSampleSizeValue },
					{ SmoothingParameterName, DefaultSmoothingValue },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ SmoothingPointCountParameterName, 10 },
					{ IsSamplingByLengthParameterName, false },
					{ SamplingByLengthDistanceParameterName, DefaultSamplingByLengthDistanceValue },
				} ),
				Detailer::GetOperatorDefinition() );
	}

	SECTION( "Frizzer" )
	{
		auto const OperatorName = "Frizz";
		auto const AmountParameterName = "amount";
		auto const AmountCurveParameterName = "amountCurve";
		auto const ScaleParameterName = "scale";
		auto const OutlierPercentParameterName = "outlierPercent";
		auto const OutlierAmountParameterName = "outlierAmount";
		auto const IsLengthDependentParameterName = "isLengthDependent";
		auto const AddOutlierChannelParameterName = "addOutlierChannel";

		auto const AmountDefaultValue = 20.0f;
		auto const ScaleDefaultValue = 1.0f;
		auto const OutlierPercentDefaultValue = 0.1f;
		auto const OutlierAmountDefaultValue = 5.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Frizz,
				{
					{ AmountParameterName, AmountDefaultValue/*, ParameterUnits::Fraction (was incorrect)*/ },
					{ AmountCurveParameterName, ParameterType::FloatArray },
					{ ScaleParameterName, ScaleDefaultValue },
					{ OutlierPercentParameterName, OutlierPercentDefaultValue, ParameterUnits::Fraction },
					{ OutlierAmountParameterName, OutlierAmountDefaultValue/*, ParameterUnits::Fraction (was incorrect)*/ },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ IsLengthDependentParameterName, false },
					{ AddOutlierChannelParameterName, false },
					{ Constants::GroupBlendDistanceParameterName, Constants::GroupBlendDistanceDefaultValue }
				} ),
			Frizzer::GetOperatorDefinition() );
	}

	SECTION( "Gravity" )
	{
		auto const OperatorName = "Gravity";
		auto const ForceParameterName = "force";
		auto const AmountCurveParameterName = "amountCurve";

		auto const DefaultForceValue = 1.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Gravity,
				{
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ ForceParameterName, DefaultForceValue, ParameterUnits::Centimeters },
					{ AmountCurveParameterName, ParameterType::FloatArray },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
				} ),
				Gravity::GetOperatorDefinition() );
	}

	SECTION( "StrandDataGenerator" )
	{
		char const OperatorName[] = "GenerateStrandData";
		char const GenerationMethodParameterName[] = "generationMethod";
		char const UseSourceValueRangeParameterName[] = "useSourceValueRange";
		char const MinimumSourceValueParameterName[] = "minimumSourceValue";
		char const MaximumSourceValueParameterName[] = "maximumSourceValue";
		char const UseTargetValueRangeParameterName[] = "useTargetValueRange";
		char const MinimumTargetValueParameterName[] = "minimumTargetValue";
		char const MaximumTargetValueParameterName[] = "maximumTargetValue";
		char const NewChannelNameParameterName[] = "newChannelName";
		char const NewChannelTypeParameterName[] = "newChannelType";
		//const char PreviousValueMultiplierParameterName[] = "previousValueMultiplier";
		//const char CurrentValueMultiplierParameterName[] = "currentValueMultiplier";
		char const IsStoringValuesParameterName[] = "isStoringValues";
		char const StoredValuesParameterName[] = "storedValues";
		char const SampleValueCountParameterName[] = "sampleValueCount";
		char const SeExprValueParameterName[] = "seExprValue";
		char const TargetDataParameterName[] = "targetData";
		char const StoredValueStrandIdsParameterName[] = "storedValueStrandIds";
		char const StoredValueFirstVertexIndicesParameterName[] = "storedValueFirstVertexIndices";
		char const TargetChannelIndexParameterName[] = "targetChannelIndex";

		wchar_t const NewChannelNameDefaultValue[] = L"My Channel";
		wchar_t const SeExpValueDefaultValue[] = L"";
		auto const MinimumSourceValueDefaultValue = 0.0f;
		auto const MaximumSourceValueDefaultValue = 1.0f;
		auto const MinimumTargetValueDefaultValue = 0.0f;
		auto const MaximumTargetValueDefaultValue = 1.0f;
		//const float PreviousValueMultiplierDefaultValue = 0.0f;
		//const float CurrentValueMultiplierDefaultValue = 1.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::GenerateStrandData,
				{
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ GenerationMethodParameterName, 0,
						{
							L"Constant",
							L"Random",
							L"Index",
							L"StrandLength",
							L"StrandCurviness",
							L"StrandDistanceToObject",
							L"IsInsideObject",
							L"WorldSpacePosition",
							L"ObjectSpacePosition",
							L"SeExpr",
							L"TextureMap",
						}
					},
					{ NewChannelNameParameterName, NewChannelNameDefaultValue },
					{ UseTargetValueRangeParameterName, true },
					{ MinimumTargetValueParameterName, MinimumTargetValueDefaultValue },
					{ MaximumTargetValueParameterName, MaximumTargetValueDefaultValue },
					{ UseSourceValueRangeParameterName, false },
					{ MinimumSourceValueParameterName, MinimumSourceValueDefaultValue },
					{ MaximumSourceValueParameterName, MaximumSourceValueDefaultValue },
					{ NewChannelTypeParameterName, 0 },
					//{ PreviousValueMultiplierParameterName, PreviousValueMultiplierDefaultValue },
					//{ CurrentValueMultiplierParameterName, CurrentValueMultiplierDefaultValue },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ IsStoringValuesParameterName, false },
					{ StoredValuesParameterName, ParameterType::FloatArray },
					{ SampleValueCountParameterName, 1 },
					{ SeExprValueParameterName, SeExpValueDefaultValue },
					{ TargetDataParameterName, 1,
						{
							L"ExistingStrandDataChannel",
							L"NewStrandDataChannel",
							L"StrandGroups",
							L"StrandIds",
							L"Widths",
							L"TextureCoordinates",
							L"StrandRotations",
						}
					},
					{ StoredValueStrandIdsParameterName, ParameterType::IntegerArray },
					{ StoredValueFirstVertexIndicesParameterName, ParameterType::IntegerArray },
					{ TargetChannelIndexParameterName, 0 }
				} ),
			StrandDataGenerator::GetOperatorDefinition() );
	}

	SECTION( "HairAnimationCache" )
	{
		char const OperatorName[] = "HairAnimationCache";

		char const FilePathParameterName[] = "filePath";
		char const TimeOffsetParameterName[] = "timeOffset";
		char const TimeScaleParameterName[] = "timeScale";
		char const ApplyAmountParameterName[] = "applyAmount";
		char const ImportWidths[] = "importWidths";
		char const ImportTextureCoordinates[] = "importTextureCoordinates";
		char const ImportChannelData[] = "importChannelData";
		char const ImportStrandGroups[] = "importStrandGroups";
		char const ImportStrandIds[] = "importStrandIds";
		char const OverwriteInputParameterName[] = "overwriteInput";

		wchar_t const FilePathDefaultValue[] = L"";
		auto const TimeOffsetDefaultValue = 0.0f;
		auto const TimeScaleDefaultValue = 1.0f;
		auto const ApplyAmountDefaultValue = 1.0f;
		auto const ImportWidthsDefaultValue = false;
		auto const ImportTextureCoordinatesDefaultValue = false;
		auto const ImportChannelDataDefaultValue = true;
		auto const ImportStrandGroupsDefaultValue = false;
		auto const ImportStrandIdsDefaultValue = true;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::HairAnimationCache,
				{
					{ FilePathParameterName, FilePathDefaultValue, ParameterUnits::FilePath },
					{ TimeOffsetParameterName, TimeOffsetDefaultValue, ParameterUnits::Seconds },
					{ TimeScaleParameterName, TimeScaleDefaultValue, ParameterUnits::Fraction },
					{ ApplyAmountParameterName, ApplyAmountDefaultValue, ParameterUnits::Fraction },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ ImportWidths, ImportWidthsDefaultValue },
					{ ImportTextureCoordinates, ImportTextureCoordinatesDefaultValue },
					{ ImportChannelData, ImportChannelDataDefaultValue },
					{ ImportStrandGroups, ImportStrandGroupsDefaultValue },
					{ ImportStrandIds, ImportStrandIdsDefaultValue },
					{ OverwriteInputParameterName, false },
				} ),
				HairAnimationCache::GetOperatorDefinition() );
	}

	SECTION( "HairFromMeshStripsGenerator" )
	{
		auto const OperatorName = "HairFromMeshStrips";
		auto const DistributionParameterName = "distribution";
		auto const HairsPerStripCountParameterName = "hairsPerStrip";
		auto const PointsPerStrandCountParameterName = "pointsPerStrand";
		auto const VolumeParameterName = "volume";
		auto const VolumeNormalBiasParameterName = "volumeNormalBias";
		auto const InheritTextureCoordinatesFromStripsParameterName = "inheritTextureCoordinatesFromStrips";
		auto const AddStripIdChannelParameterName = "addStripIdChannel";

		auto const DefaultVolumeValue = 0.0f;
		auto const DefaultVolumeNormalBiasValue = 0.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::HairFromMeshStrips,
				{
					{ InheritTextureCoordinatesFromStripsParameterName, true },
					{ DistributionParameterName, 2 },
					{ HairsPerStripCountParameterName, 100 },
					{ VolumeParameterName, DefaultVolumeValue },
					{ VolumeNormalBiasParameterName, DefaultVolumeNormalBiasValue },
					{ PointsPerStrandCountParameterName, 5 },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ AddStripIdChannelParameterName, false }
				} ),
			HairFromMeshStripsGenerator::GetOperatorDefinition() );
	}

	SECTION( "Ground" )
	{
		auto const OperatorName = "GroundStrands";

		char const IsSettingPositionsParameterName[] = "setPositions";
		char const IsSettingRotationsParameterName[] = "setRotations";
		char const IsSettingTextureCoordinatesParameterName[] = "setTextureCoordinates";
		//const char StrandFaceIndicesParameterName[] = "strandFaceIndices";
		//const char StrandFaceBarycentricCoordinatesParameterName[] = "strandFaceBarycentricCoordinates";
		char const FirstSegmentDirectionsParameterName[] = "firstSegmentDirections";
		char const InitialRootOffsetsInFaceSpaceParameterName[] = "initialRootOffsetsInFaceSpace";
		char const FlipStrandsParameterName[] = "flipStrands";
		char const StrandIdsParameterName[] = "strandIds";

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::GroundStrands,
				{
					{ IsSettingPositionsParameterName, true },
					{ IsSettingRotationsParameterName, true },
					//{ StrandFaceIndicesParameterName, ParameterType::IntegerArray }, // Replaced by SurfacePositions, cannot be matched, needs manual conversion
					//{ StrandFaceBarycentricCoordinatesParameterName, ParameterType::Vector3Array }, // Replaced by SurfacePositions, cannot be matched, needs manual conversion
					{ FirstSegmentDirectionsParameterName, ParameterType::Vector3Array },
					{ InitialRootOffsetsInFaceSpaceParameterName, ParameterType::Vector3Array },
					{ FlipStrandsParameterName, ParameterType::BooleanArray },
					{ StrandIdsParameterName, ParameterType::IntegerArray },
					{ IsSettingTextureCoordinatesParameterName, true },
					//{ Constants::VersionParameterName, 0 }, // Removed
				} ),
				Ground::GetOperatorDefinition() );
	}

	SECTION( "Lengthener" )
	{
		char const OperatorName[] = "Length";
		char const ValueParameterName[] = "value";
		char const MinimumValueParameterName[] = "minimumValue";
		char const MaximumValueParameterName[] = "maximumValue";
		char const RandomizeParameterName[] = "randomize";
		char const UseScalingParameterName[] = "useScaling";
		char const SetAbsoluteLengthParameterName[] = "setAbsoluteLength";
		char const ScaleWidthParameterName[] = "scaleWidth";

		float const ValueDefaultValue = 1;
		auto const MinimumValueDefaultValue = 0.001f;
		float const MaximumValueDefaultValue = 99999;
		float const RandomizeDefaultValue = 0;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Length,
				{
					{ ValueParameterName, ValueDefaultValue, ParameterUnits::Fraction },
					{ RandomizeParameterName, RandomizeDefaultValue, ParameterUnits::Fraction },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					// For these two the old ParameterUnits::Fraction was wrong, new Distance purpose is correct
					{ MinimumValueParameterName, MinimumValueDefaultValue/*, ParameterUnits::Fraction */ },
					{ MaximumValueParameterName, MaximumValueDefaultValue/*, ParameterUnits::Fraction */ },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ UseScalingParameterName, false },
					{ SetAbsoluteLengthParameterName, false },
					{ ScaleWidthParameterName, false },
					{ Constants::GroupBlendDistanceParameterName, Constants::GroupBlendDistanceDefaultValue }
				} ),
			Lengthener::GetOperatorDefinition() );
	}

	SECTION( "Multiplier" )
	{
		char const OperatorName[] = "Multiplier";
		char const ProbabilityParameterName[] = "probability";
		char const CopiesParameterName[] = "copies";
		char const CopiesViewPercentParameterName[] = "copiesViewPercent";
		char const SpreadCurveParameterName[] = "spreadCurve";
		char const SpreadRootParameterName[] = "spreadRoot";
		char const SpreadTipParameterName[] = "spreadTip";
		char const FluffCurveParameterName[] = "fluffCurve";
		char const FluffRootParameterName[] = "fluffRoot";
		char const FluffTipParameterName[] = "fluffTip";
		char const ShiftCurveParameterName[] = "shiftCurve";
		char const ShiftRootParameterName[] = "shiftRoot";
		char const ShiftTipParameterName[] = "shiftTip";
		char const TwistParameterName[] = "twist";
		char const TwistCurveParameterName[] = "twistCurve";
		char const PreserveStrandLengthsParameterName[] = "preserveStrandLengths";
		char const PreserveOriginalStrandParameterName[] = "preserveOriginalStrand";
		char const TargetStrandGroupParameterName[] = "resultStrandGroup";
		char const GroundGenStrandsParameterName[] = "groundGenStrands";

		auto const ProbabilityDefaultValue = 1.0f;
		auto const SpreadRootDefaultValue = 0.0f;
		auto const SpreadTipDefaultValue = 3.0f;
		auto const FluffRootDefaultValue = 0.0f;
		auto const FluffTipDefaultValue = 3.0f;
		auto const ShiftRootDefaultValue = 0.0f;
		auto const ShiftTipDefaultValue = 0.0f;
		auto const TwistDefaultValue = 0.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Multiplier,
				{
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ ProbabilityParameterName, ProbabilityDefaultValue, ParameterUnits::Fraction },
					{ CopiesParameterName, 3 },
					{ CopiesViewPercentParameterName, 100 },
					{ SpreadCurveParameterName, ParameterType::FloatArray },
					{ SpreadRootParameterName, SpreadRootDefaultValue },
					{ SpreadTipParameterName, SpreadTipDefaultValue },
					{ FluffCurveParameterName, ParameterType::FloatArray },
					{ FluffRootParameterName, FluffRootDefaultValue },
					{ FluffTipParameterName, FluffTipDefaultValue },
					{ ShiftCurveParameterName, ParameterType::FloatArray },
					{ ShiftRootParameterName, ShiftRootDefaultValue },
					{ ShiftTipParameterName, ShiftTipDefaultValue },
					{ TwistParameterName, TwistDefaultValue },
					{ TwistCurveParameterName, ParameterType::FloatArray },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ PreserveStrandLengthsParameterName, false },
					{ PreserveOriginalStrandParameterName, false },
					{ TargetStrandGroupParameterName, 0 },
					{ GroundGenStrandsParameterName, false },
				} ),
				Multiplier::GetOperatorDefinition() );
	}

	SECTION( "Noise" )
	{
		char const OperatorName[] = "Noise";
		char const AmountParameterName[] = "amount";
		char const AmountVectorParameterName[] = "amountVector";
		char const AmountCurveParameterName[] = "amountCurve";
		char const NoiseScaleParameterName[] = "noiseScale";
		char const WorldSpaceParameterName[] = "worldSpace";
		char const PreserveStrandLengthParameterName[] = "preserveStrandLength";
		char const SpaceOffsetParameterName[] = "spaceOffset";
		char const TimeOffsetParameterName[] = "timeOffset";
		char const MovementDirectionParameterName[] = "movementDirection";
		char const MovementSpeedParameterName[] = "movementSpeed";

		auto const DefaultAmountValue = 1_r;
		auto const DefaultAmountVectorValue = Vector3( 1_r, 1_r, 1_r );
		auto const DefaultNoiseScaleValue = 1_r;
		auto const DefaultWorldSpaceValue = false;
		auto const DefaultPreserveStrandLengthValue = true;
		Vector3 const DefaultSpaceOffsetValue = Vector3::Zero();
		auto const DefaultTimeOffsetValue = 0_r;
		Vector3 const DefaultMovementDirectionValue = Vector3::Zero();
		auto const DefaultMovementSpeedValue = 0_r;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Noise,
				{
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ AmountParameterName, DefaultAmountValue },
					{ AmountVectorParameterName, ParameterType::Vector3, DefaultAmountVectorValue },
					{ AmountCurveParameterName, ParameterType::FloatArray },
					{ NoiseScaleParameterName, DefaultNoiseScaleValue },
					{ WorldSpaceParameterName, DefaultWorldSpaceValue },
					{ PreserveStrandLengthParameterName, DefaultPreserveStrandLengthValue },
					{ SpaceOffsetParameterName, ParameterType::Vector3, DefaultSpaceOffsetValue },
					{ TimeOffsetParameterName, DefaultTimeOffsetValue },
					{ MovementDirectionParameterName, ParameterType::Vector3, DefaultMovementDirectionValue },
					{ MovementSpeedParameterName, DefaultMovementSpeedValue },
					{ Constants::GroupBlendDistanceParameterName, static_cast<Real>( Constants::GroupBlendDistanceDefaultValue ) }
				} ),
			Noise::GetOperatorDefinition() );
	}

	SECTION( "Normalize" )
	{
		auto const OperatorName = "NormalizeStrands";

		auto const AmountParameterName = "amount";

		auto const AmountDefaultValue = 1.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Normalize,
				{
					{ AmountParameterName, AmountDefaultValue, ParameterUnits::Fraction },
				} ),
				Normalize::GetOperatorDefinition() );
	}

	SECTION( "Oscillator" )
	{
		auto const OperatorName = "Oscillator";

		auto const AmountParameterName = "amount";
		auto const IterationCountParameterName = "iterationCount";
		auto const DampingParameterName = "damping";
		auto const TimeScaleParameterName = "timeScale";
		auto const StiffnessParameterName = "stiffness";
		auto const InertiaParameterName = "inertia";
		auto const GravityParameterName = "gravity";

		auto const AmountDefaultValue = 1.0f;
		auto const IterationCountDefaultValue = 4;
		auto const DampingDefaultValue = 0.3f;
		auto const TimeScaleDefaultValue = 1.0f;
		auto const StiffnessDefaultValue = 1.0f;
		auto const InertiaDefaultValue = 0.1f;
		Vector3 const GravityDefaultValue = Vector3::Zero();

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Oscillator,
				{
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ AmountParameterName, AmountDefaultValue },
					{ DampingParameterName, DampingDefaultValue },
					{ TimeScaleParameterName, TimeScaleDefaultValue },
					{ StiffnessParameterName, StiffnessDefaultValue },
					{ InertiaParameterName, InertiaDefaultValue },
					{ GravityParameterName, ParameterType::Vector3, GravityDefaultValue },
					{ IterationCountParameterName, IterationCountDefaultValue },
				} ),
				Oscillator::GetOperatorDefinition() );
	}

	SECTION( "Propagator" )
	{
		char const OperatorName[] = "Propagation";
		char const RootGenerationMethodParameterName[] = "generationMethod";
		char const RootCountParameterName[] = "count";
		char const VerticesPerRootCountParameterName[] = "verticesPerRootCount";
		char const CountPerStrandParameterName[] = "countPerStrand";
		char const LowRangeParameterName[] = "lowRange";
		char const HighRangeParameterName[] = "highRange";
		char const IsLengthRelativeToBaseStrandParameterName[] = "lengthRelativeToBaseStrand";
		char const IsFacingSurfaceParameterName[] = "faceSurface";
		char const SideCountParameterName[] = "sideCount";
		char const TwistValueParameterName[] = "twist";
		char const TwistRandomnessParameterName[] = "twistRandomness";
		char const TwistCurveParameterName[] = "twistCurve";
		char const FanningParameterName[] = "fanning";
		char const FanningRandomnessParameterName[] = "fanningRandomness";
		char const FanningCurveParameterName[] = "fanningCurve";
		char const LengthParameterName[] = "length";
		char const LengthRandomnessParameterName[] = "lengthRandomness";
		char const LengthCurveParameterName[] = "lengthCurve";
		char const DistributionCurveParameterName[] = "distributionCurve";
		//const char DistributionChannelParameterName[] = "distributionChannel";
		char const TargetStrandGroupParameterName[] = "resultStrandGroup";
		char const OperatorIdParameterName[] = "operatorId";
		char const UniformRandomnessParameterName[] = "uniformRandomness";
		char const AddRootPositionChannelParameterName[] = "addRootPositionChannel";

		auto const LowRangeDefaultValue = 0.0f;
		auto const HighRangeDefaultValue = 1.0f;
		auto const TwistValueDefaultValue = 0.0f;
		auto const TwistRandomnessDefaultValue = 0.1f;
		auto const FanningDefaultValue = 0.0f;
		auto const FanningRandomnessDefaultValue = 0.0f;
		auto const LengthDefaultValue = 10.0f;
		auto const LengthRandomnessDefaultValue = 0.2f;
		auto const UniformRandomnessDefaultValue = 0.0f;

		char const PerStrandSideIndicesParameterName[] = "perStrandSideIndices";
		//const char StrandSideTwistCurveKnotCountsParameterName[] = "strandSideTwistCurveKnotCounts";
		//const char StrandSideTwistCurvesPositionsCountsParameterName[] = "strandSideTwistCurvesPositionsCounts";
		//const char StrandSideTwistCurvesValuesParameterName[] = "strandSideTwistCurvesValues";
		//const char StrandSideTwistCurvesInterpolationsParameterName[] = "strandSideTwistCurvesInterpolations";
		//const char StrandSideFanningCurveKnotCountsParameterName[] = "strandSideFanningCurveKnotCounts";
		//const char StrandSideFanningCurvesPositionsParameterName[] = "strandSideFanningCurvesPositions";
		//const char StrandSideFanningCurvesValuesParameterName[] = "strandSideFanningCurvesValues";
		//const char StrandSideFanningCurvesInterpolationsParameterName[] = "strandSideFanningCurvesInterpolations";
		//const char StrandSideLengthCurveKnotCountsParameterName[] = "strandSideLengthCurveKnotCounts";
		//const char StrandSideLengthCurvesPositionsParameterName[] = "strandSideLengthCurvesPositions";
		//const char StrandSideLengthCurvesValuesParameterName[] = "strandSideLengthCurvesValues";
		//const char StrandSideLengthCurvesInterpolationsParameterName[] = "strandSideLengthCurvesInterpolations";
		//const char StrandSideDistributionCurveKnotCountsParameterName[] = "strandSideDistributionCurveKnotCounts";
		//const char StrandSideDistributionCurvesPositionsParameterName[] = "strandSideDistributionCurvesPositions";
		//const char StrandSideDistributionCurvesValuesParameterName[] = "strandSideDistributionCurvesValues";
		//const char StrandSideDistributionCurvesInterpolationsParameterName[] = "strandSideDistributionCurvesInterpolations";
		char const PerStrandSideTwistsParameterName[] = "perStrandSideTwists";
		char const PerStrandSideTwistsRandomnessParameterName[] = "perStrandSideTwistsRandomness";
		char const PerStrandSideFanningsParameterName[] = "perStrandSideFannings";
		char const PerStrandSideFanningsRandomnessParameterName[] = "perStrandSideFanningsRandomness";
		char const PerStrandSideLengthsParameterName[] = "perStrandSideLengths";
		char const PerStrandSideLengthsRandomnessParameterName[] = "perStrandSideLengthsRandomness";
		char const TextureMappingMethodParameterName[] = "textureMappingMethod";

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Propagation,
				{
					{ RootGenerationMethodParameterName, 1,
						{
							L"Uniform",
							L"Random",
							L"Vertex",
						}
					},
					{ RootCountParameterName, 50 },
					{ VerticesPerRootCountParameterName, 4 },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ CountPerStrandParameterName, true },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ LowRangeParameterName, LowRangeDefaultValue },
					{ HighRangeParameterName, HighRangeDefaultValue },
					{ IsLengthRelativeToBaseStrandParameterName, false },
					{ IsFacingSurfaceParameterName, true },
					{ SideCountParameterName, 2 },
					{ TwistValueParameterName, TwistValueDefaultValue },
					{ TwistRandomnessParameterName, TwistRandomnessDefaultValue },
					{ TwistCurveParameterName, ParameterType::FloatArray },
					{ FanningParameterName, FanningDefaultValue },
					{ FanningRandomnessParameterName, FanningRandomnessDefaultValue },
					{ FanningCurveParameterName, ParameterType::FloatArray },
					{ LengthParameterName, LengthDefaultValue },
					{ LengthRandomnessParameterName, LengthRandomnessDefaultValue },
					{ LengthCurveParameterName, ParameterType::FloatArray },
					{ DistributionCurveParameterName, ParameterType::FloatArray },
					{ TargetStrandGroupParameterName, 1 },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ OperatorIdParameterName, 0 },

					{ PerStrandSideIndicesParameterName, ParameterType::IntegerArray },
					{ PerStrandSideTwistsParameterName, ParameterType::FloatArray },
					{ PerStrandSideTwistsRandomnessParameterName, ParameterType::FloatArray },
					{ PerStrandSideFanningsParameterName, ParameterType::FloatArray },
					{ PerStrandSideFanningsRandomnessParameterName, ParameterType::FloatArray },
					{ PerStrandSideLengthsParameterName, ParameterType::FloatArray },
					{ PerStrandSideLengthsRandomnessParameterName, ParameterType::FloatArray },
					//{ DistributionChannelParameterName, 0 },
					{ TextureMappingMethodParameterName, 0,
						{
							L"None",
							L"InheritFromBaseStrand",
							L"FlatMap",
						}
					},
					{ UniformRandomnessParameterName, UniformRandomnessDefaultValue },
					{ AddRootPositionChannelParameterName, false }
				} ),
			Propagator::GetOperatorDefinition() );
	}

	SECTION( "PushAwayFromSurface" )
	{
		auto const OperatorName = "PushAwayFromSurface";
		auto const DistanceParameterName = "distance";

		auto const DistanceDefaultValue = 0.1f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::PushAwayFromSurface,
				{
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ DistanceParameterName, DistanceDefaultValue, ParameterUnits::Centimeters },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
				} ),
				PushAwayFromSurface::GetOperatorDefinition() );
	}

	SECTION( "ResolveCollisions" )
	{
		auto const OperatorName = "ResolveCollisions";

		auto const ResolveModeParameterName = "resolveMode";
		auto const AngleStepParameterName = "angleStep";
		auto const AngleMaxParameterName = "angleMax";
		auto const AngleMaxBackwardParameterName = "angleMaxBackward";
		auto const AngleSegmentMaxParameterName = "angleSegmentMax";
		auto const PlasticityParameterName = "plasticity";
		auto const RecoveryRateParameterName = "recoveryRate";
		auto const AffectedSegmentCountParameterName = "affectedSegmentCount";
		auto const AffectedFalloffParameterName = "affectedFalloff";
		auto const ScaleMinimumParameterName = "scaleMinimum";
		auto const ScaleAngleParameterName = "scaleAngle";
		auto const PreserveRotationsParameterName = "preserveRotations";

		auto const ResolveModeDefaultValue = ResolveCollisionsParameters::ResolveModeType::Stiff;
		auto const AngleStepDefaultValue = DegreesToRadians( 0.25f );
		auto const AngleMaxDefaultValue = DegreesToRadians( 180.0f );
		auto const AngleMaxBackwardDefaultValue = DegreesToRadians( 15.0f );
		auto const AngleSegmentMaxDefaultValue = DegreesToRadians( 20.0f );
		auto const PlasticityDefaultValue = 0.0f;
		auto const RecoveryRateDefaultValue = 0.02f;
		auto const AffectedSegmentCountDefaultValue = 4;
		auto const AffectedFalloffDefaultValue = 0.75f;
		auto const ScaleMinimumDefaultValue = 1.0f;
		auto const ScaleAngleDefaultValue = DegreesToRadians( 90.0f );
		auto const PreserveRotationsDefaultValue = true;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::ResolveCollisions,
				{
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ ResolveModeParameterName, static_cast<int>( ResolveModeDefaultValue ),
						{
							L"Stiff",
							L"Bend",
							L"Soft",
						}
					},
					{ AngleStepParameterName, AngleStepDefaultValue, ParameterUnits::Radians },
					{ AngleMaxParameterName, AngleMaxDefaultValue, ParameterUnits::Radians },
					{ AngleMaxBackwardParameterName, AngleMaxBackwardDefaultValue, ParameterUnits::Radians },
					{ AngleSegmentMaxParameterName, AngleSegmentMaxDefaultValue, ParameterUnits::Radians },
					{ PlasticityParameterName, PlasticityDefaultValue, ParameterUnits::Fraction },
					{ RecoveryRateParameterName, RecoveryRateDefaultValue, ParameterUnits::Fraction },
					{ AffectedSegmentCountParameterName, AffectedSegmentCountDefaultValue, ParameterUnits::Generic },
					{ AffectedFalloffParameterName, AffectedFalloffDefaultValue, ParameterUnits::Fraction },
					{ ScaleMinimumParameterName, ScaleMinimumDefaultValue, ParameterUnits::Fraction },
					{ ScaleAngleParameterName, ScaleAngleDefaultValue, ParameterUnits::Radians },
					{ PreserveRotationsParameterName, PreserveRotationsDefaultValue, ParameterUnits::Generic },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
				} ),
			ResolveCollisions::GetOperatorDefinition() );
	}

	SECTION( "Rotate" )
	{
		auto const OperatorName = "RotateStrands";
		auto const OrientBasedOnStrandShapeParameterName = "orientBasedOnStrandShape";
		auto const GlobalAngleParameterName = "globalAngle";
		auto const GlobalChaosParameterName = "globalChaos";
		auto const FaceCoordinateParameterName = "faceCoordinate";
		auto const UseLastPointIndexForStrandDirectionVectorParameterName = "useLastPointForStrandVector";
		auto const PointIndexForStrandDirectionVectorParameterName = "pointIndexForStrandVector";
		char const TwistParameterName[] = "twist";

		auto const DefaultGlobalAngleValue = 0.0f;
		auto const DefaultGlobalChaosValue = 0.0f;
		auto const TwistDefaultValue = 0.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::RotateStrands,
				{
					{ GlobalAngleParameterName, DefaultGlobalAngleValue, ParameterUnits::Radians },
					{ GlobalChaosParameterName, DefaultGlobalChaosValue, ParameterUnits::Fraction },
					{ FaceCoordinateParameterName, false },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ OrientBasedOnStrandShapeParameterName, false },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ UseLastPointIndexForStrandDirectionVectorParameterName, true },
					{ PointIndexForStrandDirectionVectorParameterName, 3 },
					{ TwistParameterName, TwistDefaultValue, ParameterUnits::Radians }
				} ),
			Rotate::GetOperatorDefinition() );
	}

	SECTION( "SurfaceComber" )
	{
		char const OperatorName[] = "SurfaceComb";
		char const AffectWholeStrandParameterName[] = "affectWholeStrand";
		char const ChaosParameterName[] = "chaos";
		char const SlopeCurveParameterName[] = "slopeCurve";
		char const ApplyToShapeParameterName[] = "applyToShape";
		char const ApplyToDirectionParameterName[] = "applyToDirection";
		char const SinkOrientationTypeParameterName[] = "sinkOrientationType";
		char const IntegerSinkDataParameterName[] = "integerSinkData";
		char const Vector3SinkDataParameterName[] = "vector3SinkData";
		char const MinimumBendValueParameterName[] = "minimumBendValue";
		char const MaximumBendValueParameterName[] = "maximumBendValue";
		char const UseVectorOrientationMapParameterName[] = "useVectorOrientationMap";

		auto const DefaultChaosValue = 0.1f;
		auto const MinimumBendValueDefaultValue = 0.0f;
		auto const MaximumBendValueDefaultValue = 0.5f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::SurfaceComb,
				{
					{ AffectWholeStrandParameterName, false },
					{ ChaosParameterName, DefaultChaosValue },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue },
					{ SlopeCurveParameterName, ParameterType::FloatArray },
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ ApplyToShapeParameterName, true },
					{ ApplyToDirectionParameterName, true },
					{ SinkOrientationTypeParameterName, 1, { L"Absolute", L"Relative" } },
					{ IntegerSinkDataParameterName, ParameterType::IntegerArray },
					{ Vector3SinkDataParameterName, ParameterType::Vector3Array },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ MinimumBendValueParameterName, MinimumBendValueDefaultValue },
					{ MaximumBendValueParameterName, MaximumBendValueDefaultValue },
					{ UseVectorOrientationMapParameterName, false },
				} ),
				SurfaceComber::GetOperatorDefinition() );
	}

	SECTION( "Symmetry" )
	{
		auto const OperatorName = "Symmetry";
		auto const UseIgnoreDistanceParameterName = "useIgnoreDistance";
		auto const IgnoreDistanceParameterName = "ignoreDistance";

		auto const IgnoreDistanceDefaultValue = 1.0f;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::Symmetry,
				{
					{ UseIgnoreDistanceParameterName, false },
					{ IgnoreDistanceParameterName, IgnoreDistanceDefaultValue, ParameterUnits::Centimeters },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue }
				} ),
			Symmetry::GetOperatorDefinition() );
	}

	SECTION( "Weaver" )
	{
		auto CurveSampleCountParameterName = "curveSampleCount";
		auto UvSpaceScaleParameterName = "uvSpaceScale";
		auto UsePatternSmoothingParameterName = "usePatternSmoothing";
		auto PutIntoTargetSpaceParameterName = "putIntoTargetSpace";
		auto PerFaceDataFaceIndicesParameterName = "perFaceDataFaceIndices";
		auto PerFaceDataBoundaryRotationsParameterName = "perFaceDataBoundaryRotations";
		auto PerFaceDataPatternIndicesOffsetsParameterName = "perFaceDataPatternIndicesOffsets";
		auto PerFaceDataPatternIndicesParameterName = "perFaceDataPatternIndices";
		auto PerEdgeDataVertexIndicesParameterName = "perEdgeDataVertexIndices";
		auto PerEdgeDataDivisionMethodsParameterName = "perEdgeDataDivisionMethods";
		auto PerEdgeDataDivisionLengthsParameterName = "perEdgeDataDivisionLengths";
		auto PerEdgeDataDivisionCountsParameterName = "perEdgeDataDivisionCounts";
		auto ConnectNearbyBoundaryEdgesParameterName = "connectNearbyBoundaryEdges";
		auto BoundaryEdgesSampleCountParameterName = "boundaryEdgesSampleCount";
		auto BoundaryEdgesDistanceThresholdParameterName = "boundaryEdgesDistanceThreshold";

		Match(
			OperatorDefinition(
				"Weaver", OperatorType::Weaver,
				{
					{ CurveSampleCountParameterName, 50 },
					{ UvSpaceScaleParameterName, 1.0f },
					{ PutIntoTargetSpaceParameterName, true },
					{ UsePatternSmoothingParameterName, true },
					{ ConnectNearbyBoundaryEdgesParameterName, true },
					{ BoundaryEdgesSampleCountParameterName, 10 },
					{ BoundaryEdgesDistanceThresholdParameterName, 1.0f, ParameterUnits::Centimeters },
					{ PerFaceDataFaceIndicesParameterName, ParameterType::IntegerArray },
					{ PerFaceDataBoundaryRotationsParameterName, ParameterType::IntegerArray },
					{ PerFaceDataPatternIndicesOffsetsParameterName, ParameterType::IntegerArray },
					{ PerFaceDataPatternIndicesParameterName, ParameterType::IntegerArray },
					{ PerEdgeDataVertexIndicesParameterName, ParameterType::IntegerArray },
					{ PerEdgeDataDivisionMethodsParameterName, ParameterType::IntegerArray },
					{ PerEdgeDataDivisionLengthsParameterName, ParameterType::FloatArray },
					{ PerEdgeDataDivisionCountsParameterName, ParameterType::IntegerArray },
				} ),
				Weaver::GetOperatorDefinition() );
	}

	SECTION( "ChangeWidth" )
	{
		char const OperatorName[] = "ChangeWidth";
		char const WidthParameterName[] = "width";
		char const WidthCurveParameterName[] = "widthCurve";
		char const RandomizeParameterName[] = "randomize";
		char const UseAbsoluteLengthParameterName[] = "useAbsoluteLength";
		char const UseLongestStrandLengthParameterName[] = "useLongestStrandLength";
		char const AbsoluteLengthParameterName[] = "absoluteLength";

		float const DefaultWidthValue = 1;
		float const RandomizeDefaultValue = 0;
		float const AbsoluteLengthDefaultValue = 50;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::ChangeWidth,
				{
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ WidthParameterName, DefaultWidthValue, ParameterUnits::Centimeters },
					{ WidthCurveParameterName, ParameterType::FloatArray },
					{ RandomizeParameterName, RandomizeDefaultValue, ParameterUnits::Fraction },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ UseAbsoluteLengthParameterName, false },
					{ UseLongestStrandLengthParameterName, false },
					{ AbsoluteLengthParameterName, AbsoluteLengthDefaultValue, ParameterUnits::Centimeters },
					{ Constants::GroupBlendDistanceParameterName, Constants::GroupBlendDistanceDefaultValue }
				} ),
			ChangeWidth::GetOperatorDefinition() );
	}

	SECTION( "GuidesEditor" )
	{
		char const OperatorName[] = "EditGuides";
		//const char DeltaParameterName[] = "delta";
		char const ApplyAmountParameterName[] = "applyAmount";
		char const ApplyRampParameterName[] = "applyRamp";

		Real const ApplyAmountDefaultValue = 1;

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::EditGuides,
				{
					//{ DeltaParameterName, ParameterType::FloatArray },
					{ ApplyAmountParameterName, ApplyAmountDefaultValue, ParameterUnits::Fraction },
					{ ApplyRampParameterName, ParameterType::FloatArray },
				} ), GuidesEditor::GetOperatorDefinition() );
	}

	SECTION( "GuidesFromMeshGenerator" )
	{
		auto const OperatorName = "GuidesFromMesh";
		auto const DistributionParameterName = "distribution";
		auto const CountParameterName = "count";
		auto const LengthParameterName = "length";
		auto const LengthRandomnessParameterName = "lengthRandomness";
		auto const PointCountParameterName = "pointCount";
		auto const TopologyBasedOrientationParameterName = "topologyBasedOrientation";
		auto const DetachRootsParameterName = "detachRoots";
		auto const DetachedRootPositionsParameterName = "detachedRootPositions";
		auto const DetachedRootDirectionsParameterName = "detachedRootDirections";
		auto const RememberRootsParameterName = "rememberRoots";

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::GuidesFromMesh,
				{
					{ DistributionParameterName, 7,
						{
							L"Uniform",
							L"RandomUV",
							L"RandomArea",
							L"RandomFace",
							L"Vertex",
							L"Unused",
							L"FaceCenter",
							L"Even",
						}
					},
					{ CountParameterName, RootGeneratorParameters::RootCount::DefaultValue() },
					{ LengthParameterName, GuidesFromMeshParameters::GuideLength::DefaultValue(), ParameterUnits::Centimeters },
					{ LengthRandomnessParameterName, GuidesFromMeshParameters::LengthRandomness::DefaultValue(), ParameterUnits::Fraction },
					{ PointCountParameterName, GuidesFromMeshParameters::PointCount::DefaultValue(), ParameterUnits::Granularity },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue, ParameterUnits::RandomSeed },
					{ TopologyBasedOrientationParameterName, true },
					{ DetachRootsParameterName, false },
					{ DetachedRootPositionsParameterName, ParameterType::Vector3Array },
					{ DetachedRootDirectionsParameterName, ParameterType::Vector3Array },
					{ RememberRootsParameterName, false },
				} ),
			GuidesFromMeshGenerator::GetOperatorDefinition() );
	}

	SECTION( "HairFromGuidesGenerator" )
	{
		char const OperatorName[] = "HairFromGuides";
		char const DistributionParameterName[] = "distribution";
		char const ViewCountParameterName[] = "viewCount";
		char const RenderCountParameterName[] = "renderCount";
		char const GuideAreaTypeParameterName[] = "guideArea";
		char const GuideAreaBarycentricTriangulationTypeParameterName[] = "guideAreaBarycentricTriangulationType";
		char const GuideAreaCircleThroughNearestParameterName[] = "guideAreaCircleThroughNearest";
		char const GuideAreaCircleRadiusParameterName[] = "guideAreaCircleRadius";
		char const InterpolationTypeParameterName[] = "interpolation2";
		char const OldInterpolationTypeParameterName[] = "interpolation";
		char const OldInterpolationGuideCountParameterName[] = "interpolationGuideCount";
		char const TopologyBasedOrientationParameterName[] = "topologyBasedOrientation";
		char const UseGuideProximityParameterName[] = "useGuideProximity";
		char const AutoGuideProximityParameterName[] = "autoGuideProximity";
		char const GuideProximityDistanceParameterName[] = "guideProximityDistance";
		char const UseGuideFacesParameterName[] = "useGuideFaces";
		char const GuideFacesIterationCountParameterName[] = "guideFacesIterationCount";
		char const InterpolateGuidesInUvSpaceParameterName[] = "interpolateGuidesInUvSpace";
		char const GeneratePerStrandDataParameterName[] = "generatePerStrandData";
		char const GeneratePerVertexDataParameterName[] = "generatePerVertexData";
		char const DisplacementMapMinimumValueParameterName[] = "displacementMapMinimumValue";
		char const TextureGenerationMethodParameterName[] = "textureGenerationMethod";
		char const GenerateRotationsParameterName[] = "generateRotations";
		char const UseStrandGroupsAsPartsParameterName[] = "useStrandGroupsAsParts";

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::HairFromGuides,
				{
					{ DistributionParameterName, 7,
						{
							L"Uniform",
							L"RandomUV",
							L"RandomArea",
							L"RandomFace",
							L"Vertex",
							L"CopyInput",
							L"FaceCenter",
							L"Even"
						}
					},
					{ ViewCountParameterName, 1000 },
					{ RenderCountParameterName, 10000 },
					{ GuideAreaTypeParameterName, static_cast<int>( HairFromGuidesParameters::GuideAreaCalculation::Method::DefaultValue() ),
						{
							L"NClosestGuides",
							L"Barycentric",
							L"Circle",
						}
					},
					{ GuideAreaBarycentricTriangulationTypeParameterName, static_cast<int>( HairFromGuidesParameters::GuideAreaCalculation::BarycentricTriangulationMethod::DefaultValue() ),
						{
							L"DistributionMesh",
							L"NearestNeighbourTriangulation",
							L"ManualTriangulation",
						}
					},
					{ GuideAreaCircleThroughNearestParameterName, HairFromGuidesParameters::GuideAreaCalculation::Circle::ThroughNearest::DefaultValue() },
					{ GuideAreaCircleRadiusParameterName, HairFromGuidesParameters::GuideAreaCalculation::Circle::Radius::DefaultValue() },
					{ InterpolationTypeParameterName, static_cast<int>( HairFromGuidesParameters::InterpolationMethod::DefaultValue() ),
						{
							L"Polar",
							L"Affine",
							L"Segment",
						}
					},
					{ OldInterpolationTypeParameterName, 4,
						{
							L"Polar",
							L"Affine",
							L"Barycentric",
							L"Segment",
							L"Unused"
						}
					},
					{ OldInterpolationGuideCountParameterName, 3 },
					{ TopologyBasedOrientationParameterName, true },
					{ UseGuideProximityParameterName, HairFromGuidesParameters::UseGuideProximity::DefaultValue() },
					{ GuideProximityDistanceParameterName, HairFromGuidesParameters::GuideProximityDistance::DefaultValue() },
					{ UseGuideFacesParameterName, HairFromGuidesParameters::UseGuideFaces::DefaultValue() },
					{ GuideFacesIterationCountParameterName, HairFromGuidesParameters::GuideFaceIterationCount::DefaultValue() },
					{ InterpolateGuidesInUvSpaceParameterName, false },
					{ GeneratePerStrandDataParameterName, HairFromGuidesParameters::GeneratePerStrandData::DefaultValue() },
					{ GeneratePerVertexDataParameterName, HairFromGuidesParameters::GeneratePerVertexData::DefaultValue() },
					{ DisplacementMapMinimumValueParameterName, HairFromGuidesParameters::DisplacementMapMinimumValue::DefaultValue() },
					{ TextureGenerationMethodParameterName, static_cast<int>( HairFromGuidesParameters::TextureGenerationMethod::DefaultValue() ),
						{
							L"None",
							L"InheritFromBase",
							L"FlatMap",
						}
					},
					{ GenerateRotationsParameterName, HairFromGuidesParameters::GenerateRotations::DefaultValue() },
					{ AutoGuideProximityParameterName, HairFromGuidesParameters::AutoGuideProximity::DefaultValue() },
					{ UseStrandGroupsAsPartsParameterName, HairFromGuidesParameters::Parting::UseStrandGroupsAsParts::DefaultValue() }
				} ),
			HairFromGuidesGenerator::GetOperatorDefinition() );
	}

	SECTION( "MeshFromHairGenerator" )
	{
		auto const OperatorName = "MeshFromStrands";
		auto const MeshTypeParameterName = "meshType";
		char const ConformToSurfaceParameterName[] = "conformToSurface";

		Match(
			OperatorDefinition(
				OperatorName, OperatorType::MeshFromStrands,
				{
					{ MeshTypeParameterName, 1,
						{
							L"Cylindrical",
							L"Ribbon",
							L"ProxyMesh",
						}
					},
					{ Constants::StrandGroupParameterName, Constants::StrandGroupDefaultValue },
					{ Constants::RandomSeedParameterName, Constants::RandomSeedDefaultValue },
					{ Constants::StrandGroupPatternParameterName, Constants::StrandGroupPatternDefaultValue },
					{ ConformToSurfaceParameterName, 0,
						{
							L"IgnoreSurface",
							L"SurfaceNormal",
							//L"SurfaceIntersection",
						}
					},
				} ),
			MeshFromHairGenerator::GetOperatorDefinition() );
	}
}
#endif