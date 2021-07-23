#include <stable_block_vector.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::Eq;

class TestObject {
 public:
  TestObject() { ++constructor_counter; }
  TestObject(const TestObject& other) { ++constructor_counter; }
  TestObject(TestObject&& other) { ++constructor_counter; }

  ~TestObject() { ++destructor_counter; }

  static void ResetCounters() {
    constructor_counter = 0;
    destructor_counter = 0;
  }

  static int constructor_counter;
  static int destructor_counter;
};

int TestObject::constructor_counter = 0;
int TestObject::destructor_counter = 0;

TEST(StableBlockVectorTest, GrowWithinFirstBlock) {
  TestObject::ResetCounters();
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(1));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));

    v.resize(3);
    EXPECT_THAT(TestObject::constructor_counter, Eq(3));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(3));
  EXPECT_THAT(TestObject::destructor_counter, Eq(3));
}

TEST(StableBlockVectorTest, ShrinkWithinFirstBlock) {
  TestObject::ResetCounters();
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(3);
    EXPECT_THAT(TestObject::constructor_counter, Eq(3));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(3));
    EXPECT_THAT(TestObject::destructor_counter, Eq(2));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(3));
  EXPECT_THAT(TestObject::destructor_counter, Eq(3));
}

TEST(StableBlockVectorTest, GrowToSecondBlock) {
  TestObject::ResetCounters();
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(1));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));

    v.resize(8);
    EXPECT_THAT(TestObject::constructor_counter, Eq(8));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(8));
  EXPECT_THAT(TestObject::destructor_counter, Eq(8));
}

TEST(StableBlockVectorTest, ShrinkFromSecondBlock) {
  TestObject::ResetCounters();
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(8);
    EXPECT_THAT(TestObject::constructor_counter, Eq(8));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(8));
    EXPECT_THAT(TestObject::destructor_counter, Eq(7));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(8));
  EXPECT_THAT(TestObject::destructor_counter, Eq(8));
}

TEST(StableBlockVectorTest, PointersStable) {
  TestObject::ResetCounters();

  theapx::stable_block_vector<TestObject, 5> v;

  const int max_count = 100;
  std::vector<TestObject*> addresses(max_count, nullptr);

  for (int i = 1; i < max_count; ++i) {
    v.resize(i);
    addresses[i - 1] = &v[i - 1];
    for (int j = 0; j < i; ++j) {
      EXPECT_THAT(addresses[j], Eq(&v[j])) << " i = " << i << "  j = " << j;
    }
  }

  v.resize(0);

  for (int i = 1; i < max_count; ++i) {
    v.resize(i);
    for (int j = 0; j < i; ++j) {
      EXPECT_THAT(addresses[j], Eq(&v[j])) << " i = " << i << "  j = " << j;
    }
  }
}

}  // namespace
