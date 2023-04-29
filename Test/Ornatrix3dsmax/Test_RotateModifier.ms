struct Test_RotateModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_OrientBasedOnStrandShape =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Bend strands in different directions
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [-20,-20,-10]
		editGuidesInstance.SelectRoots #{2}
		editGuidesInstance.MoveSelectedControlPoints [20,20,-10]
		editGuidesInstance.SelectRoots #{3}
		editGuidesInstance.MoveSelectedControlPoints [20,-20,-15]
		editGuidesInstance.SelectRoots #{4}
		editGuidesInstance.MoveSelectedControlPoints [0,20,-10]

		local rotateModifierInstance = Ox_Rotate_Strands orientBasedOnStrandShape: true
		modPanel.addModToSelection rotateModifierInstance
		
		local renderSettingsInstance = Ox_Render_Settings radius: 10
		modPanel.addModToSelection renderSettingsInstance
		
		local meshFromStrandsInstance = Ox_Mesh_From_Strands type: 1 numSides:1
		modPanel.addModToSelection meshFromStrandsInstance
		
		-- If the rotation angles were assigned properly then mesh vertices of bases of each strand will be coplanar with the distribution surface
		local distributionSurfaceNormal = [0,0,1]
		for i = 0 to 3 do
		(
			local strandBaseDirection = (distributionObject.mesh.vertices[i * 4 + 1].position) - (distributionObject.mesh.vertices[i * 4 + 2].position)
			EAssert.AreNearEqual 0 ( dot distributionSurfaceNormal strandBaseDirection )
		)
	),
	
	function Test_OrientTowardsACamera =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Add strand symmetry
		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Create a camera in the middle
		local cameraNode = FreeCamera()

		local rotateModifierInstance = Ox_Rotate_Strands orientBasedOnStrandShape:true faceCamera:on camera:cameraNode
		modPanel.addModToSelection rotateModifierInstance
		
		local renderSettingsInstance = Ox_Render_Settings radius: 10
		modPanel.addModToSelection renderSettingsInstance
		
		local meshFromStrandsInstance = Ox_Mesh_From_Strands type:1 numSides:1
		modPanel.addModToSelection meshFromStrandsInstance
		
		-- All base tangent vectors should be perpendicular to the direction to the camera
		for i = 0 to 3 do
		(
			local vertex1Position = distributionObject.mesh.vertices[i * 4 + 1].position
			local vertex2Position = distributionObject.mesh.vertices[i * 4 + 2].position
			local strandBaseDirection =vertex1Position - vertex2Position
			local cameraDirection = ( cameraNode.position * Inverse( distributionObject.transform ) ) - ( vertex2Position + ( strandBaseDirection / 2.0 ) )
			EAssert.AreNearEqual 0 ( dot cameraDirection strandBaseDirection )
		)
	),
	
	function Test_OrientBasedOnStrandShapeDoesntChangeStrandShape =
	(
		local distributionObject = Sphere radius:20 mapcoords:on isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5
		modPanel.addModToSelection (Ox_Surface_Comb())
		local rotateModifierInstance = Ox_Rotate_Strands orientBasedOnStrandShape:false
		modPanel.addModToSelection rotateModifierInstance
		
		local verticesWithoutStrandOrientation = OxGetVertices distributionObject true
		rotateModifierInstance.orientBasedOnStrandShape = true
		local verticesWithStrandOrientation = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesWithoutStrandOrientation verticesWithStrandOrientation
	)
)

EUnit.AddTestClass Test_RotateModifier
ok
