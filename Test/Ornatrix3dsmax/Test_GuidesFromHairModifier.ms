struct Test_GuidesFromHairModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_GuidesFromHair = 
	(
		-- Create a sphere
		local distributionObject = Sphere radius:5 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add hair from guides 
		local hairFromGuidesModifierInstance1 = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifierInstance1

		-- Add guides from hair 
		local guidesFromHairModifierInstance1 = Ox_Guides_from_Hair()
		modPanel.addModToSelection guidesFromHairModifierInstance1

		-- Add hair from guides 
		local hairFromGuidesModifierInstance2 = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifierInstance2
		
		-- Do a render which will evaluate render hairs
		render()
	)
)

EUnit.AddTestClass Test_GuidesFromHairModifier
ok
