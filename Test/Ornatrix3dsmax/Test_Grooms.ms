struct Test_Grooms
(
	function TestInitialize =
	(
		units.SystemType = #Centimeters
	),
	
	function GetGroomFilePath groomName =
	(
		(systemTools.getEnvVariable "ALLUSERSPROFILE") + "\\Ephere\\Ornatrix3dsMax" + ( ( maxVersion() )[8] as string ) + "\\Grooms\\" + groomName + ".oxgroom"
	),
	
	function DeleteGroom groomName =
	(
		-- This corresponds to Template::GetDefaultGroomsPath()
		local filePath = GetGroomFilePath groomName
		deleteFile filePath
	),
	
	function Test_BasePolygonMesh =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <PolygonMesh>
      <Vertices>0 0 0 1 0 0 1 1 0 0 1 0</Vertices>
      <VertexCounts>4</VertexCounts>
      <VertexConnects>0 1 2 3</VertexConnects>
    </PolygonMesh>
  </Stack>
</Groom>"

		undo on
		(
			OxCreateGroom templateXml

			local groomNode = GetNodeByName "groom"
			EAssert.IsTrue( groomNode != undefined )
			EAssert.AreEqual 4 (groomNode.mesh.vertices.count)
			EAssert.ArePointsNearEqual [1,1,0] (groomNode.mesh.vertices[3].position)
		)
		
		max undo
	),
	
	function Test_BaseSphere =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <Sphere radius=\"10\" />
  </Stack>
</Groom>"

		undo on
		(
			OxCreateGroom templateXml

			local groomNode = GetNodeByName "groom"
			EAssert.IsTrue( groomNode != undefined )
			EAssert.AreEqual 17L (groomNode.classid[1])
			EAssert.AreEqual 10 (groomNode.radius)
		)
		
		max undo
	),

	function Test_BasicOperators =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <PolygonMesh>
      <Vertices>0 0 0 1 0 0 1 1 0 0 1 0</Vertices>
      <VertexCounts>4</VertexCounts>
      <VertexConnects>0 1 2 3</VertexConnects>
    </PolygonMesh>
	<GuidesFromMesh distribution=\"Vertex\" />
	<HairFromGuides distribution=\"RandomArea\" viewCount=\"10\" renderCount=\"100\" />
  </Stack>
</Groom>"

		undo on
		(
			OxCreateGroom templateXml

			local groomNode = GetNodeByName "groom"
			EAssert.IsTrue( groomNode != undefined )
			EAssert.AreEqual 2 groomNode.modifiers.count
			local guidesFromMeshInstance = groomNode.modifiers[2]
			local hairFromGuidesInstance = groomNode.modifiers[1]
			
			EAssert.AreEqual 4 guidesFromMeshInstance.method
			EAssert.AreEqual 2 hairFromGuidesInstance.method
			EAssert.AreEqual 10 hairFromGuidesInstance.count
			EAssert.AreEqual 100 hairFromGuidesInstance.denseCount
		)
		
		max undo
	),
	
	function Test_ReferencedBaseMesh =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <PolygonMesh referenced=\"true\">
      <Vertices>0 0 0 1 0 0 1 1 0 0 1 0</Vertices>
      <VertexCounts>4</VertexCounts>
      <VertexConnects>0 1 2 3</VertexConnects>
    </PolygonMesh>
	<GuidesFromMesh distribution=\"Vertex\" />
  </Stack>
</Groom>"

		undo on
		(
			OxCreateGroom templateXml
		)
		
		local groomNode = GetNodeByName "groom"
		local baseMeshNode = GetNodeByName "groom_Base"
			
		EAssert.AreNotEqual undefined groomNode
		EAssert.AreNotEqual undefined baseMeshNode
		EAssert.AreEqual baseMeshNode (groomNode.parent)
		
		max undo
	),

	function Test_GroomExport =
	(
		undo on
		(
			local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
			distributionObject.Name = "GroomNode"
			max modify mode
			select distributionObject
			TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
			modPanel.addModToSelection (Ox_Strand_Length value:200 randomize:50)
			
			local xmlString = OxGenerateGroomXML distributionObject
			--print xmlString
			EAssert.AreEqual(
"<?xml version=\"1.0\"?>
<Groom name=\"GroomNode\">
	<Stack>
		<PolygonMesh>
			<Vertices>-12.5 -12.5 0 12.5 -12.5 0 -12.5 12.5 0 12.5 12.5 0</Vertices>
			<VertexCounts>4</VertexCounts>
			<VertexConnects>2 0 1 3</VertexConnects>
		</PolygonMesh>
		<GuidesFromMesh name=\"Ox Guides from Surface\" distribution=\"Vertex\" lengthRandomness=\"0\" pointCount=\"2\" />
		<Length name=\"Ox Strand Length\" value=\"2\" randomize=\"0.5\" />
	</Stack>
</Groom>
") xmlString
		)
		
		max undo
	),
	
	function Test_GroomWithEditGuides =
	(
		undo on
		(
			local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
			local groomNodeName = "GroomNode"
			distributionObject.Name = groomNodeName
			max modify mode
			select distributionObject
			local gfm = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
			
			-- Set surface tangent mode to "First Edge" because that's all that old (oxgroom) format supports
			gfm.surfaceTangentComputeMethod = 1
			
			local editGuidesInstance = Ox_Edit_Guides()
			editGuidesInstance.changeTracking = 1
			modPanel.addModToSelection editGuidesInstance
			
			-- Move control points
			subobjectLevel = 2
			editGuidesInstance.SelectRoots #{1,2,3,4}
			editGuidesInstance.MoveSelectedControlPoints [10,0,0]
			
			-- Remember the positions of the vertices
			local guideVertices = OxGetVertices distributionObject true
			
			-- Create a groom from the guides
			local groomFileName = "editGuidesGroom"
			OxGenerateGroom distributionObject groomFileName
			distributionObject.Name = "other"
			
			-- Load the groom from generated file
			OxCreateGroom groomFileName
			local groomObject = GetNodeByName groomFileName
			local guideVerticesFromGroom = OxGetVertices groomObject true
			
			EAssert.ArePointsAllNearEqual guideVertices guideVerticesFromGroom
			
			DeleteGroom groomFileName
		)
		
		max undo
	),
	
	function Test_GroomWithRenderSettings =
	(
		undo on
		(
			local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
			local groomNodeName = "GroomNode"
			distributionObject.Name = groomNodeName
			max modify mode
			select distributionObject
			TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
			
			local renderSettingsInstance = Ox_Render_Settings()
			modPanel.addModToSelection renderSettingsInstance
			
			-- Modify the radius curve
			local diagramData = #(103, 3, 0, 0, 3, 0.1, 3, 3, 0.5, 0.5 )
			for i = 1 to diagramData.Count do
			(
				renderSettingsInstance.diagData[i] = diagramData[i]
			)
			
			-- Create a groom from the guides
			local groomFileName = "renderSettingsGroom.oxgroom"
			OxGenerateGroom distributionObject groomFileName
			distributionObject.Name = "other"
			
			-- Load the groom from generated file
			OxCreateGroom groomFileName
			local groomObject = GetNodeByName groomNodeName
			renderSettingsInstance = groomObject.modifiers[#Ox_Render_Settings]
			
			for i = 1 to diagramData.Count do
			(
				EAssert.AreNearEqual diagramData[i] renderSettingsInstance.diagData[i]
			)
			
			DeleteGroom groomFileName
		)
		
		max undo
	),

	function Test_GroomWithSplines =
	(
		undo on
		(
			local spline = line()
			addNewSpline spline
			addKnot spline 1 #corner #line [0,0,0]
			addKnot spline 1 #corner #line [0,0,10]
			updateShape spline

			local groomNodeName = "GroomNode"
			spline.Name = groomNodeName
			max modify mode
			select spline
			
			-- Add guides from shape modifier
			modPanel.addModToSelection (Ox_Guides_From_Shape())

			local originalVertices = OxGetVertices spline true

			-- Create a groom from the guides
			-- TODO: Remove .oxgroom extension
			local groomFileName = "splinesGroom.oxgroom"
			OxGenerateGroom spline groomFileName
			spline.Name = "other"
			
			-- Load the groom from generated file
			OxCreateGroom groomFileName
			local groomObject = GetNodeByName groomNodeName
			
			local groomVertices = OxGetVertices groomObject true
			
			-- Compare hair vertices
			for i = 1 to originalVertices.Count do
			(
				EAssert.ArePointsNearEqual originalVertices[i] groomVertices[i]
			)
			
			DeleteGroom groomFileName
		)
		
		max undo
	),
	
	function Test_OptionalBaseMesh =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <PolygonMesh optional=\"true\">
      <Vertices>0 0 0 1 0 0 1 1 0 0 1 0</Vertices>
      <VertexCounts>4</VertexCounts>
      <VertexConnects>0 1 2 3</VertexConnects>
    </PolygonMesh>
	<GuidesFromMesh distribution=\"Vertex\" />
  </Stack>
</Groom>"

		undo on
		(
			-- Create a plane mesh ahead of time to which we will apply our groom
			local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
			OxCreateGroom templateXml #(distributionObject) 1 1
			
			local groomNode = GetNodeByName "groom"
			local baseMeshNode = GetNodeByName "groom_Base"
				
			-- We should have a groom node created...
			EAssert.AreNotEqual undefined groomNode
			
			-- ...but not a base node...
			EAssert.AreEqual undefined baseMeshNode
			
			-- ...instead the plane we created should be used as a base node
			EAssert.AreEqual distributionObject (groomNode.parent)
		)
		
		max undo
	),
	
	function GenerateGroomWithSphere groomFileName:"test" =
	(
		local distributionObject = Sphere redius:10 segs:32 mapcoords:on name:groomFileName
		
		max modify mode
		select distributionObject
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		OxGenerateGroom distributionObject groomFileName
		
		distributionObject
	),
	
	function Test_GenerateGroomWithSphere =
	(
		local groomFileName = "Test_GenerateGroomWithSphere"
		local distributionObject = GenerateGroomWithSphere groomFileName:groomFileName
		local originalVertices = OxGetVertices distributionObject true
		local groomNodeName = distributionObject.name
		
		OxCreateGroom groomFileName
		local groomNode = GetNodeByname groomNodeName
		
		local groomVertices = OxGetVertices groomNode true
		
		EAssert.AreEqual (originalVertices.Count) (groomVertices.Count)
		
		-- Original and groom vertices should be the same
		for i=1 to originalVertices.Count do
		(
			EAssert.ArePointsNearEqual originalVertices[i] groomVertices[i]
		)
		
		DeleteGroom groomFileName
		--delete distributionObject
		delete groomNode
	),
	
	function Test_UnsupportedModifiersAreSkipped =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		local groomNodeName = distributionObject.name
		
		max modify mode
		select distributionObject
		-- Add a supported modifier, then unsupported one, then supported one again
		local gfm = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		gfm.surfaceTangentComputeMethod = 1
		
		-- Add any unsupported modifier
		modPanel.addModToSelection (Ox_Animation_Cache())
		modPanel.addModToSelection (Bend())
		
		-- And a supported one
		modPanel.addModToSelection (Ox_Strand_Curling())
			
		local groomName = "Test_UnsupportedModifiersAreSkipped"
		-- TODO: Remove extension
		local groomFileName = groomName + ".oxgroom"
		OxGenerateGroom distributionObject groomFileName
		delete distributionObject
			
		OxCreateGroom groomFileName
		local groomNode = GetNodeByname groomNodeName
			
		EAssert.AreNotEqual undefined (groomNode.modifiers[#Ox_Strand_Curling])
		EAssert.AreNotEqual undefined (groomNode.modifiers[#Ox_Guides_From_Surface])
		EAssert.AreEqual undefined (groomNode.modifiers[#Bend])
		EAssert.AreEqual undefined (groomNode.modifiers[#Ox_Strand_Symmetry])
			
		delete groomNode
		DeleteGroom groomFileName
	),
	
	-- Tests that grooms don't use 3dsmax "sticky" values when adding groom operators without parameter definitions
	function Test_DefaultValuesAreRestored =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <PolygonMesh>
      <Vertices>0 0 0 1 0 0 1 1 0 0 1 0</Vertices>
      <VertexCounts>4</VertexCounts>
      <VertexConnects>0 1 2 3</VertexConnects>
    </PolygonMesh>
	<GuidesFromMesh />
  </Stack>
</Groom>"
		
		-- Create a guides from mesh modifier and assign it some non-default parameters, they will become "sticky" until next scene reset
		local sphere1 = Sphere mapcoords:on isSelected:on
		max modify mode
		local gfm = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:22
		gfm.surfaceTangentComputeMethod = 1
		delete sphere1

		-- Create the groom
		undo on
		(
			OxCreateGroom templateXml

			local groomNode = GetNodeByName "groom"
			EAssert.IsTrue( groomNode != undefined )
			EAssert.AreEqual 1 groomNode.modifiers.count
			local guidesFromMeshInstance = groomNode.modifiers[1]
			
			-- Point count should be the default value and not the sticky value
			EAssert.AreEqual 10 guidesFromMeshInstance.NumSegs
		)
		
		max undo
	),
	
	function Test_Scaling =
	(
		local templateXml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Groom name=\"groom\">
  <Stack>
    <PolygonMesh>
      <Vertices>0 0 0 1 0 0 1 1 0 0 1 0</Vertices>
      <VertexCounts>4</VertexCounts>
      <VertexConnects>0 1 2 3</VertexConnects>
    </PolygonMesh>
	<GuidesFromMesh distribution=\"Vertex\" lengthRandomness=\"0\" />
	<HairFromGuides distribution=\"RandomArea\" viewCount=\"10\" renderCount=\"100\" interpolateGuidesInUvSpace=\"false\" />
  </Stack>
</Groom>"

		local unscaledGroomBounds = #()
		undo on
		(
			-- First create and measure without any scaling
			OxCreateGroom templateXml

			local groomNode = GetNodeByName "groom"
			EAssert.IsTrue( groomNode != undefined )
		
			unscaledGroomBounds = TestUtilities.GetPointsBoundingBox (TestUtilities.GetPointPositions groomNode)
		)
		
		max undo
		
		local scaledGroomBounds = #()
		undo on
		(
			-- First create and measure without any scaling
			OxCreateGroom templateXml undefined 0.5 1.0

			local groomNode = GetNodeByName "groom"
			EAssert.IsTrue( groomNode != undefined )
		
			scaledGroomBounds = TestUtilities.GetPointsBoundingBox (TestUtilities.GetPointPositions groomNode)
		)
		
		local unscaledVolume = (unscaledGroomBounds[2].x - unscaledGroomBounds[1].x) * (unscaledGroomBounds[2].y - unscaledGroomBounds[1].y) * (unscaledGroomBounds[2].z - unscaledGroomBounds[1].z)
		local scaledVolume = (scaledGroomBounds[2].x - scaledGroomBounds[1].x) * (scaledGroomBounds[2].y - scaledGroomBounds[1].y) * (scaledGroomBounds[2].z - scaledGroomBounds[1].z)
		EAssert.AreNearEqual 0.5 (scaledVolume / unscaledVolume)
		max undo
	),
	
	function Test_GenerateGroomFromNodeWithReferencedBaseObject =
	(
		local groomFileName = "Test_GenerateGroomFromNodeWithReferencedBaseObject.oxgroom"
		local distributionObject = GenerateGroomWithSphere groomFileName:groomFileName
		local originalVertices = OxGetVertices distributionObject true
		delete distributionObject
		
		-- Create the groom on a sphere and then generate it again from the sphere
		local distributionObject2 = Sphere redius:10 segs:32 mapcoords:on isSelected:on
		modPanel.addModToSelection (Bend())
		
		OxCreateGroom groomFileName #(distributionObject2) 1 1
		local groomHair2 = GetNodeByName groomFileName
		local groom2FileName = "Test_GenerateGroomFromNodeWithReferencedBaseObject2.oxgroom"
		OxGenerateGroom groomHair2 groom2FileName
		
		-- The new groom file should contain a base mesh
		local groom2String = TestUtilities.GetFileContents (GetGroomFilePath "Test_GenerateGroomFromNodeWithReferencedBaseObject2")
		EAssert.IsTrue ( ( findString groom2String "<PolygonMesh" ) != undefined )
		
		DeleteGroom groomFileName
		DeleteGroom groom2FileName
	),
	
	function Test_GenerateDetachedGroom =
	(
		-- TODO: Remove .oxgroom extension
		local groomFileName = "Test_GenerateDetachedGroom.oxgroom"
		local distributionObject = Sphere redius:10 segs:32 mapcoords:on name:groomFileName isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Hair_From_Guides())
		modPanel.addModToSelection (Ox_Render_Settings())
			
		OxDetachFromSurface #(distributionObject) on
		local verticesBeforeGroom = OxGetVertices distributionObject true
			
		OxGenerateGroom distributionObject groomFileName
		delete distributionObject
			
		OxCreateGroom groomFileName
		distributionObject = $objects[1]
		local verticesAfterGroom = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesBeforeGroom verticesAfterGroom useDistance:true
			
		EAssert.IsTrue (OxIsDetachedFromSurface #(distributionObject))
		OxLockToSurface #(distributionObject) off
		--local verticesAfterReattaching = OxGetVertices distributionObject true
		--EAssert.ArePointsAllNearEqual verticesBeforeGroom verticesAfterReattaching useDistance:true
			
		DeleteGroom groomFileName
	),
	
	function Test_GroundStrandsBaseMeshAssignmentOnGroomCreation =
	(
		local groomFileName = "Test_GroundStrandsBaseMeshAssignmentOnGroomCreation.oxgroom"
		local distributionObject = Sphere mapcoords:on name:groomFileName isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Ground_Strands())
			
		OxGenerateGroom distributionObject groomFileName
		delete distributionObject
			
		distributionObject = Sphere()
		OxCreateGroom groomFileName #(distributionObject) 1 1
		local hairObject = $objects[2]
		EAssert.AreEqual distributionObject (hairObject.modifiers[#Ox_Ground_Strands].surface)
			
		DeleteGroom groomFileName
	),
	
	function Test_EditGuidesChangesPersistedInGrooms =
	(
		local groomFileName = "Test_EditGuidesChangesPersistedInGrooms"
		local distributionObject = Sphere mapcoords:on name:groomFileName isSelected:on
		max modify mode
		local gfm = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		gfm.surfaceTangentComputeMethod = 1
		
		modPanel.addModToSelection (Ox_Ground_Strands())
		local editGuidesInstance = Ox_Edit_Guides()
		editGuidesInstance.changeTracking = 1
		modPanel.addModToSelection editGuidesInstance
			
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1..30}
		editGuidesInstance.MoveSelectedControlPoints [0,0,-200]
		editGuidesInstance.SelectRoots #{30..60}
		editGuidesInstance.MoveSelectedControlPoints [0,0,200]
			
		local verticesBeforeGroomCreation = OxGetVertices distributionObject true
			
		OxGenerateGroom distributionObject groomFileName
		delete distributionObject
			
		OxCreateGroom groomFileName
		distributionObject = getNodeByName groomFileName
		
		local verticesAfterGroomCreation = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesBeforeGroomCreation verticesAfterGroomCreation
			
		DeleteGroom groomFileName
	),
	
	function Test_DetachedGroomWithBakedGuides =
	(
		local groomFileName = "Test_DetachedGroomWithBakedGuides"
		local surfaceObject = Sphere mapcoords:on
		local distributionObject = Sphere mapcoords:on name:groomFileName isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Ground_Strands surface:surfaceObject)
		modPanel.addModToSelection (Ox_Strand_Gravity())
		modPanel.addModToSelection (Ox_Hair_from_Guides())
		local verticesBeforeGroomCreation = OxGetVertices distributionObject true
		
		modPanel.setCurrentObject distributionObject.modifiers[#Ox_Guides_from_Surface]
		maxOps.CollapseNodeTo distributionObject 4 on
			
		-- Once to ground guides and once again to unground them
		OxDetachFromSurface #(distributionObject) off
		OxDetachFromSurface #(distributionObject) on
		
		OxGenerateGroom distributionObject groomFileName
		delete distributionObject

		OxCreateGroom groomFileName #(surfaceObject) 1 1
		local groomObject = $objects[2]
		
		/*OxDetachFromSurface #(groomObject) off
		local verticesAfterGroomCreation = OxGetVertices groomObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeGroomCreation verticesAfterGroomCreation useDistance:true epsilon:0.8
		
		DeleteGroom groomFileName*/
	),

	function Test_GenerateGroom =
	(
		local groomFileName = "Test_GenerateGroom.oxg.yaml"
		
		-- Make sure we start by creating a new groom file
		if not( doesFileExist groomFileName ) then deleteFile groomFileName

		local distributionObject = Sphere mapcoords:on name:groomFileName isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local verticesBeforeGroomCreation = OxGetVertices distributionObject true

		OxGenerateGroom distributionObject groomFileName

		-- Load the groom from generated file
		OxCreateGroom groomFileName
		
		local groomObject = $objects[3]
		local verticesAfterGroomCreation = OxGetVertices groomObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeGroomCreation verticesAfterGroomCreation
	)
)

EUnit.AddTestClass Test_Grooms
ok
