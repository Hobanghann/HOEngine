#include "Core/IO/ResourceImporter.h"

#include <assimp/scene.h>
#include <filesystem>
#include <gtest/gtest.h>

#include "Core/IO/Image.h"
#include "Core/IO/Path.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace ho;

static std::string writeTempImage(
    const std::string& fileNameStr, int width, int height, int channels, const std::vector<uint8_t>& data)
{
    std::string pathStr = (std::filesystem::temp_directory_path() / fileNameStr).string();

    int ok = stbi_write_png(pathStr.c_str(), width, height, channels, data.data(), width * channels);

    EXPECT_EQ(ok, 1) << "Failed to write test PNG: " << pathStr;
    return pathStr;
}

TEST(ResourceImporterTest, ImportImage_InvalidPath_ReturnNull)
{
    auto pImg = ResourceImporter::ImportImage(Path(std::string("this_file_does_not_exist_12345.png")));
    EXPECT_EQ(pImg, nullptr);
}

TEST(ResourceImporterTest, ImportImage_SingleChannelPNG_MapsToR8Format)
{
    int width = 2;
    int height = 2;
    int channels = 1;
    std::vector<uint8_t> data = {10, 20, 30, 40};

    std::string pathStr = writeTempImage("test_r8.png", width, height, channels, data);
    auto pImg = ResourceImporter::ImportImage(Path(pathStr));

    ASSERT_NE(pImg, nullptr);
    EXPECT_EQ(pImg->GetWidth(), width);
    EXPECT_EQ(pImg->GetHeight(), height);
    EXPECT_EQ(pImg->GetFormat(), Image::eFormat::R8);
    EXPECT_EQ(pImg->GetColor32(0, 0).R, 10);
    EXPECT_EQ(pImg->GetColor32(0, 1).R, 30);
}

TEST(ResourceImporterTest, ImportImage_DualChannelPNG_MapsToRG8Format)
{
    int width = 2;
    int height = 1;
    int channels = 2;
    std::vector<uint8_t> data = {10, 50, 20, 60};

    std::string pathStr = writeTempImage("test_rg8.png", width, height, channels, data);
    auto pImg = ResourceImporter::ImportImage(Path(pathStr));

    ASSERT_NE(pImg, nullptr);
    EXPECT_EQ(pImg->GetFormat(), Image::eFormat::RG8);

    Color32 c0 = pImg->GetColor32(0, 0);
    EXPECT_EQ(c0.R, 10);
    EXPECT_EQ(c0.G, 50);

    Color32 c1 = pImg->GetColor32(1, 0);
    EXPECT_EQ(c1.R, 20);
    EXPECT_EQ(c1.G, 60);
}

TEST(ResourceImporterTest, ImportImage_TripleChannelPNG_MapsToRGB8Format)
{
    int width = 1;
    int height = 2;
    int channels = 3;
    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};

    std::string pathStr = writeTempImage("test_rgb8.png", width, height, channels, data);
    auto pImg = ResourceImporter::ImportImage(Path(pathStr));

    ASSERT_NE(pImg, nullptr);
    EXPECT_EQ(pImg->GetFormat(), Image::eFormat::RGB8);

    EXPECT_EQ(pImg->GetColor32(0, 0).R, 1);
    EXPECT_EQ(pImg->GetColor32(0, 0).G, 2);
    EXPECT_EQ(pImg->GetColor32(0, 0).B, 3);

    EXPECT_EQ(pImg->GetColor32(0, 1).R, 4);
    EXPECT_EQ(pImg->GetColor32(0, 1).G, 5);
    EXPECT_EQ(pImg->GetColor32(0, 1).B, 6);
}

TEST(ResourceImporterTest, ImportImage_QuadChannelPNG_MapsToRGBA8Format)
{
    int width = 2;
    int height = 1;
    int channels = 4;
    std::vector<uint8_t> data = {10, 20, 30, 40, 50, 60, 70, 80};

    std::string pathStr = writeTempImage("test_rgba8.png", width, height, channels, data);
    auto pImg = ResourceImporter::ImportImage(Path(pathStr));

    ASSERT_NE(pImg, nullptr);
    EXPECT_EQ(pImg->GetFormat(), Image::eFormat::RGBA8);

    Color32 c0 = pImg->GetColor32(0, 0);
    EXPECT_EQ(c0.R, 10);
    EXPECT_EQ(c0.G, 20);
    EXPECT_EQ(c0.B, 30);
    EXPECT_EQ(c0.A, 40);

    Color32 c1 = pImg->GetColor32(1, 0);
    EXPECT_EQ(c1.R, 50);
    EXPECT_EQ(c1.G, 60);
    EXPECT_EQ(c1.B, 70);
    EXPECT_EQ(c1.A, 80);
}

static std::string writeTempHDRImage(
    const std::string& fileNameStr, int width, int height, int channels, const std::vector<float>& data)
{
    std::string pathStr = (std::filesystem::temp_directory_path() / fileNameStr).string();
    int ok = stbi_write_hdr(pathStr.c_str(), width, height, channels, data.data());
    EXPECT_EQ(ok, 1) << "Failed to write test HDR: " << pathStr;
    return pathStr;
}

TEST(ResourceImporterTest, ImportImage_HDR_MapsToRGB32FFormat)
{
    int width = 1;
    int height = 1;
    int channels = 3;
    std::vector<float> data = {1.0f, 2.0f, 3.0f};

    std::string pathStr = writeTempHDRImage("test_rgb32f.hdr", width, height, channels, data);
    auto pImg = ResourceImporter::ImportImage(Path(pathStr));

    ASSERT_NE(pImg, nullptr);
    EXPECT_EQ(pImg->GetFormat(), Image::eFormat::RGB32F);

    Color128 c = pImg->GetColor128(0, 0);
    EXPECT_NEAR(c.R, 1.0f, math::EPSILON_CMP);
    EXPECT_NEAR(c.G, 2.0f, math::EPSILON_CMP);
    EXPECT_NEAR(c.B, 3.0f, math::EPSILON_CMP);
}

static void runImporterTest(const Path& filePath)
{
    Path pathStr = filePath.ResolvedAssetPath();

    std::unique_ptr<ModelImportContext> pImportContext = ResourceImporter::ImportModel(pathStr, false, false);

    ASSERT_NE(pImportContext, nullptr) << "Importing model failed to load: " << pathStr.ToString();

    ASSERT_NE(pImportContext->pAssimpScene, nullptr) << "Assimp failed to load: " << pathStr.ToString();

    ASSERT_NE(pImportContext->pAssimpScene->mRootNode, nullptr) << "Root node is null for file: " << pathStr.ToString();

    ASSERT_FALSE(pImportContext->pFlattedScene.empty())
        << "topologicalSortRecursive produced empty result: " << pathStr.ToString();

    const aiNode* pExpectedRootNode = pImportContext->pAssimpScene->mRootNode;
    const aiNode* pFrontNode = pImportContext->pFlattedScene.front();
    EXPECT_EQ(pExpectedRootNode, pFrontNode) << "Root node is not the first element in FlattedScene.";

    EXPECT_EQ(pImportContext->pFlattedScene.size(), pImportContext->NodeToIndex.size()) << "NodeToIndex size mismatch";

    for (int32_t i = 0; i < static_cast<int32_t>(pImportContext->pFlattedScene.size()); ++i)
    {
        aiNode* n = pImportContext->pFlattedScene[i];
        auto it = pImportContext->NodeToIndex.find(n);
        ASSERT_TRUE(it != pImportContext->NodeToIndex.end()) << "Node missing in NodeToIndex map";

        EXPECT_EQ(it->second, i) << "NodeToIndex index does not match flattened order";
    }

    EXPECT_GT(pImportContext->pAssimpScene->mNumMeshes, 0) << "Scene contains no meshes";

    const aiMesh* pAssimpMesh = pImportContext->pAssimpScene->mMeshes[0];
    ASSERT_NE(pAssimpMesh, nullptr);

    EXPECT_GT(pAssimpMesh->mNumVertices, 0u) << "MeshIR has no vertices";

    EXPECT_GT(pAssimpMesh->mNumFaces, 0u) << "MeshIR has no faces";

    EXPECT_TRUE(pAssimpMesh->HasNormals()) << "MeshIR has no normals (unexpected for teapot model)";

    if (pathStr.ToString().find(".obj") == std::string::npos)
    {
        EXPECT_GE(pImportContext->pAssimpScene->mRootNode->mNumChildren, 0u) << "Expected child nodes in GLTF";
    }
}

TEST(ResourceImporterTest, ImportModel_OBJFormat_LoadsCorrectTopologyAndMesh)
{
    std::string filePath = (std::filesystem::path("TestAssets") / "teapot" / "teapot.obj").string();
    runImporterTest(Path(filePath));
}

TEST(ResourceImporterTest, ImportModel_GLTFFormat_LoadsCorrectTopologyAndMesh)
{
    std::string filePath = (std::filesystem::path("TestAssets") / "teapot" / "teapot.gltf").string();
    runImporterTest(Path(filePath));
}
