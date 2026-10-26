#pragma once

#include <IconsMaterialDesignIcons.h>

#include "Core/Math/Color128.h"
#include "Scene/Components/Components.h"
#include "Subsystems/Asset/Assets.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
INLINE const char* getComponentDisplayIcon(ObjectHandle<RComponent> hComp)
{
    if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTransformComponent>())
    {
        return ICON_MDI_AXIS_ARROW;
    }
    else if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RDirectionalLightComponent>())
    {
        return ICON_MDI_SUN_WIRELESS_OUTLINE;
    }
    else if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RPointLightComponent>())
    {
        return ICON_MDI_LIGHTBULB_ON_OUTLINE;
    }
    else if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RSpotLightComponent>())
    {
        return ICON_MDI_FLASHLIGHT;
    }
    else if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RCameraComponent>())
    {
        return ICON_MDI_CAMERA;
    }
    else if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RStaticMeshComponent>())
    {
        return ICON_MDI_CUBE_OUTLINE;
    }
    else if (hComp->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RSkyBoxComponent>())
    {
        return ICON_MDI_WEATHER_PARTLY_CLOUDY;
    }
    else
    {
        return ICON_MDI_PUZZLE;
    }
}

INLINE const char* getAssetTypeDisplayName(ObjectHandle<RIAsset> hAsset)
{
    if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RStaticMeshAsset>())
    {
        return "Static Mesh";
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMaterialAsset>())
    {
        return "Material";
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTextureAsset>())
    {
        switch (hAsset.Cast<RTextureAsset>()->Type)
        {
            case eTextureType::Texture1D:
                return "Texture 1D";
                break;
            case eTextureType::Texture2D:
                return "Texture 2D";
                break;
            case eTextureType::Texture3D:
                return "Texture 3D";
                break;
            case eTextureType::TextureCubeMap:
                return "Texture Cubemap";
                break;
            case eTextureType::Texture1DArray:
                return "Texture 1D Array";
                break;
            case eTextureType::Texture2DArray:
                return "Texture 2D Array";
                break;
            case eTextureType::TextureCubeMapArray:
                return "Texture Cubemap Array";
                break;
            default:
                HO_ASSERT(false, "Invalid Texture Type");
                return "Texture";
        }
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RShaderAsset>())
    {
        switch (hAsset.Cast<RShaderAsset>()->Stage)
        {
            case eShaderStage::VertexShader:
                return "Vertex Shader";
                break;
            case eShaderStage::GeometryShader:
                return "Geometry Shader";
                break;
            case eShaderStage::FragmentShader:
                return "Fragment Shader";
                break;
            case eShaderStage::ComputeShader:
                return "Compute Shader";
                break;
            default:
                HO_ASSERT(false, "Invalid Shader stage");
                return "Shader";
        }
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphTargetsAsset>())
    {
        return "Morph Targets";
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphAnimationAsset>())
    {
        return "Morph Animation";
    }
    else
    {
        return *hAsset->GetClass().GetName();
    }
}

INLINE const char* getAssetDisplayIcon(ObjectHandle<RIAsset> hAsset)
{
    if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RStaticMeshAsset>())
    {
        return ICON_MDI_CUBE_OUTLINE;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMaterialAsset>())
    {
        return ICON_MDI_BRUSH_VARIANT;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTextureAsset>())
    {
        return ICON_MDI_IMAGE;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RShaderAsset>())
    {
        return ICON_MDI_FILE_CODE;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphTargetsAsset>())
    {
        return ICON_MDI_TRANSITION;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphAnimationAsset>())
    {
        return ICON_MDI_MOVE_RESIZE_VARIANT;
    }
    else
    {
        return ICON_MDI_TEXT_BOX;
    }
}

INLINE Color128 getAssetDisplayColor(ObjectHandle<RIAsset> hAsset)
{
    if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RStaticMeshAsset>())
    {
        return Color128(0.784f, 0.275f, 0.275f, 1.0f);
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMaterialAsset>())
    {
        return Color128(0.275f, 0.667f, 0.275f, 1.0f);
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTextureAsset>())
    {
        return Color128(0.235f, 0.549f, 0.824f, 1.0f);
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RShaderAsset>())
    {
        return Color128(0.627f, 0.314f, 0.863f, 1.0f);
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphTargetsAsset>())
    {
        return Color128(0.902f, 0.510f, 0.196f, 1.0f);
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphAnimationAsset>())
    {
        return Color128(0.941f, 0.392f, 0.627f, 1.0f);
    }
    else
    {
        return Color128(Color32::sGray);
    }
}

INLINE int32_t getAssetDisplayPriority(ObjectHandle<RIAsset> hAsset)
{
    if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RStaticMeshAsset>())
    {
        return 0;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMaterialAsset>())
    {
        return 1;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RTextureAsset>())
    {
        return 2;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RShaderAsset>())
    {
        return 3;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphTargetsAsset>())
    {
        return 4;
    }
    else if (hAsset->GetClass() == ReflectionSystem::GetInstance().GetClassByType<RMorphAnimationAsset>())
    {
        return 5;
    }
    else
    {
        HO_ASSERT(false, "Invalid asset.");
        return -1;
    }
}

INLINE const char* getAssetDisplayName(ObjectHandle<RIAsset> hAsset)
{
    const std::string fullName(*hAsset->hName);

    const size_t pos = fullName.rfind("::");

    if (pos == std::string::npos)
    {
        return *hAsset->hName;
    }

    return *hAsset->hName + pos + 2;
}
} // namespace ho