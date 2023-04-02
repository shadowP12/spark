set(SP_ALL_SOURCE_FILES spark.h spark.cpp)

# Math
file(GLOB_RECURSE SP_MATH_SOURCE_FILES "math/*.h" "math/*.cpp")
set(SP_ALL_SOURCE_FILES ${SP_ALL_SOURCE_FILES} ${SP_MATH_SOURCE_FILES})

add_library(spark STATIC ${SP_ALL_SOURCE_FILES})
target_include_directories(spark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})