#include "Core/IO/Image.h"

#include <gtest/gtest.h>

#include "Core/Math/MathDefs.h"

using namespace ho;

TEST(ImageTest, GetPixelBytes_AllFormats_ReturnsCorrectByteSize)
{
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8_UNORM), 1);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8G8_UNORM), 2);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8G8B8_UNORM), 3);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8G8B8A8_UNORM), 4);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8_SRGB), 1);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8G8_SRGB), 2);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8G8B8_SRGB), 3);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R8G8B8A8_SRGB), 4);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R32_FLOAT), 4);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R32G32_FLOAT), 8);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R32G32B32_FLOAT), 12);
    EXPECT_EQ(Image::GetPixelBytes(Image::eFormat::R32G32B32A32_FLOAT), 16);
}

TEST(ImageTest, Constructor_RawData_InitializesProperly)
{
    uint8_t data[3] = {10, 20, 30};

    Image img(Path(std::string("test/path")), "myimg", Image::eFormat::R8G8B8_UNORM, 1, 1, 3, data);

    EXPECT_EQ(img.GetPath(), Path(std::string("test/path")));
    EXPECT_EQ(img.GetName(), "myimg");
    EXPECT_EQ(img.GetFormat(), Image::eFormat::R8G8B8_UNORM);
    EXPECT_EQ(img.GetWidth(), 1);
    EXPECT_EQ(img.GetHeight(), 1);
    EXPECT_EQ(img.GetChannelCount(), 3);
    const uint8_t* pBmp = img.GetBitmap();
    EXPECT_EQ(pBmp[0], 10);
    EXPECT_EQ(pBmp[1], 20);
    EXPECT_EQ(pBmp[2], 30);
}

TEST(ImageTest, MoveConstructor_ValidImage_TransfersOwnership)
{
    uint8_t data[3] = {1, 2, 3};
    Image src(Path(std::string("test/path")), "myimg", Image::eFormat::R8G8B8_UNORM, 1, 1, 3, data);

    Image dst(std::move(src));

    EXPECT_EQ(dst.GetPath(), Path(std::string("test/path")));
    EXPECT_EQ(dst.GetName(), "myimg");
    EXPECT_EQ(dst.GetFormat(), Image::eFormat::R8G8B8_UNORM);
    const uint8_t* pBmp = dst.GetBitmap();
    EXPECT_EQ(pBmp[0], 1);
    EXPECT_EQ(pBmp[1], 2);
    EXPECT_EQ(pBmp[2], 3);

    EXPECT_EQ(src.GetBitmap(), nullptr);
}

TEST(ImageTest, MoveAssignment_ValidImage_TransfersOwnership)
{
    uint8_t data[3] = {4, 5, 6};
    Image src(Path(std::string("test/path")), "src", Image::eFormat::R8G8B8_UNORM, 1, 1, 3, data);

    uint8_t empty[3] = {0, 0, 0};
    Image dst(Path(std::string("old")), "old", Image::eFormat::R8G8B8_UNORM, 1, 1, 3, empty);

    dst = std::move(src);

    EXPECT_EQ(dst.GetPath(), Path(std::string("test/path")));
    EXPECT_EQ(dst.GetName(), "src");
    const uint8_t* pBmp = dst.GetBitmap();
    EXPECT_EQ(pBmp[0], 4);
    EXPECT_EQ(pBmp[1], 5);
    EXPECT_EQ(pBmp[2], 6);

    EXPECT_EQ(src.GetBitmap(), nullptr);
}

TEST(ImageTest, GetColor32_AllFormats_ReturnsCorrectColor)
{
    {
        uint8_t data[] = {128};
        Image img(Path(std::string()), "", Image::eFormat::R8_UNORM, 1, 1, 1, data);
        Color32 c = img.GetColor32(0, 0);
        EXPECT_EQ(c, Color32(128, 128, 128, 255));
    }

    {
        uint8_t data[] = {10, 20};
        Image img(Path(std::string()), "", Image::eFormat::R8G8_UNORM, 1, 1, 2, data);
        Color32 c = img.GetColor32(0, 0);
        EXPECT_EQ(c, Color32(10, 20, 0, 255));
    }

    {
        uint8_t data[] = {100, 150, 200};
        Image img(Path(std::string()), "", Image::eFormat::R8G8B8_UNORM, 1, 1, 3, data);
        Color32 c = img.GetColor32(0, 0);
        EXPECT_EQ(c, Color32(100, 150, 200, 255));
    }

    {
        uint8_t data[] = {50, 100, 150, 200};
        Image img(Path(std::string()), "", Image::eFormat::R8G8B8A8_UNORM, 1, 1, 4, data);
        Color32 c = img.GetColor32(0, 0);
        EXPECT_EQ(c, Color32(50, 100, 150, 200));
    }

    {
        float data[] = {1.0f, 0.5f, 0.0f, 1.0f};
        Image img(
            Path(std::string()), "", Image::eFormat::R32G32B32A32_FLOAT, 1, 1, 4, reinterpret_cast<uint8_t*>(data));
        Color32 c = img.GetColor32(0, 0);
        EXPECT_EQ(c.R, 255);
        EXPECT_NEAR(c.G, 127, 1);
        EXPECT_EQ(c.B, 0);
        EXPECT_EQ(c.A, 255);
    }
}

TEST(ImageTest, GetColor128_AllFormats_ReturnsCorrectColor)
{
    {
        float data[] = {0.75f};
        Image img(Path(std::string()), "", Image::eFormat::R32_FLOAT, 1, 1, 1, reinterpret_cast<uint8_t*>(data));
        Color128 c = img.GetColor128(0, 0);
        EXPECT_FLOAT_EQ(c.R, 0.75f);
        EXPECT_FLOAT_EQ(c.G, 0.75f);
        EXPECT_FLOAT_EQ(c.B, 0.75f);
        EXPECT_FLOAT_EQ(c.A, 1.0f);
    }

    {
        float data[] = {0.1f, 0.2f};
        Image img(Path(std::string()), "", Image::eFormat::R32G32_FLOAT, 1, 1, 2, reinterpret_cast<uint8_t*>(data));
        Color128 c = img.GetColor128(0, 0);
        EXPECT_FLOAT_EQ(c.R, 0.1f);
        EXPECT_FLOAT_EQ(c.G, 0.2f);
        EXPECT_FLOAT_EQ(c.B, 0.0f);
    }

    {
        float data[] = {0.1f, 0.2f, 0.3f, 0.4f};
        Image img(
            Path(std::string()), "", Image::eFormat::R32G32B32A32_FLOAT, 1, 1, 4, reinterpret_cast<uint8_t*>(data));
        Color128 c = img.GetColor128(0, 0);
        EXPECT_FLOAT_EQ(c.R, 0.1f);
        EXPECT_FLOAT_EQ(c.G, 0.2f);
        EXPECT_FLOAT_EQ(c.B, 0.3f);
        EXPECT_FLOAT_EQ(c.A, 0.4f);
    }

    {
        uint8_t data[] = {255, 127, 0, 255};
        Image img(Path(std::string()), "", Image::eFormat::R8G8B8A8_UNORM, 1, 1, 4, data);
        Color128 c = img.GetColor128(0, 0);
        EXPECT_NEAR(c.R, 1.0f, math::EPSILON_CMP);
        EXPECT_NEAR(c.G, 127.0f / 255.0f, math::EPSILON_CMP);
        EXPECT_NEAR(c.B, 0.0f, math::EPSILON_CMP);
        EXPECT_NEAR(c.A, 1.0f, math::EPSILON_CMP);
    }
}