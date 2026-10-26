#pragma once

#include <imgui.h>

#include "Core/Math/Quaternion.h"
#include "DirectoryHierarchy.h"
#include "Subsystems/Asset/IAsset.h"
#include "Subsystems/Object/ObjectHandle.h"

namespace ho
{
class Property;

class PropertyDrawer
{
  public:
    template <typename TOwner>
    static bool DrawClassProperties(TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        bool bChanged = false;

        ImGui::PushID(pInstance);
        if (ImGui::BeginTable("##PropertyTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch);

            bChanged = drawClassPropertiesEntryRecursive(pInstance->GetClass(), pInstance);

            ImGui::EndTable();
        }
        ImGui::PopID();

        return bChanged;
    }

    static void DrawPropertyName(const Property& prop)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(*prop.GetName());
    }

    template <typename TOwner>
    static bool DrawPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        switch (prop.GetUIStyle())
        {
            case ePropertyUIStyle::DragInt:
                return drawDragIntPropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragInt2:
                return drawDragInt2PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragInt3:
                return drawDragInt3PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragInt4:
                return drawDragInt4PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragFloat:
                return drawDragFloatPropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragFloat2:
                return drawDragFloat2PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragFloat3:
                return drawDragFloat3PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragFloat4:
                return drawDragFloat4PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragVector:
                return drawDragVectorPropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragAngle:
                return drawDragAnglePropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::DragAndDropAsset:
                return drawDragAndDropAssetPropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::Color3:
                return drawColor3PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::Color4:
                return drawColor4PropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::Checkbox:
                return drawCheckboxPropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::ComboBox:
                return drawEnumPropertyValue(prop, pInstance);
                break;

            case ePropertyUIStyle::EulerAngles:
                return drawDragEulerAnglesPropertyValue(prop, pInstance);
                break;

            default:
                HO_ASSERT(false, "Invalid property UI style.");
                return false;
        }
    }

  private:
    template <typename TOwner>
    static bool drawClassPropertiesEntryRecursive(const Class& currentClass, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        bool bChanged = false;

        if (const Class* pParentClass = currentClass.GetParent())
        {
            bChanged |= drawClassPropertiesEntryRecursive(*pParentClass, pInstance);
        }

        for (const auto& prop : currentClass.GetProperties())
        {
            if (prop.GetFlag() & PropertyFlags_VisibleInEditor)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                DrawPropertyName(prop);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                bChanged |= DrawPropertyValue(prop, pInstance);
            }
        }

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragIntPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        int* pValue = static_cast<int*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged =
            ImGui::DragInt("##Value", pValue, 0.1f, static_cast<int>(prop.GetMin()), static_cast<int>(prop.GetMax()));
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragInt2PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        int* pValue = static_cast<int*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged =
            ImGui::DragInt2("##Value", pValue, 0.1f, static_cast<int>(prop.GetMin()), static_cast<int>(prop.GetMax()));
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragInt3PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        int* pValue = static_cast<int*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged =
            ImGui::DragInt3("##Value", pValue, 0.1f, static_cast<int>(prop.GetMin()), static_cast<int>(prop.GetMax()));
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragInt4PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        int* pValue = static_cast<int*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged =
            ImGui::DragInt4("##Value", pValue, 0.1f, static_cast<int>(prop.GetMin()), static_cast<int>(prop.GetMax()));
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragFloatPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::DragFloat("##Value", pValue, 0.1f, prop.GetMin(), prop.GetMax());
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragFloat2PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::DragFloat2("##Value", pValue, 0.1f, prop.GetMin(), prop.GetMax());
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragFloat3PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::DragFloat3("##Value", pValue, 0.1f, prop.GetMin(), prop.GetMax());
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragFloat4PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::DragFloat4("##Value", pValue, 0.1f, prop.GetMin(), prop.GetMax());
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragVectorPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValues = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValues != nullptr, "Property value pointer is null.");

        const int32_t componentCount = static_cast<int32_t>(prop.GetType()->GetType().GetSize() / sizeof(float));
        HO_ASSERT(componentCount >= 1 && componentCount <= 4, "Invalid vector component count.");

        static const char* labels[] = {"X", "Y", "Z", "W"};

        const float availWidth = ImGui::GetContentRegionAvail().x;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float textWidth = ImGui::CalcTextSize("X").x;

        const float totalTextWidth = textWidth * static_cast<float>(componentCount);
        const float totalSpacing = spacing * static_cast<float>((componentCount * 2) - 1);
        float itemWidth = (availWidth - totalTextWidth - totalSpacing) / static_cast<float>(componentCount);

        if (itemWidth < 1.0f)
        {
            itemWidth = 1.0f;
        }

        bool bChanged = false;

        ImGui::PushID(pValues);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));

        for (int32_t i = 0; i < componentCount; ++i)
        {
            if (i > 0)
            {
                ImGui::SameLine();
            }

            ImGui::TextDisabled("%s", labels[i]);
            ImGui::SameLine();

            ImGui::SetNextItemWidth(itemWidth);

            ImGui::PushID(i);
            if (ImGui::DragFloat("##Value", &pValues[i], 0.1f, prop.GetMin(), prop.GetMax(), "%.3f"))
            {
                bChanged = true;
            }
            ImGui::PopID();
        }

        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragAnglePropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        float degrees = math::RadToDeg(*pValue);
        float minDegrees = math::RadToDeg(prop.GetMin());
        float maxDegrees = math::RadToDeg(prop.GetMax());

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));

        bool bChanged = ImGui::DragFloat("##Value", &degrees, 1.0f, minDegrees, maxDegrees, "%.1f deg");
        if (bChanged)
        {
            *pValue = math::DegToRad(degrees);
        }

        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    // TODO: Currently, all texture types (2D, Cubemap, etc.) are defined as RTextureAsset.
    // Thus, a texture handle can be bound regardless of the actual texture type.
    // To fix this, texture assets should be specialized per texture type.
    template <typename TOwner>
    static bool drawDragAndDropAssetPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");
        ObjectHandle<RIAsset>* phDstAsset = static_cast<ObjectHandle<RIAsset>*>(prop.GetPtr(pInstance));
        HO_ASSERT(phDstAsset != nullptr, "Property value pointer is null.");

        bool bChanged = false;

        ImGui::PushID(phDstAsset);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

        char buf[256] = {};
        const char* assetName = (*phDstAsset).IsValid() ? getAssetDisplayName(*phDstAsset) : "None";
        snprintf(buf, HO_ARRAY_SIZE(buf), "%s%s", assetName, "##DropButton");
        ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));

        ImGui::PopStyleVar();

        if (ImGui::BeginPopupContextItem())
        {
            const bool bHasAsset = phDstAsset->IsValid();
            if (ImGui::MenuItem("Clear", nullptr, false, bHasAsset))
            {
                *phDstAsset = {};
                bChanged = true;
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
            {
                if (payload->IsDataType("DND_ITEM"))
                {
                    DirectoryHierarchy::DirectoryItem* pDraggedItem =
                        *reinterpret_cast<DirectoryHierarchy::DirectoryItem**>(payload->Data);

                    if (phDstAsset->IsAssignableFrom(pDraggedItem->hAsset))
                    {
                        if (ImGui::AcceptDragDropPayload("DND_ITEM"))
                        {
                            *phDstAsset = pDraggedItem->hAsset;
                            bChanged = true;
                        }
                    }
                    else if (!ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SetItemTooltip("Drop %s here.", *(*phDstAsset).GetActualObjectType().GetName());

        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawColor3PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::ColorEdit3("##Value", pValue);
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawColor4PropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        float* pValue = static_cast<float*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::ColorEdit4("##Value", pValue);
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawCheckboxPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        bool* pValue = static_cast<bool*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));
        bool bChanged = ImGui::Checkbox("##Value", pValue);
        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawEnumPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        void* pValue = prop.GetPtr(pInstance);
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        const Enum& type = prop.GetType()->GetAs<Enum>();

        const int32_t enumSize = type.GetSize();
        const FixedArray<EnumEntry>& entries = type.GetEntries();

        auto writeValue = [](void* pDst, const EnumValue& val, size_t size, bool bSigned)
        {
            if (bSigned)
            {
                switch (size)
                {
                    case 1:
                        *static_cast<int8_t*>(pDst) = val;
                        break;
                    case 2:
                        *static_cast<int16_t*>(pDst) = val;
                        break;
                    case 4:
                        *static_cast<int32_t*>(pDst) = val;
                        break;
                    case 8:
                        *static_cast<int64_t*>(pDst) = val;
                        break;
                }
            }
            else
            {
                switch (size)
                {
                    case 1:
                        *static_cast<uint8_t*>(pDst) = val;
                        break;
                    case 2:
                        *static_cast<uint16_t*>(pDst) = val;
                        break;
                    case 4:
                        *static_cast<uint32_t*>(pDst) = val;
                        break;
                    case 8:
                        *static_cast<uint64_t*>(pDst) = val;
                        break;
                }
            }
        };

        const char* currentLabel = "Unknown";
        for (const auto& entry : entries)
        {
            uint64_t tempBuf = 0;
            writeValue(&tempBuf, entry.GetValue(), enumSize, entry.IsSigned());

            if (std::memcmp(pValue, &tempBuf, enumSize) == 0)
            {
                currentLabel = *entry.GetName();
                break;
            }
        }

        bool bChanged = false;

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));

        if (ImGui::BeginCombo("##Value", currentLabel))
        {
            for (const auto& entry : entries)
            {
                uint64_t tempBuf = 0;
                writeValue(&tempBuf, entry.GetValue(), enumSize, entry.IsSigned());

                const bool isSelected = (std::memcmp(pValue, &tempBuf, enumSize) == 0);

                if (ImGui::Selectable(*entry.GetName(), isSelected))
                {
                    if (!isSelected)
                    {
                        writeValue(pValue, entry.GetValue(), enumSize, entry.IsSigned());
                        bChanged = true;
                    }
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::EndDisabled();
        ImGui::PopID();

        return bChanged;
    }

    template <typename TOwner>
    static bool drawDragEulerAnglesPropertyValue(const Property& prop, TOwner* pInstance)
    {
        HO_ASSERT(pInstance, "Instance pointer is null.");
        HO_ASSERT(prop.GetFlag() & PropertyFlags_VisibleInEditor, "Can't draw invisible property.");

        Quaternion* pValue = static_cast<Quaternion*>(prop.GetPtr(pInstance));
        HO_ASSERT(pValue != nullptr, "Property value pointer is null.");

        Vector3 eulerAngle = pValue->ToEuler() * math::RAD1_IN_DEG;

        static const char* labels[] = {"X", "Y", "Z"};
        constexpr int32_t componentCount = 3;

        float availWidth = ImGui::GetContentRegionAvail().x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float textWidth = ImGui::CalcTextSize("X").x;

        float itemWidth = (availWidth - (textWidth * 3.0f) - (spacing * 5.0f)) / 3.0f;
        if (itemWidth < 1.0f)
        {
            itemWidth = 1.0f;
        }

        ImGui::PushID(pValue);
        ImGui::BeginDisabled(!(prop.GetFlag() & PropertyFlags_Editable));

        bool bChanged = false;

        for (int32_t i = 0; i < componentCount; ++i)
        {
            if (i > 0)
            {
                ImGui::SameLine();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("%s", labels[i]);
            ImGui::SameLine();

            ImGui::SetNextItemWidth(itemWidth);

            ImGui::PushID(i);
            if (ImGui::DragFloat(
                    "##Value", &eulerAngle.Data[i], 0.05f, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_WrapAround))
            {
                bChanged = true;
            }
            ImGui::PopID();
        }

        ImGui::EndDisabled();
        ImGui::PopID();

        if (bChanged)
        {
            eulerAngle *= math::DEG1_IN_RAD;
            *pValue = Quaternion::FromEuler(eulerAngle.X, eulerAngle.Y, eulerAngle.Z);
        }

        return bChanged;
    }
};
} // namespace ho