#include "Core/Memory/UniqueTypedObjectPool.h"

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

using UniqueTestPool = UniqueTypedObjectPool<TestValue>;

TEST(UniqueTypedObjectPoolTest, Constructor_Default_InitialSizeIsZero)
{
    UniqueTestPool pool;

    EXPECT_EQ(pool.GetSize(), 0);
    EXPECT_EQ(pool.GetFullSize(), 1);
}

TEST(UniqueTypedObjectPoolTest, Add_MultipleValuesByCopy_StoresCorrectData)
{
    UniqueTestPool pool;

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

TEST(UniqueTypedObjectPoolTest, Add_MultipleSameValuesByCopy_CreatesIdenticalEntries)
{
    UniqueTestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint64_t id1 = pool.Add(v1);
    uint64_t id2 = pool.Add(v2);

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_EQ(id1, id2);
    EXPECT_EQ(pool.GetSize(), 1);

    TestValue* p1 = pool.Get(id1);
    TestValue* p2 = pool.Get(id2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, true);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p1, p2);
}

TEST(UniqueTypedObjectPoolTest, Add_MultipleValuesByMove_StoresCorrectData)
{
    UniqueTestPool pool;

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

TEST(UniqueTypedObjectPoolTest, Add_MultipleSameValuesByMove_CreatesIdenticalEntries)
{
    UniqueTestPool pool;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    uint64_t id1 = pool.Add(std::move(v1));
    uint64_t id2 = pool.Add(std::move(v2));

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_EQ(id1, id2);
    EXPECT_EQ(pool.GetSize(), 1);

    TestValue* p1 = pool.Get(id1);
    TestValue* p2 = pool.Get(id2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, true);

    EXPECT_EQ(p1, p2);
}

TEST(UniqueTypedObjectPoolTest, Emplace_MultipleValues_StoresCorrectData)
{
    UniqueTestPool pool;

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

TEST(UniqueTypedObjectPoolTest, Emplace_MultipleSameValuesByMove_CreatesIdenticalEntries)
{
    UniqueTestPool pool;

    uint64_t id1 = pool.Emplace(10, "same");
    uint64_t id2 = pool.Emplace(10, "same");

    EXPECT_EQ(id1, id2);
    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_EQ(pool.GetSize(), 1);

    const TestValue* p1 = pool.Get(id1);
    const TestValue* p2 = pool.Get(id2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p1, p2);
}

TEST(UniqueTypedObjectPoolTest, Remove_ExistingEntries_DecrementsSizeAndRemovesEntry)
{
    UniqueTestPool pool;

    uint64_t id1 = pool.Emplace(10, "A");
    uint64_t id2 = pool.Emplace(20, "B");
    TestValue valC(30, "C");
    uint64_t id3 = pool.Add(valC);

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_TRUE(pool.Has(id3));
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

TEST(UniqueTypedObjectPoolTest, Emplace_WhenThereIsFreeIndex_ReuseFreeIndexWithNewGeneration)
{
    UniqueTestPool pool;

    uint64_t id1 = pool.Emplace(1, "item1");
    uint64_t id2 = pool.Emplace(2, "item2");

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
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

TEST(UniqueTypedObjectPoolTest, Has_AfterEmplace_ReturnsTrueForExistingIDs)
{
    UniqueTestPool pool;
    EXPECT_FALSE(pool.Has(0));

    uint64_t id1 = pool.Emplace(10, "first");
    uint64_t id2 = pool.Emplace(20, "second");
    uint64_t id3 = pool.Emplace(30, "third");

    EXPECT_TRUE(pool.Has(id1));
    EXPECT_TRUE(pool.Has(id2));
    EXPECT_TRUE(pool.Has(id3));
}

TEST(UniqueTypedObjectPoolTest, Find_AfterEmplace_ReturnsPointerForExistingIDs)
{
    UniqueTestPool pool;
    EXPECT_EQ(pool.Get(0), nullptr);

    uint64_t id1 = pool.Emplace(10, "first");
    uint64_t id2 = pool.Emplace(20, "second");
    uint64_t id3 = pool.Emplace(30, "third");

    EXPECT_NE(pool.Get(id1), nullptr);
    EXPECT_NE(pool.Get(id2), nullptr);
    EXPECT_NE(pool.Get(id3), nullptr);
}

TEST(UniqueTypedObjectPoolTest, Clear_ExistingEntries_InvalidatesAllIDsAndResetsSize)
{
    UniqueTestPool pool;

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