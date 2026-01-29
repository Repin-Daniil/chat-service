#!/usr/bin/env bash
set -e

# где будут .profraw
export LLVM_PROFILE_FILE="coverage-%p.profraw"

make test-release COVERAGE_ENABLED=1

if ! cd build-release; then
    echo "build-release not found"
    exit 1
fi

# собрать profdata
llvm-profdata merge -sparse coverage-*.profraw -o coverage.profdata

# html-отчет
llvm-cov show \
  ./chat_service \
  -instr-profile=coverage.profdata \
  -format=html \
  -output-dir=coverage_html \
  -ignore-filename-regex=".*(_deps|/build(-debug|-release)?/|/usr/|/third_party/).*"


llvm-cov export \
  ./chat_service \
  -instr-profile=coverage.profdata \
  -format=lcov \
  -ignore-filename-regex=".*(_deps|/build(-debug|-release)?/|/usr/|/third_party/).*" \
  > ../coverage.info

genhtml ../coverage.info --output-directory coverage_html