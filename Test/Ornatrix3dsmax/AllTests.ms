fileIn "../../UnitTest.ms"
fileIn "../../TestUtilities.ms"
fileIn "OrnatrixUtils.ms"
EUnit.CollectAllTests root:(getFilenamePath( getSourceFileName() ))
EUnit_GetAssertCount = OxGetAssertCount
EUnit_GetLatestAssertMessage = OxGetLatestAssertMessage
EUnit_AdditionalLogger = OxLog

fileIn "PerformanceTest.ms"

-- Run the whole file with Ctrl-E to run all unit test files in this folder
if true then
(
	EUnit.RunAllTests()
)
-- or select and run with Shift-Enter the first two lines, then
else
(
	-- select and run with Shift-Enter individual classes below
	EUnit.RunTestClass Test_AdoptExternalGuidesModifier
	EUnit.RunTestClass Test_AlembicExporter
	EUnit.RunTestClass Test_AnimationModifier
	EUnit.RunTestClass Test_BakedGuidesObject
	EUnit.RunTestClass Test_BakedHairObject
	EUnit.RunTestClass Test_BraidGuidesObject
	EUnit.RunTestClass Test_ClumpModifier
	EUnit.RunTestClass Test_CurlingModifier
	EUnit.RunTestClass Test_DetailModifier
	EUnit.RunTestClass Test_DrawHairEffect
	EUnit.RunTestClass Test_DynamicsModifier
	EUnit.RunTestClass Test_EditGuidesModifier
	EUnit.RunTestClass Test_FilterModifier
	EUnit.RunTestClass Test_FrizzModifier
	EUnit.RunTestClass Test_GenerateGuideDataModifier
	EUnit.RunTestClass Test_GravityModifier
	EUnit.RunTestClass Test_Grooms
	EUnit.RunTestClass Test_GroomsSavingLoading
	EUnit.RunTestClass Test_GroundStrandsModifier
	EUnit.RunTestClass Test_GuideClusteringModifier
	EUnit.RunTestClass Test_GuidesFromGuideMeshModifier
	EUnit.RunTestClass Test_GuidesFromHairModifier
	EUnit.RunTestClass Test_GuidesFromMeshModifier
	EUnit.RunTestClass Test_GuidesFromMeshObject
	EUnit.RunTestClass Test_GuidesFromShapeModifier
	EUnit.RunTestClass Test_GuidesFromShapeObject
	EUnit.RunTestClass Test_GuidesOnSplineModifier
	EUnit.RunTestClass Test_HairClusteringModifier
	EUnit.RunTestClass Test_HairFromGuidesModifier
	EUnit.RunTestClass Test_HairFromMeshStripsObject
	EUnit.RunTestClass Test_HairFromParticlesObject
	EUnit.RunTestClass Test_HairShellsModifier
	EUnit.RunTestClass Test_LengthModifier
	EUnit.RunTestClass Test_MaterialShaders
	EUnit.RunTestClass Test_MeshFromStrandsModifier
	EUnit.RunTestClass Test_Modifiers
	EUnit.RunTestClass Test_MoovPhysicsModifier
	EUnit.RunTestClass Test_MultiplierModifier
	EUnit.RunTestClass Test_PropagationModifier
	EUnit.RunTestClass Test_PushAwayFromSurfaceModifier
	EUnit.RunTestClass Test_RenderSettingsModifier
	EUnit.RunTestClass Test_ResolveCollisionsModifier
	EUnit.RunTestClass Test_RotateModifier
	EUnit.RunTestClass Test_StrandLengthModifier
	EUnit.RunTestClass Test_SurfaceCombModifier
	EUnit.RunTestClass Test_SurfaceLocking
	EUnit.RunTestClass Test_SymmetryModifier
	EUnit.RunTestClass Test_WeaverObject

	-- or change the if condition to false and comment out unimportant test classes above,
	-- then run the rest with Ctrl-E
)

-- or copy the following line to the listener and run it to execute individual tests
-- EUnit.RunTestMethod Test_Class #Test_MethodName

-- or use this line to keep the results of the test
-- EUnit.RunTestMethod Test_Class #Test_MethodName doCleanup:#never