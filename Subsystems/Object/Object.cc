#include "Object.h"

#include "ObjectHandle.h"
#include "Subsystems/Reflection/ClassRegister.h"
#include "Subsystems/Reflection/ReflectionSystem.h"

namespace ho
{
HO_REGISTER_CLASS_BEGIN(RObject, "Object")
HO_REGISTER_CLASS_PROPERTY(mGlobalID, "Global ID")
HO_REGISTER_CLASS_END()
} // namespace ho
