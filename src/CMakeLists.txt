add_library(spark STATIC
        spark.h
        spark.cpp)

# Core
add_subdirectory(sp_core)

# Math
add_subdirectory(sp_math)

# RHI
add_subdirectory(sp_rhi)