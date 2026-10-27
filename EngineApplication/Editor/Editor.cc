#include "Editor.h"

#include "AssetBrowserWindow/EditorAssetBrowserWindow.h"
#include "CameraPreviewWindow/EditorCameraPreviewWindow.h"
#include "Consolewindow/EditorConsoleWindow.h"
#include "InspectorWindow/EditorInspectorWindow.h"
#include "MainWindow/EditorMainWindow.h"
#include "Scene/SceneManager.h"
#include "SceneHierarchyWindow/EditorSceneHierarchyWindow.h"
#include "SceneViewWindow/EditorSceneViewWindow.h"
#include "Subsystems/Asset/AssetSystem.h"
#include "Subsystems/UI/UISystem.h"

namespace ho
{
Editor::Editor()
  : IEngineApplication()
{
    mpWindows[0] = std::make_unique<EditorMainWindow>(this);
    mpWindows[1] = std::make_unique<EditorSceneHierarchyWindow>(this);
    mpWindows[2] = std::make_unique<EditorSceneViewWindow>(this);
    mpWindows[3] = std::make_unique<EditorCameraPreviewWindow>(this);
    mpWindows[4] = std::make_unique<EditorInspectorWindow>(this);
    mpWindows[5] = std::make_unique<EditorAssetBrowserWindow>();
    mpWindows[6] = std::make_unique<EditorConsoleWindow>();
}

Editor::~Editor() = default;

bool Editor::OnInit()
{
    if (!IEngineApplication::OnInit())
    {
        return false;
    }

    if (!AssetSystem::GetInstance().LoadBuiltInAssets())
    {
        return false;
    }

    mpWindows[0]->SetOpen(true);
    mpWindows[1]->SetOpen(true);
    mpWindows[2]->SetOpen(true);
    mpWindows[4]->SetOpen(true);
    mpWindows[5]->SetOpen(true);

    return true;
}

bool Editor::OnPreUpdate(float deltaTime)
{
    if (!IEngineApplication::OnPreUpdate(deltaTime))
    {
        return false;
    }

    return true;
}

bool Editor::OnUpdate(float deltaTime)
{
    if (!IEngineApplication::OnUpdate(deltaTime))
    {
        return false;
    }
    return true;
}

bool Editor::OnPostUpdate(float deltaTime)
{
    return true;
}

bool Editor::OnRender()
{
    if (!IEngineApplication::OnRender())
    {
        return false;
    }
    return true;
}

void Editor::OnShutdown()
{
    IEngineApplication::OnShutdown();
}

void Editor::SetCurrentScene(ObjectHandle<RScene> hCurrentScene)
{
    mhCurrentScene = hCurrentScene;
}

ObjectHandle<RScene> Editor::GetCurrentScene() const
{
    return mhCurrentScene;
}

void Editor::SetSelectedGameObject(ObjectHandle<RGameObject> hSelectedGameObject)
{
    if (mhSelectedGameObject.IsValid())
    {
        mhSelectedGameObject->SetSelectedInEditor(false);
        if (mhSelectedGameObject->HasComponent<RStaticMeshComponent>())
        {
            mhSelectedGameObject->GetComponent<RStaticMeshComponent>()->SetStencilEnabled(false);
            mhSelectedGameObject->GetComponent<RStaticMeshComponent>()->SetStencilReference(
                eStencilMode::StencilMode_None);
        }
    }

    mhSelectedGameObject = hSelectedGameObject;
    if (mhSelectedGameObject.IsValid())
    {
        mhSelectedGameObject->SetSelectedInEditor(true);
        if (mhSelectedGameObject->HasComponent<RStaticMeshComponent>())
        {
            mhSelectedGameObject->GetComponent<RStaticMeshComponent>()->SetStencilEnabled(true);
            mhSelectedGameObject->GetComponent<RStaticMeshComponent>()->SetStencilReference(
                eStencilMode::StencilMode_Outline);
        }
    }
}

ObjectHandle<RGameObject> Editor::GetSelectedGameObject() const
{
    return mhSelectedGameObject;
}

SceneRenderer& Editor::GetSceneRenderer()
{
    return mSceneRenderer;
}
} // namespace ho