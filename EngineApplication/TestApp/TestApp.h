#pragma once

#include "../IEngineApplication.h"
#include "Core/Math/Projection.h"
#include "Core/Math/Transform3D.h"
#include "Core/Templates/GlobalPoolIndex.h"
#include "Subsystems/Asset/Assets.h"
#include "UIWindows/TestAppMainWindow.h"

namespace ho
{
class TestApp final : public IEngineApplication
{
  public:
    TestApp();
    ~TestApp() override = default;

    bool OnInit() override;
    bool OnPreUpdate() override;
    bool OnUpdate() override;
    bool OnPostUpdate() override;
    bool OnRender() override;
    void OnShutdown() override;

  private:
    StringHandle mhMainFrameBufferName;
    std::unique_ptr<TestAppMainWindow> mpMainWindow;
    StaticMeshHandle mhMesh;
    FixedArray<MaterialHandle> mhMaterials;
    TextureHandle mhCubeMap;
    Transform3D mWorldTransform;
    Vector3 mCameraPos;
    Vector3 mCameraForward;
    Vector3 mCameraRight;
    float mCameraPitch = 0.0f;
    float mCameraYaw = 0.0f;
    Projection mProjection;
};
} // namespace ho