#include "Core/Templates/ID.h"

#include <gtest/gtest.h>
#include <unordered_map>

using namespace ho;

// Dummy
struct Test
{
};

using TestID = ID<Test>;

TEST(IDTest, Constructor_Default_ValidInitialState)
{
    TestID id;

    EXPECT_EQ(id, TestID::sNull);
}

TEST(IDTest, Increment_MultipleTimes_AccumulatesValueCorrectly)
{
    TestID id;

    id.Increment();
    EXPECT_EQ(id.GetID(), 1ull);

    id.Increment().Increment().Increment();
    EXPECT_EQ(id.GetID(), 4ull);

    EXPECT_TRUE(id.IsValid());
}

TEST(IDTest, Decrement_MultipleTimes_AccumulatesValueCorrectly)
{
    TestID id;
    id.Increment().Increment().Increment().Increment();

    id.Decrement();
    EXPECT_EQ(id.GetID(), 3ull);

    id.Decrement().Decrement().Decrement();
    EXPECT_EQ(id.GetID(), 0ull);
}

TEST(IDTest, ComparisonOperators_AfterStateChange_ReflectsEquality)
{
    TestID a;
    TestID b;

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    a.Increment();
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);

    b.Increment();
    EXPECT_TRUE(a == b);
}

TEST(IDTest, Hash_AsKey_WorksWithStlMap)
{
    std::unordered_map<TestID, int32_t> map;

    TestID a;
    TestID b;

    a.Increment();
    b.Increment().Increment();

    map[a] = 10;
    map[b] = 20;

    EXPECT_EQ(map[a], 10);
    EXPECT_EQ(map[b], 20);

    EXPECT_NE(map[a], map[b]);
}
