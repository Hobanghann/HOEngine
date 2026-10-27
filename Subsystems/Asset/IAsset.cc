#include "IAsset.h"

#include "Subsystems/Reflection/ClassRegister.h"

namespace ho
{
HO_REGISTER_CLASS_DERIVED_BEGIN(RIAsset, "Asset", RObject)
HO_REGISTER_CLASS_PROPERTY(hName, "Name")
HO_REGISTER_CLASS_END()
} // namespace ho