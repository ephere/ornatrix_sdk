// Must compile with VC 2012 / GCC 4.8

#pragma once

namespace Ephere { namespace Geometry
{

//! Determines how the per-strand transform up directions are computed on base surface
enum class SurfaceTangentComputeMethod
{
	//! The tangents are aligned with the object-space X direction
	ObjectSpace,
	//! First edge of face where coordinate is located is used to calculate the tangents
	FirstEdge,
	//! The tangents are aligned with V texture coordinate
	TextureCoordinate,
	//! Geodesic directions
	Geodesic
};

} }
