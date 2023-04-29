struct Test_GroomsSavingLoading
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function TestInitialize =
	(
		units.SystemType = #Centimeters
	),
	
	function GetGroomFilePath groomName =
	(
		(systemTools.getEnvVariable "ALLUSERSPROFILE") + "\\Ephere\\Ornatrix\\Grooms\\" + groomName + ".oxgroom"
	),
	
	function DeleteGroom groomName =
	(
		-- This corresponds to Template::GetDefaultGroomsPath()
		local filePath = GetGroomFilePath groomName
		deleteFile filePath
	),
	
	function GetPropertyType obj parameterName =
	(
		local propertyString = stringStream ""
		showProperties obj parameterName to:propertyString
		propertyString = propertyString as string
		if propertyString == false then print ("Cannot find parameter " + parameterName)
				
		local colonPosition = 0
		local result = propertyString
		while colonPosition != undefined do
		(
			colonPosition = findString result ":"
			if colonPosition != undefined then
			(
				result = subString result (colonPosition + 2) (result.count - colonPosition - 2)
			)
		)
		
		return result
	),
	
	function GetNonDefaultValue obj parameterName nonDefaultParameter =
	(
		if (nonDefaultParameter.Count == 2 ) then
		(
			return nonDefaultParameter[2]
		)
		else
		(
			local propertyType = GetPropertyType obj parameterName
			if propertyType == "worldUnits" then return 0.234
			else if propertyType == "float" then return 0.234
			else if propertyType == "integer" then return 234
			else if propertyType == "angle" then return 0.234
			else if propertyType == "percent" then return 0.234
			else if (propertyType == "Boolean" or propertyType == "boolean") then return true
			else if propertyType == "string" then return "foo"
			else print ("Property type not supported: '" + propertyType + "'")
			
			return undefined
		)
	),
	
	function SaveLoadGroom modifierInstance operatorName nonDefaultParameters expectedDefaultAttributeCount addGuides:true addHair: false =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode
		
		if addGuides then
		(
			TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		)
		
		if addHair then
		(
			modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		)
		
		modPanel.addModToSelection modifierInstance
		
		local xmlString = OxGenerateGroomXML distributionObject
		local xmlDocument = dotNetObject "System.Xml.XmlDocument"
		xmlDocument.loadXml xmlString
		local curlXmlNode = xmlDocument.SelectSingleNode ("/Groom/Stack/" + operatorName)
		
		-- Two attributes expected by default: name and curve
		if curlXmlNode == undefined then
		(
			print ("Operator XML node '" +  operatorName + "' not found, XML string: " + xmlString)
		)
		
		EAssert.AreEqual expectedDefaultAttributeCount (curlXmlNode.Attributes.Count) message:xmlString
				
		for nonDefaultParameter in nonDefaultParameters do
		(
			local parameterName = nonDefaultParameter[1]
			local nonDefaultValue = GetNonDefaultValue modifierInstance parameterName nonDefaultParameter			
			setProperty modifierInstance parameterName nonDefaultValue
		)
		
		local xmlString = OxGenerateGroomXML distributionObject
		local xmlDocument = dotNetObject "System.Xml.XmlDocument"
		xmlDocument.loadXml xmlString
		local curlXmlNode = xmlDocument.SelectSingleNode ("/Groom/Stack/" + operatorName)
		
		-- Two default attributes plus all assigned attributes
		EAssert.AreEqual (expectedDefaultAttributeCount + nonDefaultParameters.Count) (curlXmlNode.Attributes.Count) message:xmlString
		delete distributionObject
		
		-- Load the groom
		OxCreateGroom xmlString
		local groomNode = GetNodeByName distributionObjectName
		EAssert.IsTrue( groomNode != undefined )
		
		-- Check all parameters
		for nonDefaultParameter in nonDefaultParameters do
		(
			local parameterName = nonDefaultParameter[1]
			local nonDefaultValue = GetNonDefaultValue modifierInstance parameterName nonDefaultParameter			
			local propertyValue = getProperty modifierInstance parameterName
			local propertyType = GetPropertyType modifierInstance parameterName
			if propertyType == "float" or propertyType == "angle" or propertyType == "worldUnits" or propertyType == "percent" then
			(
				EAssert.AreNearEqual nonDefaultValue propertyValue
			)
			else EAssert.AreEqual nonDefaultValue propertyValue
		)
	),
	
	function Test_Curl =
	(
		OxResetClassParameters (Ox_Strand_Curling())
		SaveLoadGroom (Ox_Strand_Curling()) "Curl" #( #("form", 0),
				#("magn"),
				#("phase"),
				#("phaseOff"),
				#("stretch"),
				#("lenDep", on),
				#("stdScale"),
				#("stdAmount") ) 1
	),
	
	function Test_Detail =
	(
		OxResetClassParameters (Ox_Strand_Detail())
		SaveLoadGroom (Ox_Strand_Detail()) "Detail" #( #("viewCount"),
				#("rendercount"),
				#("angleThreshold"),
				#("sampleByThreshold", on),
				#("sampleSize"),
				#("smoothing") ) 1
	),
	
	function Test_EditGuides =
	(
		OxResetClassParameters (Ox_Edit_Guides())
		SaveLoadGroom (Ox_Edit_Guides()) "EditGuides" #( 
				#("applyAmount") ) 1
	),
	
	function Test_Frizz =
	(
		OxResetClassParameters (Ox_Strand_Frizz())
		SaveLoadGroom (Ox_Strand_Frizz()) "Frizz" #( 
				#("scale"),
				#("outPercent"),
				#("outAmount"),
				#("seed") ) 1
	),
	
	function Test_GenerateGuideData =
	(
		OxResetClassParameters (Ox_Generate_Guide_Data())
		SaveLoadGroom (Ox_Generate_Guide_Data()) "GenerateStrandData" #( 
				#("strandGroupPattern"),
				#("randomSeed"),
				#("newChannelType", 1),
				#("newChannelName") ,
				#("generationMethod", 2),
				#("useTargetValueRange", false),
				#("minimumTargetValue"),
				#("maximumTargetValue"),
				#("useSourceValueRange", true),
				#("minimumSourceValue"),
				#("maximumSourceValue") ) 1
	),
	
	function Test_Gravity =
	(
		OxResetClassParameters (Ox_Strand_Gravity())
		SaveLoadGroom (Ox_Strand_Gravity()) "Gravity" #( 
				#("gravForce"),
				#("strandGroupPattern") ) 1
	),
	
	function Test_GroundStrands =
	(
		OxResetClassParameters (Ox_Ground_Strands())
		SaveLoadGroom (Ox_Ground_Strands()) "GroundStrands" #() 1
	),
	
	function Test_GuideCluster =
	(
		OxResetClassParameters (Ox_Strand_Clustering())
		SaveLoadGroom (Ox_Strand_Clustering()) "GuideCluster" #( 
				#("clustAmt"),
				#("twistAmt"),
				#("wispSize"),
				#("wispOffset"),
				#("useWisps", true) ) 1 addHair:true
	),
	
	function Test_GuidesFromCurves =
	(
		OxResetClassParameters (Ox_Guides_From_Shape())
		SaveLoadGroom (Ox_Guides_From_Shape()) "GuidesFromCurves" #( 
				#("NumSegs") ) 1
	),
	
	function Test_GuidesFromHair =
	(
		OxResetClassParameters (Ox_Guides_From_Hair())
		SaveLoadGroom (Ox_Guides_From_Hair()) "GuidesFromHair" #() 1 addHair:true
	),
	
	function Test_GuidesFromMesh =
	(
		OxResetClassParameters (Ox_Guides_From_Surface())
		SaveLoadGroom (Ox_Guides_From_Surface()) "GuidesFromMesh" #( 
				#("method", 4),
				#("count"),
				#("guideLength"),
				#("randomness"),
				#("NumSegs"),
				#("randSeed"),
				#("topoOrient", false),
				#("detachRoots", true),
				#("rememberPositions", true) ) 1 addGuides:false
	),
	
	function Test_HairCluster =
	(
		OxResetClassParameters (Ox_Hair_Clustering())
		SaveLoadGroom (Ox_Hair_Clustering()) "HairCluster" #( 
				#("genCount"),
				#("genSeed"),
				#("thresh"),
				-- TODO: Change expected values to 2 when diagram is implemented
				#("useUV", true) ) 1 addHair:true
	),
	
	/*function Test_HairFromGuides =
	(
		OxResetClassParameters (Ox_Hair_From_Guides())
		-- Default attributes: guideArea="Barycentric" interpolation="Unused"
		SaveLoadGroom (Ox_Hair_From_Guides()) "HairFromGuides" #( 
				#("method", 4),
				#("count"),
				#("denseCount"),
				#("interpolation2", 0),
				#("interpNum", 1),
				#("topoOrient", false),
				#("guideAreaCircleRadius", 0.0),
				#("gpDist") ) 1
	),*/
	
	function Test_Length =
	(
		OxResetClassParameters (Ox_Strand_Length())
		SaveLoadGroom (Ox_Strand_Length()) "Length" #( 
				#("value"),
				#("randomize"),
				#("seed"),
				#("minimumValue"),
				#("maximumValue") ) 1
	),
	
	function Test_MeshFromHair =
	(
		OxResetClassParameters (Ox_Mesh_From_Strands())
		SaveLoadGroom (Ox_Mesh_From_Strands()) "MeshFromStrands" #( 
				#("type", 0),
				#("strandGroup"),
				#("strandGroupPattern"),
				#("seed") ) 1
	),
	
	function Test_Multiplier =
	(
		OxResetClassParameters (Ox_Strand_Multiplier())
		SaveLoadGroom (Ox_Strand_Multiplier()) "Multiplier" #( 
				#("seed"),
				#("strandGroupPattern"),
				#("probability"),
				#("copies"),
				#("spreadRoot"),
				#("spreadTip"),
				#("fluffRoot"),
				#("fluffTip"),
				#("shiftRoot"),
				#("shiftTip"),
				#("twist") ) 1
	),
	
	function Test_Propagation =
	(
		OxResetClassParameters (Ox_Strand_Propagation())
		SaveLoadGroom (Ox_Strand_Propagation()) "Propagation" #( 
				#("method", 0),
				#("count", 1),
				#("NumSegs", 2),
				#("strandGroupPattern"),
				#("perStrCount", false),
				#("randSeed"),
				#("lowRange"),
				#("highRange"),
				#("isLengthRelativeToBaseStrand", true),
				#("faceSurf", false),
				#("numTwistSides"),
				#("globTwist"),
				#("twistRand"),
				#("globFanDir"),
				#("fanRand"),
				#("globLength"),
				#("lengthRand"),
				#("assignGroupID") ) 4
	),
	
	function Test_PushAwayFromSurface =
	(
		OxResetClassParameters (Ox_Push_Away_From_Surface())
		SaveLoadGroom (Ox_Push_Away_From_Surface()) "PushAwayFromSurface" #( 
				#("strandGroupPattern"),
				#("distance") ) 2
	),
	
	function Test_RenderSettings =
	(
		OxResetClassParameters (Ox_Render_Settings())
		SaveLoadGroom (Ox_Render_Settings()) "RenderSettings" #( 
				#("radius"),
				#("groupID") ) 1
	),
	
	function Test_RotateStrands =
	(
		OxResetClassParameters (Ox_Rotate_Strands())
		SaveLoadGroom (Ox_Rotate_Strands()) "RotateStrands" #( 
				#("angle"),
				#("chaos"),
				#("faceCamera", true),
				#("seed"),
				#("strandGroupPattern"),
				#("orientBasedOnStrandShape", true )	) 1
	),
	
	function Test_SurfaceComb =
	(
		OxResetClassParameters (Ox_Surface_Comb())
		SaveLoadGroom (Ox_Surface_Comb()) "SurfaceComb" #( 
				#("affStr", true),
				#("surfChaos"),
				#("seed"),
				#("applyToShape", false),
				#("applyToDirection", false ) ) 1
	),
	
	function Test_Symmetry =
	(
		OxResetClassParameters (Ox_Strand_Symmetry())
		SaveLoadGroom (Ox_Strand_Symmetry()) "Symmetry" #( 
				#("useIgnoreDistance", true),
				#("ignoreDistance") ) 1
	)
)

EUnit.AddTestClass Test_GroomsSavingLoading
ok
