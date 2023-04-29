struct Test_MaterialShaders
(
	-- Tests that strand symmetry doubles the number of incoming strands
	function Test_NoCrashOnAssignShader =
	(
		-- Open Material Editor into compact mode
		MatEditor.mode = #basic
		MatEditor.Open() 
		
		local standardMaterial1 = Standardmaterial()
		meditMaterials[1] = standardMaterial1
		
		-- Assign all Ox shader types one by one
		standardMaterial1.shaderType = 5
		standardMaterial1.shaderType = 6
		standardMaterial1.shaderType = 7
		
		-- If no crashing happened by this point we are good
		
		-- Close material editor
		MatEditor.Close()
	)
)

EUnit.AddTestClass Test_MaterialShaders
ok
