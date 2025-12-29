target_compile_options(${PROJECT_NAME}_objs PRIVATE -fprofile-instr-generate
                                                    -fcoverage-mapping)
target_link_options(
${PROJECT_NAME}_objs
PRIVATE
-fprofile-instr-generate
-fcoverage-mapping)

target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-instr-generate
                                                -fcoverage-mapping)
target_link_options(
${PROJECT_NAME}
PRIVATE
-fprofile-instr-generate
-fcoverage-mapping)

add_custom_target(
coverage
COMMAND ../scripts/coverage.sh
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Running coverage utils...")

add_dependencies(coverage test)