#pragma once

#include <functional>
#include <utility>

namespace NUtils {

template <typename Value, typename Tag>
class TStrongTypedef {
 public:
  using ValueType = Value;
  using TagType = Tag;

  explicit TStrongTypedef(Value&& v) : value_(std::move(v)) {}
  explicit TStrongTypedef(const Value& v) : value_(v) {}

  const Value& operator*() const { return value_; }
  Value& operator*() { return value_; }

  auto operator<=>(const TStrongTypedef<Value, Tag>&) const = default;

 private:
  Value value_;
};

template <typename TaggedValue>
struct TaggedHasher {
  std::size_t operator()(const TaggedValue& value) const {
    return std::hash<typename TaggedValue::ValueType>{}(*value);
  }
};
}  // namespace NUtil