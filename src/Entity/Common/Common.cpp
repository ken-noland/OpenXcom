#include "Name.h"

#include <simplerttr.h>
SIMPLERTTR
{
	SimpleRTTR::Registration().Type<OpenXcom::NameComponent>()
		.Property(&OpenXcom::NameComponent::name, "name");
}
