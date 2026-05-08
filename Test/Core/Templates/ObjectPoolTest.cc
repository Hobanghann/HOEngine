#include "Core/Templates/ObjectPool.h"

#include <gtest/gtest.h>
#include <string>

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

using TestPool = ObjectPool<TestValue>;

TEST(ObjectPoolTest, Constructor_Default_InitialSizeIsZero)
{
    TestPool pool;

    EXPECT_EQ(pool.GetSize(), 0);
    EXPECT_EQ(pool.GetFullSize(), 1);
}

TEST(ObjectPoolTest, Add_MultipleValuesByCopy_StoresCorrectData)
{
    TestPool pool;

    TestValue v1(10, "first");
    TestValue v2(20, "second");

    uint32_t idx1 = pool.Add(v1);
    uint32_t idx2 = pool.Add(v2);

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(idx1);
    TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "first");
    EXPECT_EQ(p1->bCopyed, true);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 20);
    EXPECT_EQ(p2->b, "second");
    EXPECT_EQ(p2->bCopyed, true);
    EXPECT_EQ(p2->bMoved, false);
}

TEST(ObjectPoolTest, Add_MultipleSameValuesByCopy_CreatesDistinctEntries)
{
    TestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint32_t idx1 = pool.Add(v1);
    uint32_t idx2 = pool.Add(v2);

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(idx1);
    TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, true);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 10);
    EXPECT_EQ(p2->b, "same");
    EXPECT_EQ(p2->bCopyed, true);
    EXPECT_EQ(p2->bMoved, false);
}

TEST(ObjectPoolTest, Add_MultipleValuesByMove_StoresCorrectData)
{
    TestPool pool;

    TestValue v1(10, "first");
    TestValue v2(20, "second");

    uint32_t idx1 = pool.Add(std::move(v1));
    uint32_t idx2 = pool.Add(std::move(v2));

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(idx1);
    TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "first");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, true);

    EXPECT_EQ(p2->a, 20);
    EXPECT_EQ(p2->b, "second");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, true);
}

TEST(ObjectPoolTest, Add_MultipleSameValuesByMove_CreatesDistinctEntries)
{
    TestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint32_t idx1 = pool.Add(std::move(v1));
    uint32_t idx2 = pool.Add(std::move(v2));

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(idx1);
    TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, true);

    EXPECT_EQ(p2->a, 10);
    EXPECT_EQ(p2->b, "same");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, true);
}

TEST(ObjectPoolTest, Emplace_MultipleValues_StoresCorrectData)
{
    TestPool pool;

    uint32_t idx1 = pool.Emplace(10, "first");
    uint32_t idx2 = pool.Emplace(20, "second");

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    const TestValue* p1 = pool.Get(idx1);
    const TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "first");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 20);
    EXPECT_EQ(p2->b, "second");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, false);
}

TEST(ObjectPoolTest, Emplace_MultipleSameValuesByMove_CreatesDistinctEntries)
{
    TestPool pool;

    uint32_t idx1 = pool.Emplace(10, "same");
    uint32_t idx2 = pool.Emplace(10, "same");

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    const TestValue* p1 = pool.Get(idx1);
    const TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 10);
    EXPECT_EQ(p2->b, "same");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, false);
}

TEST(ObjectPoolTest, Remove_ExistingEntries_DecrementsSizeAndRemovesEntry)
{
    TestPool pool;

    uint32_t idx1 = pool.Emplace(10, "A");
    pool.Emplace(20, "B");
    TestValue valC(30, "C");
    uint32_t idx3 = pool.Add(valC);

    EXPECT_EQ(pool.GetSize(), 3);

    bool bDeleted = pool.Remove(valC);

    EXPECT_TRUE(bDeleted);
    EXPECT_EQ(pool.GetSize(), 2);
    EXPECT_FALSE(pool.Has(idx3));
    EXPECT_EQ(pool.Get(idx3), nullptr);

    bDeleted = pool.Remove(idx1);

    EXPECT_TRUE(bDeleted);
    EXPECT_EQ(pool.GetSize(), 1);
    EXPECT_FALSE(pool.Has(idx1));
    EXPECT_EQ(pool.Get(idx1), nullptr);
}

TEST(ObjectPoolTest, Emplace_WhenThereIsFreeIndex_ReuseFreeIndex)
{
    TestPool pool;

    uint32_t idx1 = pool.Emplace(1, "item1");
    pool.Emplace(2, "item2");

    EXPECT_EQ(pool.GetSize(), 2);

    pool.Remove(idx1);
    EXPECT_EQ(pool.GetSize(), 1);
    EXPECT_FALSE(pool.Has(idx1));

    uint32_t idx3 = pool.Emplace(3, "item3");

    EXPECT_EQ(idx3, idx1);
    EXPECT_EQ(pool.GetSize(), 2);
    EXPECT_EQ(pool.Get(idx3)->a, 3);
}

TEST(ObjectPoolTest, Has_AfterEmplace_ReturnsTrueForExistingIDs)
{
    TestPool pool;
    EXPECT_FALSE(pool.Has(0));

    uint32_t idx1 = pool.Emplace(10, "first");
    uint32_t idx2 = pool.Emplace(20, "second");
    uint32_t idx3 = pool.Emplace(30, "third");

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_TRUE(pool.Has(idx3));
}

TEST(ObjectPoolTest, Find_AfterEmplace_ReturnsPointerForExistingIDs)
{
    TestPool pool;
    EXPECT_EQ(pool.Get(0), nullptr);

    uint32_t idx1 = pool.Emplace(10, "first");
    uint32_t idx2 = pool.Emplace(20, "second");
    uint32_t idx3 = pool.Emplace(30, "third");

    EXPECT_NE(pool.Get(idx1), nullptr);
    EXPECT_NE(pool.Get(idx2), nullptr);
    EXPECT_NE(pool.Get(idx3), nullptr);
}

TEST(ObjectPoolTest, Clear_ExistingEntries_InvalidatesAllIDsAndResetsSize)
{
    TestPool pool;

    uint32_t idx1 = pool.Emplace(1, "1");
    uint32_t idx2 = pool.Emplace(2, "2");
    uint32_t idx3 = pool.Emplace(3, "3");

    EXPECT_EQ(pool.GetSize(), 3);

    pool.Clear();
    EXPECT_EQ(pool.GetSize(), 0);

    EXPECT_EQ(pool.GetFullSize(), 1);
    EXPECT_FALSE(pool.Has(idx1));
    EXPECT_FALSE(pool.Has(idx2));
    EXPECT_FALSE(pool.Has(idx3));
}
