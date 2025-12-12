
#include <utils/uuid/uuid_generator.hpp>

#include <userver/utest/utest.hpp>

#include <regex>
#include <unordered_set>

using NUtils::NId::UuidGenerator;

class UuidGeneratorTest : public ::testing::Test {
 protected:
  UuidGenerator generator_;
};

TEST_F(UuidGeneratorTest, GeneratesValidUuidFormat) {
  auto uuid = generator_.Generate();

  std::regex uuid_pattern("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");
  EXPECT_TRUE(std::regex_match(uuid, uuid_pattern));
}

TEST_F(UuidGeneratorTest, GeneratesUniqueUuids) {
  std::unordered_set<std::string> uuids;
  const int count = 100;

  for (int i = 0; i < count; ++i) {
    uuids.insert(generator_.Generate());
  }

  EXPECT_EQ(uuids.size(), count);
}

TEST_F(UuidGeneratorTest, GeneratesCorrectLength) {
  auto uuid = generator_.Generate();
  EXPECT_EQ(uuid.length(), 36);  // 8-4-4-4-12 + 4 dashes
}
