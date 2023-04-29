struct Test_HairFromParticlesObject
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that hair from mesh strips can be used with another Ornatrix modifier and the hair goes through the pipeline
	function Test_HairFromParticlesWithModifier =
	(
		local particleObject = Snow()

		local hairObjectInstance = HairFromParticlesObject()
			
		-- Add frizz modifier on top and ensure that all the hairs make it out frizzed
		select hairObjectInstance
		max modify mode
		modPanel.addModToSelection (Ox_Strand_Frizz())

		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
	),
	
	function Test_AddRemoveParticles =
	(
		local particleObject1 = Snow width:5 pos:[20, 0, 0]
		local particleObject2 = Snow width:5 pos:[0, 0, 0]
		local particleObject3 = Snow width:5 pos:[-20, 0, 0]

		local hairObjectInstance = HairFromParticlesObject hairsCount:10 pointsPerStrand:10
		
		-- No hairs by default
		local hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 0 (myHair.NumVerts())
		delete hairCopy
			
		hairObjectInstance.AddStripObject particleObject1

		CompleteRedraw()

		-- Hairs on first strip
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 10 (myHair.NumRoots())
		EAssert.AreEqual 100 (myHair.NumVerts())
		delete hairCopy
			
		hairObjectInstance.AddStripObject particleObject2
		hairObjectInstance.AddStripObject particleObject3

		CompleteRedraw()

		-- More hairs as strips are added
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		--EAssert.AreEqual 30 (myHair.NumRoots())
		EAssert.AreEqual 300 (myHair.NumVerts())
		delete hairCopy
			
		
	),
	
	function Test_ChangeHairsPerStripCount =
	(
		stripObject1 = Snow width:10 pos:[20, 0, 0]

		hairObjectInstance = HairFromParticlesObject()
		hairObjectInstance.hairsCount = 10
		hairObjectInstance.pointsPerStrand = 10
		hairObjectInstance.AddStripObject stripObject1
		
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 10 (myHair.NumRoots())
		delete hairCopy
			
		hairObjectInstance.hairsCount = 7
			
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 7 (myHair.NumRoots())
		delete hairCopy
	),

	-- Checks for a bug which made Max crash when point count is changed after a modifier is applied when Nitrous hair is used
	function Test_ChangePointCountAfterApplyingAModifier =
	(
		local stripObject = Snow()	
		local hairObjectInstance = HairFromParticlesObject isSelected:on hairsCount:10 pointsPerStrand:10
		max modify mode
		hairObjectInstance.AddStripObject stripObject
		completeRedraw()
		
		select hairObjectInstance
		local renderSettingsModifierInstance = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifierInstance
		modPanel.setCurrentObject hairObjectInstance
		hairObjectInstance.pointsPerStrand = 60
		completeRedraw()
	)/*,
	
	function Test_UndoRedo =
	(
		-- Dummy planes
		local snowL = Snow length:10 width:5 pos:[20, 0, 0]
		local snowC = Snow length:10 width:5 pos:[0, 0, 0]
		local snowR = Snow length:10 width:5 pos:[-20, 0, 0]
	
		local hairObjectInstance = HairFromParticlesObject distributionMethod:1
		hairObjectInstance.AddStripObject snowL
		hairObjectInstance.AddStripObject snowC
		hairObjectInstance.AddStripObject snowR

		hairObjectInstance.HairsPerStrip = 2
		hairObjectInstance.PointsPerStrand = 5

		select hairObjectInstance

		-- tosses out the restore objects since the last Begin() but still continues holding
		theHold.Accept "setup"
		theHold.begin()
		
		local vertices0 = OxGetVertices hairObjectInstance true
		
		hairObjectInstance.HairsCount = 10
		
		hairObjectInstance.HairsCount = 20
		
		local vertices1 = OxGetVertices hairObjectInstance true
		theHold.Accept "test"

		max undo		
		
		local vertices2 = OxGetVertices hairObjectInstance true
		
		max redo		
		
		local vertices3 = OxGetVertices hairObjectInstance true

		EAssert.AreNotEqual ( vertices0.count ) ( vertices1.count )
		EAssert.AreEqual ( vertices0.count ) ( vertices2.count )
		EAssert.AreEqual ( vertices1.count ) ( vertices3.count )

		for i = 1 to vertices0.count do
		(
			EAssert.ArePointsNearEqual ( vertices0[i] ) ( vertices2[i] )
		)

		for i = 1 to vertices1.count do
		(
			EAssert.ArePointsNearEqual ( vertices1[i] ) ( vertices3[i] )
		)
		
		max undo
		max undo
		max undo
	)*/
)

-- Freezes the build agent
EUnit.AddTestClass Test_HairFromParticlesObject
ok
