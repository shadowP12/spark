cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 14)

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

# glfw
add_subdirectory(extern/glfw EXCLUDE_FROM_ALL glfw.out)
target_link_libraries(spark PUBLIC glfw)

# spirv_reflect
add_library(spirv_reflect STATIC extern/spirv_reflect/spirv_reflect.c extern/spirv_reflect/spirv_reflect.h)
target_include_directories(spark PUBLIC extern/spirv_reflect)
target_link_libraries(spark PUBLIC spirv_reflect)

# volk
add_library(volk STATIC extern/volk/volk.c extern/volk/volk.h)
target_include_directories(volk PUBLIC extern/volk)
target_include_directories(volk PUBLIC extern/Vulkan-headers/include)
if (WIN32)
    target_compile_definitions(volk PRIVATE VK_USE_PLATFORM_WIN32_KHR)
endif()
target_link_libraries(spark PUBLIC volk)

# uuid4
add_library(uuid4 STATIC extern/uuid4/uuid4.c extern/uuid4/uuid4.h)
target_include_directories(spark PUBLIC extern/uuid4)
target_link_libraries(spark PUBLIC uuid4)