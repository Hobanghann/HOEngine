#include "Core/Templates/LocalPoolIndex.h"

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

    TestValue(TestValue&& v)
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

using ObjPool = ObjectPool<TestValue>;
using UniquePool = UniqueObjectPool<TestValue>;

using Index = LocalPoolIndex<TestValue, ObjectPool>;
using UniqueIndex = LocalPoolIndex<TestValue, UniqueObjectPool>;

TEST(LocalPoolIndexTest, Constructor_Default_InitialIndexIsNull)
{
    Index nullIdx;

    EXPECT_TRUE(nullIdx.IsNULL());
    EXPECT_FALSE(nullIdx.IsValid());
    EXPECT_EQ(nullIdx.GetIndex(), 0);
    EXPECT_EQ(nullIdx, Index::sNull);
}

TEST(LocalPoolIndexTest, Constructor_CopyInitialValue_AddValueInPool)
{
    ObjPool pool;
    UniquePool uniquePool;

    TestValue v1(10, "first");
    TestValue v2(20, "second");

    Index idx1(&pool, v1);
    Index idx2(&pool, v2);

    UniqueIndex uniqueIdx1(&uniquePool, v1);
    UniqueIndex uniqueIdx2(&uniquePool, v2);

    EXPECT_TRUE(idx1.IsValid());
    EXPECT_TRUE(idx2.IsValid());

    EXPECT_EQ(pool.GetSize(), 2);

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

    EXPECT_EQ(uniquePool.GetSize(), 2);

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

TEST(LocalPoolIndexTest, Constructor_MoveInitialValue_AddValueInPool)
{
    ObjPool pool;
    UniquePool uniquePool;

    TestValue v1(10, "first");
    TestValue v2(20, "second");

    Index idx1(&pool, std::move(v1));
    Index idx2(&pool, std::move(v2));

    TestValue uv1(10, "first");
    TestValue uv2(20, "second");

    UniqueIndex uniqueIdx1(&uniquePool, std::move(uv1));
    UniqueIndex uniqueIdx2(&uniquePool, std::move(uv2));

    EXPECT_TRUE(idx1.IsValid());
    EXPECT_TRUE(idx2.IsValid());

    EXPECT_EQ(pool.GetSize(), 2);

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

    EXPECT_EQ(uniquePool.GetSize(), 2);

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

TEST(LocalPoolIndexTest, Emplace_MultipleValues_AddValueInPool)
{
    ObjPool pool;

    Index idx1 = Index::Emplace(&pool, 10, "first");
    Index idx2 = Index::Emplace(&pool, 20, "second");

    EXPECT_TRUE(idx1.IsValid());
    EXPECT_TRUE(idx2.IsValid());

    EXPECT_EQ(pool.GetSize(), 2);

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

TEST(LocalPoolIndexTest, Release_ValidIndex_InvalidatesIndexAndRemovesFromPool)
{
    ObjPool pool;
    Index idx = Index::Emplace(&pool, 10, "first");

    EXPECT_TRUE(idx.IsValid());
    EXPECT_EQ(pool.GetSize(), 1);

    bool bReleased = idx.Release();

    EXPECT_TRUE(bReleased);
    EXPECT_FALSE(idx.IsValid());
    EXPECT_TRUE(idx.IsNULL());
    EXPECT_EQ(idx.GetIndex(), 0);
    EXPECT_EQ(pool.GetSize(), 0);
}

TEST(LocalPoolIndexTest, ComparisonOperators_BetweenIndices_ReflectsEquality)
{
    ObjPool pool;
    Index idx1 = Index::Emplace(&pool, 10, "first");
    Index idx2 = Index::Emplace(&pool, 20, "second");
    Index idx1Copy = idx1;

    EXPECT_TRUE(idx1 == idx1Copy);
    EXPECT_FALSE(idx1 == idx2);
    EXPECT_TRUE(idx1 != idx2);
}

TEST(LocalPoolIndexTest, Hash_AsKey_WorksWithStlMap)
{
    ObjPool pool;
    Index idx1 = Index::Emplace(&pool, 10, "first");
    Index idx2 = Index::Emplace(&pool, 20, "second");

    std::unordered_map<Index, int32_t> indexMap;
    indexMap.insert({idx1, 1});
    indexMap.insert({idx2, 2});

    EXPECT_EQ(indexMap.size(), 2);
    EXPECT_TRUE(indexMap.find(idx1) != indexMap.end());
    EXPECT_TRUE(indexMap.find(Index::sNull) == indexMap.end());
}
