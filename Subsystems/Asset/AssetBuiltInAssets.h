#pragma once

#include "StaticMeshAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{

struct AssetBuiltInAssets
{
    ObjectHandle<RStaticMeshAsset> hCubeMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hSparseSphereMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hSphereMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hCapsuleMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hCylinderMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hPlaneMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hQuadMesh = ObjectHandle<RStaticMeshAsset>::sNULL;

    ObjectHandle<RStaticMeshAsset> hStageLightMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hLightBulbMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
    ObjectHandle<RStaticMeshAsset> hFlashLightMesh = ObjectHandle<RStaticMeshAsset>::sNULL;

    ObjectHandle<RStaticMeshAsset> hCameraMesh = ObjectHandle<RStaticMeshAsset>::sNULL;
};
} // namespace ho