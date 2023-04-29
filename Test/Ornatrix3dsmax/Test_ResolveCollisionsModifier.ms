struct Test_ResolveCollisionsModifier
(
	function TestInitialize =
	(
		OxResetClassParameters()
	),
	
	function TestCleanup =
	(
		delete $objects
	),
	
	function GetPenetratingVertexCount strandObject collisionObject allowedPenetrationDepth:0.4 =
	(
		local vertices = OxGetVertices strandObject true
		local penetratingVertexCount = 0
		local penetrationDepth = 0
		local penetratingPoint = [0,0,0]
		for i = 1 to vertices.Count do
		(
			local vertexInWorldCoordinates = vertices[i] * strandObject.transform
			local vertexDistanceFromCenter = distance vertexInWorldCoordinates [0,0,0]
			if( ( vertexDistanceFromCenter + allowedPenetrationDepth ) < collisionObject.radius ) then
			(
				penetrationDepth = vertexDistanceFromCenter - collisionObject.radius
				penetratingPoint = vertexInWorldCoordinates
				penetratingVertexCount += 1
			)
		)
		
		penetratingVertexCount
	),
	
	function Test_ResolvePenetrations =
	(
		-- Create a downward-facing plane
		local distributionObject = Plane length:40 width:40 lengthSegs:4 widthSegs:4 pos:[0,0,50] isSelected:on
		max modify mode
		modPanel.addModToSelection (Normalmodifier flip:on)
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5
		local collisionObject = Sphere radius:20
		
		-- Some vertices should penetrate the sphere volume
		EAssert.IsTrue ((GetPenetratingVertexCount distributionObject collisionObject) > 0)
		
		-- Add resolve modifier, the vertices should no longer penetrate it
		local pushAwayFromSurfaceModifierInstance = Ox_Resolve_Collisions()
		modPanel.addModToSelection pushAwayFromSurfaceModifierInstance
		
		append pushAwayFromSurfaceModifierInstance.collisionMeshes collisionObject
		
		EAssert.AreEqual 0 (GetPenetratingVertexCount distributionObject collisionObject)
	),
	
	function Test_ResolvePenetrationsWithModifiedMesh =
	(
		-- Create a sphere with displaced pivot to test if mesh transform is obtained correctly
		local collisionObject = Sphere radius:20 pos:[0,0,-50] isSelected:on
		AlignPivot collisionObject
		collisionObject.objectoffsetpos = [0, 0, -50]
		-- Rotate sphere back to the origin
		rotate collisionObject ( eulerangles 180 0 0 )

		-- Create a downward-facing plane
		local distributionObject = Plane length:40 width:40 lengthSegs:4 widthSegs:4 pos:[0,0,50] isSelected:on
		max modify mode
		modPanel.addModToSelection (Normalmodifier flip:on)
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5
		
		-- Some vertices should penetrate the sphere volume
		EAssert.IsTrue ((GetPenetratingVertexCount distributionObject collisionObject allowedPenetrationDepth:0.5) > 0)
		
		-- Add resolve modifier, the vertices should no longer penetrate it
		local pushAwayFromSurfaceModifierInstance = Ox_Resolve_Collisions()
		modPanel.addModToSelection pushAwayFromSurfaceModifierInstance
		
		append pushAwayFromSurfaceModifierInstance.collisionMeshes collisionObject
		
		EAssert.AreEqual 0 (GetPenetratingVertexCount distributionObject collisionObject allowedPenetrationDepth:0.5)
	),
	
	function Test_ResolveAnimatedStrandsWithSoftMode =
	(
		local distributionObject = Sphere radius:40 mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 count:100 NumSegs:3)
		local gravityMod = Ox_Strand_Gravity()
		modPanel.addModToSelection gravityMod
		set animate on
		sliderTime = 0f
		gravityMod.gravForce = 1
		sliderTime = 5f
		gravityMod.gravForce = 0
		set animate off
		
		local verticesAtFrame5 = OxGetVertices distributionObject true
		sliderTime = 0f
		
		-- resolveMode = Soft (2)
		modPanel.addModToSelection (Ox_Resolve_Collisions resolveMode:2 recoveryRate:1 collideWithDistributionMesh:off preserveRotations:on)
		
		for i = 0 to 5 do
		(
			sliderTime += 1f
		)
		
		local verticesAtFrame5WithResolveCollisions = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesAtFrame5 verticesAtFrame5WithResolveCollisions
	)
)

EUnit.AddTestClass Test_ResolveCollisionsModifier
ok
