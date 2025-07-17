#pragma once

#define VK_DEBUG
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#if defined(_WIN32)
#define NOMINMAX
#endif

#define VK_NO_PROTOTYPES
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include <spirv_reflect.h>
#include <volk.h>

#if defined(_WIN32)
#undef GetObject
#endif

#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define VK_LOGE(...)                                 \
    do                                               \
    {                                                \
        fprintf(stderr, "[VK_ERROR]: " __VA_ARGS__); \
        fflush(stderr);                              \
    } while (false)

#define VK_LOGW(...)                                \
    do                                              \
    {                                               \
        fprintf(stderr, "[VK_WARN]: " __VA_ARGS__); \
        fflush(stderr);                             \
    } while (false)

#define VK_LOGI(...)                                \
    do                                              \
    {                                               \
        fprintf(stderr, "[VK_INFO]: " __VA_ARGS__); \
        fflush(stderr);                             \
    } while (false)

#define VK_ASSERT(x)                                                 \
    do                                                               \
    {                                                                \
        if (x != VK_SUCCESS)                                         \
        {                                                            \
            VK_LOGE("Vulkan error at %s:%d.\n", __FILE__, __LINE__); \
            abort();                                                 \
        }                                                            \
    } while (0)

#ifdef __cplusplus
#ifndef EZ_MAKE_ENUM_FLAG
#define EZ_MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)                        \
    static inline ENUM_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b)   \
    {                                                             \
        return (ENUM_TYPE)((TYPE)(a) | (TYPE)(b));                \
    }                                                             \
    static inline ENUM_TYPE operator&(ENUM_TYPE a, ENUM_TYPE b)   \
    {                                                             \
        return (ENUM_TYPE)((TYPE)(a) & (TYPE)(b));                \
    }                                                             \
    static inline ENUM_TYPE operator|=(ENUM_TYPE& a, ENUM_TYPE b) \
    {                                                             \
        return a = (a | b);                                       \
    }                                                             \
    static inline ENUM_TYPE operator&=(ENUM_TYPE& a, ENUM_TYPE b) \
    {                                                             \
        return a = (a & b);                                       \
    }
#endif
#else
#define EZ_MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)
#endif

inline constexpr uint32_t ez_align_to(uint32_t value, uint32_t alignment)
{
    return ((value + alignment - 1) / alignment) * alignment;
}

inline constexpr uint64_t ez_align_to(uint64_t value, uint64_t alignment)
{
    return ((value + alignment - 1) / alignment) * alignment;
}

// Core
void ez_init();

void ez_terminate();

void ez_submit();

VkCommandBuffer ez_cmd();

VkDevice ez_device();

void ez_flush();

uint32_t ez_get_format_stride(VkFormat format);

// Support features
enum EzFeature {
    EZ_FEATURE_NONE         = 0,
    EZ_FEATURE_RAYTRACING   = 1 << 0,
};
EZ_MAKE_ENUM_FLAG(uint32_t, EzFeature)

bool ez_support_feature(EzFeature feature);

// Props
float ez_get_timestamp_period();

uint32_t ez_get_shader_group_handle_size();

uint32_t ez_get_shader_group_handle_alignment();

uint32_t ez_get_shader_group_base_alignment();

// Swapchain
struct EzSwapchain_T {
    uint32_t width;
    uint32_t height;
    uint32_t image_index;
    uint32_t image_count;
    VkAccessFlags2 access_mask = 0;
    VkPipelineStageFlags2 stage_mask = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSwapchainKHR handle;
    VkSurfaceKHR surface;
    VkSemaphore acquire_semaphore = VK_NULL_HANDLE;
    VkSemaphore release_semaphore = VK_NULL_HANDLE;
    std::vector<VkImage> images;
};
VK_DEFINE_HANDLE(EzSwapchain)

enum class EzSwapchainStatus {
    Ready,
    Resized,
    NotReady,
};

void ez_create_swapchain(void* window, EzSwapchain& swapchain);

void ez_destroy_swapchain(EzSwapchain swapchain);

EzSwapchainStatus ez_update_swapchain(EzSwapchain swapchain);

void ez_acquire_next_image(EzSwapchain swapchain);

void ez_present(EzSwapchain swapchain);

// Buffer
struct EzBuffer_T {
    size_t size;
    VkBuffer handle;
    VmaMemoryUsage memory_usage;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkAccessFlags2 access_mask = 0;
    VkPipelineStageFlags2 stage_mask = 0;
    VkDeviceAddress address = 0;
};
VK_DEFINE_HANDLE(EzBuffer)

struct EzBufferDesc {
    size_t size;
    VkBufferUsageFlags usage;
    VmaMemoryUsage memory_usage;
};
void ez_create_buffer(const EzBufferDesc& desc, EzBuffer& buffer);

void ez_destroy_buffer(EzBuffer buffer);

void ez_map_memory(EzBuffer buffer, void** memory_ptr);

void ez_unmap_memory(EzBuffer buffer);

void ez_clear_buffer(EzBuffer buffer, uint32_t size, uint32_t offset);

void ez_copy_buffer(EzBuffer src_buffer, EzBuffer dst_buffer, VkBufferCopy range);

void ez_update_buffer(EzBuffer buffer, uint32_t size, uint32_t offset, void* data);

struct EzStageAllocation {
    uint64_t offset = 0;
    EzBuffer buffer = VK_NULL_HANDLE;
};
EzStageAllocation ez_alloc_stage_buffer(size_t size);

// Texture
struct EzTextureView {
    VkImageView handle;
    VkImageSubresourceRange subresource_range;
};

struct EzTexture_T {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t levels;
    uint32_t layers;
    VkFormat format;
    VkImage handle;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkAccessFlags2 access_mask = 0;
    VkPipelineStageFlags2 stage_mask = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    std::vector<EzTextureView> views;
};
VK_DEFINE_HANDLE(EzTexture)

struct EzTextureDesc {
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    uint32_t levels = 1;
    uint32_t layers = 1;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageType image_type = VK_IMAGE_TYPE_2D;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
};
void ez_create_texture(const EzTextureDesc& desc, EzTexture& texture);

void ez_destroy_texture(EzTexture texture);

int ez_create_texture_view(EzTexture texture, VkImageViewType view_type, VkImageAspectFlags aspect_mask,
                           uint32_t base_level, uint32_t level_count,
                           uint32_t base_layer, uint32_t layer_count);

void ez_copy_image(EzTexture src_texture, EzTexture dst_texture, const VkImageCopy& region);

void ez_copy_image(EzTexture src_texture, EzSwapchain dst_swapchain, const VkImageCopy& region);

void ez_copy_buffer_to_image(EzBuffer buffer, EzTexture texture, VkBufferImageCopy range);

void ez_clear_color_image(EzTexture texture, int texture_view, float c[4]);

void ez_update_image(EzTexture texture, VkBufferImageCopy range, void* data);

struct EzSampler_T {
    VkSampler handle = VK_NULL_HANDLE;
};
VK_DEFINE_HANDLE(EzSampler)

struct EzSamplerDesc {
    VkFilter mag_filter = VK_FILTER_LINEAR;
    VkFilter min_filter = VK_FILTER_LINEAR;
    VkSamplerAddressMode address_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkBorderColor border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    bool anisotropy_enable = false;
    bool compare_enable = false;
    VkCompareOp compare_op = VK_COMPARE_OP_ALWAYS;
};
void ez_create_sampler(const EzSamplerDesc& desc, EzSampler& sampler);

void ez_destroy_sampler(EzSampler sampler);

// AccelerationStructure
struct EzAccelerationStructureBuildSizes {
    size_t as_size;
    size_t update_scratch_size;
    size_t build_scratch_size;
};

struct EzAccelerationStructure_T {
    VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
    EzBuffer buffer = VK_NULL_HANDLE;
    EzAccelerationStructureBuildSizes sizes_info{};
};
VK_DEFINE_HANDLE(EzAccelerationStructure)

struct EzAccelerationStructureTriangles {
    VkGeometryFlagsKHR flags;
    VkFormat vertex_format;
    uint32_t vertex_count = 0;
    uint32_t vertex_offset = 0;
    uint32_t vertex_stride = 0;
    EzBuffer vertex_buffer = VK_NULL_HANDLE;
    VkIndexType index_type;
    uint32_t index_count = 0;
    uint32_t index_offset = 0;
    EzBuffer index_buffer = VK_NULL_HANDLE;
};

struct EzAccelerationStructureInstances {
    VkGeometryFlagsKHR flags;
    uint32_t offset = 0;
    uint32_t count = 0;
    EzBuffer instance_buffer = VK_NULL_HANDLE;
};

struct EzAccelerationStructureGeometrySet {
    std::vector<EzAccelerationStructureTriangles> triangles;
    std::vector<EzAccelerationStructureInstances> instances;
};

struct EzAccelerationStructureBuildInfo {
    VkAccelerationStructureTypeKHR type;
    VkBuildAccelerationStructureFlagsKHR flags;
    VkBuildAccelerationStructureModeKHR mode;
    EzAccelerationStructureGeometrySet geometry_set;
    EzBuffer scratch_buffer = VK_NULL_HANDLE;
};

void ez_create_acceleration_structure(const EzAccelerationStructureBuildInfo& build_info, EzAccelerationStructure& as);

void ez_destroy_acceleration_structure(EzAccelerationStructure as);

void ez_build_acceleration_structure(const EzAccelerationStructureBuildInfo& build_info, EzAccelerationStructure as);

// Pipeline
struct EzShader_T {
    VkShaderModule handle = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo stage_info = {};
    VkPushConstantRange pushconstants = {};
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
    SpvReflectShaderModule reflect;
};
VK_DEFINE_HANDLE(EzShader)

void ez_create_shader(void* data, size_t size, EzShader& shader);

void ez_destroy_shader(EzShader shader);

struct EzPipeline_T {
    VkPipelineBindPoint bind_point;
    VkPipeline handle = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
    VkPushConstantRange pushconstants = {};
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
};
VK_DEFINE_HANDLE(EzPipeline)

struct EzBlendState {
    bool blend_enable = false;
    VkBlendFactor src_color = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dst_color = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp color_op = VK_BLEND_OP_ADD;
    VkBlendFactor src_alpha = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dst_alpha = VK_BLEND_FACTOR_ONE;
    VkBlendOp alpha_op = VK_BLEND_OP_ADD;
};

struct EzDepthState {
    bool depth_test = true;
    bool depth_write = true;
    VkCompareOp depth_func = VK_COMPARE_OP_LESS_OR_EQUAL;
};

struct EzStencilState {
    bool stencil_test = false;
    uint8_t stencil_read_mask = 0xff;
    uint8_t stencil_write_mask = 0xff;
    VkStencilOp front_stencil_fail_op = VK_STENCIL_OP_KEEP;
    VkStencilOp front_stencil_depth_fail_op = VK_STENCIL_OP_KEEP;
    VkStencilOp front_stencil_pass_op = VK_STENCIL_OP_KEEP;
    VkCompareOp front_stencil_func = VK_COMPARE_OP_ALWAYS;
    VkStencilOp back_stencil_fail_op = VK_STENCIL_OP_KEEP;
    VkStencilOp back_stencil_depth_fail_op = VK_STENCIL_OP_KEEP;
    VkStencilOp back_stencil_pass_op = VK_STENCIL_OP_KEEP;
    VkCompareOp back_stencil_func = VK_COMPARE_OP_ALWAYS;
};

struct EzMultisampleState {
    bool sample_shading = false;
    bool alpha_to_coverage = false;
    bool alpha_to_one = false;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
};

#define EZ_NUM_VERTEX_BUFFERS 8
#define EZ_NUM_VERTEX_ATTRIBS 8
struct EzVertexAttrib {
    uint32_t binding = 0;
    uint32_t offset = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
};

struct EzVertexBinding {
    uint32_t vertex_stride = 0;
    VkVertexInputRate vertex_rate = VK_VERTEX_INPUT_RATE_VERTEX;
};

struct EzVertexLayout {
    uint32_t vertex_binding_mask = 0;
    EzVertexBinding vertex_bindings[EZ_NUM_VERTEX_BUFFERS] = {};
    uint32_t vertex_attrib_mask = 0;
    EzVertexAttrib vertex_attribs[EZ_NUM_VERTEX_ATTRIBS] = {};
};

struct EzPipelineState {
    EzShader vertex_shader = VK_NULL_HANDLE;
    EzShader fragment_shader = VK_NULL_HANDLE;
    EzShader compute_shader = VK_NULL_HANDLE;
    EzVertexLayout vertex_layout = {};
    EzBlendState blend_state = {};
    EzDepthState depth_state = {};
    EzStencilState stencil_state = {};
    EzMultisampleState multisample_state = {};
    VkPolygonMode fill_mode = VK_POLYGON_MODE_FILL;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkCullModeFlagBits cull_mode = VK_CULL_MODE_NONE;
};

void ez_destroy_pipeline(EzPipeline pipeline);

void ez_set_pipeline_state(const EzPipelineState& pipeline_state);

void ez_reset_pipeline_state();

void ez_set_vertex_shader(EzShader shader);

void ez_set_fragment_shader(EzShader shader);

void ez_set_compute_shader(EzShader shader);

void ez_set_vertex_binding(uint32_t binding, uint32_t stride, VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX);

void ez_set_vertex_attrib(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset = 0);

void ez_set_vertex_binding(EzVertexLayout& vertex_layout, uint32_t binding, uint32_t stride, VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX);

void ez_set_vertex_attrib(EzVertexLayout& vertex_layout, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset = 0);

void ez_set_vertex_layout(const EzVertexLayout& vertex_layout);

void ez_set_blend_state(const EzBlendState& blend_state);

void ez_set_depth_state(const EzDepthState& depth_state);

void ez_set_stencil_state(const EzStencilState& stencil_state);

void ez_set_multisample_state(const EzMultisampleState& multisample_state);

void ez_set_polygon_mode(VkPolygonMode fill_mode);

void ez_set_primitive_topology(VkPrimitiveTopology topology);

void ez_set_front_face(VkFrontFace front_face);

void ez_set_cull_mode(VkCullModeFlagBits cull_mode);

struct EzRenderingAttachmentInfo {
    EzTexture texture = nullptr;
    int texture_view = 0;
    EzTexture resolve_texture = nullptr;
    int resolve_texture_view = 0;
    VkResolveModeFlagBits resolve_mode = VK_RESOLVE_MODE_AVERAGE_BIT;
    VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clear_value{};
};

struct EzRenderingInfo {
    uint32_t width;
    uint32_t height;
    std::vector<EzRenderingAttachmentInfo> depth;
    std::vector<EzRenderingAttachmentInfo> colors;
};
void ez_begin_rendering(const EzRenderingInfo& rendering_info);

void ez_end_rendering();

void ez_set_scissor(int32_t left, int32_t top, int32_t right, int32_t bottom);

void ez_set_viewport(float x, float y, float w, float h, float min_depth = 0.0f, float max_depth = 1.0f);

void ez_bind_vertex_buffer(uint32_t binding, EzBuffer vertex_buffer, uint64_t offset = 0);

void ez_bind_vertex_buffers(uint32_t first_binding, uint32_t binding_count, EzBuffer* vertex_buffers, const uint64_t* offsets = nullptr);

void ez_bind_index_buffer(EzBuffer index_buffer, VkIndexType type, uint64_t offset = 0);

void ez_bind_texture(uint32_t binding, EzTexture texture, int texture_view);

void ez_bind_texture_array(uint32_t binding, EzTexture texture, int texture_view, int array_idx);

void ez_bind_buffer(uint32_t binding, EzBuffer buffer);

void ez_bind_buffer(uint32_t binding, EzBuffer buffer, uint64_t size, uint64_t offset = 0);

void ez_bind_sampler(uint32_t binding, EzSampler sampler);

void ez_bind_sampler_array(uint32_t binding, EzSampler sampler, int array_idx);

void ez_push_constants(const void* data, uint32_t size, uint32_t offset);

void ez_draw(uint32_t vertex_count, uint32_t vertex_offset);

void ez_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t vertex_offset, uint32_t instance_offset);

void ez_draw_indexed(uint32_t index_count, uint32_t index_offset, int32_t vertex_offset);

void ez_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t index_offset, int32_t vertex_offset, uint32_t instance_offset);

void ez_draw_indirect(EzBuffer buffer, uint64_t offset, uint32_t draw_count, uint32_t stride);

void ez_draw_indexed_indirect(EzBuffer buffer, uint64_t offset, uint32_t draw_count, uint32_t stride);

void ez_dispatch(uint32_t thread_group_x, uint32_t thread_group_y, uint32_t thread_group_z);

void ez_dispatch_indirect(EzBuffer buffer, uint64_t offset);

struct EzRaytracingShderGruop {
    VkRayTracingShaderGroupTypeKHR type;
    uint32_t general_shader = VK_SHADER_UNUSED_KHR;
    uint32_t closesthit_shader = VK_SHADER_UNUSED_KHR;
    uint32_t anyhit_shader = VK_SHADER_UNUSED_KHR;
    uint32_t intersection_shader = VK_SHADER_UNUSED_KHR;
};

struct EzRaytracingPipelineDesc {
    uint32_t max_trace_recursion_depth = 1;
    std::vector<EzShader> shaders;
    std::vector<EzRaytracingShderGruop> groups;
};
void ez_create_raytracing_pipeline(const EzRaytracingPipelineDesc& desc, EzPipeline& pipeline);

void ez_bind_raytracing_pipeline(EzPipeline pipeline);

void ez_get_raytracing_group_handle(EzPipeline pipeline, uint32_t first_group, uint32_t group_count, size_t data_size, void* data);

// Barrier
enum EzResourceState {
    EZ_RESOURCE_STATE_UNDEFINED = 0,
    EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
    EZ_RESOURCE_STATE_INDEX_BUFFER = 0x2,
    EZ_RESOURCE_STATE_RENDERTARGET = 0x4,
    EZ_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
    EZ_RESOURCE_STATE_DEPTH_WRITE = 0x10,
    EZ_RESOURCE_STATE_DEPTH_READ = 0x20,
    EZ_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
    EZ_RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
    EZ_RESOURCE_STATE_STREAM_OUT = 0x100,
    EZ_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
    EZ_RESOURCE_STATE_COPY_DEST = 0x400,
    EZ_RESOURCE_STATE_COPY_SOURCE = 0x800,
    EZ_RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
    EZ_RESOURCE_STATE_PRESENT = 0x1000,
    EZ_RESOURCE_STATE_COMMON = 0x2000
};
EZ_MAKE_ENUM_FLAG(uint32_t, EzResourceState)

VkImageMemoryBarrier2 ez_image_barrier(EzSwapchain swapchain, EzResourceState resource_state);

VkImageMemoryBarrier2 ez_image_barrier(EzTexture texture, EzResourceState resource_state);

VkBufferMemoryBarrier2 ez_buffer_barrier(EzBuffer buffer, EzResourceState resource_state);

VkMemoryBarrier2 ez_memory_barrier(VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
                                   VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask);

void ez_pipeline_barrier(VkDependencyFlags dependency_flags,
                         size_t buffer_barrier_count = 0,
                         const VkBufferMemoryBarrier2* buffer_barriers = nullptr,
                         size_t image_barrier_count = 0,
                         const VkImageMemoryBarrier2* image_barriers = nullptr,
                         size_t memory_barrier_count = 0,
                         const VkMemoryBarrier2* memory_barriers = nullptr);

struct EzQueryPool_T {
    VkQueryPool handle;
    VkQueryType type;
    uint32_t query_count;
};
VK_DEFINE_HANDLE(EzQueryPool)

void ez_create_query_pool(uint32_t query_count, VkQueryType type, EzQueryPool& query_pool);

void ez_destroy_query_pool(EzQueryPool query_pool);

void ez_reset_query_pool(EzQueryPool query_pool, uint32_t start_query, uint32_t query_count);

void ez_write_timestamp(EzQueryPool query_pool, uint32_t query_index);

void ez_get_query_pool_results(EzQueryPool query_pool,
                               uint32_t first_query,
                               uint32_t query_count,
                               uint32_t data_size,
                               void* data,
                               uint32_t stride,
                               VkQueryResultFlags flags);

void ez_begin_debug_label(const char* label_name, const float color[4]);

void ez_end_debug_label();