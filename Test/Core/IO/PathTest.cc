#include "Core/IO/Path.h"

#include <gtest/gtest.h>
#include <string>

using namespace ho;

TEST(Path, Constructor_ValidString_CorrectlyInitializes)
{
    std::string pathStr = "a/b/c.txt";
    Path p(pathStr);

    EXPECT_EQ(p.ToString(), std::filesystem::path(pathStr).string());
}

TEST(Path, OperatorSlash_TwoPaths_JoinsCorrectly)
{
    std::string baseStr = "a/b";
    std::string fileStr = "c.txt";

    Path base(baseStr);
    Path file(fileStr);

    Path result = base / file;

    std::filesystem::path expected = std::filesystem::path(baseStr) / fileStr;

    EXPECT_EQ(result.ToString(), expected.string());
}

TEST(Path, GetParentPath_ValidPath_ReturnsParentDirectory)
{
    std::string pathStr = "a/b/c.txt";
    Path p(pathStr);

    EXPECT_EQ(p.GetParentPath().ToString(), std::filesystem::path(pathStr).parent_path().string());
}

TEST(Path, GetFileName_ValidPath_ReturnsNameWithExtension)
{
    std::string pathStr = "a/b/c.txt";
    Path p(pathStr);

    EXPECT_EQ(p.GetFileName().ToString(), "c.txt");
}

TEST(Path, ResolveProjectPath_RelativePath_ConvertsToProjectAbsolute)
{
    std::string relPathStr = "src/main.cpp";
    Path p(relPathStr);

    p.ResolveProjectPath();

    std::filesystem::path expected = std::filesystem::path(HO_PROJECT_ROOT) / relPathStr;

    EXPECT_TRUE(std::filesystem::path(p.ToString()).is_absolute());
    EXPECT_EQ(p.ToString(), expected.lexically_normal().string());
}

TEST(Path, ResolveProjectPath_AbsolutePath_StaysUnchanged)
{
    std::filesystem::path absPath = std::filesystem::path(HO_PROJECT_ROOT) / "src/main.cpp";

    std::string absPathStr = absPath.string();
    Path p(absPathStr);

    p.ResolveProjectPath();

    EXPECT_EQ(p.ToString(), absPath.string());
}

TEST(Path, ResolveAssetPath_RelativePath_ConvertsToAssetAbsolute)
{
    std::string relPathStr = "textures/albedo.png";
    Path p(relPathStr);

    p.ResolveAssetPath();

    std::filesystem::path expected = std::filesystem::path(HO_ASSET_ROOT) / relPathStr;

    EXPECT_TRUE(std::filesystem::path(p.ToString()).is_absolute());
    EXPECT_EQ(p.ToString(), expected.lexically_normal().string());
}

TEST(Path, ResolveAssetPath_AbsolutePath_StaysUnchanged)
{
    std::filesystem::path absPath = std::filesystem::path(HO_ASSET_ROOT) / "textures/albedo.png";

    std::string absPathStr = absPath.string();
    Path p(absPathStr);

    p.ResolveAssetPath();

    EXPECT_EQ(p.ToString(), absPath.string());
}
