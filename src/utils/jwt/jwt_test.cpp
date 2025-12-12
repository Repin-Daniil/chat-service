#include "jwt.hpp"
#include <string>
#include <userver/utest/utest.hpp>

TEST(JWTUtilsTest, GenerateAndDecodeSuccess) {
  std::string user_id = "user_12345";

  std::string token = NUtils::NTokens::GenerateJWT(user_id);
  ASSERT_FALSE(token.empty());

  auto decoded_id = NUtils::NTokens::DecodeJWT(token);

  ASSERT_TRUE(decoded_id.has_value()) << "Token should be successfully decoded";
  EXPECT_EQ(decoded_id.value(), user_id) << "Decoded ID must match original ID";
}

TEST(JWTUtilsTest, DecodeGarbageString) {
  std::string garbage_token = "not.a.valid.token";

  auto result = NUtils::NTokens::DecodeJWT(garbage_token);

  EXPECT_FALSE(result.has_value()) << "Garbage token should return nullopt";
}

TEST(JWTUtilsTest, HandleEmptyString) {
  auto result = NUtils::NTokens::DecodeJWT("");
  EXPECT_FALSE(result.has_value());
}

TEST(JWTUtilsTest, SupportsDifferentIDs) {
  std::string id1 = "admin";
  std::string id2 = "guest";

  auto t1 = NUtils::NTokens::GenerateJWT(id1);
  auto t2 = NUtils::NTokens::GenerateJWT(id2);

  EXPECT_NE(t1, t2) << "Tokens for different users must be different";
  EXPECT_EQ(NUtils::NTokens::DecodeJWT(t1).value(), id1);
  EXPECT_EQ(NUtils::NTokens::DecodeJWT(t2).value(), id2);
}
