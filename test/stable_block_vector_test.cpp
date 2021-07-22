#include <stable_block_vector.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(StableBlockVectorTest, BuildsAndLinks) {
  EXPECT_THAT(1, testing::Eq(1));
  EXPECT_THAT(2, testing::Ne(1));
}

}  // namespace
