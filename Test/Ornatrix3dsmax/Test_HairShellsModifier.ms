struct Test_HairShellsModifier
(
	-- Tests that sink weights have an effect on resulting strand orientation
	function Test_AddRemoveShells =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		distributionObject = Teapot mapcoords:on radius:17.7927
		
		shellObjectSphere = Sphere radius:20.0 mapcoords:on pos:[50.0,50.0,0]
		shellObjectBox = Box lengthsegs:1 widthsegs:1 heightsegs:1 length:40.0 width:40.0 height:40.0 pos:[-50.0,-50.0,0] mapcoords:on

		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		guidesFromSurfaceInstance = Ox_Guides_from_Surface ()
		modPanel.addModToSelection guidesFromSurfaceInstance

		hairShellsInstance = Ox_Hair_Shells()
		modPanel.addModToSelection hairShellsInstance
		
		--set
		hairShellsInstance.AddShell shellObjectBox
		hairShellsInstance.AddShell shellObjectBox
		hairShellsInstance.AddShell shellObjectSphere
		hairShellsInstance.AddShell shellObjectSphere
		
		EAssert.AreEqual 2 ( hairShellsInstance.shells.count )
		
		hairShellsInstance.RemoveShell 0
		EAssert.AreEqual 1 ( hairShellsInstance.shells.count )
		EAssert.IsTrue( hairShellsInstance.shells[1] == shellObjectSphere )
		
		hairShellsInstance.curPenetr = off
		EAssert.IsFalse( hairShellsInstance.curPenetr )
		
		hairShellsInstance.curPenetr = on
		EAssert.IsTrue( hairShellsInstance.curPenetr )
		
		--get
		--hairShellsInstance.RemoveShell 0
		--hairShellsInstance.curPenetr = on		
		
		--- Cleanup
		delete distributionObject
		delete shellObjectSphere
		delete shellObjectBox
	),
	
	function Test_ShellWithAnimatedTransform =
	(
		local shell1 = Plane width:1 length:1 lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		local shell2 = copy shell1
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairShellsInstance = Ox_Hair_Shells()
		modPanel.addModToSelection hairShellsInstance
		hairShellsInstance.AddShell shell2
		
		-- Animate the shell moving
		set animate on
		sliderTime = 0f
		move shell2 [0,0,1]
		sliderTime = 1f
		move shell2 [1,0,1]
		set animate off
			
		sliderTime = 0f
		local pointPositionsAtFrame0 = TestUtilities.GetPointPositions shell1
		sliderTime = 1f
		local pointPositionsAtFrame1 = TestUtilities.GetPointPositions shell1
		
		-- Points should be different since the plane is animated
		EAssert.ArePointsNotAllNearEqual pointPositionsAtFrame0 pointPositionsAtFrame1
		
		-- Cleanup
		delete $objects
	)
)

EUnit.AddTestClass Test_HairShellsModifier
ok
