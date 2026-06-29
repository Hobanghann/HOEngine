#include "IEngineApplication.h"

#include "Macros.h"
#include "TestApp/TestApp.h"

namespace ho
{
std::unique_ptr<IEngineApplication> IEngineApplication::CreateApplication(eEngineApplicationType appType)
{
    switch (appType)
    {
        case eEngineApplicationType::TestApp:
            return std::make_unique<TestApp>();
        // case eEngineApplicationType::Editor:
        //     return std::make_unique<Editor>();
        // case eEngineApplicationType::Game:
        //     HO_ASSERT(false, "Currently not supported.");
        //     return nullptr;
        default:
            HO_ASSERT(false, "Invalid application type.");
            return nullptr;
    }
}

IEngineApplication::IEngineApplication(const std::string& titleStr)
  : mTitleStr(titleStr)
{
}
} // namespace ho