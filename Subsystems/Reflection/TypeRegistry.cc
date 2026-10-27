#include "TypeRegistry.h"

#include "Type.h"

namespace ho
{
TypeRegistry::~TypeRegistry()
{
    delete mpType;
}
} // namespace ho