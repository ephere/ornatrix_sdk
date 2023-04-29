struct Test_DetailModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that detail modifier can be applied to many animated/deforming strands and will not pop over time
	function Test_NoPoppingOnAnimatedStrands =
	(
		OxResetClassParameters ()
		
		local distributionObject = Plane lengthsegs:2 widthsegs:2
		max modify mode

		-- Add strand symmetry
		select distributionObject
		
		-- Add an animated noise to distribution surface to cause animation over time
		local noiseModifierInstance = NoiseModifier animate:on
		modPanel.addModToSelection noiseModifierInstance

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 1
		guidesFromSurfaceModifier.count = 1000
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Add a curling modifier to give strands some shape
		modPanel.addModToSelection (Ox_Strand_Curling magn:6 phase:2)
		modPanel.addModToSelection (Ox_Strand_Detail viewCount:4)

		-- Test that vertex positions do not change by a lot over time
		local previousVertexPositions = OxGetVertices distributionObject true
		for i=1 to 5 do
		(
			sliderTime += 1f
			local vertexPositions = OxGetVertices distributionObject true
			for vertexIndex = 1 to vertexPositions.Count do
			(
				EAssert.IsLessThan 0.5 ( length ( previousVertexPositions[vertexIndex] - vertexPositions[vertexIndex] ) )
			)
		)
	),
	
	function CreateGuidesWithTwoStrandGroups =
	(
		max modify mode
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on

		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2

		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 3
		
		#(distributionObject)
	),
	
	function Test_StrandGroupSupport =
	(
		local params = CreateGuidesWithTwoStrandGroups()
		local distributionObject = params[1]
		
		-- Convert to hair, note set interpolation to affine to make sure each strand gets its own guide dependency
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4 interpolation:1 interpNum:2)
		
		local strandDetailModifier = Ox_Strand_Detail viewCount:5 strandGroupPattern:"2"
		modPanel.addModToSelection strandDetailModifier
		
		EAssert.AreEqual (2 * 2 + 2 * 5) ((OxGetVertices distributionObject false).Count)
		
		strandDetailModifier.viewCount = 7
		distributionObject.modifiers[#Ox_Guides_From_Surface].NumSegs = 3
		EAssert.AreEqual (2 * 3 + 2 * 7) ((OxGetVertices distributionObject false).Count)
	),
	
	function Test_StrandGroupsAreExported =
	(
		OxResetClassParameters( Ox_Strand_Detail() )
		local params = CreateGuidesWithTwoStrandGroups()
		local distributionObject = params[1]
		local strandDetailModifier = Ox_Strand_Detail viewCount:5
		modPanel.addModToSelection strandDetailModifier
		
		local strandCurlModifier = Ox_Strand_Frizz amount:10 strandGroupPattern:"0"
		modPanel.addModToSelection strandCurlModifier
		
		-- If groups are exported properly from detail modifier then changing strand group on Curling modifier will alter the result
		local verticesWithoutGroups = OxGetVertices distributionObject true
		strandCurlModifier.strandGroupPattern = "2"
		local verticesWithGroup2 = OxGetVertices distributionObject true
		strandCurlModifier.strandGroupPattern = "3"
		local verticesWithGroup3 = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual verticesWithoutGroups verticesWithGroup2
		EAssert.ArePointsNotAllNearEqual verticesWithoutGroups verticesWithGroup3
		EAssert.ArePointsNotAllNearEqual verticesWithGroup2 verticesWithGroup3
	),
	
	function Test_WidthSupport =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 NumSegs:2)
		modPanel.addModToSelection (Ox_Strand_Detail viewCount:4)
		local meshFromStrands = Ox_Mesh_From_Strands type:1
		modPanel.addModToSelection meshFromStrands
		
		local verticesBeforeChangingWidth = TestUtilities.GetVertexArray distributionObject
		
		-- Change width before the detail operator
		meshFromStrands.enabled = off
		addModifier distributionObject (Ox_Change_Width width:10) before:2
		
		meshFromStrands.enabled = on
		local verticesAfterChangingWidth = TestUtilities.GetVertexArray distributionObject
		
		-- The width of first strand should be different after we changed it
		local widthBeforeChange = distance verticesBeforeChangingWidth[2] verticesBeforeChangingWidth[1]
		local widthAfterChange = distance verticesAfterChangingWidth[2] verticesAfterChangingWidth[1]
		EAssert.AreNotNearEqual widthBeforeChange widthAfterChange
	),
	
	function Test_RotationSupport =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 NumSegs:2)
		modPanel.addModToSelection (Ox_Strand_Detail viewCount:4)
		local meshFromStrands = Ox_Mesh_From_Strands type:1
		modPanel.addModToSelection meshFromStrands
		
		local verticesBeforeChangingRotation = TestUtilities.GetVertexArray distributionObject
		
		-- Change width before the detail operator
		meshFromStrands.enabled = off
		addModifier distributionObject (Ox_Rotate_Strands angle:20) before:2
		
		meshFromStrands.enabled = on
		local verticesAfterChangingRotation = TestUtilities.GetVertexArray distributionObject
		
		-- The width of first strand should be different after we changed it
		local vectorBeforeChange = verticesBeforeChangingRotation[2] - verticesBeforeChangingRotation[1]
		local vectorAfterChange = verticesAfterChangingRotation[2] - verticesAfterChangingRotation[1]
		EAssert.ArePointsNotNearEqual vectorBeforeChange vectorAfterChange
	),
	
	fn GetVectorsAngle v1 v2 =
	(
		theAngle = acos(dot (normalize v1) (normalize v2))
	),
	
	-- Determines the number of bends in a point strip
	function GetBendCount points threshold:10 =
	(
		result = 0
		for i = 2 to (points.count - 1) do
		(
			if( (GetVectorsAngle (points[i] - points[i-1]) (points[i + 1] - points[i]) ) > threshold ) then result+=1
		)
		
		result
	),

	function Test_SampleByThreshold =
	(
		OxResetClassParameters( Plane() )
		OxResetClassParameters( Ox_Guides_from_Surface() )
		OxResetClassParameters( Ox_Edit_Guides() )
		OxResetClassParameters( Ox_Strand_Detail() )

		-- Create very small plane to have roots at origin
		local distributionObject = Plane width:0.1 length:0.1 lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:0 NumSegs:4 count:1 length:4)
		editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		-- Move vertices to form zigzag shape
		editGuidesInstance.SetStrandShape 0 #( [0,0,0], [0,0.5,1], [0,-0.5,2], [0,0,3] )
		detailInstance = ( Ox_Strand_Detail viewCount:10 smoothing:0 sampleByThreshold:true angleThreshold:150.0 )
		modPanel.addModToSelection detailInstance

		detailInstance.sampleByThreshold = false
		local verticesWithoutThreshold = OxGetVertices distributionObject true
		EAssert.AreEqual 2 (GetBendCount verticesWithoutThreshold)
		
		detailInstance.sampleByThreshold = true
		local verticesWithThreshold = OxGetVertices distributionObject true

		-- Jagged strand should be straightened when Sample By Threshold is on
		EAssert.AreEqual 0 (GetBendCount verticesWithThreshold)
	),

	function Test_DuplicatedPointsFiltering =
	(
		-- Tests if closely spaced vertices are handled properly when smoothing (can lead to wildly varying NURBS curves and hair "freaking out" for certain values)

		OxResetClassParameters( Plane() )
		OxResetClassParameters( Ox_Guides_from_Surface() )
		OxResetClassParameters( Ox_Edit_Guides() )
		OxResetClassParameters( Ox_Strand_Detail() )

		-- Create very small plane to have roots at origin
		local distributionObject = Plane width:0.1 length:0.1 lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:0 NumSegs:11 count:1 length:10)
		editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		-- Move vertices to form zigzag shape
		editGuidesInstance.SetStrandShape 0 #( [0,0,0], [0,0.5,1], [0,0.5,1], [0,-0.5,2], [0,0.5,3], [0,-0.5,4], [0,0.5,5], [0,-0.5,6], [0,0.5,7], [0,-0.5,8], [0,0,9] )
		detailInstance = ( Ox_Strand_Detail viewCount:20 smoothing:1 smoothingPointCount:10 )
		modPanel.addModToSelection detailInstance

		local verticesWithSmoothing = OxGetVertices distributionObject true
		detailInstance.smoothing = 0
		local verticesWithoutSmoothing = OxGetVertices distributionObject true

		EAssert.ArePointsAllNearEqual verticesWithSmoothing verticesWithoutSmoothing epsilon:1
	),

	function Test_IsAlwaysSmoothing =
	(
		-- Tests if smoothing works for equal point counts

		OxResetClassParameters()

		local distributionObject = Plane width:1 length:1 lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:1 NumSegs:10 count:0 length:10)
		editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		-- Create root at origin
		editGuidesInstance.CreateRootAtCoordinate 0 [0, 0]
		-- Move vertices to form zigzag shape
		editGuidesInstance.SetStrandShape 0 #( [0,0,0], [0,0.5,1], [0,0,2], [0,0.5,3], [0,0,4], [0,0.5,5], [0,0,6], [0,0.5,7], [0,0,8], [0,0.5,9] )
		detailInstance = ( Ox_Strand_Detail viewCount:10 smoothing:1 smoothingPointCount:10 )
		modPanel.addModToSelection detailInstance

		local verticesWithSmoothing = OxGetVertices distributionObject true
		detailInstance.smoothing = 0
		local verticesWithoutSmoothing = OxGetVertices distributionObject true

		EAssert.ArePointsNotAllNearEqual verticesWithSmoothing verticesWithoutSmoothing
	),
	
	function Test_TextureCoordinatesArePreserved =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 NumSegs:2)
		
		-- TODO: Test all channels and multiple data types
		local textureCoordinatesBeforeDetail = OxGetTextureCoordinates distributionObject 0 0 4 0
		modPanel.addModToSelection (Ox_Strand_Detail())
		local textureCoordinatesAfterDetail = OxGetTextureCoordinates distributionObject 0 0 4 0
			
		EAssert.ArePointsAllNearEqual textureCoordinatesBeforeDetail textureCoordinatesAfterDetail
	)
)

EUnit.AddTestClass Test_DetailModifier
ok
