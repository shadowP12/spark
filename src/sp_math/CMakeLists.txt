add_library(sp_math STATIC
        sp_math.h
        sp_math.cpp)

# glm
add_library(glm INTERFACE)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE=1)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glm)
target_link_libraries(sp_math PUBLIC glm)

# Final link
target_link_libraries(spark PUBLIC sp_math)