struct Test_SurfaceLocking
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_LockToSurface =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Surface_Comb())
		modPanel.addModToSelection (Ox_Hair_From_Guides())
		modPanel.addModToSelection (Ox_Hair_Clustering())
			
		EAssert.IsFalse (OxIsLockedToSurface #(distributionObject))
		OxLockToSurface #(distributionObject) on
		EAssert.IsTrue (OxIsLockedToSurface #(distributionObject))
		OxLockToSurface #(distributionObject) off
		EAssert.IsFalse (OxIsLockedToSurface #(distributionObject))
	),
	
	function Test_Detach =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode

		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Edit_Guides())
		modPanel.addModToSelection (Ox_Surface_Comb())
		modPanel.addModToSelection (Ox_Hair_From_Guides())
		modPanel.addModToSelection (Ox_Hair_Clustering())
		modPanel.addModToSelection (Ox_Render_Settings())
			
		local modifierCountBeforeDetaching = distributionObject.modifiers.count
		local verticesBeforeDetaching = OxGetVertices distributionObject true
			
		EAssert.IsFalse (OxIsDetachedFromSurface #(distributionObject))
		OxDetachFromSurface #(distributionObject) on
		EAssert.IsTrue (OxIsDetachedFromSurface #(distributionObject))
		
		-- An extra caching modifier was inserted
		local modifierCountAfterDetaching = distributionObject.modifiers.count
		EAssert.AreEqual (modifierCountBeforeDetaching + 1) modifierCountAfterDetaching
			
		-- And it was inserted before the render settings node
		EAssert.AreEqual distributionObject.modifiers[2].name "Viewport Hair Preview"
			
		local verticesAfterDetaching = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesBeforeDetaching verticesAfterDetaching
			
		-- We should survive a scene reload
		holdMaxFile()
		fetchMaxFile quiet:true
			
		distributionObject = GetNodeByName distributionObjectName
		local verticesAfterLoading = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesBeforeDetaching verticesAfterLoading
			
		OxDetachFromSurface #(distributionObject) off
		EAssert.IsFalse (OxIsDetachedFromSurface #(distributionObject))	
	)
)

EUnit.AddTestClass Test_SurfaceLocking
ok
