#include <gtest/gtest.h>
#include <unordered_set>

#include "Core/IO/AnimationIR.h"
#include "Core/IO/MaterialIR.h"
#include "Core/IO/MeshIR.h"
#include "Core/IO/ModelIR.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "Core/IO/ShaderIR.h"
#include "Core/IO/SkeletonIR.h"
#include "Core/IO/SkinIR.h"
#include "Core/IO/TextureIR.h"

using namespace ho;
using namespace ho::parser;

TEST(ResourceParseFuncsTest, parseModelFile_OBJ_VerifiesFullSceneHierarchyAndMaterialAttributes)
{
    Path path(std::string("Test/Core/IO/Assets/TestObj/Cube/cube.obj"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("TestCube"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();

    // Check Mesh
    for (const MeshIR::SubMesh& subMeshIR : pMeshIR->SubMeshes)
    {
        EXPECT_GT(subMeshIR.Positions.size(), 0);

        EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Normals.size());

        EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Tangents.size());

        EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.UVs.size());

        EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Colors.size());

        EXPECT_EQ(subMeshIR.BoneWeights.size(), 0);

        for (uint32_t idx : subMeshIR.Indices)
        {
            EXPECT_LT(idx, subMeshIR.Positions.size());
        }

        EXPECT_EQ(subMeshIR.PrimitiveType, MeshIR::ePrimitiveType::Triangle);
        EXPECT_EQ(subMeshIR.Indices.size() % 3, 0);

        EXPECT_EQ(subMeshIR.MorphTargets.size(), subMeshIR.MorphNameToIndexMap.size());

        EXPECT_TRUE(subMeshIR.RenderMaterialIndex != -1);
        EXPECT_LE(subMeshIR.Aabb.MinEdges.X, subMeshIR.Aabb.MaxEdges.X);
        EXPECT_LE(subMeshIR.Aabb.MinEdges.Y, subMeshIR.Aabb.MaxEdges.Y);
        EXPECT_LE(subMeshIR.Aabb.MinEdges.Z, subMeshIR.Aabb.MaxEdges.Z);

        EXPECT_TRUE(subMeshIR.Sphere.Radius > 0.0f);
    }

    EXPECT_EQ(pMeshIR->SubMeshes.size(), pMeshIR->SubMeshNameToIndexMap.size());

    EXPECT_LE(pMeshIR->Aabb.MinEdges.X, pMeshIR->Aabb.MaxEdges.X);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Y, pMeshIR->Aabb.MaxEdges.Y);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Z, pMeshIR->Aabb.MaxEdges.Z);

    EXPECT_TRUE(pMeshIR->Sphere.Radius > 0.0f);

    // Check MaterialIR
    const MaterialIR* pTexMaterialIR = pModelIR->pMaterialIRs[0].get();
    EXPECT_EQ(pTexMaterialIR->Ambient, Color128(0.001f, 0.001f, 0.001f));
    EXPECT_TRUE(pTexMaterialIR->Diffuse.IsEqualApprox(Color128(0.6f, 0.6f, 0.6f)));
    EXPECT_EQ(pTexMaterialIR->Specular, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pTexMaterialIR->Shininess, 32.0f);
    EXPECT_EQ(pTexMaterialIR->Albedo, Color128(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(pTexMaterialIR->Metallic, 0.0f);
    EXPECT_EQ(pTexMaterialIR->Roughness, 0.5f);
    EXPECT_EQ(pTexMaterialIR->IndexOfRefraction, 1.0f);
    EXPECT_EQ(pTexMaterialIR->Opacity, 1.0f);
    EXPECT_EQ(pTexMaterialIR->AlphaMode, MaterialIR::eAlphaMode::Opaque);
    EXPECT_EQ(pTexMaterialIR->AlphaThreshold, 0.5f);
    EXPECT_EQ(pTexMaterialIR->BlendMode, MaterialIR::eAlphaBlendMode::Default);
    EXPECT_EQ(pTexMaterialIR->Emissive, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pTexMaterialIR->EmissiveIntensity, 1.0f);
    EXPECT_EQ(pTexMaterialIR->TransmissionFactor, 0.0f);
    EXPECT_EQ(pTexMaterialIR->ClearcoatFactor, 0.0f);
    EXPECT_EQ(pTexMaterialIR->ClearcoatRoughness, 0.0f);
    EXPECT_EQ(pTexMaterialIR->SheenColor, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pTexMaterialIR->SheenRoughnessFactor, 0.0f);
    EXPECT_EQ(pTexMaterialIR->VolumeThicknessFactor, 0.0f);
    EXPECT_EQ(pTexMaterialIR->VolumeAttenuationDist, 0.0f);
    EXPECT_EQ(pTexMaterialIR->VolumeAttenuationColor, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pTexMaterialIR->AnisotropyFactor, 0.0f);
    EXPECT_EQ(pTexMaterialIR->AnisotropyRotation, 0.0f);
    EXPECT_EQ(pTexMaterialIR->NormalScale, 1.0f);
    EXPECT_EQ(pTexMaterialIR->OcclusionStrength, 1.0f);
    EXPECT_EQ(pTexMaterialIR->ParallaxScale, 0.05f);
    EXPECT_EQ(pTexMaterialIR->bWireframe, false);
    EXPECT_EQ(pTexMaterialIR->bBackfaceCulling, true);

    const TextureIR* pDiffuseIR =
        pModelIR
            ->pTextureIRs[pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Diffuse)]]
            .get();
    EXPECT_EQ(pDiffuseIR->NameStr, std::string("test_diffuse"));
    const TextureIR* pSpecularIR =
        pModelIR
            ->pTextureIRs[pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Specular)]]
            .get();
    EXPECT_EQ(pSpecularIR->NameStr, std::string("test_specular"));
    const TextureIR* pSpecularHighlight =
        pModelIR
            ->pTextureIRs[pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Shininess)]]
            .get();
    EXPECT_EQ(pSpecularHighlight->NameStr, std::string("test_specular_highlight"));
    const TextureIR* pHeightIR =
        pModelIR->pTextureIRs[pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Height)]]
            .get();
    EXPECT_EQ(pHeightIR->NameStr, std::string("test_bump"));
    const TextureIR* pAlphaIR =
        pModelIR
            ->pTextureIRs[pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Opacity)]]
            .get();
    EXPECT_EQ(pAlphaIR->NameStr, std::string("test_alpha"));
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Albedo)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Emissive)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Metallic)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Roughness)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::MetallicRoughness)] ==
                -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::AmbientOcclusion)] ==
                -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::SubSurface)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Sheen)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Clearcoat)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::ClearcoatNormal)] ==
                -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Transmission)] == -1);
    EXPECT_TRUE(pTexMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Anisotropy)] == -1);

    const MaterialIR* pAttrMaterialIR = pModelIR->pMaterialIRs[1].get();

    EXPECT_EQ(pAttrMaterialIR->Ambient, Color128(0.2f, 0.2f, 0.2f));
    EXPECT_EQ(pAttrMaterialIR->Diffuse, Color128(0.1f, 0.1f, 0.1f));
    EXPECT_EQ(pAttrMaterialIR->Specular, Color128(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(pAttrMaterialIR->Shininess, 10.0f);
    EXPECT_EQ(pAttrMaterialIR->Albedo, Color128(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(pAttrMaterialIR->Metallic, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->Roughness, 0.5f);
    EXPECT_EQ(pAttrMaterialIR->IndexOfRefraction, 1.5f);
    EXPECT_EQ(pAttrMaterialIR->Opacity, 0.5f);
    EXPECT_EQ(pAttrMaterialIR->AlphaMode, MaterialIR::eAlphaMode::Blend);
    EXPECT_EQ(pAttrMaterialIR->AlphaThreshold, 0.5f);
    EXPECT_EQ(pAttrMaterialIR->BlendMode, MaterialIR::eAlphaBlendMode::Default);
    EXPECT_EQ(pAttrMaterialIR->Emissive, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pAttrMaterialIR->EmissiveIntensity, 1.0f);
    EXPECT_EQ(pAttrMaterialIR->TransmissionFactor, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->ClearcoatFactor, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->ClearcoatRoughness, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->SheenColor, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pAttrMaterialIR->SheenRoughnessFactor, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->VolumeThicknessFactor, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->VolumeAttenuationDist, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->VolumeAttenuationColor, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pAttrMaterialIR->AnisotropyFactor, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->AnisotropyRotation, 0.0f);
    EXPECT_EQ(pAttrMaterialIR->NormalScale, 1.0f);
    EXPECT_EQ(pAttrMaterialIR->OcclusionStrength, 1.0f);
    EXPECT_EQ(pAttrMaterialIR->ParallaxScale, 0.05f);
    EXPECT_EQ(pAttrMaterialIR->bWireframe, false);
    EXPECT_EQ(pAttrMaterialIR->bBackfaceCulling, true);

    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Diffuse)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Specular)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Shininess)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Opacity)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Height)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Normal)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Albedo)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Emissive)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Metallic)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Roughness)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::MetallicRoughness)] ==
                -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::AmbientOcclusion)] ==
                -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::SubSurface)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Sheen)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Clearcoat)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::ClearcoatNormal)] ==
                -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Transmission)] == -1);
    EXPECT_TRUE(pAttrMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Anisotropy)] == -1);
}

TEST(ResourceParseFuncsTest, parseModelFile_OBJ_DeduplicatesIdenticalTextures)
{
    Path path(std::string("Test/Core/IO/Assets/TestObj/CubeWithDuplicatedTexture/cube.obj"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("TestCube"), false);
    ASSERT_NE(pModelIR, nullptr);

    EXPECT_EQ(pModelIR->pTextureIRs.size(), 1);

    const MaterialIR* pMaterialIR = pModelIR->pMaterialIRs[0].get();
    const TextureIR* pDiffuseIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Diffuse)]]
            .get();
    const TextureIR* pSpecularIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Specular)]]
            .get();
    const TextureIR* pSpecularHighlightIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Specular)]]
            .get();

    const TextureIR* pNormalIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Height)]]
            .get();
    const TextureIR* pAlphaIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Opacity)]]
            .get();

    EXPECT_EQ(pDiffuseIR, pSpecularIR);
    EXPECT_EQ(pDiffuseIR, pSpecularHighlightIR);
    EXPECT_EQ(pDiffuseIR, pNormalIR);
    EXPECT_EQ(pDiffuseIR, pAlphaIR);
}

TEST(ResourceParseFuncsTest, parseModelFile_OBJ_CorrectlyParsesMultipleSubMeshes)
{
    Path path(std::string("Test/Core/IO/Assets/TestObj/MultipleMesh/multi_mesh.obj"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("TestCube"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();

    EXPECT_EQ(pMeshIR->SubMeshes.size(), 2);

    EXPECT_EQ(pMeshIR->SubMeshes.size(), pMeshIR->SubMeshNameToIndexMap.size());

    std::vector<const MaterialIR*> pMetalMaterialIRs;
    std::vector<const MaterialIR*> pPlasticMaterialIRs;

    for (const auto& subMeshIR : pMeshIR->SubMeshes)
    {
        const MaterialIR* pMaterialIR = pModelIR->pMaterialIRs[subMeshIR.RenderMaterialIndex].get();
        ASSERT_NE(pMaterialIR, nullptr);

        if (pMaterialIR->NameStr == "metal")
        {
            pMetalMaterialIRs.push_back(pMaterialIR);
        }
        else if (pMaterialIR->NameStr == "plastic")
        {
            pPlasticMaterialIRs.push_back(pMaterialIR);
        }
    }

    EXPECT_GE(pMetalMaterialIRs.size(), 1);
    EXPECT_GE(pPlasticMaterialIRs.size(), 1);

    for (int32_t i = 1; i < static_cast<int32_t>(pMetalMaterialIRs.size()); ++i)
    {
        EXPECT_EQ(pMetalMaterialIRs[0], pMetalMaterialIRs[i]);
    }

    EXPECT_EQ(pModelIR->pTextureIRs.size(), 1);

    const MaterialIR* pMetalMaterialIR = nullptr;
    const MaterialIR* pPlasticMaterialIR = nullptr;
    for (int32_t i = 0; i < static_cast<int32_t>(pModelIR->pMaterialIRs.size()); ++i)
    {
        const MaterialIR* pMaterialIR = pModelIR->pMaterialIRs[i].get();
        ASSERT_NE(pMaterialIR, nullptr);

        if (pMaterialIR->NameStr == "metal")
        {
            pMetalMaterialIR = pMaterialIR;
        }
        else if (pMaterialIR->NameStr == "plastic")
        {
            pPlasticMaterialIR = pMaterialIR;
        }
    }

    const TextureIR* pMetalDiffuseIR =
        pModelIR
            ->pTextureIRs[pMetalMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Diffuse)]]
            .get();
    const TextureIR* pPlasticDiffuseIR =
        pModelIR
            ->pTextureIRs[pPlasticMaterialIR
                              ->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Diffuse)]]
            .get();

    EXPECT_EQ(pMetalDiffuseIR, pPlasticDiffuseIR);
    EXPECT_EQ(pMetalDiffuseIR->NameStr, "test_diffuse");

    EXPECT_NE(pMetalMaterialIR->Shininess, pPlasticMaterialIR->Shininess);
    EXPECT_GT(pMetalMaterialIR->Shininess, pPlasticMaterialIR->Shininess);

    EXPECT_NE(pMetalMaterialIR->Specular, pPlasticMaterialIR->Specular);
}

TEST(ResourceParseFuncsTest, parseModelFile_glTFBox_ValidatesStandardSpecification)
{
    Path path(std::string("Test/Core/IO/Assets/TestGltf/Box/glTF/Box.gltf"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("TestBox"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();

    // Check Mesh
    EXPECT_EQ(pMeshIR->SubMeshes.size(), 1);
    EXPECT_EQ(pMeshIR->SubMeshes[0].NameStr, std::string("Mesh"));

    const MeshIR::SubMesh& subMeshIR = pMeshIR->SubMeshes[0];

    EXPECT_EQ(subMeshIR.Positions.size(), 24);

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Normals.size());

    EXPECT_TRUE(subMeshIR.Tangents.empty());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.UVs.size());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Colors.size());

    EXPECT_TRUE(subMeshIR.BoneWeights.empty());

    for (uint32_t idx : subMeshIR.Indices)
    {
        EXPECT_LT(idx, subMeshIR.Positions.size());
    }

    EXPECT_EQ(subMeshIR.PrimitiveType, MeshIR::ePrimitiveType::Triangle);
    EXPECT_EQ(subMeshIR.Indices.size() % 3, 0);

    EXPECT_TRUE(subMeshIR.MorphTargets.empty());
    EXPECT_TRUE(subMeshIR.MorphNameToIndexMap.empty());

    EXPECT_TRUE(subMeshIR.RenderMaterialIndex != -1);
    EXPECT_LT(subMeshIR.RenderMaterialIndex, pModelIR->pMaterialIRs.size());
    EXPECT_LE(subMeshIR.Aabb.MinEdges.X, subMeshIR.Aabb.MaxEdges.X);
    EXPECT_LE(subMeshIR.Aabb.MinEdges.Y, subMeshIR.Aabb.MaxEdges.Y);
    EXPECT_LE(subMeshIR.Aabb.MinEdges.Z, subMeshIR.Aabb.MaxEdges.Z);

    EXPECT_TRUE(subMeshIR.Sphere.Radius > 0.0f);

    EXPECT_EQ(pMeshIR->SubMeshes.size(), pMeshIR->SubMeshNameToIndexMap.size());

    EXPECT_LE(pMeshIR->Aabb.MinEdges.X, pMeshIR->Aabb.MaxEdges.X);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Y, pMeshIR->Aabb.MaxEdges.Y);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Z, pMeshIR->Aabb.MaxEdges.Z);

    EXPECT_TRUE(pMeshIR->Sphere.Radius > 0.0f);

    // Check Material
    const MaterialIR* pMaterialIR = nullptr;
    for (int32_t i = 0; i < static_cast<int32_t>(pModelIR->pMaterialIRs.size()); ++i)
    {
        if (pModelIR->pMaterialIRs[i]->NameStr == "Red")
        {
            pMaterialIR = pModelIR->pMaterialIRs[i].get();
        }
    }
    ASSERT_NE(pMaterialIR, nullptr);
    EXPECT_EQ(pMaterialIR->Ambient, Color128(0.001f, 0.001f, 0.001f));
    EXPECT_TRUE(pMaterialIR->Diffuse.IsEqualApprox(Color128(0.800000011920929f, 0.0f, 0.0f)));
    EXPECT_EQ(pMaterialIR->Specular, Color128(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(pMaterialIR->Shininess, 32.0f);
    EXPECT_TRUE(pMaterialIR->Albedo.IsEqualApprox(Color128(0.800000011920929f, 0.0f, 0.0f)));
    EXPECT_EQ(pMaterialIR->Metallic, 0.0f);
    EXPECT_EQ(pMaterialIR->Roughness, 1.0f);
    EXPECT_EQ(pMaterialIR->Opacity, 1.0f);
    EXPECT_EQ(pMaterialIR->Emissive, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pMaterialIR->EmissiveIntensity, 1.0f);
    EXPECT_EQ(pMaterialIR->BlendMode, MaterialIR::eAlphaBlendMode::Default);
    EXPECT_EQ(pMaterialIR->bWireframe, false);
    EXPECT_EQ(pMaterialIR->bBackfaceCulling, true);

    // Check SkeletonIR
    const SkeletonIR* pSkeletonIR = pModelIR->pSkeletonIR.get();
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), 1);
    EXPECT_EQ(pSkeletonIR->LocalTransforms.size(), 1);
    EXPECT_EQ(pSkeletonIR->Parents.size(), 1);
    EXPECT_EQ(pSkeletonIR->Children.size(), 1);
    EXPECT_EQ(pSkeletonIR->BoneNameToIndexMap.size(), 1);

    EXPECT_EQ(pSkeletonIR->Parents[0], -1);
    EXPECT_EQ(pSkeletonIR->Children[0].size(), 0);
    Matrix4x4 local_t(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                      Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                      Vector4(0.0f, -1.0f, 0.0f, 0.0f),
                      Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    EXPECT_EQ(pSkeletonIR->LocalTransforms[0], Transform3D(local_t));
    EXPECT_EQ(pSkeletonIR->Parents[0], -1);

    // Check Skin
    const SkinIR* pSkinIR = pModelIR->pSkinIR.get();
    EXPECT_EQ(pSkinIR->OffsetTransforms.size(), 1);
    EXPECT_EQ(pSkinIR->OffsetTransforms[0], Transform3D());
}

// FIXME: Temporarily disabled due to Assimp error in BoxInterleaved.gltf.
// Check if Assimp's post-processing flags need adjustment for interleaved data.
TEST(ResourceParseFuncsTest, parseModelFile_glTFBoxInterleaved_ValidatesStandardSpecification)
{
    Path path(std::string("Test/Core/IO/Assets/TestGltf/BoxInterleaved/glTF/BoxInterleaved.gltf"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("BoxInterleaved"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();

    // Check Mesh
    EXPECT_EQ(pMeshIR->SubMeshes.size(), 1);
    EXPECT_EQ(pMeshIR->SubMeshes[0].NameStr, std::string("Mesh"));

    const MeshIR::SubMesh& subMeshIR = pMeshIR->SubMeshes[0];
    EXPECT_EQ(subMeshIR.Positions.size(), 24);

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Normals.size());

    EXPECT_TRUE(subMeshIR.Tangents.empty());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.UVs.size());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Colors.size());

    EXPECT_TRUE(subMeshIR.BoneWeights.empty());

    for (uint32_t idx : subMeshIR.Indices)
    {
        EXPECT_LT(idx, subMeshIR.Positions.size());
    }

    EXPECT_EQ(subMeshIR.PrimitiveType, MeshIR::ePrimitiveType::Triangle);
    EXPECT_EQ(subMeshIR.Indices.size() % 3, 0);

    EXPECT_TRUE(subMeshIR.MorphTargets.empty());
    EXPECT_TRUE(subMeshIR.MorphNameToIndexMap.empty());

    EXPECT_TRUE(subMeshIR.RenderMaterialIndex != -1);
    EXPECT_LT(subMeshIR.RenderMaterialIndex, pModelIR->pMaterialIRs.size());
    EXPECT_LE(subMeshIR.Aabb.MinEdges.X, subMeshIR.Aabb.MaxEdges.X);
    EXPECT_LE(subMeshIR.Aabb.MinEdges.Y, subMeshIR.Aabb.MaxEdges.Y);
    EXPECT_LE(subMeshIR.Aabb.MinEdges.Z, subMeshIR.Aabb.MaxEdges.Z);

    EXPECT_TRUE(subMeshIR.Sphere.Radius > 0.0f);

    EXPECT_EQ(pMeshIR->SubMeshes.size(), pMeshIR->SubMeshNameToIndexMap.size());

    EXPECT_LE(pMeshIR->Aabb.MinEdges.X, pMeshIR->Aabb.MaxEdges.X);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Y, pMeshIR->Aabb.MaxEdges.Y);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Z, pMeshIR->Aabb.MaxEdges.Z);

    EXPECT_TRUE(pMeshIR->Sphere.Radius > 0.0f);

    // Check Material
    const MaterialIR* pMaterialIR = nullptr;
    for (int32_t i = 0; i < static_cast<int32_t>(pModelIR->pMaterialIRs.size()); ++i)
    {
        if (pModelIR->pMaterialIRs[i]->NameStr == "BoxInterleaved_unnamed_material_0")
        {
            pMaterialIR = pModelIR->pMaterialIRs[i].get();
        }
    }
    ASSERT_NE(pMaterialIR, nullptr);
    EXPECT_EQ(pMaterialIR->Ambient, Color128(0.001f, 0.001f, 0.001f));
    EXPECT_TRUE(pMaterialIR->Diffuse.IsEqualApprox(Color128(0.800000011920929f, 0.0f, 0.0f)));
    EXPECT_EQ(pMaterialIR->Specular, Color128(1.0f, 1.0f, 1.0f));
    EXPECT_EQ(pMaterialIR->Shininess, 32.0f);
    EXPECT_TRUE(pMaterialIR->Albedo.IsEqualApprox(Color128(0.800000011920929f, 0.0f, 0.0f)));
    EXPECT_EQ(pMaterialIR->Metallic, 1.0f);
    EXPECT_EQ(pMaterialIR->Roughness, 1.0f);
    EXPECT_EQ(pMaterialIR->Opacity, 1.0f);
    EXPECT_EQ(pMaterialIR->Emissive, Color128(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(pMaterialIR->EmissiveIntensity, 1.0f);
    EXPECT_EQ(pMaterialIR->BlendMode, MaterialIR::eAlphaBlendMode::Default);
    EXPECT_EQ(pMaterialIR->bWireframe, false);
    EXPECT_EQ(pMaterialIR->bBackfaceCulling, true);

    // Check SkeletonIR
    const SkeletonIR* pSkeletonIR = pModelIR->pSkeletonIR.get();
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), 1);
    EXPECT_EQ(pSkeletonIR->LocalTransforms.size(), 1);
    EXPECT_EQ(pSkeletonIR->Parents.size(), 1);
    EXPECT_EQ(pSkeletonIR->Children.size(), 1);
    EXPECT_EQ(pSkeletonIR->BoneNameToIndexMap.size(), 1);

    EXPECT_EQ(pSkeletonIR->Parents[0], -1);
    EXPECT_EQ(pSkeletonIR->Children[0].size(), 0);
    Matrix4x4 local_t(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                      Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                      Vector4(0.0f, -1.0f, 0.0f, 0.0f),
                      Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    EXPECT_EQ(pSkeletonIR->LocalTransforms[0], Transform3D(local_t));

    // Check Skin
    const SkinIR* pSkinIR = pModelIR->pSkinIR.get();
    EXPECT_EQ(pSkinIR->OffsetTransforms.size(), 1);
    EXPECT_EQ(pSkinIR->OffsetTransforms[0], Transform3D());
}

TEST(ResourceParseFuncsTest, parseModelFile_glTFBoxVertexColors_ValidatesStandardSpecification)
{
    Path path(std::string("Test/Core/IO/Assets/TestGltf/BoxVertexColors/glTF/BoxVertexColors.gltf"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("BoxVertexColors"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();

    ASSERT_EQ(pMeshIR->SubMeshes.size(), 1);
    const auto& subMeshIR = pMeshIR->SubMeshes[0];

    EXPECT_GT(subMeshIR.Positions.size(), 0);
    EXPECT_FALSE(subMeshIR.Colors.empty());
    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Colors.size());

    EXPECT_TRUE(subMeshIR.RenderMaterialIndex != -1);
}

TEST(ResourceParseFuncsTest, parseModelFile_glTFBoxTextured_ValidatesStandardSpecification)
{
    Path path(std::string("Test/Core/IO/Assets/TestGltf/BoxTextured/glTF/BoxTextured.gltf"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("BoxTextured"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();

    EXPECT_EQ(pMeshIR->SubMeshes.size(), 1);
    const MeshIR::SubMesh& subMeshIR = pMeshIR->SubMeshes[0];

    // Check Mesh
    EXPECT_EQ(subMeshIR.Positions.size(), 24);

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Normals.size());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Tangents.size());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.UVs.size());

    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.Colors.size());

    EXPECT_TRUE(subMeshIR.BoneWeights.empty());

    for (uint32_t idx : subMeshIR.Indices)
    {
        EXPECT_LT(idx, subMeshIR.Positions.size());
    }

    EXPECT_EQ(subMeshIR.PrimitiveType, MeshIR::ePrimitiveType::Triangle);
    EXPECT_EQ(subMeshIR.Indices.size() % 3, 0);

    EXPECT_TRUE(subMeshIR.MorphTargets.empty());
    EXPECT_TRUE(subMeshIR.MorphNameToIndexMap.empty());

    EXPECT_TRUE(subMeshIR.RenderMaterialIndex != -1);
    EXPECT_LT(subMeshIR.RenderMaterialIndex, pModelIR->pMaterialIRs.size());
    EXPECT_LE(subMeshIR.Aabb.MinEdges.X, subMeshIR.Aabb.MaxEdges.X);
    EXPECT_LE(subMeshIR.Aabb.MinEdges.Y, subMeshIR.Aabb.MaxEdges.Y);
    EXPECT_LE(subMeshIR.Aabb.MinEdges.Z, subMeshIR.Aabb.MaxEdges.Z);

    EXPECT_TRUE(subMeshIR.Sphere.Radius > 0.0f);

    EXPECT_EQ(pMeshIR->SubMeshes.size(), pMeshIR->SubMeshNameToIndexMap.size());

    EXPECT_LE(pMeshIR->Aabb.MinEdges.X, pMeshIR->Aabb.MaxEdges.X);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Y, pMeshIR->Aabb.MaxEdges.Y);
    EXPECT_LE(pMeshIR->Aabb.MinEdges.Z, pMeshIR->Aabb.MaxEdges.Z);

    EXPECT_TRUE(pMeshIR->Sphere.Radius > 0.0f);

    // Check Material
    EXPECT_TRUE(subMeshIR.RenderMaterialIndex != -1);
    const MaterialIR* pMaterialIR = pModelIR->pMaterialIRs[subMeshIR.RenderMaterialIndex].get();

    const TextureIR* pAlbedoIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Albedo)]]
            .get();
    EXPECT_EQ(pAlbedoIR->NameStr, std::string("test_albedo"));
    const TextureIR* pMetallicRoughnessIR =
        pModelIR
            ->pTextureIRs[pMaterialIR
                              ->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::MetallicRoughness)]]
            .get();
    EXPECT_EQ(pMetallicRoughnessIR->NameStr, std::string("test_metallic_roughness"));
    const TextureIR* pNormal =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Normal)]]
            .get();
    EXPECT_EQ(pNormal->NameStr, std::string("test_normal"));
    const TextureIR* pAmbientOcclusionIR =
        pModelIR
            ->pTextureIRs[pMaterialIR
                              ->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::AmbientOcclusion)]]
            .get();
    EXPECT_EQ(pAmbientOcclusionIR->NameStr, std::string("test_occlusion"));
    const TextureIR* pEmissiveIR =
        pModelIR->pTextureIRs[pMaterialIR->TextureIRIndices[static_cast<int32_t>(MaterialIR::eTextureUsage::Emissive)]]
            .get();
    EXPECT_EQ(pEmissiveIR->NameStr, std::string("test_emissive"));
}

TEST(ResourceParseFuncsTest, parseModelFile_glTFRiggidFiture_ValidatesStandardSpecification)
{
    Path path(std::string("Test/Core/IO/Assets/TestGltf/RiggedFigure/glTF/RiggedFigure.gltf"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("RiggedFigure"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();
    const SkeletonIR* pSkeletonIR = pModelIR->pSkeletonIR.get();
    const SkinIR* pSkinIR = pModelIR->pSkinIR.get();

    const auto& subMeshIR = pMeshIR->SubMeshes[0];
    EXPECT_LT(0, subMeshIR.Positions.size());
    EXPECT_EQ(subMeshIR.Positions.size(), subMeshIR.BoneWeights.size());
    for (const MeshIR::BoneWeight& boneWeight : subMeshIR.BoneWeights)
    {
        float sum = 0.0f;
        for (int32_t bwi = 0; bwi < boneWeight.BoneWeightCount; ++bwi)
        {
            EXPECT_GE(boneWeight.BoneIndices[bwi], 0);
            EXPECT_LT(boneWeight.BoneIndices[bwi], pSkeletonIR->BoneNameStrs.size());
            sum += boneWeight.Weights[bwi];
        }
        EXPECT_NEAR(sum, 1.0f, math::EPSILON_CMP);
    }

    EXPECT_LT(0, pSkeletonIR->BoneNameStrs.size());
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), pSkeletonIR->LocalTransforms.size());
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), pSkeletonIR->Parents.size());
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), pSkeletonIR->Children.size());
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), pSkeletonIR->BoneNameToIndexMap.size());
    EXPECT_EQ(pSkeletonIR->BoneNameStrs.size(), pSkinIR->OffsetTransforms.size());

    // Check Bone Hierarchy

    // Torso
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("torso_joint_1").size(), 3);
    {
        std::unordered_set<std::string> expectedChildren = {"torso_joint_2", "leg_joint_L_1", "leg_joint_R_1"};

        for (uint32_t i = 0; i < 3; ++i)
        {
            const std::string& childNameStr =
                pSkeletonIR->BoneNameStrs[pSkeletonIR->GetChildIndicesByBoneName("torso_joint_1")[i]];
            EXPECT_TRUE(expectedChildren.find(childNameStr) != expectedChildren.end());
        }
    }

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("torso_joint_2"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("torso_joint_2").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("torso_joint_2")[0],
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_3")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("torso_joint_3"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_2")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("torso_joint_3").size(), 3);
    {
        std::unordered_set<std::string> expectedChildren = {"arm_joint_R_1", "arm_joint_L_1", "neck_joint_1"};

        for (uint32_t i = 0; i < 3; ++i)
        {
            const std::string& childNameStr =
                pSkeletonIR->BoneNameStrs[pSkeletonIR->GetChildIndicesByBoneName("torso_joint_3")[0]];
            EXPECT_TRUE(expectedChildren.find(childNameStr) != expectedChildren.end());
        }
    }

    // Neck
    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("neck_joint_1"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_3")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("neck_joint_1").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("neck_joint_1")[0],
              pSkeletonIR->BoneNameToIndexMap.find("neck_joint_2")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("neck_joint_2"),
              pSkeletonIR->BoneNameToIndexMap.find("neck_joint_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("neck_joint_2").size(), 0);

    // Right Arm
    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("arm_joint_R_1"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_3")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_R_1").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_R_1")[0],
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_R_2")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("arm_joint_R_2"),
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_R_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_R_2").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_R_2")[0],
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_R_3")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("arm_joint_R_3"),
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_R_2")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_R_3").size(), 0u);

    // Left Arm
    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("arm_joint_L_1"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_3")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_L_1").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_L_1")[0],
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_L_2")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("arm_joint_L_2"),
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_L_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_L_2").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_L_2")[0],
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_L_3")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("arm_joint_L_3"),
              pSkeletonIR->BoneNameToIndexMap.find("arm_joint_L_2")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("arm_joint_L_3").size(), 0);

    // Right Leg
    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_R_1"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_1").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_1")[0],
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_R_2")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_R_2"),
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_R_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_2").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_2")[0],
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_R_3")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_R_3"),
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_R_2")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_3").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_3")[0],
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_R_5")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_R_5"),
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_R_3")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_R_5").size(), 0);

    // Left Leg
    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_L_1"),
              pSkeletonIR->BoneNameToIndexMap.find("torso_joint_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_1").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_1")[0],
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_L_2")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_L_2"),
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_L_1")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_2").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_2")[0],
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_L_3")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_L_3"),
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_L_2")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_3").size(), 1);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_3")[0],
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_L_5")->second);

    EXPECT_EQ(pSkeletonIR->GetParentIndexByBoneName("leg_joint_L_5"),
              pSkeletonIR->BoneNameToIndexMap.find("leg_joint_L_3")->second);
    EXPECT_EQ(pSkeletonIR->GetChildIndicesByBoneName("leg_joint_L_5").size(), 0);

    // Check Bone Local Transform
    Matrix3x3 scale;
    Matrix3x3 rotate;
    Matrix3x3 rotateScale;
    Vector3 translate;

    Matrix4x4 localTransform;

    // torso_joint_1
    scale = Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0000001192092896f));
    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.03792940452694893f, -0.002913428470492363f, -0.00011058452219003811f, -0.9992762207984924f));
    rotateScale = rotate * scale;
    translate = Vector3(2.7939699442924854e-9f, -1.4156600514070309e-7f, -0.6860002279281616f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("torso_joint_1").Matrix;
    Matrix3x3 temp = localTransform.ToMatrix3x3();
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale)) << "Expected:\n"
                                                                         << rotateScale.ToString() << "\nActural:\n"
                                                                         << temp.ToString() << "\n";
    EXPECT_TRUE(Vector3(localTransform.GetCol3()).IsEqualApprox(translate));

    // leg_joint_R_1
    scale = Matrix3x3(
        Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.000000238418579f, 0.0f), Vector3(0.0f, 0.0f, 0.9999999403953552f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.02339874766767025f, 0.6542636752128601f, 0.754464864730835f, 0.046630214899778369f));

    rotateScale = rotate * scale;
    translate = Vector3(-0.06845717132091522f, 0.004460853058844805f, 0.07147114723920822f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_R_1").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_R_2
    scale = Matrix3x3(Vector3(1.0000001192092896f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.21606147289276124f, -0.08108008652925492f, -0.010079992935061457f, -0.9729552268981934f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 0.2661120891571045f, -1.4901200273698124e-8f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_R_2").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_R_3
    scale = Matrix3x3(Vector3(0.9999999403953552f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.8471670746803284f, 0.03204828128218651f, -0.024840382859110833f, -0.5297772288322449f));

    rotateScale = rotate * scale;
    translate = Vector3(-7.450579708745408e-9f, 0.27582409977912905f, 3.725289854372704e-9f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_R_3").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_R_5
    scale = Matrix3x3(Vector3(0.9999999403953552f, 0.0f, 0.0f),
                      Vector3(0.0f, 0.9999997615814208f, 0.0f),
                      Vector3(0.0f, 0.0f, 0.9999991655349731f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.03414325416088104f, 0.3191778957843781f, 0.9461711049079896f, -0.04146831855177879f));

    rotateScale = rotate * scale;
    translate = Vector3(-0.0014585329918190837f, -0.06619873642921448f, -0.027856800705194474f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_R_5").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_L_1
    scale = Matrix3x3(Vector3(1.0000001192092896f, 0.0f, 0.0f),
                      Vector3(0.0f, 1.000000238418579f, 0.0f),
                      Vector3(0.0f, 0.0f, 1.0000001192092896f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.21088078618049625f, 0.6243308186531067f, 0.724772036075592f, -0.2011117935180664f));

    rotateScale = rotate * scale;
    translate = Vector3(0.06761927157640457f, 0.004461091011762619f, 0.07226461172103882f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_L_1").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_L_2
    scale = Matrix3x3(Vector3(1.0000001192092896f, 0.0f, 0.0f),
                      Vector3(0.0f, 1.0000001192092896f, 0.0f),
                      Vector3(0.0f, 0.0f, 0.9999999403953552f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.2111543715000153f, 0.29843246936798098f, -0.04688597097992897f, -0.92959862947464f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 0.2661122083663941f, 0.0f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_L_2").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_L_3
    scale = Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0000001192092896f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.8477688431739807f, 0.002281580353155732f, -0.006338709034025669f, -0.530323326587677f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 0.27582401037216189f, 0.0f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_L_3").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // leg_joint_L_5
    scale = Matrix3x3(
        Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0000007152557374f, 0.0f), Vector3(0.0f, 0.0f, 1.0000009536743165f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.024532083421945573f, 0.3199966549873352f, 0.9446004033088684f, 0.06878151744604111f));

    rotateScale = rotate * scale;
    translate = Vector3(-0.0023464928381145f, -0.06617330759763718f, -0.02785675972700119f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("leg_joint_L_5").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // torso_joint_2
    scale = Matrix3x3(
        Vector3(1.0000001192092896f, 0.0f, 0.0f), Vector3(0.0f, 0.9999999403953552f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.7380747199058533f, 0.001967150717973709f, 0.0021518853027373554f, -0.6747126579284668f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0009999809553846717f, -4.842879874900064e-8f, -0.1714905947446823f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("torso_joint_2").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // torso_joint_3
    scale = Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0000001192092896f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.6378589272499085f, 4.2587172677244209e-10f, -3.5271871534625629e-10f, -0.770153284072876f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 0.21801769733428956f, -3.725289854372704e-9f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("torso_joint_3").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // arm_joint_R_1
    scale = Matrix3x3(
        Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.9999999403953552f, 0.0f), Vector3(0.0f, 0.0f, 0.9999999403953552f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.27643078565597536f, -0.05186379700899124f, -0.665187418460846f, -0.6916804909706116f));

    rotateScale = rotate * scale;
    translate = Vector3(-0.08800055086612702f, -0.0001992879988392815f, 0.0009773969650268557f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("arm_joint_R_1").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // arm_joint_R_2
    scale = Matrix3x3(Vector3(0.9999999403953552f, 0.0f, 0.0f),
                      Vector3(0.0f, 0.9999995827674866f, 0.0f),
                      Vector3(0.0f, 0.0f, 0.9999998211860656f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.2280062586069107f, -0.9096477627754213f, -0.1480233669281006f, -0.3140748739242554f));

    rotateScale = rotate * scale;
    translate = Vector3(-7.450579708745408e-9f, 0.24452559649944304f, 5.96045985901128e-8f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("arm_joint_R_2").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // arm_joint_R_3
    scale = Matrix3x3(
        Vector3(1.0000001192092896f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.9999999403953552f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.07854889333248139f, -0.14253509044647218f, -0.014102266170084477f, -0.9865672588348388f));

    rotateScale = rotate * scale;
    translate = Vector3(-5.96045985901128e-8f, 0.1855168044567108f, 0.0f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("arm_joint_R_3").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // arm_joint_L_1
    scale = Matrix3x3(
        Vector3(1.0000001192092896f, 0.0f, 0.0f), Vector3(0.0f, 1.0000004768371585f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.6789423823356628f, -0.6879449486732483f, -0.24067795276641849f, -0.08856399357318878f));

    rotateScale = rotate * scale;
    translate = Vector3(0.08800055086612702f, -0.0001992879988392815f, 0.0009773969650268557f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("arm_joint_L_1").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // arm_joint_L_2
    scale = Matrix3x3::sIdentity;

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-0.0024000618141144516f, 0.13981154561042789f, -0.2718312442302704f, -0.9521317481994628f));

    rotateScale = rotate * scale;
    translate = Vector3(1.8626500342122655e-9f, 0.24452590942382813f, 5.96045985901128e-8f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("arm_joint_L_2").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    //  arm_joint_L_3
    scale = Matrix3x3(
        Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0000001192092896f, 0.0f), Vector3(0.0f, 0.0f, 1.0000001192092896f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.05729063600301743f, 0.02822729200124741f, -0.0555599257349968f, -0.996410608291626f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 0.1855167001485825f, 0.0f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("arm_joint_L_3").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // neck_joint_1
    scale = Matrix3x3::sIdentity;

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(0.635452151298523f, 1.0367853739773274e-15f, -8.512669473202695e-16f, -0.7721402645111084f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 7.450579886381094e-8f, -0.05255972966551781f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("neck_joint_1").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // neck_joint_2
    scale = Matrix3x3(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0000001192092896f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    rotate = Matrix3x3::FromQuaternion(
        Quaternion(-4.2157907720330458e-10f, -0.9999844431877136f, -0.005583992227911949f, -7.549667913053783e-8f));

    rotateScale = rotate * scale;
    translate = Vector3(0.0f, 0.06650590896606446f, 0.0f);

    localTransform = pSkeletonIR->GetLocalTransformByBoneName("neck_joint_2").Matrix;
    EXPECT_TRUE(localTransform.ToMatrix3x3().IsEqualApprox(rotateScale));
    EXPECT_TRUE(Vector3(localTransform.GetCol3().X, localTransform.GetCol3().Y, localTransform.GetCol3().Z)
                    .IsEqualApprox(translate));

    // Check Animation
    EXPECT_EQ(pModelIR->pAnimationIRs.size(), 1);
    EXPECT_LE(pModelIR->pAnimationIRs[0]->SkeletalTracks.size(), pSkeletonIR->BoneNameStrs.size());

    auto TestSkeletalTrack = [&](const char* boneName)
    {
        EXPECT_TRUE(pModelIR->pAnimationIRs[0]->BoneIndexToSkeletalTrackIndexMap.find(
                        pSkeletonIR->BoneNameToIndexMap.find(boneName)->second) !=
                    pModelIR->pAnimationIRs[0]->BoneIndexToSkeletalTrackIndexMap.end());
        const AnimationIR::SkeletalTrack& torso_joint_1Track = pModelIR->pAnimationIRs[0]->GetSkeletalTrackByBoneIndex(
            pSkeletonIR->BoneNameToIndexMap.find(boneName)->second);
        EXPECT_EQ(torso_joint_1Track.TranslationInterpMode, AnimationIR::eInterpolationMode::Linear);
        EXPECT_GT(torso_joint_1Track.TranslationKeySequence.size(), 0);
        EXPECT_EQ(torso_joint_1Track.RotationInterpMode, AnimationIR::eInterpolationMode::Linear);
        EXPECT_GT(torso_joint_1Track.RotationKeySequence.size(), 0);
        EXPECT_EQ(torso_joint_1Track.ScalingInterpMode, AnimationIR::eInterpolationMode::Linear);
        EXPECT_GT(torso_joint_1Track.ScalingKeySequence.size(), 0);
    };

    TestSkeletalTrack("torso_joint_1");
    TestSkeletalTrack("torso_joint_2");
    TestSkeletalTrack("torso_joint_3");
    TestSkeletalTrack("neck_joint_1");
    TestSkeletalTrack("neck_joint_2");
    TestSkeletalTrack("arm_joint_R_1");
    TestSkeletalTrack("arm_joint_R_2");
    TestSkeletalTrack("arm_joint_R_3");
    TestSkeletalTrack("arm_joint_L_1");
    TestSkeletalTrack("arm_joint_L_2");
    TestSkeletalTrack("arm_joint_L_3");
    TestSkeletalTrack("leg_joint_R_1");
    TestSkeletalTrack("leg_joint_R_2");
    TestSkeletalTrack("leg_joint_R_3");
    TestSkeletalTrack("leg_joint_R_5");
    TestSkeletalTrack("leg_joint_L_1");
    TestSkeletalTrack("leg_joint_L_2");
    TestSkeletalTrack("leg_joint_L_3");
    TestSkeletalTrack("leg_joint_L_5");
}

TEST(ResourceParseFuncsTest, parseModelFile_glTFAnimatedMorphCube_ValidatesStandardSpecification)
{
    Path path(std::string("Test/Core/IO/Assets/TestGltf/AnimatedMorphCube/glTF/AnimatedMorphCube.gltf"));
    path.ResolveProjectPath();
    std::unique_ptr<const ModelIR> pModelIR = parseModelFile(path, std::string("AnimatedMorphCube"), false);
    ASSERT_NE(pModelIR, nullptr);
    const MeshIR* pMeshIR = pModelIR->pMeshIR.get();
    const SkeletonIR* pSkeletonIR = pModelIR->pSkeletonIR.get();

    // Check Morph Target
    const MeshIR::SubMesh& subMeshIR = pMeshIR->GetSubMeshByName("Cube");
    EXPECT_EQ(subMeshIR.MorphTargets.size(), 2);
    for (const MeshIR::MorphTarget& morphTarget : subMeshIR.MorphTargets)
    {
        EXPECT_LE(0, subMeshIR.Positions.size());
        EXPECT_EQ(subMeshIR.Positions.size(), morphTarget.Positions.size());
        EXPECT_EQ(subMeshIR.Positions.size(), morphTarget.Normals.size());
        EXPECT_EQ(subMeshIR.Positions.size(), morphTarget.Tangents.size());
    }

    // Check AnimationIR
    EXPECT_EQ(pModelIR->pAnimationIRs.size(), 1);
    EXPECT_EQ(pModelIR->pAnimationIRs[0]->MorphTargetTracks.size(), 1);
    EXPECT_TRUE(pModelIR->pAnimationIRs[0]->BoneIndexToMorphTargetTrackIndexMap.find(0) !=
                pModelIR->pAnimationIRs[0]->BoneIndexToMorphTargetTrackIndexMap.end());
    const AnimationIR::MorphTargetTrack& track = pModelIR->pAnimationIRs[0]->GetMorphTargetTrackByBoneIndex(
        pSkeletonIR->BoneNameToIndexMap.find("AnimatedMorphCube")->second);
    EXPECT_EQ(track.KeySequance.size(), 127);
    for (const AnimationIR::MorphingKey& key : track.KeySequance)
    {
        EXPECT_EQ(key.MorphTargetIndices.size(), key.Weights.size());
        EXPECT_EQ(key.Weights.size(), 2);
    }
}