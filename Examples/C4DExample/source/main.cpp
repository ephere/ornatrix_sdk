// This is the main file of the CINEMA 4D SDK
//
// When you create your own projects much less code is needed (this file is rather long as it tries to show all kinds of different uses).
//
// An empty project simply looks like this:
//
// #include "c4d.h"
//
// Bool PluginStart()
// {
//   ...do or register something...
//   return true;
// }
//
// void PluginEnd()
// {
// }
//
// Bool PluginMessage(Int32 id, void *data)
// {
//   return false;
// }
//

#include "main.h"

Bool PluginStart()
{
	// shader plugin examples
	if ( !RegisterOrnatrixOrnatrixSdkExampleCommand() )
	{
		ApplicationOutput( "RegisterOrnatrixOrnatrixSdkExampleCommand() failed." );
		return false;
	}

	return true;
}

void PluginEnd()
{
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!g_resource.Init())
				return false;		// don't start plugin without resource

			return true;

		case C4DMSG_PRIORITY:
			// react to this message to set a plugin priority (to determine in which order plugins are initialized or loaded
			// SetPluginPriority(data, mypriority);
			return true;

		case C4DPL_BUILDMENU:
			// react to this message to dynamically enhance the menu
			// EnhanceMainMenu();
			break;

		case C4DPL_COMMANDLINEARGS:
			// sample implementation of command line rendering:
			// CommandLineRendering((C4DPL_CommandLineArgs*)data);

			// react to this message to react to command line arguments on startup
			/*
			{
				C4DPL_CommandLineArgs *args = (C4DPL_CommandLineArgs*)data;
				Int32 i;

				for (i = 0; i<args->argc; i++)
				{
					if (!args->argv[i]) continue;

					if (!strcmp(args->argv[i],"--help") || !strcmp(args->argv[i],"-help"))
					{
						// do not clear the entry so that other plugins can make their output!!!
						ApplicationOutput("\x01-SDK is here :-)");
					}
					else if (!strcmp(args->argv[i],"-SDK"))
					{
						args->argv[i] = nullptr;
						ApplicationOutput("\x01-SDK executed:-)");
					}
					else if (!strcmp(args->argv[i],"-plugincrash"))
					{
						args->argv[i] = nullptr;
						*((Int32*)0) = 1234;
					}
				}
			}
			*/
			break;

		case C4DPL_EDITIMAGE:
			/*{
				C4DPL_EditImage *editimage = (C4DPL_EditImage*)data;
				if (!data) break;
				if (editimage->return_processed) break;
				ApplicationOutput("C4DSDK - Edit Image Hook: "+editimage->imagefn->GetString());
				// editimage->return_processed = true; if image was processed
			}*/
			return false;
	}

	return false;
}
