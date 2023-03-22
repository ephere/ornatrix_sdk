// Must compile with VC 2012 / GCC 4.8

#pragma once

namespace Ephere { namespace Ornatrix
{

/** This struct holds information about topology of a simple hair strand
*/
struct StrandTopology
{
	//! Index into vertices array
	unsigned startingVertexIndex;

	//! Number of vertices in this strand
	unsigned vertexCount;
};

} }
