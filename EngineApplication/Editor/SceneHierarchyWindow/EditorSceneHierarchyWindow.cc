#include "EditorSceneHierarchyWindow.h"

#include <IconsMaterialDesignIcons.h>
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>
#include <vector>

#include "../Editor.h"
#include "Platforms/IPlatformApplication.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/SceneManager.h"
#include "Subsystems/Asset/AssetSystem.h"

namespace ho
{
EditorSceneHierarchyWindow::EditorSceneHierarchyWindow(Editor* pOwner)
  : IEngineWindow(StringHandle("EditorSceneHierarchyWindow"))
  , mhRenamingGameObject(ObjectHandle<RGameObject>::sNULL)
  , mpOwner(pOwner)
{
}

EditorSceneHierarchyWindow::~EditorSceneHierarchyWindow() = default;

bool EditorSceneHierarchyWindow::OnInit()
{
    return true;
}

bool EditorSceneHierarchyWindow::OnPreUpdate(float deltaTime)
{
    return true;
}

bool EditorSceneHierarchyWindow::OnUpdate(float deltaTime)
{
    return true;
}

bool EditorSceneHierarchyWindow::OnPostUpdate(float deltaTime)
{
    return true;
}

bool EditorSceneHierarchyWindow::OnRender()
{
    return true;
}

void EditorSceneHierarchyWindow::OnShutdown() {}

void EditorSceneHierarchyWindow::DrawUI()
{
    drawMainWindow();
    drawPopups();
}

void EditorSceneHierarchyWindow::drawMainWindow()
{
    if (ImGui::Begin("Scene Hierarchy", &mbOpen, 0))
    {
        SetFocused(ImGui::IsWindowFocused());

        drawSceneArea();
        ImGui::Spacing();
        drawToolbar();
        drawGameObjectHierarchy();
    }
    ImGui::End();
}

void EditorSceneHierarchyWindow::drawSceneArea()
{
    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Scene:  ");

    ImGui::SameLine();

    ImGui::SetNextItemWidth(-1);
    const char* preview = hCurrScene.IsNULL() ? "None" : *hCurrScene->GetName();
    if (ImGui::BeginCombo("##CurrentSceneCombo", preview, ImGuiComboFlags_PopupAlignLeft))
    {
        for (const auto& hScene : SceneManager::GetInstance().GetScenes())
        {
            const bool bIsSelected = (hScene == hCurrScene);
            if (ImGui::Selectable(*hScene->GetName(), bIsSelected))
            {
                mpOwner->SetCurrentScene(hScene);
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
}

void EditorSceneHierarchyWindow::drawToolbar()
{
    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();

    if (ImGui::Button(ICON_MDI_PLUS " Add"))
    {
        if (hCurrScene.IsValid())
        {
            mbOpenAddPopup = true;
        }
    }
    drawAddPopup();

    ImGui::SameLine();

    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextWithHint("##GameObjectFilter",
                                 ICON_MDI_MAGNIFY "    Search Game Objects",
                                 mGameObjectFilter.InputBuf,
                                 IM_COUNTOF(mGameObjectFilter.InputBuf),
                                 ImGuiInputTextFlags_EscapeClearsAll))
    {
        mGameObjectFilter.Build();

        if (hCurrScene.IsValid())
        {
            for (const auto& hObj : hCurrScene->GetGameObjects<RGameObject>())
            {
                if (hObj->GetComponent<RTransformComponent>()->GetParent().IsNULL())
                {
                    updateGameObjectFilteredRecursive(hObj);
                }
            }
        }
    }
}

void EditorSceneHierarchyWindow::drawGameObjectHierarchy()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    if (ImGui::BeginChild(
            "##GameObjectHierarchy", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();

        static const float sItemGap = 8.0f;

        if (hCurrScene.IsValid())
        {
            static const int32_t sColumnCount = 2;

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, sItemGap * 0.25f));

            if (ImGui::BeginTable("##GameObjectHierarchy", sColumnCount, 0, ImVec2(0.0f, 0.0f)))
            {
                ImGui::TableSetupColumn(
                    "Buttons", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_IndentDisable, 0.0f);

                ImGui::TableSetupColumn(
                    "TreeNode", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_IndentEnable, 0.0f);

                for (const auto& hObj : hCurrScene->GetGameObjects<RGameObject>())
                {
                    if (hObj->GetComponent<RTransformComponent>()->GetParent().IsNULL())
                    {
                        drawGameObjectTreeRecursive(hObj);
                    }
                }

                ImGui::EndTable();
            }
            ImGui::PopStyleVar();

            ImGui::Dummy(ImGui::GetContentRegionAvail());

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJ"))
                {
                    const ObjectHandle<RGameObject> hDraggedObj =
                        *reinterpret_cast<ObjectHandle<RGameObject>*>(payload->Data);

                    const ObjectHandle<RTransformComponent> hSrcParent =
                        hDraggedObj->GetComponent<RTransformComponent>()->GetParent();
                    if (hSrcParent.IsValid())
                    {
                        hSrcParent->RemoveChild(hDraggedObj->GetComponent<RTransformComponent>());
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor();
}

void EditorSceneHierarchyWindow::drawAddPopup()
{
    if (mbOpenAddPopup)
    {
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos({buttonMin.x, buttonMin.y + buttonSize.y});
        ImGui::OpenPopup("##SceneHierarchyAddPopup");
        mbOpenAddPopup = false;
    }

    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
    const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();

    auto createNon3DObj = [&](const std::string& baseNameStr, ObjectHandle<RComponent> hNewComp)
    {
        const std::string newObjNameStr = getUniqueGameObjectName(baseNameStr);
        const ObjectHandle<RGameObject> hNewObj =
            hCurrScene->CreateGameObject<RGameObject>(StringHandle(newObjNameStr));
        hNewObj->AttachComponent(hNewComp);
        mhRenamingGameObject = hNewObj;
        mpOwner->SetSelectedGameObject(hNewObj);
    };
    auto create3DObj = [&](const std::string& baseNameStr, ObjectHandle<RStaticMeshAsset> hMesh)
    {
        const std::string newObjNameStr = getUniqueGameObjectName(baseNameStr);
        const ObjectHandle<RGameObject> hNewObj =
            hCurrScene->CreateGameObject<RGameObject>(StringHandle(newObjNameStr));
        const ObjectHandle<RStaticMeshComponent> hNewComp = hCurrScene->CreateComponent<RStaticMeshComponent>();
        hNewComp->SetMesh(hMesh);
        hNewObj->AttachComponent(hNewComp);
        mhRenamingGameObject = hNewObj;
        mpOwner->SetSelectedGameObject(hNewObj);
    };

    ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.0f));
    if (ImGui::BeginPopup("##SceneHierarchyAddPopup", ImGuiWindowFlags_NoDecoration))
    {
        if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE "  Create Empty"))
        {
            const std::string newObjNameStr = getUniqueGameObjectName("GameObject");
            const ObjectHandle<RGameObject> hNewObj =
                hCurrScene->CreateGameObject<RGameObject>(StringHandle(newObjNameStr));
            mhRenamingGameObject = hNewObj;
            mpOwner->SetSelectedGameObject(hNewObj);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE "  Create Empty Child"))
        {
            const std::string newObjNameStr = getUniqueGameObjectName("GameObject");
            const ObjectHandle<RGameObject> hNewObj =
                hCurrScene->CreateGameObject<RGameObject>(StringHandle(newObjNameStr));
            if (hSelectedObj.IsValid())
            {
                hSelectedObj->GetComponent<RTransformComponent>()->AddChild(
                    hNewObj->GetComponent<RTransformComponent>());
            }
            mhRenamingGameObject = hNewObj;
            mpOwner->SetSelectedGameObject(hNewObj);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::BeginMenu(ICON_MDI_SHAPE_OUTLINE "  3D Object"))
        {
            const auto& builtInAssets = AssetSystem::GetInstance().GetBuiltInAsset();
            if (ImGui::MenuItem(ICON_MDI_CUBE_OUTLINE " Cube"))
            {
                create3DObj("Cube", builtInAssets.hCubeMesh);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_SPHERE " Sphere"))
            {
                create3DObj("Sphere", builtInAssets.hSphereMesh);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_PILL " Capsule"))
            {
                create3DObj("Capsule", builtInAssets.hCapsuleMesh);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_CYLINDER " Cylinder"))
            {
                create3DObj("Cylinder", builtInAssets.hCylinderMesh);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_SQUARE " Plane"))
            {
                create3DObj("Plane", builtInAssets.hPlaneMesh);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_SQUARE_OUTLINE " Quad"))
            {
                create3DObj("Quad", builtInAssets.hQuadMesh);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_MDI_TRACK_LIGHT "  Light"))
        {
            if (ImGui::MenuItem(ICON_MDI_SUN_WIRELESS_OUTLINE "Directional Light"))
            {
                const ObjectHandle<RDirectionalLightComponent> hNewComp =
                    hCurrScene->CreateComponent<RDirectionalLightComponent>();
                createNon3DObj("Directional Light", hNewComp);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_LIGHTBULB_ON_OUTLINE "Point Light"))
            {
                const ObjectHandle<RPointLightComponent> hNewComp = hCurrScene->CreateComponent<RPointLightComponent>();
                createNon3DObj("Point Light", hNewComp);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem(ICON_MDI_FLASHLIGHT "Spot Light"))
            {
                const ObjectHandle<RSpotLightComponent> hNewComp = hCurrScene->CreateComponent<RSpotLightComponent>();
                createNon3DObj("Spot Light", hNewComp);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_MDI_PANORAMA_VARIANT "  Environment"))
        {
            if (ImGui::MenuItem(ICON_MDI_WEATHER_PARTLY_CLOUDY "  Sky Box"))
            {
                const ObjectHandle<RSkyBoxComponent> hNewComp = hCurrScene->CreateComponent<RSkyBoxComponent>();
                createNon3DObj("Sky Box", hNewComp);
                const ObjectHandle<RGameObject> hNewObj = mpOwner->GetSelectedGameObject();
                if (hNewObj->GetOwner()->GetMainSkyBox().IsNULL())
                {
                    hNewObj->GetOwner()->SetMainSkyBox(hNewComp);
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(ICON_MDI_VIDEO "  Camera"))
        {
            const ObjectHandle<RCameraComponent> hNewComp = hCurrScene->CreateComponent<RCameraComponent>();
            createNon3DObj("Camera", hNewComp);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
}

void EditorSceneHierarchyWindow::drawGameObjectTreeRecursive(ObjectHandle<RGameObject> hGameObj)
{
    if (!hGameObj->IsFilteredInEditor())
    {
        return;
    }

    ImGui::PushID(hGameObj.Get());

    drawGameObjectRowBackground(hGameObj);
    drawGameObjectVisibilityControls(hGameObj);

    const bool bOpened = drawGameObjectTreeNode(hGameObj);

    processGameObjectDragAndDrop(hGameObj);
    processGameObjectSelection(hGameObj);
    drawGameObjectName(hGameObj);

    ImGui::PopID();

    if (bOpened)
    {
        ObjectHandle<RTransformComponent> hChild = hGameObj->GetComponent<RTransformComponent>()->GetFirstChild();

        while (hChild.IsValid())
        {
            drawGameObjectTreeRecursive(hChild->GetOwner());
            hChild = hChild->GetNextSibling();
        }

        ImGui::TreePop();
    }
}

void EditorSceneHierarchyWindow::drawGameObjectRowBackground(ObjectHandle<RGameObject> hGameObj)
{
    const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();

    ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFrameHeight());

    const ImVec2 rowMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
    const ImVec2 rowMax = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max;

    const bool bRowHovered = ImGui::IsMouseHoveringRect(rowMin, rowMax, false);
    const bool bSelected = hSelectedObj == hGameObj->GetHandle();

    if (bSelected)
    {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_ButtonActive, 0.7f));
    }
    else if (bRowHovered)
    {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_ButtonHovered, 0.7f));
    }
}

void EditorSceneHierarchyWindow::drawGameObjectVisibilityControls(ObjectHandle<RGameObject> hGameObj)
{
    static const float sItemGap = 8.0f;

    ImGui::TableNextColumn();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sItemGap);

    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_ChildBg, 0.5f));

    ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    const bool bRowHovered = ImGui::IsMouseHoveringRect(
        ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min,
        ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max,
        false);

    const char* visibleIcon = hGameObj->IsVisibleInEditor() ? ICON_MDI_EYE_OUTLINE : ICON_MDI_EYE_OFF_OUTLINE;

    const char* clickableIcon =
        hGameObj->IsClickableInEditor() ? ICON_MDI_HAND_BACK_RIGHT_OUTLINE : ICON_MDI_HAND_BACK_RIGHT_OFF_OUTLINE;

    if (!bRowHovered && hGameObj->IsVisibleInEditor())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    if (ImGui::Button(visibleIcon))
    {
        setGameObjectVisibleInEditorRecursive(hGameObj, !hGameObj->IsVisibleInEditor());
    }

    if (!bRowHovered && hGameObj->IsVisibleInEditor())
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (!bRowHovered && hGameObj->IsClickableInEditor())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    if (ImGui::Button(clickableIcon))
    {
        setGameObjectClickableInEditorRecursive(hGameObj, !hGameObj->IsClickableInEditor());
    }

    if (!bRowHovered && hGameObj->IsClickableInEditor())
    {
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

bool EditorSceneHierarchyWindow::drawGameObjectTreeNode(ObjectHandle<RGameObject> hGameObj)
{
    ImGui::TableNextColumn();

    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_WindowBg, 0.5f));

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                               ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowOverlap;

    if (hGameObj->GetComponent<RTransformComponent>()->GetFirstChild().IsNULL())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (mGameObjectFilter.IsActive())
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (hGameObj->IsOpenedInEditor() ||
        (mhRenamingGameObject.IsValid() && mhRenamingGameObject->GetComponent<RTransformComponent>()->GetParent() ==
                                               hGameObj->GetComponent<RTransformComponent>()))
    {
        ImGui::SetNextItemOpen(true);
    }

    ImGui::PushStyleColor(ImGuiCol_Header, 0);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);

    const bool bOpened = ImGui::TreeNodeEx(hGameObj.Get(), flags, ICON_MDI_CUBE_OUTLINE " ");

    ImGui::PopStyleColor(3);

    hGameObj->SetOpenedInEditor(bOpened);

    return bOpened;
}

void EditorSceneHierarchyWindow::processGameObjectDragAndDrop(ObjectHandle<RGameObject> hGameObj)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("DND_GAMEOBJ", &hGameObj, sizeof(ObjectHandle<RGameObject>));

        ImGui::PushFont(nullptr, 30.0f);
        ImGui::Text(ICON_MDI_CUBE_OUTLINE);
        ImGui::PopFont();

        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJ"))
        {
            const ObjectHandle<RGameObject> hDraggedObj = *reinterpret_cast<ObjectHandle<RGameObject>*>(payload->Data);

            if (hDraggedObj != hGameObj)
            {
                bool bValidParent = true;

                const ObjectHandle<RTransformComponent> hTargetTransform =
                    hGameObj->GetComponent<RTransformComponent>();

                const ObjectHandle<RTransformComponent> hDraggedTransform =
                    hDraggedObj->GetComponent<RTransformComponent>();

                if (hTargetTransform->GetParent().IsValid() && hDraggedObj == hTargetTransform->GetParent()->GetOwner())
                {
                    bValidParent = false;
                }

                if (bValidParent && !hDraggedTransform->IsDescendantRecursive(hTargetTransform))
                {
                    const ObjectHandle<RTransformComponent> hSrcParent = hDraggedTransform->GetParent();

                    if (hSrcParent.IsValid())
                    {
                        hSrcParent->RemoveChild(hDraggedTransform);
                    }

                    hTargetTransform->AddChild(hDraggedTransform);
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void EditorSceneHierarchyWindow::processGameObjectSelection(ObjectHandle<RGameObject> hGameObj)
{
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        mpOwner->SetSelectedGameObject(hGameObj);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        mpOwner->SetSelectedGameObject(hGameObj);
        mbOpenGameObjRightClickPopup = true;
    }
}

void EditorSceneHierarchyWindow::drawGameObjectName(ObjectHandle<RGameObject> hGameObj)
{
    static char sRenameBuffer[256] = "";
    static ObjectHandle<RGameObject> shLastRenamingGameObj = ObjectHandle<RGameObject>::sNULL;

    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();

    ImGui::SameLine();

    if (mhRenamingGameObject == hGameObj)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f);

        if (shLastRenamingGameObj != hGameObj)
        {
            shLastRenamingGameObj = hGameObj;

            strncpy_s(sRenameBuffer, *hGameObj->GetName(), sizeof(sRenameBuffer) - 1);

            ImGui::SetKeyboardFocusHere();
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 10.0f);

        ImGui::InputText("##GameObjRenameInput", sRenameBuffer, sizeof(sRenameBuffer));

        if (ImGui::IsItemDeactivated())
        {
            if (strlen(sRenameBuffer) > 0)
            {
                hCurrScene->RenameGameObject(hGameObj, StringHandle(sRenameBuffer));
            }

            mhRenamingGameObject = ObjectHandle<RGameObject>::sNULL;
            shLastRenamingGameObj = ObjectHandle<RGameObject>::sNULL;
        }

        ImGui::PopStyleVar();
    }
    else
    {
        ImGui::Text("%s", *hGameObj->GetName());
    }
}

void EditorSceneHierarchyWindow::drawPopups()
{
    drawGameObjRightClickPopup();
    drawGameObjPropertiesPopup();
}

void EditorSceneHierarchyWindow::drawGameObjRightClickPopup()
{
    if (mbOpenGameObjRightClickPopup)
    {
        ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
        ImGui::OpenPopup("##SceneHierarchyWindowRightClickPopup");
        mbOpenGameObjRightClickPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 8.0f));
    if (ImGui::BeginPopup("##SceneHierarchyWindowRightClickPopup"))
    {
        const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
        const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();

        if (hCurrScene.IsValid() && hSelectedObj.IsValid())
        {
            if (ImGui::MenuItem(ICON_MDI_FORM_TEXTBOX "   Rename", "M") || ImGui::IsKeyPressed(ImGuiKey_M))
            {
                mhRenamingGameObject = hSelectedObj;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem(ICON_MDI_DELETE "   Delete", "Del") || ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                hCurrScene->DestroyGameObject(hSelectedObj);
                ImGui::CloseCurrentPopup();
            }

            const bool bIsShiftDown = ImGui::GetIO().KeyShift;
            if (ImGui::MenuItem(ICON_MDI_DELETE_SWEEP "   Delete All", "Shift+Del") ||
                (bIsShiftDown && ImGui::IsKeyPressed(ImGuiKey_Delete)))
            {
                hCurrScene->DestroyGameObjectAndDescendantRecursive(hSelectedObj);
                ImGui::CloseCurrentPopup();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::MenuItem(ICON_MDI_INFORMATION_SLAB_CIRCLE "   Properties", "R") ||
                ImGui::IsKeyPressed(ImGuiKey_R))
            {
                mbOpenGameObjPropertiesPopup = true;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

void EditorSceneHierarchyWindow::drawGameObjPropertiesPopup()
{
    const ObjectHandle<RGameObject> hSelectedObj = mpOwner->GetSelectedGameObject();

    if (hSelectedObj.IsNULL())
    {
        return;
    }

    if (mbOpenGameObjPropertiesPopup)
    {
        ImGui::OpenPopup(
            (std::string(*hSelectedObj->GetName()) + std::string(" Properties###GameObjectPropertiesPopup")).c_str());
        mbOpenGameObjPropertiesPopup = false;
    }

    static uint64_t sLastPopupGameObjectID = 0;
    static std::string sPopupNameStr;
    static char sNameBuffer[256];

    if (sLastPopupGameObjectID != hSelectedObj->GetGlobalID() || sPopupNameStr.empty())
    {
        sLastPopupGameObjectID = hSelectedObj->GetGlobalID();
        sPopupNameStr = std::string(*hSelectedObj->GetName()) + std::string(" Properties###GameObjectPropertiesPopup");
        strncpy_s(sNameBuffer, sizeof(sNameBuffer), *hSelectedObj->GetName(), _TRUNCATE);
    }

    ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));

    if (ImGui::BeginPopupModal(sPopupNameStr.c_str(),
                               nullptr,
                               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        const float contentWidth = ImGui::GetContentRegionAvail().x;
        const float windowPaddingX = ImGui::GetStyle().WindowPadding.x;

        // =====================================================================
        // GameObject Name
        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(ICON_MDI_CUBE_OUTLINE);
        ImGui::SameLine();

        const float inputWidth = contentWidth - ImGui::GetCursorPosX() + windowPaddingX;
        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputText("##GameObjectRenameInput", sNameBuffer, sizeof(sNameBuffer));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // =====================================================================
        // GameObject Metadata
        auto drawRightAlignedText = [contentWidth, windowPaddingX](const char* pLabel, const char* pValue)
        {
            ImGui::Text("%s", pLabel);
            ImGui::SameLine();

            const float textWidth = ImGui::CalcTextSize(pValue).x;
            ImGui::SetCursorPosX(contentWidth + windowPaddingX - textWidth);

            ImGui::Text("%s", pValue);
        };

        drawRightAlignedText("Type:", *hSelectedObj->GetClass().GetName());

        const ObjectHandle<RTransformComponent> hTransformComp = hSelectedObj->GetComponent<RTransformComponent>();
        const char* parentName =
            hTransformComp->GetParent().IsNULL() ? "None" : *(hTransformComp->GetParent()->GetOwner()->GetName());
        drawRightAlignedText("Parent:", parentName);
        int32_t childCount = 0;
        ObjectHandle<RTransformComponent> hFirstChild = hTransformComp->GetFirstChild();
        while (hFirstChild.IsValid())
        {
            ++childCount;
            hFirstChild = hFirstChild->GetNextSibling();
        }
        drawRightAlignedText("Children:", std::to_string(childCount).c_str());

        // =====================================================================
        // Buttons
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float buttonWidth = 80.0f;
        const float totalButtonsWidth = (buttonWidth * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonsWidth - ImGui::GetStyle().WindowPadding.x);

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            if (strlen(sNameBuffer) > 0)
            {
                mpOwner->GetCurrentScene()->RenameGameObject(hSelectedObj, StringHandle(sNameBuffer));
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Apply", ImVec2(buttonWidth, 0.0f)))
        {
            if (strlen(sNameBuffer) > 0)
            {
                mpOwner->GetCurrentScene()->RenameGameObject(hSelectedObj, StringHandle(sNameBuffer));
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
}

void EditorSceneHierarchyWindow::updateGameObjectFilteredRecursive(ObjectHandle<RGameObject> hGameObj) const
{
    if (hGameObj.IsNULL())
    {
        return;
    }

    if (!mGameObjectFilter.IsActive())
    {
        hGameObj->SetFilteredInEditor(true);
        ObjectHandle<RTransformComponent> hChild = hGameObj->GetComponent<RTransformComponent>()->GetFirstChild();
        while (hChild.IsValid())
        {
            updateGameObjectFilteredRecursive(hChild->GetOwner());
            hChild = hChild->GetNextSibling();
        }
        return;
    }

    hGameObj->SetFilteredInEditor(false);

    if (mGameObjectFilter.PassFilter(*hGameObj->GetName()))
    {
        hGameObj->SetFilteredInEditor(true);
        ObjectHandle<RTransformComponent> hParent = hGameObj->GetComponent<RTransformComponent>()->GetParent();
        while (hParent.IsValid())
        {
            hParent->GetOwner()->SetOpenedInEditor(true);
            hParent->GetOwner()->SetFilteredInEditor(true);
            hParent = hParent->GetParent();
        }
    }

    ObjectHandle<RTransformComponent> hChild = hGameObj->GetComponent<RTransformComponent>()->GetFirstChild();
    while (hChild.IsValid())
    {
        updateGameObjectFilteredRecursive(hChild->GetOwner());
        hChild = hChild->GetNextSibling();
    }
}

void EditorSceneHierarchyWindow::setGameObjectVisibleInEditorRecursive(ObjectHandle<RGameObject> hGameObj,
                                                                       bool bVisible) const
{
    hGameObj->SetVisibleInEditor(bVisible);

    ObjectHandle<RTransformComponent> hChild = hGameObj->GetComponent<RTransformComponent>()->GetFirstChild();
    while (hChild.IsValid())
    {
        setGameObjectVisibleInEditorRecursive(hChild->GetOwner(), bVisible);
        hChild = hChild->GetNextSibling();
    }
}

void EditorSceneHierarchyWindow::setGameObjectClickableInEditorRecursive(ObjectHandle<RGameObject> hGameObj,
                                                                         bool bClickable) const
{
    hGameObj->SetClickableInEditor(bClickable);

    ObjectHandle<RTransformComponent> hChild = hGameObj->GetComponent<RTransformComponent>()->GetFirstChild();
    while (hChild.IsValid())
    {
        setGameObjectClickableInEditorRecursive(hChild->GetOwner(), bClickable);
        hChild = hChild->GetNextSibling();
    }
}

// NOTE: Supposed duplicated name handled to 'Name (n)'
std::string EditorSceneHierarchyWindow::getUniqueGameObjectName(const std::string& baseNameStr) const
{
    static const int32_t sMaxDupCount = 2048;
    static std::vector<bool> sbIsNumUsed(sMaxDupCount);

    const ObjectHandle<RScene> hCurrScene = mpOwner->GetCurrentScene();
    HO_ASSERT(hCurrScene.IsValid(), "This function must be called with valid handle. Check caller.");

    std::fill(sbIsNumUsed.begin(), sbIsNumUsed.end(), false);
    int32_t maxNum = -1;
    for (const auto& hGameObj : hCurrScene->GetGameObjects<RGameObject>())
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
} // namespace ho