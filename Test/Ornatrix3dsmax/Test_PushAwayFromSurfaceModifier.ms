struct Test_PushAwayFromSurfaceModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_DeleteCollisionObjectInScene =
	(
		-- Create a box with zero width
		local distributionObject = Sphere radius:10 mapcoords:on isSelected:on
		max modify mode

		-- With small strands numSegs value gravity modifier has same issues, let's leave defaults
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Strand_Gravity gravForce:1)
		
		local pushAwayFromSurfaceModifierInstance = Ox_Push_Away_From_Surface()
		modPanel.addModToSelection pushAwayFromSurfaceModifierInstance
		
		local colliderObject = Sphere radius:10 mapcoords:on
		append pushAwayFromSurfaceModifierInstance.objects colliderObject
		local dummyCount = (OxGetVertices distributionObject true).Count
		delete colliderObject
		
		-- Just do a dummy evaluation to make sure gravity modifier recomputes
		EAssert.AreEqual dummyCount (OxGetVertices distributionObject true).Count
	),
	
	function Test_CollisionObject =
	(
		-- Create a downward-facing plane
		local distributionObject = Plane length:40 width:40 lengthSegs:4 widthSegs:4 pos:[0,0,50] isSelected:on
		max modify mode
		modPanel.addModToSelection (Normalmodifier flip:on)
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5
		
		local pushAwayFromSurfaceModifierInstance = Ox_Push_Away_From_Surface distance:1
		modPanel.addModToSelection pushAwayFromSurfaceModifierInstance
		local verticesBeforeCollisions = OxGetVertices distributionObject true
		
		local collisionObject = Sphere radius:20
		
		-- Some vertices should penetrate the sphere volume
		local penetratingVertexCount = 0
		for i = 1 to verticesBeforeCollisions.Count do
		(
			local vertexInWorldCoordinates = verticesBeforeCollisions[i] * distributionObject.transform
			if( (distance vertexInWorldCoordinates [0,0,0]) < collisionObject.radius ) then
			(
				penetratingVertexCount += 1
			)
		)
		
		EAssert.IsTrue (penetratingVertexCount > 0)
		
		-- Add collision object, the vertices should no longer penetrate it
		append pushAwayFromSurfaceModifierInstance.objects collisionObject
		local verticesAfterCollisions = OxGetVertices distributionObject true
		penetratingVertexCount = 0
		local penetrationDepth = 0
		local penetratingPoint = [0,0,0]
		local allowedPenetrationDepth = 0.4
		for i = 1 to verticesAfterCollisions.Count do
		(
			local vertexInWorldCoordinates = verticesAfterCollisions[i] * distributionObject.transform
			local vertexDistanceFromCenter = distance vertexInWorldCoordinates [0,0,0]
			if( ( vertexDistanceFromCenter + allowedPenetrationDepth ) < collisionObject.radius ) then
			(
				penetrationDepth = vertexDistanceFromCenter - collisionObject.radius
				penetratingPoint = vertexInWorldCoordinates
				penetratingVertexCount += 1
			)
		)
		
		EAssert.AreEqual 0 penetratingVertexCount
	)
)

EUnit.AddTestClass Test_PushAwayFromSurfaceModifier
ok
