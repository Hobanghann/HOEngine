#include "Core/Templates/IDMap.h"

#include <gtest/gtest.h>
#include <string>

#include "Core/Templates/ID.h"

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
};

// Dummy
struct Test
{
};

using TestID = ID<Test>;
using TestMap = IDMap<TestID, TestValue>;

TEST(IDMapTest, Constructor_Default_InitialSizeIsZero)
{
    TestMap map;

    EXPECT_EQ(map.GetSize(), 0);
}

TEST(IDMapTest, Add_MultipleValuesByCopy_StoresCorrectData)
{
    TestMap map;

    TestValue v1(10, "first");
    TestValue v2(20, "second");
    TestValue v3(30, "third");

    TestID id1 = map.Add(v1);
    TestID id2 = map.Add(v2);
    TestID id3 = map.Add(v3);

    EXPECT_TRUE(id1.IsValid());
    EXPECT_TRUE(id2.IsValid());
    EXPECT_TRUE(id3.IsValid());
    EXPECT_EQ(map.GetSize(), 3);

    TestValue* p1 = map.Find(id1);
    TestValue* p2 = map.Find(id2);
    TestValue* p3 = map.Find(id3);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    EXPECT_EQ(p1->a, 10);
    EXPECT_EQ(p1->b, "first");
    EXPECT_EQ(p1->bCopyed, true);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 20);
    EXPECT_EQ(p2->b, "second");
    EXPECT_EQ(p2->bCopyed, true);
    EXPECT_EQ(p2->bMoved, false);

    EXPECT_EQ(p3->a, 30);
    EXPECT_EQ(p3->b, "third");
    EXPECT_EQ(p3->bCopyed, true);
    EXPECT_EQ(p3->bMoved, false);
}

TEST(IDMapTest, Add_MultipleSameValuesByCopy_CreatesDistinctEntries)
{
    TestMap map;

    TestValue v1(10, "same");
    TestValue v2(10, "same");

    TestID id1 = map.Add(v1);
    TestID id2 = map.Add(v2);

    EXPECT_TRUE(id1.IsValid());
    EXPECT_TRUE(id2.IsValid());
    EXPECT_EQ(map.GetSize(), 2);

    TestValue* p1 = map.Find(id1);
    TestValue* p2 = map.Find(id2);

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

TEST(IDMapTest, Add_MultipleValuesByMove_StoresCorrectData)
{
    TestMap map;

    TestValue v1(100, "first");
    TestValue v2(200, "second");
    TestValue v3(300, "third");

    TestID id1 = map.Add(std::move(v1));
    TestID id2 = map.Add(std::move(v2));
    TestID id3 = map.Add(std::move(v3));

    EXPECT_TRUE(id1.IsValid());
    EXPECT_TRUE(id2.IsValid());
    EXPECT_TRUE(id3.IsValid());
    EXPECT_EQ(map.GetSize(), 3);

    TestValue* p1 = map.Find(id1);
    TestValue* p2 = map.Find(id2);
    TestValue* p3 = map.Find(id3);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    EXPECT_EQ(p1->a, 100);
    EXPECT_EQ(p1->b, "first");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, true);

    EXPECT_EQ(p2->a, 200);
    EXPECT_EQ(p2->b, "second");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, true);

    EXPECT_EQ(p3->a, 300);
    EXPECT_EQ(p3->b, "third");
    EXPECT_EQ(p3->bCopyed, false);
    EXPECT_EQ(p3->bMoved, true);
}

TEST(IDMapTest, Add_MultipleSameValuesByMove_CreatesDistinctEntries)
{
    TestMap map;

    TestValue v1(20, "same");
    TestValue v2(20, "same");

    TestID id1 = map.Add(std::move(v1));
    TestID id2 = map.Add(std::move(v2));

    EXPECT_TRUE(id1.IsValid());
    EXPECT_TRUE(id2.IsValid());
    EXPECT_EQ(map.GetSize(), 2);

    TestValue* p1 = map.Find(id1);
    TestValue* p2 = map.Find(id2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 20);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, true);

    EXPECT_EQ(p2->a, 20);
    EXPECT_EQ(p2->b, "same");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, true);

    EXPECT_EQ(id2.GetID(), id1.GetID() + 1);
}

TEST(IDMapTest, Emplace_MultipleValues_StoresCorrectData)
{
    TestMap map;

    TestID id1 = map.Emplace(101, "first");
    TestID id2 = map.Emplace(202, "second");
    TestID id3 = map.Emplace(303, "third");

    EXPECT_TRUE(id1.IsValid());
    EXPECT_TRUE(id2.IsValid());
    EXPECT_TRUE(id3.IsValid());

    EXPECT_EQ(map.GetSize(), 3);

    const TestValue* p1 = map.Find(id1);
    const TestValue* p2 = map.Find(id2);
    const TestValue* p3 = map.Find(id3);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    EXPECT_EQ(p1->a, 101);
    EXPECT_EQ(p1->b, "first");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 202);
    EXPECT_EQ(p2->b, "second");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, false);

    EXPECT_EQ(p3->a, 303);
    EXPECT_EQ(p3->b, "third");
    EXPECT_EQ(p3->bCopyed, false);
    EXPECT_EQ(p3->bMoved, false);
}

TEST(IDMapTest, Emplace_MultipleSameValuesByMove_CreatesDistinctEntries)
{
    TestMap map;

    TestID id1 = map.Emplace(77, "same");
    TestID id2 = map.Emplace(77, "same");

    EXPECT_TRUE(id1.IsValid());
    EXPECT_TRUE(id2.IsValid());
    EXPECT_EQ(map.GetSize(), 2);

    const TestValue* p1 = map.Find(id1);
    const TestValue* p2 = map.Find(id2);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    EXPECT_EQ(p1->a, 77);
    EXPECT_EQ(p1->b, "same");
    EXPECT_EQ(p1->bCopyed, false);
    EXPECT_EQ(p1->bMoved, false);

    EXPECT_EQ(p2->a, 77);
    EXPECT_EQ(p2->b, "same");
    EXPECT_EQ(p2->bCopyed, false);
    EXPECT_EQ(p2->bMoved, false);
}

TEST(IDMapTest, Has_AfterEmplace_ReturnsTrueForExistingIDs)
{
    TestMap map;
    EXPECT_FALSE(map.Has(TestID()));

    TestID id1 = map.Emplace(101, "first");
    TestID id2 = map.Emplace(202, "second");
    TestID id3 = map.Emplace(303, "third");

    EXPECT_TRUE(map.Has(id1));
    EXPECT_TRUE(map.Has(id2));
    EXPECT_TRUE(map.Has(id3));
}

TEST(IDMapTest, Find_AfterEmplace_ReturnsPointerForExistingIDs)
{
    TestMap map;
    EXPECT_EQ(map.Find(TestID()), nullptr);

    TestID id1 = map.Emplace(101, "first");
    TestID id2 = map.Emplace(202, "second");
    TestID id3 = map.Emplace(303, "third");

    EXPECT_NE(map.Find(id1), nullptr);
    EXPECT_NE(map.Find(id2), nullptr);
    EXPECT_NE(map.Find(id3), nullptr);
}

TEST(IDMapTest, Remove_ExistingEntries_DecrementsSizeAndRemovesEntry)
{
    TestMap map;
    EXPECT_FALSE(map.Remove(TestID()));

    TestID id1 = map.Emplace(101, "first");
    TestID id2 = map.Emplace(202, "second");
    TestID id3 = map.Emplace(303, "third");

    EXPECT_TRUE(map.Remove(id1));
    EXPECT_EQ(map.GetSize(), 2);
    EXPECT_EQ(map.Find(id1), nullptr);

    EXPECT_TRUE(map.Remove(id2));
    EXPECT_EQ(map.GetSize(), 1);
    EXPECT_EQ(map.Find(id2), nullptr);

    EXPECT_TRUE(map.Remove(id3));
    EXPECT_EQ(map.GetSize(), 0);
    EXPECT_EQ(map.Find(id3), nullptr);
}

TEST(IDMapTest, Clear_ExistingEntries_InvalidatesAllIDsAndResetsSize)
{
    TestMap map;

    TestID id1 = map.Emplace(101, "first");
    TestID id2 = map.Emplace(202, "second");
    TestID id3 = map.Emplace(303, "third");

    EXPECT_EQ(map.GetSize(), 3);

    map.Clear();
    EXPECT_EQ(map.GetSize(), 0);

    EXPECT_FALSE(map.Has(id1));
    EXPECT_FALSE(map.Has(id2));
    EXPECT_FALSE(map.Has(id3));
    EXPECT_EQ(map.Find(id1), nullptr);
    EXPECT_EQ(map.Find(id2), nullptr);
    EXPECT_EQ(map.Find(id3), nullptr);
}
