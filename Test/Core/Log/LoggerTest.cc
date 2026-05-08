#include "Core/Log/Logger.h"

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace ho;

class LoggerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        Logger::Init();
    }

    void TearDown() override
    {
        Logger::Shutdown();
    }

    void FlushAsyncLogs()
    {
        Logger::Shutdown();
    }
};

TEST_F(LoggerTest, GetFileName_VariousPathFormats_ExtractsOnlyName)
{
    EXPECT_STREQ(Logger::GetFileName("src/Core/main.cpp"), "main.cpp");
    EXPECT_STREQ(Logger::GetFileName("src\\Core\\main.cpp"), "main.cpp");
    EXPECT_STREQ(Logger::GetFileName("main.cpp"), "main.cpp");
}

TEST_F(LoggerTest, Init_WithImguiSink_IsAvailableAndEmpty)
{
    LogSink_imgui* pSink = Logger::GetImGuiSink();

    ASSERT_NE(pSink, nullptr);
    EXPECT_TRUE(pSink->IsEmpty());
}

TEST_F(LoggerTest, Macros_AllLogLevels_AreSuccessfullyCapturedBySink)
{
    HO_LOG_TRACE("Trace Message");
    HO_LOG_DEBUG("Debug Message");
    HO_LOG_INFO("Info Message");
    HO_LOG_WARN("Warn Message");
    HO_LOG_ERROR("Error Message");
    HO_LOG_CRITICAL("Critical Message");

    FlushAsyncLogs();

    LogSink_imgui* pSink = Logger::GetImGuiSink();
    ASSERT_NE(pSink, nullptr);
    EXPECT_FALSE(pSink->IsEmpty());

    LogEntry entry;

    pSink->PopLog(&entry);
    EXPECT_EQ(entry.Level, eLogLevel::Trace);
    EXPECT_TRUE(entry.Message.find("Trace Message") != std::string::npos);

    pSink->PopLog(&entry);
    EXPECT_EQ(entry.Level, eLogLevel::Debug);
    EXPECT_TRUE(entry.Message.find("Debug Message") != std::string::npos);

    pSink->PopLog(&entry);
    EXPECT_EQ(entry.Level, eLogLevel::Info);
    EXPECT_TRUE(entry.Message.find("Info Message") != std::string::npos);

    pSink->PopLog(&entry);
    EXPECT_EQ(entry.Level, eLogLevel::Warn);
    EXPECT_TRUE(entry.Message.find("Warn Message") != std::string::npos);

    pSink->PopLog(&entry);
    EXPECT_EQ(entry.Level, eLogLevel::Error);
    EXPECT_TRUE(entry.Message.find("Error Message") != std::string::npos);

    pSink->PopLog(&entry);
    EXPECT_EQ(entry.Level, eLogLevel::Critical);
    EXPECT_TRUE(entry.Message.find("Critical Message") != std::string::npos);

    EXPECT_TRUE(pSink->IsEmpty());
}

TEST_F(LoggerTest, Macro_WithFormattingAndMetadata_CapturesCorrectContext)
{
    int32_t theAnswer = 42;
    int32_t currentLine = __LINE__ + 1;
    HO_LOG_INFO("The answer to life is %d", theAnswer);

    FlushAsyncLogs();

    LogSink_imgui* pSink = Logger::GetImGuiSink();
    LogEntry entry;
    pSink->PopLog(&entry);

    EXPECT_TRUE(entry.Message.find("The answer to life is 42") != std::string::npos);

    std::string expectedFileInfo = "LoggerTest.cc:" + std::to_string(currentLine);
    EXPECT_TRUE(entry.Message.find(expectedFileInfo) != std::string::npos) << entry.Message;
}

TEST_F(LoggerTest, TraceScope_BasicFlow_CapturesExecutionTime)
{
    {
        HO_LOG_TRACE_SCOPE("PhysicsUpdate");

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    FlushAsyncLogs();

    LogSink_imgui* pSink = Logger::GetImGuiSink();
    LogEntry entry1, entry2;

    pSink->PopLog(&entry1);
    pSink->PopLog(&entry2);

    EXPECT_TRUE(entry1.Message.find("[START] PhysicsUpdate") != std::string::npos);

    EXPECT_TRUE(entry2.Message.find("[END] PhysicsUpdate") != std::string::npos);

    EXPECT_TRUE(entry2.Message.find("Took") != std::string::npos);
    EXPECT_TRUE(entry2.Message.find("ms)") != std::string::npos);
}