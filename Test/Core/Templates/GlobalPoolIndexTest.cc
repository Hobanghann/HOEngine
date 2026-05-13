#include "Core/Templates/GlobalPoolIndex.h"

#include <gtest/gtest.h>
#include <string>
#include <unordered_set>

#include "Core/Templates/ObjectPool.h"
#include "Core/Templates/UniqueObjectPool.h"

using namespace ho;

struct TestValue
{
    int32_t a;
    std::string b;
    bool bCopyed = false;
    bool bMoved = false;

    TestValue(int32_t p_a, std::string p_b)
      : a(p_a)
      , b(std::move(p_b))
    {
    }

    TestValue(const TestValue& v)
      : a(v.a)
      , b(v.b)
      , bCopyed(true)
      , bMoved(false)
    {
    }

    TestValue(TestValue&& v) noexcept
      : a(v.a)
      , b(std::move(v.b))
      , bCopyed(false)
      , bMoved(true)
    {
    }

    bool operator==(const TestValue& rhs) const
    {
        return a == rhs.a && b == rhs.b;
    }
};

using GlobalIndex = GlobalPoolIndex<TestValue, ObjectPool>;
using GlobalUniqueIndex = GlobalPoolIndex<TestValue, UniqueObjectPool>;

class GlobalPoolIndexTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        GlobalIndex::GetMutablePool().Clear();
        GlobalUniqueIndex::GetMutablePool().Clear();
    }
};

TEST_F(GlobalPoolIndexTest, Constructor_CopyInitialValue_AddValueInPool)
{
    TestValue v1(10, "first");
    TestValue v2(20, "second");

    GlobalIndex idx1(v1);
    GlobalIndex idx2(v2);

    GlobalUniqueIndex uniqueIdx1(v1);
    GlobalUniqueIndex uniqueIdx2(v2);

    EXPECT_TRUE(idx1.IsValid());
    EXPECT_TRUE(idx2.IsValid());

    EXPECT_EQ(idx1.GetPool().GetSize(), 2);

    ASSERT_NE(idx1.Get(), nullptr);
    ASSERT_NE(idx2.Get(), nullptr);

    EXPECT_EQ(idx1.Get()->a, 10);
    EXPECT_EQ(idx1.Get()->b, "first");
    EXPECT_EQ(idx1.Get()->bCopyed, true);
    EXPECT_EQ(idx1.Get()->bMoved, false);
    EXPECT_EQ(idx2.Get()->a, 20);
    EXPECT_EQ(idx2.Get()->b, "second");
    EXPECT_EQ(idx2.Get()->bCopyed, true);
    EXPECT_EQ(idx2.Get()->bMoved, false);

    EXPECT_TRUE(uniqueIdx1.IsValid());
    EXPECT_TRUE(uniqueIdx2.IsValid());

    EXPECT_EQ(uniqueIdx1.GetPool().GetSize(), 2);

    ASSERT_NE(uniqueIdx1.Get(), nullptr);
    ASSERT_NE(uniqueIdx2.Get(), nullptr);

    EXPECT_EQ(uniqueIdx1.Get()->a, 10);
    EXPECT_EQ(uniqueIdx1.Get()->b, "first");
    EXPECT_EQ(uniqueIdx1.Get()->bCopyed, true);
    EXPECT_EQ(uniqueIdx1.Get()->bMoved, false);
    EXPECT_EQ(uniqueIdx2.Get()->a, 20);
    EXPECT_EQ(uniqueIdx2.Get()->b, "second");
    EXPECT_EQ(uniqueIdx2.Get()->bCopyed, true);
    EXPECT_EQ(uniqueIdx2.Get()->bMoved, false);
}

TEST_F(GlobalPoolIndexTest, Constructor_MoveInitialValue_AddValueInPool)
{
    TestValue v1(10, "first");
    TestValue v2(20, "second");

    GlobalIndex idx1(std::move(v1));
    GlobalIndex idx2(std::move(v2));

    TestValue uv1(10, "first");
    TestValue uv2(20, "second");

    GlobalUniqueIndex uniqueIdx1(std::move(uv1));
    GlobalUniqueIndex uniqueIdx2(std::move(uv2));

    EXPECT_TRUE(idx1.IsValid());
    EXPECT_TRUE(idx2.IsValid());

    EXPECT_EQ(idx1.GetPool().GetSize(), 2);

    ASSERT_NE(idx1.Get(), nullptr);
    ASSERT_NE(idx2.Get(), nullptr);

    EXPECT_EQ(idx1.Get()->a, 10);
    EXPECT_EQ(idx1.Get()->b, "first");
    EXPECT_EQ(idx1.Get()->bCopyed, false);
    EXPECT_EQ(idx1.Get()->bMoved, true);
    EXPECT_EQ(idx2.Get()->a, 20);
    EXPECT_EQ(idx2.Get()->b, "second");
    EXPECT_EQ(idx2.Get()->bCopyed, false);
    EXPECT_EQ(idx2.Get()->bMoved, true);

    EXPECT_TRUE(uniqueIdx1.IsValid());
    EXPECT_TRUE(uniqueIdx2.IsValid());

    EXPECT_EQ(uniqueIdx1.GetPool().GetSize(), 2);

    ASSERT_NE(uniqueIdx1.Get(), nullptr);
    ASSERT_NE(uniqueIdx2.Get(), nullptr);

    EXPECT_EQ(uniqueIdx1.Get()->a, 10);
    EXPECT_EQ(uniqueIdx1.Get()->b, "first");
    EXPECT_EQ(uniqueIdx1.Get()->bCopyed, false);
    EXPECT_EQ(uniqueIdx1.Get()->bMoved, true);
    EXPECT_EQ(uniqueIdx2.Get()->a, 20);
    EXPECT_EQ(uniqueIdx2.Get()->b, "second");
    EXPECT_EQ(uniqueIdx2.Get()->bCopyed, false);
    EXPECT_EQ(uniqueIdx2.Get()->bMoved, true);
}

TEST_F(GlobalPoolIndexTest, Emplace_MultipleValues_AddValueInPool)
{
    GlobalIndex idx1 = GlobalIndex::Emplace(10, "first");
    GlobalIndex idx2 = GlobalIndex::Emplace(20, "second");

    EXPECT_TRUE(idx1.IsValid());
    EXPECT_TRUE(idx2.IsValid());

    EXPECT_EQ(idx1.GetPool().GetSize(), 2);

    ASSERT_NE(idx1.Get(), nullptr);
    ASSERT_NE(idx2.Get(), nullptr);

    EXPECT_EQ(idx1.Get()->a, 10);
    EXPECT_EQ(idx1.Get()->b, "first");
    EXPECT_EQ(idx1.Get()->bCopyed, false);
    EXPECT_EQ(idx1.Get()->bMoved, false);
    EXPECT_EQ(idx2.Get()->a, 20);
    EXPECT_EQ(idx2.Get()->b, "second");
    EXPECT_EQ(idx2.Get()->bCopyed, false);
    EXPECT_EQ(idx2.Get()->bMoved, false);
}

TEST_F(GlobalPoolIndexTest, Release_ValidIndex_InvalidatesIndexAndRemovesFromPool)
{
    GlobalIndex idx = GlobalIndex::Emplace(10, "first");

    EXPECT_TRUE(idx.IsValid());
    EXPECT_EQ(idx.GetPool().GetSize(), 1);

    bool bReleased = idx.Release();

    EXPECT_TRUE(bReleased);
    EXPECT_FALSE(idx.IsValid());
    EXPECT_TRUE(idx.IsNULL());
    EXPECT_EQ(idx.GetIndex(), 0);
    EXPECT_EQ(idx.GetPool().GetSize(), 0);

    EXPECT_FALSE(idx.Release());
}

TEST_F(GlobalPoolIndexTest, ComparisonOperators_BetweenIndices_ReflectsEquality)
{
    GlobalIndex idx1 = GlobalIndex::Emplace(10, "first");
    GlobalIndex idx2 = GlobalIndex::Emplace(20, "second");
    GlobalIndex idx1Copy = idx1;

    EXPECT_TRUE(idx1 == idx1Copy);
    EXPECT_FALSE(idx1 == idx2);
    EXPECT_TRUE(idx1 != idx2);
}

TEST_F(GlobalPoolIndexTest, Hash_AsKey_WorksWithStlMap)
{
    GlobalIndex idx1 = GlobalIndex::Emplace(10, "first");
    GlobalIndex idx2 = GlobalIndex::Emplace(20, "second");

    std::unordered_map<GlobalIndex, int32_t> indexMap;
    indexMap.insert({idx1, 1});
    indexMap.insert({idx2, 2});

    EXPECT_EQ(indexMap.size(), 2);
    EXPECT_TRUE(indexMap.find(idx1) != indexMap.end());
    EXPECT_TRUE(indexMap.find(GlobalIndex::sNULL) == indexMap.end());
}
