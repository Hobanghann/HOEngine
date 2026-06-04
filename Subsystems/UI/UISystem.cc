#include "UISystem.h"

#include <imgui.h>

#include "Platforms/IPlatformApplication.h"
#include "Subsystems/Rendering/IRenderingSystem.h"

namespace ho
{
UISystem& UISystem::GetInstance()
{
    HO_ASSERT(spInstance, "UISystem is not created");
    return *spInstance;
}

void UISystem::SubmitRenderWindow(IUIWindow* pWindow)
{
    HO_ASSERT(pWindow, "Window is null.");
    if (mRenderWindowQueue.IsFull())
    {
        HO_ASSERT(false, "Render window queue is full.");
        return;
    }
    mRenderWindowQueue.Push(pWindow);
}

UISystem::UISystem()
{
    HO_ASSERT(!spInstance, "UI system muse be created by 'createInstance' function.");
}

void UISystem::createInstance()
{
    HO_ASSERT(!spInstance, "UISystem was already created.");
    spInstance = new UISystem();

    spInstance->mRenderWindowQueue = FixedQueue<IUIWindow*>(sRenderWindowQueueSize);
}

void UISystem::deleteInstance()
{
    HO_ASSERT(spInstance, "UISystem is not created");
    delete spInstance;
    spInstance = nullptr;
}

bool UISystem::init()
{
    return true;
}

void UISystem::submitDrawCommandForUI()
{
    while (!mRenderWindowQueue.IsEmpty())
    {
        IUIWindow* pWindow = mRenderWindowQueue.Front();
        if (pWindow->IsVisible())
        {
            pWindow->onGUI();
        }
        mRenderWindowQueue.Pop();
    }

    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
    }

    const ImDrawData* mainDrawData = ImGui::GetDrawData();

    IRenderingSystem::GetInstance().SubmitUIViewportData(IPlatformApplication::GetInstance().GetMainWindow(),
                                                         *mainDrawData);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
        for (int32_t i = 0; i < platformIO.Viewports.Size; ++i)
        {
            const ImGuiViewport* viewport = platformIO.Viewports[i];

            if (viewport->ID == ImGui::GetMainViewport()->ID)
            {
                continue;
            }

            const IPlatformWindow* pTargetWindow = static_cast<IPlatformWindow*>(viewport->RendererUserData);

            if (viewport->DrawData && pTargetWindow)
            {
                IRenderingSystem::GetInstance().SubmitUIViewportData(pTargetWindow, *viewport->DrawData);
            }
        }
    }
}

void UISystem::shutdown() {}

UISystem* UISystem::spInstance = nullptr;
} // namespace ho