// Must compile with VC 2012 / GCC 4.8

#pragma once

namespace Ephere { namespace Plugins { namespace Ornatrix
{

class IHair__deprecated;

//! Any object which contains a hair instance
class IHairContainer
{
public:

	/** Gets the hair instance inside this container or nullptr if there is none
		@return Hair interface instance
	*/
	virtual IHair__deprecated* GetHair__deprecated() = 0;

protected:

	~IHairContainer()
	{
	}
};

} } } // Ephere::Plugins::Ornatrix
