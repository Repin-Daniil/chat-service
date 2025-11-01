#include <greeting.hpp>

#include <cstdint>   // for std::uint64_t
#include <iterator>  // for std::size
#include <string_view>

#include <benchmark/benchmark.h>
#include <userver/engine/run_standalone.hpp>

using chat_service::UserType;

void HelloBenchmark(benchmark::State& state) {
  userver::engine::RunStandalone([&] {
    constexpr std::string_view kNames[] = {"userver", "is", "awesome", "!"};
    std::uint64_t i = 0;

    for (auto _ : state) {
      const auto name = kNames[i++ % std::size(kNames)];
      auto result = chat_service::SayHelloTo(name, UserType::kFirstTime);
      benchmark::DoNotOptimize(result);
    }
  });
}

BENCHMARK(HelloBenchmark);