#include "Core/Math/AABB.h"

#include <gtest/gtest.h>

using namespace ho;

TEST(AABBTest, Constructor_DefaultAndAllParameters_InitializesCorrectly)
{
    AABB dbox;
    EXPECT_EQ(dbox.MinEdges, Vector3::sZero);
    EXPECT_EQ(dbox.MaxEdges, Vector3::sZero);

    Vector3 min(-1.0f, -2.0f, -3.0f);
    Vector3 max(3.0f, 2.0f, 1.0f);
    AABB box(min, max);
    EXPECT_EQ(box.MinEdges, min);
    EXPECT_EQ(box.MaxEdges, max);

    Vector3 expectedCenter = 0.5f * (min + max);
    EXPECT_FLOAT_EQ(box.GetCenter().X, expectedCenter.X);
    EXPECT_FLOAT_EQ(box.GetCenter().Y, expectedCenter.Y);
    EXPECT_FLOAT_EQ(box.GetCenter().Z, expectedCenter.Z);
}

TEST(AABBTest, OperatorAssign_AnotherAABB_CopiesEdgesCorrectly)
{
    AABB a(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB b;
    b = a;
    EXPECT_EQ(a, b);
}

TEST(AABBTest, OperatorComparison_DifferentBoxes_ReturnsCorrectBoolean)
{
    AABB a(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB b(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB c(Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a == c);
}

TEST(AABBTest, IsEqualApprox_WithinEpsilon_ReturnsTrue)
{
    AABB a(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB b(Vector3(-1.0000001f, -1.0000001f, -1.0000001f), Vector3(1.0000001f, 1.0000001f, 1.0000001f));
    AABB c(Vector3(-2.0f, -2.0f, -2.0f), Vector3(2.0f, 2.0f, 2.0f));

    EXPECT_TRUE(a.IsEqualApprox(b));
    EXPECT_FALSE(a.IsNotEqualApprox(b));

    EXPECT_FALSE(a.IsEqualApprox(c));
    EXPECT_TRUE(a.IsNotEqualApprox(c));
}

TEST(AABBTest, FromPositions_ArrayOfPoints_GeneratesTightestBox)
{
    std::vector<Vector3> positions = {
        Vector3(1.0f, 2.0f, 3.0f),
        Vector3(-2.0f, 5.0f, 1.0f),
        Vector3(0.0f, -1.0f, 4.0f),
    };

    AABB box = AABB::FromPositions(positions.data(), positions.size());

    EXPECT_FLOAT_EQ(box.MinEdges.X, -2.0f);
    EXPECT_FLOAT_EQ(box.MinEdges.Y, -1.0f);
    EXPECT_FLOAT_EQ(box.MinEdges.Z, 1.0f);

    EXPECT_FLOAT_EQ(box.MaxEdges.X, 1.0f);
    EXPECT_FLOAT_EQ(box.MaxEdges.Y, 5.0f);
    EXPECT_FLOAT_EQ(box.MaxEdges.Z, 4.0f);

    Vector3 expectedCenter = 0.5f * (box.MinEdges + box.MaxEdges);
    EXPECT_EQ(box.GetCenter(), expectedCenter);

    positions = {Vector3(1.0f, 2.0f, 3.0f)};
    box = AABB::FromPositions(positions.data(), positions.size());
    EXPECT_FLOAT_EQ(box.MinEdges.X, 1.0f);
    EXPECT_FLOAT_EQ(box.MinEdges.Y, 2.0f);
    EXPECT_FLOAT_EQ(box.MinEdges.Z, 3.0f);
    EXPECT_FLOAT_EQ(box.MaxEdges.X, 1.0f);
    EXPECT_FLOAT_EQ(box.MaxEdges.Y, 2.0f);
    EXPECT_FLOAT_EQ(box.MaxEdges.Z, 3.0f);
}

TEST(AABBTest, Merge_TwoSeparatedBoxes_EncapsulatesBoth)
{
    AABB box1(Vector3(-3.0f, -1.0f, -1.0f), Vector3(-1.0f, 1.0f, 1.0f));
    AABB box2(Vector3(2.0f, -2.0f, -2.0f), Vector3(4.0f, 2.0f, 2.0f));

    AABB merged = AABB::Merge(box1, box2);

    EXPECT_FLOAT_EQ(merged.MinEdges.X, -3.0f);
    EXPECT_FLOAT_EQ(merged.MinEdges.Y, -2.0f);
    EXPECT_FLOAT_EQ(merged.MinEdges.Z, -2.0f);

    EXPECT_FLOAT_EQ(merged.MaxEdges.X, 4.0f);
    EXPECT_FLOAT_EQ(merged.MaxEdges.Y, 2.0f);
    EXPECT_FLOAT_EQ(merged.MaxEdges.Z, 2.0f);

    Vector3 expectedCenter = 0.5f * (merged.MinEdges + merged.MaxEdges);
    EXPECT_TRUE(merged.GetCenter().IsEqualApprox(expectedCenter));
}

TEST(AABBTest, Merge_TwoOverlappingBoxes_EncapsulatesBoth)
{
    AABB box1(Vector3(-2.0f, -2.0f, -2.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB box2(Vector3(0.0f, 0.0f, 0.0f), Vector3(3.0f, 3.0f, 3.0f));

    AABB merged = AABB::Merge(box1, box2);

    EXPECT_FLOAT_EQ(merged.MinEdges.X, -2.0f);
    EXPECT_FLOAT_EQ(merged.MinEdges.Y, -2.0f);
    EXPECT_FLOAT_EQ(merged.MinEdges.Z, -2.0f);

    EXPECT_FLOAT_EQ(merged.MaxEdges.X, 3.0f);
    EXPECT_FLOAT_EQ(merged.MaxEdges.Y, 3.0f);
    EXPECT_FLOAT_EQ(merged.MaxEdges.Z, 3.0f);

    Vector3 expectedCenter = 0.5f * (merged.MinEdges + merged.MaxEdges);
    EXPECT_TRUE(merged.GetCenter().IsEqualApprox(expectedCenter));
}

TEST(AABBTest, Merge_OuterAndInnerBoxes_ReturnsOuterBox)
{
    AABB outer(Vector3(-5.0f, -5.0f, -5.0f), Vector3(5.0f, 5.0f, 5.0f));
    AABB inner(Vector3(-2.0f, -2.0f, -2.0f), Vector3(2.0f, 2.0f, 2.0f));

    AABB merged = AABB::Merge(outer, inner);

    EXPECT_TRUE(merged.IsEqualApprox(outer));
}

TEST(AABBTest, Merge_IdenticalBoxes_ReturnsSameBox)
{
    AABB box(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f));
    AABB merged = AABB::Merge(box, box);

    EXPECT_TRUE(merged.IsEqualApprox(box));
}