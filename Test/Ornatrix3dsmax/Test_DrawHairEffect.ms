struct Test_DrawHairEffect
(
	function CreateAndRenderEffect =
	(
		-- Create some hair
		hairObject = Plane lengthsegs:2 widthsegs:2
		max modify mode
		select hairObject
		guidesFromSurfaceInstance = Ox_Guides_from_Surface()
		guidesFromSurfaceInstance.count = 4
		modPanel.addModToSelection guidesFromSurfaceInstance

		-- Add "Ox Hair Draw" effect to the scene
		effect1 = Ox_Hair_Draw()
		addEffect effect1
		effect1.AddHair hairObject

		render()

		deleteEffect 1
		delete hairObject
	),

	function Test_ReRenderDrawHair =
	(
		CreateAndRenderEffect()

		resetMaxFile #noPrompt

		CreateAndRenderEffect()
	)
)

-- Doesn't work in 3dsMax 2021
--EUnit.AddTestClass Test_DrawHairEffect
ok
