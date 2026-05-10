#include "Core/Templates/UniqueObjectPool.h"

#include <functional>
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

namespace std
{
template <>
struct hash<TestValue>
{
    size_t operator()(const TestValue& k) const noexcept
    {
        size_t h1 = std::hash<int32_t>{}(k.a);
        size_t h2 = std::hash<std::string>{}(k.b);
        return h1 ^ (h2 << 1);
    }
};
} // namespace std

using UniqueTestPool = UniqueObjectPool<TestValue>;

TEST(UniqueObjectPoolTest, Constructor_Default_InitialSizeIsZero)
{
    UniqueTestPool pool;

    EXPECT_EQ(pool.GetSize(), 0);
    EXPECT_EQ(pool.GetFullSize(), 1);
}

TEST(UniqueObjectPoolTest, Add_MultipleValuesByCopy_StoresCorrectData)
{
    UniqueTestPool pool;

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

TEST(UniqueObjectPoolTest, Add_MultipleSameValuesByCopy_CreatesIdenticalEntries)
{
    UniqueTestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint32_t idx1 = pool.Add(v1);
    uint32_t idx2 = pool.Add(v2);

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(idx1, idx2);
    EXPECT_EQ(pool.GetSize(), 1);

    TestValue* p1 = pool.Get(idx1);
    TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, true);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p1, p2);
}

TEST(UniqueObjectPoolTest, Add_MultipleValuesByMove_StoresCorrectData)
{
    UniqueTestPool pool;

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

TEST(UniqueObjectPoolTest, Add_MultipleSameValuesByMove_CreatesIdenticalEntries)
{
    UniqueTestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint32_t idx1 = pool.Add(std::move(v1));
    uint32_t idx2 = pool.Add(std::move(v2));

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(idx1, idx2);
    EXPECT_EQ(pool.GetSize(), 1);

    TestValue* p1 = pool.Get(idx1);
    TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, true);

    EXPECT_EQ(p1, p2);
}

TEST(UniqueObjectPoolTest, Emplace_MultipleValues_StoresCorrectData)
{
    UniqueTestPool pool;

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

TEST(UniqueObjectPoolTest, Emplace_MultipleSameValuesByMove_CreatesIdenticalEntries)
{
    UniqueTestPool pool;

    uint32_t idx1 = pool.Emplace(10, "same");
    uint32_t idx2 = pool.Emplace(10, "same");

    EXPECT_EQ(idx1, idx2);
    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 1);

    const TestValue* p1 = pool.Get(idx1);
    const TestValue* p2 = pool.Get(idx2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p1, p2);
}

TEST(UniqueObjectPoolTest, Remove_ExistingEntries_DecrementsSizeAndRemovesEntry)
{
    UniqueTestPool pool;

    uint32_t idx1 = pool.Emplace(10, "A");
    uint32_t idx2 = pool.Emplace(20, "B");
    TestValue valC(30, "C");
    uint32_t idx3 = pool.Add(valC);

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_TRUE(pool.Has(idx3));
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

TEST(UniqueObjectPoolTest, Emplace_WhenThereIsFreeIndex_ReuseFreeIndex)
{
    UniqueTestPool pool;

    uint32_t idx1 = pool.Emplace(1, "item1");
    uint32_t idx2 = pool.Emplace(2, "item2");

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_EQ(pool.GetSize(), 2);

    pool.Remove(idx1);
    EXPECT_EQ(pool.GetSize(), 1);
    EXPECT_FALSE(pool.Has(idx1));

    uint32_t idx3 = pool.Emplace(3, "item3");

    EXPECT_EQ(idx3, idx1);
    EXPECT_EQ(pool.GetSize(), 2);
    EXPECT_EQ(pool.Get(idx3)->a, 3);
}

TEST(UniqueObjectPoolTest, Has_AfterEmplace_ReturnsTrueForExistingIDs)
{
    UniqueTestPool pool;
    EXPECT_FALSE(pool.Has(0));

    uint32_t idx1 = pool.Emplace(10, "first");
    uint32_t idx2 = pool.Emplace(20, "second");
    uint32_t idx3 = pool.Emplace(30, "third");

    EXPECT_TRUE(pool.Has(idx1));
    EXPECT_TRUE(pool.Has(idx2));
    EXPECT_TRUE(pool.Has(idx3));
}

TEST(UniqueObjectPoolTest, Find_AfterEmplace_ReturnsPointerForExistingIDs)
{
    UniqueTestPool pool;
    EXPECT_EQ(pool.Get(0), nullptr);

    uint32_t idx1 = pool.Emplace(10, "first");
    uint32_t idx2 = pool.Emplace(20, "second");
    uint32_t idx3 = pool.Emplace(30, "third");

    EXPECT_NE(pool.Get(idx1), nullptr);
    EXPECT_NE(pool.Get(idx2), nullptr);
    EXPECT_NE(pool.Get(idx3), nullptr);
}

TEST(UniqueObjectPoolTest, Clear_ExistingEntries_InvalidatesAllIDsAndResetsSize)
{
    UniqueTestPool pool;

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