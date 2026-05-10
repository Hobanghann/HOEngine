#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>

#include "ID.h"
#include "Macros.h"

namespace ho
{
template <typename IDType, typename ValueType>
class IDMap
{
  public:
    IDMap()
      : mBaseID()
      , mIDtoValue()
    {
    }

    template <typename U = ValueType>
    IDType Add(U&& v)
    {
        static_assert(std::is_base_of_v<ValueType, std::decay_t<U>>);

        mBaseID.Increment();
        const IDType id = mBaseID;

        mIDtoValue.emplace(id, std::make_unique<std::decay_t<U>>(std::forward<U>(v)));

        return id;
    }

    template <typename U = ValueType, typename... Args>
    IDType Emplace(Args&&... args)
    {
        mBaseID.Increment();
        const IDType id = mBaseID;
        mIDtoValue.emplace(id, std::make_unique<U>(std::forward<Args>(args)...));
        return id;
    }

    bool Remove(const IDType& id)
    {
        if (id.IsNULL())
        {
            return false;
        }
        return mIDtoValue.erase(id) > 0;
    }

    [[nodiscard]] ValueType* Find(const IDType& id)
    {
        if (id.IsNULL())
        {
            return nullptr;
        }
        auto it = mIDtoValue.find(id);
        return (it == mIDtoValue.end()) ? nullptr : it->second.get();
    }

    [[nodiscard]] const ValueType* Find(const IDType& id) const
    {
        if (id.IsNULL())
        {
            return nullptr;
        }
        auto it = mIDtoValue.find(id);
        return (it == mIDtoValue.end()) ? nullptr : it->second.get();
    }

    [[nodiscard]] bool Has(const IDType& id) const
    {
        return !id.IsNULL() && mIDtoValue.find(id) != mIDtoValue.end();
    }

    void Clear()
    {
        mIDtoValue.clear();
    }

    int32_t GetSize() const
    {
        return static_cast<int32_t>(mIDtoValue.size());
    }

    void GetAllValues(std::vector<ValueType*>* pOutVector) const
    {
        HO_ASSERT(pOutVector, "pOutVector is null.");
        pOutVector->clear();
        pOutVector->reserve(mIDtoValue.size());
        for (const auto& [_, pValue] : mIDtoValue)
        {
            pOutVector->push_back(pValue.get());
        }
    }

  private:
    IDType mBaseID;
    std::unordered_map<IDType, std::unique_ptr<ValueType>> mIDtoValue;
};
} // namespace ho