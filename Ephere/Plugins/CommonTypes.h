// Must compile with VC 2012 / GCC 4.8

#pragma once

#if defined( AUTODESK_3DSMAX )
#include "Ephere/Plugins/AutodeskMaxTypes.h"
#elif defined( AUTODESK_MAYA )
#include "Ephere/Plugins/AutodeskMayaTypes.h"
#elif defined( UNITY )
#include "Ephere/Plugins/UnityTypes.h"
#elif defined( SIDEFX_HOUDINI )
#include "Ephere/Plugins/SideFxHoudiniTypes.h"
#elif defined( MAXON_C4D )
#include "Ephere/Plugins/MaxonC4DTypes.h"
#elif defined( STANDALONE_ORNATRIX )
#include "Ephere/Plugins/EphereTypes.h"
#else
#error Ornatrix plugin error: Unknown host.
#endif
