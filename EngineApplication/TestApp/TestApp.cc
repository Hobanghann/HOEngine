#include "TestApp.h"

#include <imgui.h>

#include "Core/IO/Path.h"
#include "Core/IO/ResourceParseFuncs.h"
#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Asset/AssetImportFuncs.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/Rendering/IRenderingSystem.h"
#include "Subsystems/UI/UISystem.h"

namespace ho
{
TestApp::TestApp()
  : IEngineApplication("Test")
  , mhMainFrameBufferName(StringHandle("DefaultFrameBuffer"))
  , mProjection(Projection::CreatePerspective(math::PI / 3.0f, 16.0f / 9.0f, 0.5f, 1000.0f))
{
}

bool TestApp::OnInit()
{
    // Create FrameBuffer
    IRenderingSystem::FrameBufferDesc frameBufferDesc;
    frameBufferDesc.hName = mhMainFrameBufferName;
    frameBufferDesc.Width = IPlatformApplication::GetInstance().GetMainWindow()->GetClientWidth();
    frameBufferDesc.Height = IPlatformApplication::GetInstance().GetMainWindow()->GetClientHeight();
    frameBufferDesc.SampleCount = 4;
    frameBufferDesc.TargetDesc[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Color)].ClearColor =
        Color128(1.0f, 1.0f, 1.0f);
    frameBufferDesc.TargetDesc[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Color)].Format =
        IRenderingSystem::eRenderTargetFormat::R8G8B8A8_UNORM;
    frameBufferDesc.TargetDesc[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Depth)].ClearDepth = 1.0f;
    frameBufferDesc.TargetDesc[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Depth)].ClearStencil = 0;
    frameBufferDesc.TargetDesc[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Depth)].Format =
        IRenderingSystem::eRenderTargetFormat::D24_UNORM_S8_UINT;
    IRenderingSystem::GetInstance().EnqueueCreateFramebuffer(frameBufferDesc);

    mpMainWindow = std::make_unique<TestAppMainWindow>(mhMainFrameBufferName);
    mpMainWindow->SetVisible(true);

    // Load Assets
    Path modelPath(std::string("TestAssets/Sponza/glTF/Sponza.gltf"));
    modelPath.ResolveAssetPath();
    std::unique_ptr<const parser::ModelIR> pParsedModel = parser::parseModelFile(modelPath, "Cube", false, false);
    mhMesh = AssetSystem::GetInstance().AddStaticMesh(std::move(*(importer::importStaticMesh(
        *pParsedModel->pMeshIR, pParsedModel->pMaterialIRs, *pParsedModel->pSkeletonIR, *pParsedModel->pSkinIR))));

    mhMaterials = FixedArray<MaterialHandle>(static_cast<int32_t>(pParsedModel->pMaterialIRs.size()));
    for (int32_t i = 0; i < static_cast<int32_t>(pParsedModel->pMaterialIRs.size()); ++i)
    {
        mhMaterials[i] = AssetSystem::GetInstance().AddMaterial(std::move(*(importer::importTexturedMaterial(
            *(pParsedModel->pMaterialIRs[i]), eMaterialAssetType::Legacy, pParsedModel->pTextureIRs))));
    }

    AssetSystem::GetInstance().ResolveStaticMeshMaterials(mhMesh);

    for (int32_t i = 0; i < static_cast<int32_t>(pParsedModel->pTextureIRs.size()); ++i)
    {
        if (pParsedModel->pTextureIRs[i]->Img.IsLinear())
        {
            AssetSystem::GetInstance().AddTexture(std::move(
                *importer::importTexture2D(*pParsedModel->pTextureIRs[i], eTextureFormat::R8G8B8A8_UNORM, true)));
        }
        else
        {
            AssetSystem::GetInstance().AddTexture(std::move(
                *importer::importTexture2D(*pParsedModel->pTextureIRs[i], eTextureFormat::R8G8B8A8_SRGB, true)));
        }
    }

    for (int32_t i = 0; i < mhMaterials.GetSize(); ++i)
    {
        AssetSystem::GetInstance().ResolveMaterialTextures(mhMaterials[i]);
    }

    std::unique_ptr<const parser::TextureIR> pPosX = parser::parseTextureFile(
        Path(std::string("TestAssets/Yokohama/posx.jpg")).ResolvedAssetPath(), "Yokohama right", false);
    std::unique_ptr<const parser::TextureIR> pNegX = parser::parseTextureFile(
        Path(std::string("TestAssets/Yokohama/negx.jpg")).ResolvedAssetPath(), "Yokohama left", false);
    std::unique_ptr<const parser::TextureIR> pPosY = parser::parseTextureFile(
        Path(std::string("TestAssets/Yokohama/posy.jpg")).ResolvedAssetPath(), "Yokohama top", false);
    std::unique_ptr<const parser::TextureIR> pNegY = parser::parseTextureFile(
        Path(std::string("TestAssets/Yokohama/negy.jpg")).ResolvedAssetPath(), "Yokohama bottom", false);
    std::unique_ptr<const parser::TextureIR> pPosZ = parser::parseTextureFile(
        Path(std::string("TestAssets/Yokohama/posz.jpg")).ResolvedAssetPath(), "Yokohama front", false);
    std::unique_ptr<const parser::TextureIR> pNegZ = parser::parseTextureFile(
        Path(std::string("TestAssets/Yokohama/negz.jpg")).ResolvedAssetPath(), "Yokohama back", false);
    mhCubeMap = AssetSystem::GetInstance().AddTexture(std::move(*importer::importTextureCubeMap(
        "Yokohama", *pPosX, *pNegX, *pPosY, *pNegY, *pPosZ, *pNegZ, eTextureFormat::R8G8B8A8_SRGB, true)));

    AssetSystem& as = AssetSystem::GetInstance();
    (void)as;

    IRenderingSystem::GetInstance().EnqueueUploadStaticMesh(mhMesh, true);
    for (int32_t i = 0; i < mhMaterials.GetSize(); ++i)
    {
        IRenderingSystem::GetInstance().EnqueueUploadMaterial(mhMaterials[i], true);
    }
    IRenderingSystem::GetInstance().EnqueueUploadTexture(mhCubeMap, true);

    // Set initial transform
    mWorldTransform.Scale(Vector3(30.0f, 30.0f, 30.0f));
    mWorldTransform.RotateAxisAngle(Vector3::sUnitY, math::PI);
    mCameraPos = Vector3(0.0f, 50.0f, -50.0f);
    Transform3D viewTransform;
    viewTransform.LookAt(mWorldTransform.GetOrigin(), Vector3::sUnitY, false);
    mCameraForward = -viewTransform.GetForward();
    mCameraPitch = math::Asin(mCameraForward.Y);
    mCameraYaw = math::Atan2(mCameraForward.Z, mCameraForward.X);
    mCameraRight = -viewTransform.GetRight();
    mProjection = Projection::CreatePerspective(
        math::PI / 3.0f,
        static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientWidth()) /
            static_cast<float>(IPlatformApplication::GetInstance().GetMainWindow()->GetClientHeight()),
        0.5f,
        1000.0f);

    return true;
}

bool TestApp::OnPreUpdate()
{
    UISystem::GetInstance().SubmitRenderWindow(mpMainWindow.get());
    return true;
}

bool TestApp::OnUpdate()
{
    ImGuiIO& io = ImGui::GetIO();

    bool isGuiCapturingMouse = io.WantCaptureMouse;

    if (!isGuiCapturingMouse)
    {
        if (ImGui::IsMouseClicked(0))
        {
        }
    }
    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        mCameraPos += 1.0f * mCameraForward;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        mCameraPos += 1.0f * mCameraRight;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        mCameraPos -= 1.0f * mCameraForward;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        mCameraPos -= 1.0f * mCameraRight;
    }
    if (ImGui::IsMouseDown(1))
    {
        float mouseSensitivity = 0.005f;

        mCameraYaw += io.MouseDelta.x * mouseSensitivity;
        mCameraPitch -= io.MouseDelta.y * mouseSensitivity;

        const float pitchLimit = math::PI / 2.0f - 0.01f;
        if (mCameraPitch > pitchLimit)
        {
            mCameraPitch = pitchLimit;
        }
        if (mCameraPitch < -pitchLimit)
        {
            mCameraPitch = -pitchLimit;
        }

        mCameraForward.X = math::Cos(mCameraPitch) * math::Cos(mCameraYaw);
        mCameraForward.Y = math::Sin(mCameraPitch);
        mCameraForward.Z = math::Cos(mCameraPitch) * math::Sin(mCameraYaw);
        mCameraForward.Normalize();
    }
    if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
    {
        mWorldTransform.RotateEulerLocal(math::PI / 100.0f, 0.0f, 0.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
    {
        mWorldTransform.RotateEulerLocal(0.0f, math::PI / 100.0f, 0.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
    {
        mWorldTransform.RotateEulerLocal(-math::PI / 100.0f, 0.0f, 0.0f);
    }
    if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
    {
        mWorldTransform.RotateEulerLocal(0.0f, -math::PI / 100.0f, 0.0f);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
    }
    return true;
}

bool TestApp::OnPostUpdate()
{
    return true;
}

bool TestApp::OnRender()
{
    AssetSystem& as = AssetSystem::GetInstance();
    (void)as;

    IRenderingSystem::RenderPassDesc renderPass;
    renderPass.hFrameBufferName = mhMainFrameBufferName;
    renderPass.bClearTarget[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Color)] = true;
    renderPass.bClearTarget[static_cast<int32_t>(IRenderingSystem::eRenderTargetType::Depth)] = true;

    renderPass.Viewport.X = 0;
    renderPass.Viewport.Y = 0;
    renderPass.Viewport.Width = IPlatformApplication::GetInstance().GetMainWindow()->GetClientWidth();
    renderPass.Viewport.Height = IPlatformApplication::GetInstance().GetMainWindow()->GetClientHeight();

    renderPass.WorldCameraPos = mCameraPos;
    Transform3D viewTransform;
    viewTransform.SetOrigin(mCameraPos);
    viewTransform.LookAt(mCameraForward + mCameraPos, Vector3::sUnitY, false);
    mCameraRight = -viewTransform.GetRight();
    renderPass.ViewMat = viewTransform.Inverse().Matrix;
    renderPass.ProjMat = mProjection.GetMatrix(-1.0f, 1.0f);

    renderPass.DirLights[0].Color = Color128(1.0f, 1.0f, 1.0f);
    renderPass.DirLights[0].Direction = Vector3(1.0f, -1.0f, 1.0f);
    renderPass.DirLights[0].Intensity = 1.0f;
    renderPass.DirLightCount = 1;

    renderPass.PointLights[0].Position = Vector3(30.0f, 10.0f, -20.0f);
    renderPass.PointLights[0].Color = Color128(1.0f, 0.2f, 0.2f);
    renderPass.PointLights[0].Intensity = 30.0f;

    renderPass.PointLights[0].Constant = 1.0f;
    renderPass.PointLights[0].Linear = 0.09f;
    renderPass.PointLights[0].Quadratic = 0.032f;
    renderPass.PointLightCount = 1;

    renderPass.SpotLights[0].Position = mCameraPos;
    renderPass.SpotLights[0].Direction = mCameraForward;
    renderPass.SpotLights[0].Color = Color128(0.2f, 1.0f, 0.2f);
    renderPass.SpotLights[0].Intensity = 10.0f;

    renderPass.SpotLights[0].InnerCutOff = math::Cos(math::PI4 / 4.0f);
    renderPass.SpotLights[0].OuterCutOff = math::Cos(math::PI4);

    renderPass.SpotLights[0].Constant = 1.0f;
    renderPass.SpotLights[0].Linear = 0.09f;
    renderPass.SpotLights[0].Quadratic = 0.032f;
    renderPass.SpotLightCount = 1;

    renderPass.hSkyMap = mhCubeMap.Get()->hRenderProxy;
    renderPass.hIrradianceMap = mhCubeMap.Get()->hRenderProxy;

    IRenderingSystem::GetInstance().BeginRenderPass(renderPass);

    for (const auto& subMesh : mhMesh.Get()->SubMeshes)
    {
        IRenderingSystem::DrawCommandDesc cmdDesc;
        cmdDesc.hGpuStaticMesh = mhMesh.Get()->hRenderProxy;
        cmdDesc.hGpuMaterial = subMesh.hRenderMaterial.Get()->hRenderProxy;
        cmdDesc.VertexOffset = subMesh.PositionOffset;
        cmdDesc.IndexOffset = subMesh.IndexOffset;
        cmdDesc.IndexCount = subMesh.IndexCount;
        cmdDesc.AlphaMode = subMesh.hRenderMaterial.Get()->PipelineState.AlphaMode;
        cmdDesc.WorldMat = (mWorldTransform * subMesh.LocalTransform).Matrix;
        IRenderingSystem::GetInstance().SubmitDrawCommand(cmdDesc);
    }

    IRenderingSystem::GetInstance().EndRenderPass();

    IRenderingSystem& rs = IRenderingSystem::GetInstance();
    (void)rs;
    return true;
}

void TestApp::OnShutdown()
{
    IRenderingSystem::GetInstance().EnqueueDestroyFramebuffer(mhMainFrameBufferName);
}
} // namespace ho