#pragma once

#include "StaticMeshAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{

struct AssetBuiltInAssets
{
    ObjectHandle<RStaticMeshAsset> hCubeMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hSparseSphereMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hSphereMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hCapsuleMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hCylinderMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hPlaneMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hQuadMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();

    ObjectHandle<RStaticMeshAsset> hStageLightMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hLightBulbMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
    ObjectHandle<RStaticMeshAsset> hFlashLightMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();

    ObjectHandle<RStaticMeshAsset> hCameraMesh = ObjectHandle<RStaticMeshAsset>::GetNULL();
};
} // namespace ho