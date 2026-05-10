#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Math/AABB.h"
#include "Core/Math/Color32.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Transform3D.h"
#include "Core/Math/Vector2.h"
#include "Core/Templates/ID.h"

namespace ho
{
struct MeshIR
{
    static const int32_t sMaxUVChannel = 2;
    static const int32_t sMaxColorChannel = 1;
    static const int32_t sMaxBoneChannel = 4;

    enum class ePrimitiveType
    {
        Point = 0,
        Line,
        LineStrip,
        Triangle,
        TriangleStrip
    };

    struct BoneWeight
    {
        int32_t BoneWeightCount = 0;
        std::array<int32_t, sMaxBoneChannel> BoneIndices = {};
        std::array<float, sMaxBoneChannel> Weights = {};
    };

    struct MorphTarget
    {
        MorphTarget(std::string&& nameStr,
                    std::vector<Vector3>&& positions,
                    std::vector<Vector3>&& normals,
                    std::vector<Vector4>&& tangents,
                    float weight) noexcept
          : NameStr(std::move(nameStr))
          , Positions(std::move(positions))
          , Normals(std::move(normals))
          , Tangents(std::move(tangents))
          , Weight(weight)
        {
            HO_ASSERT((Normals.empty() ? true : (Positions.size() == Normals.size())) &&
                          (Tangents.empty() ? true : (Positions.size() == Tangents.size())),
                      "Invalid MorphTarget format.");
        }

        MorphTarget(MorphTarget&& rhs) noexcept = default;
        MorphTarget& operator=(MorphTarget&& rhs) noexcept = default;

        MorphTarget(const MorphTarget&) = delete;
        MorphTarget& operator=(const MorphTarget&) = delete;

        ~MorphTarget() = default;

        std::string NameStr;
        std::vector<Vector3> Positions;
        std::vector<Vector3> Normals;
        std::vector<Vector4> Tangents;
        float Weight;
    };

    struct SubMesh
    {
        SubMesh(std::string&& nameStr,
                ePrimitiveType primitiveType,
                std::vector<Vector3>&& positions,
                std::vector<Vector3>&& normals,
                std::vector<Vector4>&& tangents,
                std::vector<std::array<Vector2, sMaxUVChannel>>&& uvs,
                std::vector<std::array<Color32, sMaxColorChannel>>&& colors,
                std::vector<BoneWeight>&& boneWeights,
                std::vector<uint32_t>&& indices,
                std::vector<MorphTarget>&& morphTargets,
                int32_t renderMaterialIndex) noexcept
          : NameStr(std::move(nameStr))
          , PrimitiveType(primitiveType)
          , Positions(std::move(positions))
          , Normals(std::move(normals))
          , Tangents(std::move(tangents))
          , UVs(std::move(uvs))
          , Colors(std::move(colors))
          , BoneWeights(std::move(boneWeights))
          , Indices(std::move(indices))
          , MorphTargets(std::move(morphTargets))
          , RenderMaterialIndex(renderMaterialIndex)
        {
            HO_ASSERT((Normals.empty() ? true : (Positions.size() == Normals.size())) &&
                          (Tangents.empty() ? true : (Positions.size() == Tangents.size())) &&
                          (UVs.empty() ? true : (Positions.size() == UVs.size())) &&
                          (Colors.empty() ? true : (Positions.size() == Colors.size())) &&
                          (BoneWeights.empty() ? true : (Positions.size() == BoneWeights.size())),
                      "Invalid SubMesh format.");

            if (!Positions.empty())
            {
                Aabb = AABB::FromPositions(Positions.data(), static_cast<int32_t>(Positions.size()));
                Sphere = Sphere::FromPositionsRitter(Positions.data(), static_cast<int32_t>(Positions.size()));
            }
            MorphNameToIndex.reserve(MorphTargets.size());
            for (int32_t i = 0; i < static_cast<int32_t>(MorphTargets.size()); ++i)
            {
                MorphNameToIndex[MorphTargets[i].NameStr] = i;
            }
        }

        SubMesh(SubMesh&& rhs) noexcept = default;
        SubMesh& operator=(SubMesh&& rhs) noexcept = default;

        SubMesh(const SubMesh&) = delete;
        SubMesh& operator=(const SubMesh&) = delete;

        ~SubMesh() = default;

        FORCE_INLINE const MorphTarget& GetMorphTarget(const std::string& nameStr) const
        {
            auto it = MorphNameToIndex.find(nameStr);
            HO_ASSERT(it != MorphNameToIndex.end(),
                      (std::string("There is no morph target ") + nameStr + "in submesh " + NameStr).c_str());
            return MorphTargets[it->second];
        }

        FORCE_INLINE int32_t GetMorphTargetIndex(const std::string& nameStr) const
        {
            auto it = MorphNameToIndex.find(nameStr);
            HO_ASSERT(it != MorphNameToIndex.end(),
                      (std::string("There is no morph target ") + nameStr + "in submesh " + NameStr).c_str());
            return it->second;
        }

        FORCE_INLINE int32_t GetMorphTargetCount() const
        {
            return static_cast<int32_t>(MorphTargets.size());
        }

        std::string NameStr;
        ePrimitiveType PrimitiveType;

        std::vector<Vector3> Positions;
        std::vector<Vector3> Normals;
        std::vector<Vector4> Tangents;

        std::vector<std::array<Vector2, sMaxUVChannel>> UVs;
        std::vector<std::array<Color32, sMaxColorChannel>> Colors;
        std::vector<BoneWeight> BoneWeights;

        std::vector<uint32_t> Indices;

        std::vector<MorphTarget> MorphTargets;

        // Index of ModelIR's pMaterialIRs.
        int32_t RenderMaterialIndex;

        AABB Aabb;
        Sphere Sphere;

        std::unordered_map<std::string, int32_t> MorphNameToIndex;
    };

    MeshIR(std::string&& nameStr, std::vector<SubMesh>&& subMeshes) noexcept
      : NameStr(std::move(nameStr))
      , SubMeshes(std::move(subMeshes))
    {
        if (!SubMeshes.empty())
        {
            // Merge all submesh bounding volume
            Aabb = SubMeshes[0].Aabb;
            for (int32_t i = 1; i < static_cast<int32_t>(SubMeshes.size()); ++i)
            {
                Aabb = AABB::Merge(Aabb, SubMeshes[i].Aabb);
            }

            std::vector<Vector3> allPositions;
            int32_t allVertexCount = 0;
            for (const auto& sm : SubMeshes)
            {
                allVertexCount += static_cast<int32_t>(sm.Positions.size());
            }
            allPositions.reserve(allVertexCount);
            for (const auto& sm : SubMeshes)
            {
                allPositions.insert(allPositions.end(), sm.Positions.begin(), sm.Positions.end());
            }
            if (!allPositions.empty())
            {
                Sphere = Sphere::FromPositionsRitter(allPositions.data(), static_cast<int32_t>(allPositions.size()));
            }
        }
        SubMeshNameToIndex.reserve(SubMeshes.size());
        for (int32_t i = 0; i < static_cast<int32_t>(SubMeshes.size()); ++i)
        {
            SubMeshNameToIndex[SubMeshes[i].NameStr] = i;
        }
    }

    MeshIR(MeshIR&& rhs) noexcept = default;
    MeshIR& operator=(MeshIR&& rhs) noexcept = default;

    MeshIR(const MeshIR&) = delete;
    MeshIR& operator=(const MeshIR&) = delete;

    FORCE_INLINE const SubMesh& GetSubMesh(const std::string& nameStr) const
    {
        auto it = SubMeshNameToIndex.find(nameStr);
        HO_ASSERT(it != SubMeshNameToIndex.end(),
                  (std::string("There is no submesh ") + nameStr + "in mesh " + NameStr).c_str());
        return SubMeshes[it->second];
    }

    FORCE_INLINE int32_t GetSubMeshIndex(const std::string& nameStr) const
    {
        auto it = SubMeshNameToIndex.find(nameStr);
        HO_ASSERT(it != SubMeshNameToIndex.end(),
                  (std::string("There is no submesh ") + nameStr + "in mesh " + NameStr).c_str());
        return it->second;
    }

    FORCE_INLINE int32_t GetSubMeshCount() const
    {
        return static_cast<int32_t>(SubMeshes.size());
    }

    std::string NameStr;

    std::vector<SubMesh> SubMeshes;

    AABB Aabb;
    Sphere Sphere;

    std::unordered_map<std::string, int32_t> SubMeshNameToIndex;
};

} // namespace ho
