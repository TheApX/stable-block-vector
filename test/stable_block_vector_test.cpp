#include <stable_block_vector.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::Eq;

class TestObject {
 public:
  TestObject() {
    ++constructor_counter;
    std::cout << "Construct: " << this << std::endl;
  }
  TestObject(const TestObject& other) {
    ++constructor_counter;
    ++copy_counter;
    tag = other.tag;
    std::cout << "Construct copy: " << this << " from " << &other << std::endl;
  }
  TestObject(TestObject&& other) {
    ++constructor_counter;
    ++move_counter;
    tag = other.tag;
    std::cout << "Construct move: " << this << " from " << &other << std::endl;
  }
  explicit TestObject(int t) {
    ++constructor_counter;
    tag = t;
    std::cout << "Construct with value: " << this << " value " << tag
              << std::endl;
  }

  ~TestObject() {
    ++destructor_counter;
    std::cout << "Destruct: " << this << std::endl;
  }

  static void ResetCounters() {
    constructor_counter = 0;
    destructor_counter = 0;
    copy_counter = 0;
    move_counter = 0;
  }

  int tag = 0;

  static int constructor_counter;
  static int destructor_counter;
  static int copy_counter;
  static int move_counter;
};

int TestObject::constructor_counter = 0;
int TestObject::destructor_counter = 0;
int TestObject::copy_counter = 0;
int TestObject::move_counter = 0;

class StableBlockVectorTest : public testing::Test {
 public:
  StableBlockVectorTest() { TestObject::ResetCounters(); }
};

TEST_F(StableBlockVectorTest, GrowWithinFirstBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(1));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));

    v.resize(3);
    EXPECT_THAT(TestObject::constructor_counter, Eq(3));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(3));
  EXPECT_THAT(TestObject::destructor_counter, Eq(3));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, ShrinkWithinFirstBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(3);
    EXPECT_THAT(TestObject::constructor_counter, Eq(3));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(3));
    EXPECT_THAT(TestObject::destructor_counter, Eq(2));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(3));
  EXPECT_THAT(TestObject::destructor_counter, Eq(3));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, GrowToSecondBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(1));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));

    v.resize(8);
    EXPECT_THAT(TestObject::constructor_counter, Eq(8));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(8));
  EXPECT_THAT(TestObject::destructor_counter, Eq(8));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, ShrinkFromSecondBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(8);
    EXPECT_THAT(TestObject::constructor_counter, Eq(8));
    EXPECT_THAT(TestObject::destructor_counter, Eq(0));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));

    v.resize(1);
    EXPECT_THAT(TestObject::constructor_counter, Eq(8));
    EXPECT_THAT(TestObject::destructor_counter, Eq(7));
    EXPECT_THAT(TestObject::copy_counter, Eq(0));
    EXPECT_THAT(TestObject::move_counter, Eq(0));
  }
  EXPECT_THAT(TestObject::constructor_counter, Eq(8));
  EXPECT_THAT(TestObject::destructor_counter, Eq(8));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, PointersStable) {
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

TEST_F(StableBlockVectorTest, CapacityInitiallyZero) {
  theapx::stable_block_vector<TestObject, 5> v;

  EXPECT_THAT(v.capacity(), Eq(0));
}

TEST_F(StableBlockVectorTest, CapacityPartialBlock) {
  theapx::stable_block_vector<TestObject, 5> v;

  v.reserve(3);
  EXPECT_THAT(v.capacity(), Eq(5));
  v.reserve(3);
  EXPECT_THAT(v.capacity(), Eq(5));
}

TEST_F(StableBlockVectorTest, CapacityOneBlock) {
  theapx::stable_block_vector<TestObject, 5> v;

  v.reserve(5);
  EXPECT_THAT(v.capacity(), Eq(5));
  v.reserve(5);
  EXPECT_THAT(v.capacity(), Eq(5));
}

TEST_F(StableBlockVectorTest, CapacityTwoBlocksExactly) {
  theapx::stable_block_vector<TestObject, 5> v;

  v.reserve(5);
  EXPECT_THAT(v.capacity(), Eq(5));
  v.reserve(10);
  EXPECT_THAT(v.capacity(), Eq(10));
  v.reserve(10);
  EXPECT_THAT(v.capacity(), Eq(10));
}

TEST_F(StableBlockVectorTest, CapacityMultipleBlocks) {
  theapx::stable_block_vector<TestObject, 5> v;

  v.reserve(13);
  EXPECT_THAT(v.capacity(), Eq(15));
  v.reserve(13);
  EXPECT_THAT(v.capacity(), Eq(15));
}

TEST_F(StableBlockVectorTest, CapacityDoesntShrink) {
  theapx::stable_block_vector<TestObject, 5> v;

  v.reserve(13);
  EXPECT_THAT(v.capacity(), Eq(15));
  v.reserve(3);
  EXPECT_THAT(v.capacity(), Eq(15));
}

// push_back with copy

TEST_F(StableBlockVectorTest, PushBackToEmpty) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    TestObject obj(123);
    v.push_back(obj);
    EXPECT_THAT(v.capacity(), Eq(5));
    EXPECT_THAT(v.size(), Eq(1));
    EXPECT_THAT(v[0].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(2));
  EXPECT_THAT(TestObject::destructor_counter, Eq(2));
  EXPECT_THAT(TestObject::copy_counter, Eq(1));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, PushBackToFirstBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(2);

    TestObject obj(123);
    v.push_back(obj);
    EXPECT_THAT(v.capacity(), Eq(5));
    EXPECT_THAT(v.size(), Eq(3));
    EXPECT_THAT(v[2].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(4));
  EXPECT_THAT(TestObject::destructor_counter, Eq(4));
  EXPECT_THAT(TestObject::copy_counter, Eq(1));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, PushBackToEndOfFirstBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(4);

    TestObject obj(123);
    v.push_back(obj);
    EXPECT_THAT(v.capacity(), Eq(5));
    EXPECT_THAT(v.size(), Eq(5));
    EXPECT_THAT(v[4].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(6));
  EXPECT_THAT(TestObject::destructor_counter, Eq(6));
  EXPECT_THAT(TestObject::copy_counter, Eq(1));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

TEST_F(StableBlockVectorTest, PushBackToBeginningOfSecondBlock) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(5);

    TestObject obj(123);
    v.push_back(obj);
    EXPECT_THAT(v.capacity(), Eq(10));
    EXPECT_THAT(v.size(), Eq(6));
    EXPECT_THAT(v[5].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(7));
  EXPECT_THAT(TestObject::destructor_counter, Eq(7));
  EXPECT_THAT(TestObject::copy_counter, Eq(1));
  EXPECT_THAT(TestObject::move_counter, Eq(0));
}

// push_back with move

TEST_F(StableBlockVectorTest, PushBackToEmptyMove) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    TestObject obj(123);
    v.push_back(std::move(obj));
    EXPECT_THAT(v.capacity(), Eq(5));
    EXPECT_THAT(v.size(), Eq(1));
    EXPECT_THAT(v[0].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(2));
  EXPECT_THAT(TestObject::destructor_counter, Eq(2));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(1));
}

TEST_F(StableBlockVectorTest, PushBackToFirstBlockMove) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(2);

    TestObject obj(123);
    v.push_back(std::move(obj));
    EXPECT_THAT(v.capacity(), Eq(5));
    EXPECT_THAT(v.size(), Eq(3));
    EXPECT_THAT(v[2].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(4));
  EXPECT_THAT(TestObject::destructor_counter, Eq(4));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(1));
}

TEST_F(StableBlockVectorTest, PushBackToEndOfFirstBlockMove) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(4);

    TestObject obj(123);
    v.push_back(std::move(obj));
    EXPECT_THAT(v.capacity(), Eq(5));
    EXPECT_THAT(v.size(), Eq(5));
    EXPECT_THAT(v[4].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(6));
  EXPECT_THAT(TestObject::destructor_counter, Eq(6));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(1));
}

TEST_F(StableBlockVectorTest, PushBackToBeginningOfSecondBlockMove) {
  {
    theapx::stable_block_vector<TestObject, 5> v;

    v.resize(5);

    TestObject obj(123);
    v.push_back(std::move(obj));
    EXPECT_THAT(v.capacity(), Eq(10));
    EXPECT_THAT(v.size(), Eq(6));
    EXPECT_THAT(v[5].tag, Eq(123));
  }

  EXPECT_THAT(TestObject::constructor_counter, Eq(7));
  EXPECT_THAT(TestObject::destructor_counter, Eq(7));
  EXPECT_THAT(TestObject::copy_counter, Eq(0));
  EXPECT_THAT(TestObject::move_counter, Eq(1));
}

}  // namespace
