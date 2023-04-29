struct Test_PropagationModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that strand propagation can be added error-free in the modifier panel. Mainly to do with loading UI diagrams error-free
	function Test_AddStrandPropagationModifier =
	(
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:2 widthsegs:2

		max modify mode

		-- Add strand propagation
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier

		local strandPropagationModifierInstance = Ox_Strand_Propagation()
		modPanel.addModToSelection strandPropagationModifierInstance

		completeRedraw()
		-- If everything is good up to this point we're done
	),
	
	-- Tests that we can drag the strand propagation modifier around in the modifier stack without problems
	function Test_MoveStrandPropagationInModifierStack =
	(
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on

		max modify mode

		-- Add strand propagation
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface method:4
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local strandPropagationModifierInstance = Ox_Strand_Propagation()
		modPanel.addModToSelection strandPropagationModifierInstance
		
		addModifier $ (Ox_Strand_Propagation()) before:2
		
		-- Doing a complete redraw will force the viewport mesh to be re-generated which reproduces a regression crash due to vertex count in the mesh not being updated
		completeRedraw()
		
		deleteModifier $ 1
	),
	
	-- Tests that we don't get a crash when changing parameters during hair propagation
	function Test_HairPropagation =
	(
		local distributionObject = Sphere radius:10 mapcoords:on isSelected:on

		max modify mode

		-- Add strand propagation
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface method:4
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		modPanel.addModToSelection (Ox_Hair_from_Guides viewCountPercentage:100 denseCount:10)
		
		local strandPropagationModifierInstance = Ox_Strand_Propagation()
		modPanel.addModToSelection strandPropagationModifierInstance
		
		modPanel.addModToSelection (Ox_Render_Settings())
		
		strandPropagationModifierInstance.method = 0
		completeRedraw()
		strandPropagationModifierInstance.method = 1
		--completeRedraw()
			
		-- Make sure we have a good bounding box
		EAssert.IsGreaterThan 0.001 (length (distributionObject.max))
		EAssert.IsGreaterThan 0.001 (length (distributionObject.min))
	),
	
	-- Tests that strands which were propagated will be updated when the shape of their base strand changes
	function Test_PositionUpdateOfPropagatedStrands =
	(
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Strand_Propagation())
		OxResetClassParameters (Ox_Surface_Comb())
				
		local distributionObject = Sphere radius:10 segs:4 mapcoords:on isSelected:on
		local	propagatedStrandCount = 2
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 NumSegs:propagatedStrandCount)
		local baseStrandCount = OxGetStrandCount distributionObject
		modPanel.addModToSelection (Ox_Strand_Propagation method:2 NumSegs:2)
		
		local verticesBeforeSurfaceComb = OxGetVertices distributionObject true
		
		-- Check that propagated strands were placed at the tips of base strands
		local strand1TipIndex = 2
		local strand2TipIndex = 4
		local propagatedStrand1TipIndex = baseStrandCount * 2 + propagatedStrandCount * 1 + 1
		local propagatedStrand2TipIndex = baseStrandCount * 2 + propagatedStrandCount * 3 + 1
		EAssert.ArePointsNearEqual verticesBeforeSurfaceComb[strand1TipIndex] verticesBeforeSurfaceComb[propagatedStrand1TipIndex]
		EAssert.ArePointsNearEqual verticesBeforeSurfaceComb[strand2TipIndex] verticesBeforeSurfaceComb[propagatedStrand2TipIndex]
		
		-- Surface comb modifier will change the shape of all strands
		modPanel.addModToSelection (Ox_Surface_Comb())
		
		-- The propagated strands should've been moved over to the modified positions of surface comb base strand shapes
		local verticesAfterSurfaceComb = OxGetVertices distributionObject true
		EAssert.ArePointsNearEqual verticesAfterSurfaceComb[strand1TipIndex] verticesAfterSurfaceComb[propagatedStrand1TipIndex]
		EAssert.ArePointsNearEqual verticesAfterSurfaceComb[strand2TipIndex] verticesAfterSurfaceComb[propagatedStrand2TipIndex]
	),
	
	function Test_MultipleLevels =
	(
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Strand_Propagation())
				
		local distributionObject = Plane lengthsegs:2 widthsegs:2 lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		-- Add strand propagation
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 NumSegs:2)
		local strandPropagationModifierInstance = Ox_Strand_Propagation count:3 NumSegs:2 lowRange:1 globLength:10 numTwistSides:3 globFanDir:-0.5
		modPanel.addModToSelection strandPropagationModifierInstance
		EAssert.AreEqual 32 ((OxGetVertices distributionObject true).Count)
		
		addModifier distributionObject strandPropagationModifierInstance
		EAssert.AreEqual 128 ((OxGetVertices distributionObject true).Count)
		
		addModifier distributionObject strandPropagationModifierInstance
		EAssert.AreEqual 512 ((OxGetVertices distributionObject true).Count)
		
		addModifier distributionObject strandPropagationModifierInstance
		EAssert.AreEqual 2048 ((OxGetVertices distributionObject true).Count)
	),
	
	function Test_PropagationOnHairFromMeshStrips =
	(
		local stripMesh = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject stripMesh
		
		max modify mode
		
		local boundingBoxBeforePropagation = TestUtilities.GetBoundingBoxVolume( TestUtilities.GetPointsBoundingBox( OxGetVertices hairObjectInstance true ) )
		local strandPropagationModifierInstance = Ox_Strand_Propagation count:3 NumSegs:2 lowRange:1 globLength:10 numTwistSides:3 globFanDir:-0.5
		modPanel.addModToSelection strandPropagationModifierInstance
		local boundingBoxAfterPropagation = TestUtilities.GetBoundingBoxVolume( TestUtilities.GetPointsBoundingBox( OxGetVertices hairObjectInstance true ) )
		
		-- The bounding box of hair should have increased as a result of propagation
		EAssert.IsGreaterThan 100 ( boundingBoxAfterPropagation - boundingBoxBeforePropagation )
	),
	
	function Test_ReuseOfExistingGroupsChannel =
	(
		OxResetClassParameters()
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4)
		
		-- Add edit guides, enable groups, and assign group 1 to all strands
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1..4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 1
		subobjectLevel = 0
		
		modPanel.addModToSelection (Ox_Strand_Propagation assignGroupID:2)
		
		local frizzInstance = Ox_Strand_Frizz amount:10 groupID:2
		modPanel.addModToSelection frizzInstance
		local verticesWithFrizz10 = OxGetVertices distributionObject true
		frizzInstance.amount = 20
		local verticesWithFrizz20 = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual verticesWithFrizz10 verticesWithFrizz20
	),
	
	function Test_PropagateSingleStrandWithUniformDistribution =
	(
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Strand_Propagation())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		-- Add strand propagation
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4)
		modPanel.addModToSelection (Ox_Strand_Propagation method:0 count:1)
		
		OxGetVertices distributionObject true
		
		-- If no crash happened we are good
	),
	
	function Test_PropagatedStrandsGroup =
	(
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Strand_Propagation())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4)
		
		-- Important to test with HFG containing incoming guide groups, it reproduces a bug
		modPanel.addModToSelection (Ox_Generate_Guide_Data newChannelName:"Groups")
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		modPanel.addModToSelection (Ox_Strand_Propagation method:0 count:1 assignGroupID:7)
		local strandFrizz = Ox_Strand_Frizz groupID:1 amount:50
		modPanel.addModToSelection strandFrizz
		
		-- At this point nothing should be frizzed, so get the vertex positions of this state
		local nonFrizzedVertices = OxGetVertices distributionObject true
		
		-- Changing the frizz group to 7 should frizz all propagated strands
		strandFrizz.groupID = 7
		local frizzedVertices = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual nonFrizzedVertices frizzedVertices
	)
)

EUnit.AddTestClass Test_PropagationModifier
ok
