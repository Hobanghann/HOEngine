#include "Core/Memory/TypedObjectPool.h"

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

using TestPool = TypedObjectPool<TestValue>;

TEST(TypedObjectPoolTest, Constructor_Default_InitialSizeIsZero)
{
    TestPool pool;

    EXPECT_EQ(pool.GetSize(), 0);
    EXPECT_EQ(pool.GetFullSize(), 1);
}

TEST(TypedObjectPoolTest, Add_MultipleValuesByCopy_StoresCorrectData)
{
    TestPool pool;

    TestValue v1(10, "first");
    TestValue v2(20, "second");

    uint64_t id1 = pool.Add(v1);
    uint64_t id2 = pool.Add(v2);

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(id1);
    TestValue* p2 = pool.Get(id2);

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

TEST(TypedObjectPoolTest, Add_MultipleSameValuesByCopy_CreatesDistinctEntries)
{
    TestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint64_t id1 = pool.Add(v1);
    uint64_t id2 = pool.Add(v2);

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_NE(id1, id2);
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(id1);
    TestValue* p2 = pool.Get(id2);

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

TEST(TypedObjectPoolTest, Add_MultipleValuesByMove_StoresCorrectData)
{
    TestPool pool;

    TestValue v1(10, "first");
    TestValue v2(20, "second");

    uint64_t id1 = pool.Add(std::move(v1));
    uint64_t id2 = pool.Add(std::move(v2));

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(id1);
    TestValue* p2 = pool.Get(id2);

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

TEST(TypedObjectPoolTest, Add_MultipleSameValuesByMove_CreatesDistinctEntries)
{
    TestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint64_t id1 = pool.Add(std::move(v1));
    uint64_t id2 = pool.Add(std::move(v2));

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_NE(id1, id2);
    EXPECT_EQ(pool.GetSize(), 2);

    TestValue* p1 = pool.Get(id1);
    TestValue* p2 = pool.Get(id2);

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

TEST(TypedObjectPoolTest, Emplace_MultipleValues_StoresCorrectData)
{
    TestPool pool;

    uint64_t id1 = pool.Emplace(10, "first");
    uint64_t id2 = pool.Emplace(20, "second");

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_EQ(pool.GetSize(), 2);

    const TestValue* p1 = pool.Get(id1);
    const TestValue* p2 = pool.Get(id2);

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

TEST(TypedObjectPoolTest, Emplace_MultipleSameValuesByMove_CreatesDistinctEntries)
{
    TestPool pool;

    uint64_t id1 = pool.Emplace(10, "same");
    uint64_t id2 = pool.Emplace(10, "same");

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_NE(id1, id2);
    EXPECT_EQ(pool.GetSize(), 2);

    const TestValue* p1 = pool.Get(id1);
    const TestValue* p2 = pool.Get(id2);

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

TEST(TypedObjectPoolTest, Remove_ExistingEntries_DecrementsSizeAndRemovesEntry)
{
    TestPool pool;

    uint64_t id1 = pool.Emplace(10, "A");
    pool.Emplace(20, "B");
    TestValue valC(30, "C");
    uint64_t id3 = pool.Add(valC);

    EXPECT_EQ(pool.GetSize(), 3);

    bool bDeleted = pool.Remove(valC);

    EXPECT_TRUE(bDeleted);
    EXPECT_EQ(pool.GetSize(), 2);
    EXPECT_FALSE(pool.Has(id3));
    EXPECT_EQ(pool.Get(id3), nullptr);

    bDeleted = pool.Remove(id1);

    EXPECT_TRUE(bDeleted);
    EXPECT_EQ(pool.GetSize(), 1);
    EXPECT_FALSE(pool.Has(id1));
    EXPECT_EQ(pool.Get(id1), nullptr);
}

TEST(TypedObjectPoolTest, Emplace_WhenThereIsFreeIndex_ReuseFreeIndexWithNewGeneration)
{
    TestPool pool;

    uint64_t id1 = pool.Emplace(1, "item1");
    pool.Emplace(2, "item2");

    EXPECT_EQ(pool.GetSize(), 2);

    pool.Remove(id1);
    EXPECT_EQ(pool.GetSize(), 1);
    EXPECT_FALSE(pool.Has(id1));

    uint64_t id3 = pool.Emplace(3, "item3");

    uint32_t index1 = static_cast<uint32_t>(id1 & 0xFFFFFFFF);
    uint32_t index3 = static_cast<uint32_t>(id3 & 0xFFFFFFFF);

    EXPECT_EQ(index3, index1);
    EXPECT_NE(id3, id1);

    EXPECT_FALSE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id3));
    EXPECT_EQ(pool.GetSize(), 2);
    EXPECT_EQ(pool.Get(id3)->a, 3);
}

TEST(TypedObjectPoolTest, Has_AfterEmplace_ReturnsTrueForExistingIDs)
{
    TestPool pool;
    EXPECT_FALSE(pool.Has(0));

    uint64_t id1 = pool.Emplace(10, "first");
    uint64_t id2 = pool.Emplace(20, "second");
    uint64_t id3 = pool.Emplace(30, "third");

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_TRUE(pool.Has(id3));
}

TEST(TypedObjectPoolTest, Find_AfterEmplace_ReturnsPointerForExistingIDs)
{
    TestPool pool;
    EXPECT_EQ(pool.Get(0), nullptr);

    uint64_t id1 = pool.Emplace(10, "first");
    uint64_t id2 = pool.Emplace(20, "second");
    uint64_t id3 = pool.Emplace(30, "third");

    EXPECT_NE(pool.Get(id1), nullptr);
    EXPECT_NE(pool.Get(id2), nullptr);
    EXPECT_NE(pool.Get(id3), nullptr);
}

TEST(TypedObjectPoolTest, Clear_ExistingEntries_InvalidatesAllIDsAndResetsSize)
{
    TestPool pool;

    uint64_t id1 = pool.Emplace(1, "1");
    uint64_t id2 = pool.Emplace(2, "2");
    uint64_t id3 = pool.Emplace(3, "3");

    EXPECT_EQ(pool.GetSize(), 3);

    pool.Clear();
    EXPECT_EQ(pool.GetSize(), 0);

    EXPECT_EQ(pool.GetFullSize(), 1);
    EXPECT_FALSE(pool.Has(id1));
    EXPECT_FALSE(pool.Has(id2));
    EXPECT_FALSE(pool.Has(id3));
}