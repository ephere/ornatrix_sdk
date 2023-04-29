struct Test_GuidesFromGuideMeshModifier
(
	-- Tests that guides from guide mesh modifier assigns its distribution object as distribution mesh
	-- instead of using the guides mesh itself
	function Test_HairFromGuidesAfterGuidesFromGuideMesh =
	(
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Mesh_From_Strands())
		OxResetClassParameters (Ox_Guides_From_Guide_Mesh())
		OxResetClassParameters (Ox_Hair_From_Guides())
			
		planeObject = Plane lengthsegs:1 widthsegs:1
		distributionObject = reference planeObject
		
		-- Setup guides, converted to mesh, converted back to guides, and hair generator
		max modify mode
		select distributionObject
		
		-- Guides from surface
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Mesh from strands exporting guide mesh
		meshFromStrandsModifierInstance = Ox_Mesh_From_Strands()
		meshFromStrandsModifierInstance.type = 1
		meshFromStrandsModifierInstance.guideMeshExp = on
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		-- Guides from guide mesh and use plane object for distribution
		guidesFromGuideMeshModifierInstance = Ox_Guides_From_Guide_Mesh()
		guidesFromGuideMeshModifierInstance.surface = planeObject
		modPanel.addModToSelection guidesFromGuideMeshModifierInstance
		
		/*-- Hair from guides, use vertex distribution
		hairFromGuidesModifierInstance = Ox_Hair_From_Guides()
		hairFromGuidesModifierInstance.method = 4
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		-- Only 4 hairs should be generated, one for each plane vertex
		hairCopy = instance distributionObject
		convertTo hairCopy HairObject
		myHairs = hairCopy.GetHair()
		EAssert.AreEqual 4 (myHairs.NumRoots())
		delete hairCopy
			
		-- Cleanup
		delete distributionObject
		delete planeObject*/
	)
)

EUnit.AddTestClass Test_GuidesFromGuideMeshModifier
ok
