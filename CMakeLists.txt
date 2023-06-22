cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

project(spark)
add_subdirectory(src)

# glm
add_subdirectory(extern/glm)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL=1)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE=1)
target_link_libraries(spark PUBLIC glm)

# rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extern/rapidjson/include)
target_link_libraries(spark PUBLIC rapidjson)