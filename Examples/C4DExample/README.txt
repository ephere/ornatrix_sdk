
ORNATRIX SDK FOR CINEMA 4D R19/R20

Instructions to build the example project for C4D R20:

- Unzip Cinema 4D sdk.zip and build the frameworks projects following Maxon docs:

https://developers.maxon.net/docs/Cinema4DCPPSDK/html/page_maxonapi_sdk.html

- Copy OrnatrixSdk to your Cinema 4D plugins directory

- Download and install the Maxon Project Tool

Project Tool Download:
https://developers.maxon.net/?page_id=1118

Project Tool docs:
https://developers.maxon.net/docs/Cinema4DCPPSDK/html/page_maxonapi_projecttool.html

- Edit the generator scripts and set TOOL_PATH and FRAMEWORKS_PATH

OSX:	OrnatrixSdk/C4DExample/generate.sh
Win:	OrnatrixSdk/C4DExample/generate.cmd

- Run the scripts from command line to generate the example project

- Open the example project and build:

OSX (XCode 10):	OrnatrixSdk/C4DExample/project/c4dexample.xcodeproj
Win (VS 2015):	OrnatrixSdk/C4DExample/project/c4dexample.c4dexample.vcxproj

- If you get the MSB8013 error in VS, the frameworks references may be broken. Add manually to the solution (Solution, Add, Existing Project) and update the references (c4dexample, References, Add Reference)

frameworks\core.framework\project\core.framework.vcxproj
frameworks\cinema.framework\project\cinema.framework.vcxproj
frameworks\mesh_misc.framework\project\mesh_misc.framework.vcxproj

- Start Cinema 4D and create some hair with Ornatrix (Plugins menu > Ornatrix > Add Hair > Fur Ball)

- Select the Ornatrix HairObject

- To display the strand count, run from the Python console:

c4d.CallCommand(1000010)

- To create a spline object containing all the hair strands, run from the Python console:

c4d.CallCommand(1000010,1)

- If a Mesh From Strands modifier is present, the Hair Object can extract a PolgonObject. Run from the Python console:

c4d.CallCommand(1000010,2)

- Make sure to use your own plugin ID on your implementation. 1000010 is a generic development id.
How to get plugin IDs: https://developers.maxon.net/?page_id=3224
