#include "EditorUtils.h"

#include <charconv>
#include <vector>

#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
std::string getUniqueGameObjectName(ObjectHandle<const RScene> hScene, const std::string& baseNameStr)
{
    static const int32_t sMaxDupCount = 2048;
    static std::vector<bool> sbIsNumUsed(sMaxDupCount);

    if (hScene.IsNULL())
    {
        HO_ASSERT(false, "Invalid scene.");
        return baseNameStr;
    }

    std::fill(sbIsNumUsed.begin(), sbIsNumUsed.end(), false);
    int32_t maxNum = -1;
    for (const auto& hGameObj : hScene->GetGameObjects<RGameObject>())
    {
        std::string objNameStr(*hGameObj->GetName());
        if (baseNameStr.size() > objNameStr.size())
        {
            continue;
        }

        const std::string::size_type idx = objNameStr.find(baseNameStr);
        if (idx != 0)
        {
            continue;
        }

        int32_t num = -1;
        if (baseNameStr.size() == objNameStr.size())
        {
            num = 0;
        }
        else if (objNameStr.find(" (", idx) == baseNameStr.size() && objNameStr[objNameStr.size() - 1] == ')')
        {
            const char* first = objNameStr.c_str() + baseNameStr.size() + 2;
            const char* last = objNameStr.c_str() + objNameStr.size() - 1;

            int value = 0;
            auto result = std::from_chars(first, last, value);

            if (result.ec == std::errc{} && result.ptr == last)
            {
                num = value;
            }
        }

        HO_ASSERT(num < sMaxDupCount, "Duplicated object count exceeds maximum limit.");
        if (num >= 0)
        {
            sbIsNumUsed[num] = true;
            if (maxNum < num)
            {
                maxNum = num;
            }
        }
    }

    int32_t targetNum = -1;
    for (int32_t i = 0; i <= maxNum; ++i)
    {
        if (!sbIsNumUsed[i])
        {
            targetNum = i;
            break;
        }
    }

    if (targetNum == -1)
    {
        targetNum = maxNum + 1;
    }

    if (targetNum == 0)
    {
        return baseNameStr;
    }

    return std::string(baseNameStr).append(" (").append(std::to_string(targetNum)).append(")");
}

std::string getUniqueAssetName(const std::string& baseNameStr)
{
    static const int32_t sMaxDupCount = 2048;
    static std::vector<bool> sbIsNumUsed(sMaxDupCount);

    std::fill(sbIsNumUsed.begin(), sbIsNumUsed.end(), false);
    int32_t maxNum = -1;
    for (const auto& hAsset : AssetSystem::GetInstance().GetAssets<RIAsset>())
    {
        std::string objNameStr(*hAsset->hName);
        if (baseNameStr.size() > objNameStr.size())
        {
            continue;
        }

        const std::string::size_type idx = objNameStr.find(baseNameStr);
        if (idx != 0)
        {
            continue;
        }

        int32_t num = -1;
        if (baseNameStr.size() == objNameStr.size())
        {
            num = 0;
        }
        else if (objNameStr.find(" (", idx) == baseNameStr.size() && objNameStr[objNameStr.size() - 1] == ')')
        {
            const char* first = objNameStr.c_str() + baseNameStr.size() + 2;
            const char* last = objNameStr.c_str() + objNameStr.size() - 1;

            int value = 0;
            auto result = std::from_chars(first, last, value);

            if (result.ec == std::errc{} && result.ptr == last)
            {
                num = value;
            }
        }

        HO_ASSERT(num < sMaxDupCount, "Duplicated object count exceeds maximum limit.");
        if (num >= 0)
        {
            sbIsNumUsed[num] = true;
            if (maxNum < num)
            {
                maxNum = num;
            }
        }
    }

    int32_t targetNum = -1;
    for (int32_t i = 0; i <= maxNum; ++i)
    {
        if (!sbIsNumUsed[i])
        {
            targetNum = i;
            break;
        }
    }

    if (targetNum == -1)
    {
        targetNum = maxNum + 1;
    }

    if (targetNum == 0)
    {
        return baseNameStr;
    }

    return std::string(baseNameStr).append(" (").append(std::to_string(targetNum)).append(")");
}
} // namespace ho