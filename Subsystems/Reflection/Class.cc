#include "Class.h"

#include "IReflectable.h"

namespace ho
{
IReflectable* Class::New() const
{
    return mpInstantiator();
}

IReflectable* Class::NewAt(void* ptr) const
{
    HO_ASSERT(ptr, "Destination is null.");

    return mpInPlaceInstantiator(ptr);
}

void Class::Destroy(IReflectable* pInstance) const
{
    HO_ASSERT(pInstance, "Instance is null.");
    mpDestroyer(pInstance);
}

void Class::Delete(IReflectable* pInstance) const
{
    HO_ASSERT(pInstance, "Instance is null.");
    mpDeleter(pInstance);
}
} // namespace ho