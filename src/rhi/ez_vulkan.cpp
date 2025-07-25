#include "ez_vulkan.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <deque>
#include <unordered_map>

#define EZ_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define EZ_MIN(a, b) (((a) < (b)) ? (a) : (b))

template<class T>
constexpr void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct Context {
    uint64_t frame_count = 0;
    VkDevice device = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties2 properties2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    VkPhysicalDeviceVulkan11Properties properties_1_1 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
    VkPhysicalDeviceVulkan12Properties properties_1_2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
    VkPhysicalDeviceVulkan13Properties properties_1_3 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_structure_properties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};

    VkPhysicalDeviceFeatures2KHR features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR};
    VkPhysicalDeviceVulkan11Features features_1_1 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    VkPhysicalDeviceVulkan12Features features_1_2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    VkPhysicalDeviceVulkan13Features features_1_3 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    VkPhysicalDeviceRayQueryFeaturesKHR raytracing_query_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};

    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkCommandPool cmd_pool = VK_NULL_HANDLE;
    uint32_t max_sets = 256;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    uint32_t image_index = 0;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSemaphore acquire_semaphore = VK_NULL_HANDLE;
    VkSemaphore release_semaphore = VK_NULL_HANDLE;
    EzPipelineState pipeline_state = {};
    EzRenderingInfo rendering_info = {};
    EzPipeline pipeline = VK_NULL_HANDLE;
    std::unordered_map<std::size_t, EzPipeline> pipeline_cache;
    VmaAllocator allocator = VK_NULL_HANDLE;
    EzFeature support_features = EZ_FEATURE_NONE;
} ctx;

struct ResourceManager {
    uint64_t frame_count = 0;
    std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> destroyer_images;
    std::deque<std::pair<VkImageView, uint64_t>> destroyer_imageviews;
    std::deque<std::pair<std::pair<VkBuffer, VmaAllocation>, uint64_t>> destroyer_buffers;
    std::deque<std::pair<VkSampler, uint64_t>> destroyer_samplers;
    std::deque<std::pair<VkDescriptorPool, uint64_t>> destroyer_descriptor_pools;
    std::deque<std::pair<VkDescriptorSetLayout, uint64_t>> destroyer_descriptor_set_layouts;
    std::deque<std::pair<VkDescriptorUpdateTemplate, uint64_t>> destroyer_descriptor_update_templates;
    std::deque<std::pair<VkShaderModule, uint64_t>> destroyer_shadermodules;
    std::deque<std::pair<VkPipelineLayout, uint64_t>> destroyer_pipeline_layouts;
    std::deque<std::pair<VkPipeline, uint64_t>> destroyer_pipelines;
    std::deque<std::pair<VkQueryPool, uint64_t>> destroyer_query_pools;
    std::deque<std::pair<VkAccelerationStructureKHR, uint64_t>> destroyer_acceleration_structures;
} res_mgr;

void update_res_mgr(uint64_t current_frame_count)
{
    res_mgr.frame_count = current_frame_count;
    while (!res_mgr.destroyer_images.empty())
    {
        if (res_mgr.destroyer_images.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_images.front();
            res_mgr.destroyer_images.pop_front();
            vmaDestroyImage(ctx.allocator, item.first.first, item.first.second);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_imageviews.empty())
    {
        if (res_mgr.destroyer_imageviews.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_imageviews.front();
            res_mgr.destroyer_imageviews.pop_front();
            vkDestroyImageView(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_buffers.empty())
    {
        if (res_mgr.destroyer_buffers.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_buffers.front();
            res_mgr.destroyer_buffers.pop_front();
            vmaDestroyBuffer(ctx.allocator, item.first.first, item.first.second);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_samplers.empty())
    {
        if (res_mgr.destroyer_samplers.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_samplers.front();
            res_mgr.destroyer_samplers.pop_front();
            vkDestroySampler(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_descriptor_pools.empty())
    {
        if (res_mgr.destroyer_descriptor_pools.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_descriptor_pools.front();
            res_mgr.destroyer_descriptor_pools.pop_front();
            vkDestroyDescriptorPool(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_descriptor_set_layouts.empty())
    {
        if (res_mgr.destroyer_descriptor_set_layouts.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_descriptor_set_layouts.front();
            res_mgr.destroyer_descriptor_set_layouts.pop_front();
            vkDestroyDescriptorSetLayout(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_descriptor_update_templates.empty())
    {
        if (res_mgr.destroyer_descriptor_update_templates.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_descriptor_update_templates.front();
            res_mgr.destroyer_descriptor_update_templates.pop_front();
            vkDestroyDescriptorUpdateTemplate(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_shadermodules.empty())
    {
        if (res_mgr.destroyer_shadermodules.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_shadermodules.front();
            res_mgr.destroyer_shadermodules.pop_front();
            vkDestroyShaderModule(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_pipeline_layouts.empty())
    {
        if (res_mgr.destroyer_pipeline_layouts.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_pipeline_layouts.front();
            res_mgr.destroyer_pipeline_layouts.pop_front();
            vkDestroyPipelineLayout(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_pipelines.empty())
    {
        if (res_mgr.destroyer_pipelines.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_pipelines.front();
            res_mgr.destroyer_pipelines.pop_front();
            vkDestroyPipeline(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_query_pools.empty())
    {
        if (res_mgr.destroyer_query_pools.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_query_pools.front();
            res_mgr.destroyer_query_pools.pop_front();
            vkDestroyQueryPool(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
    while (!res_mgr.destroyer_acceleration_structures.empty())
    {
        if (res_mgr.destroyer_acceleration_structures.front().second < res_mgr.frame_count)
        {
            auto item = res_mgr.destroyer_acceleration_structures.front();
            res_mgr.destroyer_acceleration_structures.pop_front();
            vkDestroyAccelerationStructureKHR(ctx.device, item.first, nullptr);
        }
        else
        {
            break;
        }
    }
}

void clear_res_mgr()
{
    update_res_mgr(~0);
}

struct StageBufferPool {
    uint64_t size = 0;
    uint64_t offset = 0;
    EzBuffer current_buffer = VK_NULL_HANDLE;
} stage_buffer_pool;

void clear_stage_buffer_pool()
{
    stage_buffer_pool.size = 0;
    stage_buffer_pool.offset = 0;
    if (stage_buffer_pool.current_buffer != VK_NULL_HANDLE)
    {
        ez_destroy_buffer(stage_buffer_pool.current_buffer);
        stage_buffer_pool.current_buffer = VK_NULL_HANDLE;
    }
}

static const uint32_t EZ_CBV_COUNT = 15;
static const uint32_t EZ_SRV_COUNT = 64;
static const uint32_t EZ_UAV_COUNT = 16;
static const uint32_t EZ_SAMPLER_COUNT = 16;
static const uint32_t EZ_BINDING_COUNT = 32;
struct ResourceBinding {
    VkDescriptorBufferInfo buffer;
    std::vector<VkDescriptorImageInfo> images;
    VkDeviceSize dynamic_offset;
};

struct BindingTable {
    bool dirty;
    uint32_t pushconstants_size;
    uint8_t pushconstants_data[128];
    ResourceBinding bindings[EZ_BINDING_COUNT];
    std::vector<VkWriteDescriptorSet> descriptor_writes;
} binding_table;

void init_descriptor_pool()
{
    uint32_t max_sets = ctx.max_sets;
    VkDescriptorPoolSize pool_sizes[9] = {};
    uint32_t pool_size_count = 0;

    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = EZ_CBV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    pool_sizes[1].descriptorCount = EZ_SRV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    pool_sizes[2].descriptorCount = EZ_SRV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[3].descriptorCount = EZ_SRV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    pool_sizes[4].descriptorCount = EZ_UAV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    pool_sizes[5].descriptorCount = EZ_UAV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[6].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[6].descriptorCount = EZ_UAV_COUNT * max_sets;
    pool_size_count++;

    pool_sizes[7].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    pool_sizes[7].descriptorCount = EZ_SAMPLER_COUNT * max_sets;
    pool_size_count++;

    VkDescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = pool_size_count;
    pool_create_info.pPoolSizes = pool_sizes;
    pool_create_info.maxSets = max_sets;
    VK_ASSERT(vkCreateDescriptorPool(ctx.device, &pool_create_info, nullptr, &ctx.descriptor_pool));
}

void destroy_descriptor_pool()
{
    res_mgr.destroyer_descriptor_pools.emplace_back(ctx.descriptor_pool, ctx.frame_count);
    ctx.descriptor_pool = VK_NULL_HANDLE;
}

void flush_binding_table()
{
    if (!binding_table.dirty)
        return;
    binding_table.dirty = false;

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ctx.descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ctx.pipeline->descriptor_set_layout;

    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    VkResult res = vkAllocateDescriptorSets(ctx.device, &alloc_info, &descriptor_set);
    while (res == VK_ERROR_OUT_OF_POOL_MEMORY)
    {
        ctx.max_sets *= 2;
        destroy_descriptor_pool();
        init_descriptor_pool();
        alloc_info.descriptorPool = ctx.descriptor_pool;
        res = vkAllocateDescriptorSets(ctx.device, &alloc_info, &descriptor_set);
    }

    binding_table.descriptor_writes.clear();

    uint32_t i = 0;
    for (auto& x : ctx.pipeline->layout_bindings)
    {
        if (x.pImmutableSamplers != nullptr)
        {
            i++;
            continue;
        }

        uint32_t binding = x.binding;

        binding_table.descriptor_writes.emplace_back();
        auto& write = binding_table.descriptor_writes.back();
        write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_set;
        write.dstArrayElement = 0;
        write.descriptorType = x.descriptorType;
        write.dstBinding = x.binding;
        write.descriptorCount = x.descriptorCount;

        switch (x.descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
                write.pImageInfo = binding_table.bindings[binding].images.data();
            }
            break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
                write.pBufferInfo = &binding_table.bindings[binding].buffer;
            }
            break;

            default:
                break;
        }
    }

    vkUpdateDescriptorSets(ctx.device, (uint32_t)binding_table.descriptor_writes.size(), binding_table.descriptor_writes.data(), 0, nullptr);

    vkCmdBindDescriptorSets(ctx.cmd, ctx.pipeline->bind_point, ctx.pipeline->pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
}

void ez_create_compute_pipeline(const EzPipelineState& pipeline_state, EzPipeline& pipeline);
void ez_create_graphics_pipeline(const EzPipelineState& pipeline_state, const EzRenderingInfo& rendering_info, EzPipeline& pipeline);
void ez_destroy_pipeline(EzPipeline pipeline);

#ifdef VK_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_cb(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                        void* user_data)
{
    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        VK_LOGW("Validation Verbose %s: %s\n", callback_data->pMessageIdName, callback_data->pMessage);
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        VK_LOGW("Validation Info %s: %s\n", callback_data->pMessageIdName, callback_data->pMessage);
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        VK_LOGW("Validation Warning %s: %s\n", callback_data->pMessageIdName, callback_data->pMessage);
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        VK_LOGE("Validation Error %s: %s\n", callback_data->pMessageIdName, callback_data->pMessage);
    }
    return VK_FALSE;
}
#endif

static bool is_layer_supported(const char* required, const std::vector<VkLayerProperties>& available)
{
    for (const VkLayerProperties& availableLayer : available)
    {
        if (strcmp(availableLayer.layerName, required) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool is_extension_supported(const char* required, const std::vector<VkExtensionProperties>& available)
{
    for (const VkExtensionProperties& available_extension : available)
    {
        if (strcmp(available_extension.extensionName, required) == 0)
        {
            return true;
        }
    }
    return false;
}

void ez_init()
{
    VK_ASSERT(volkInitialize());

    uint32_t num_instance_available_layers;
    VK_ASSERT(vkEnumerateInstanceLayerProperties(&num_instance_available_layers, nullptr));
    std::vector<VkLayerProperties> instance_supported_layers(num_instance_available_layers);
    VK_ASSERT(vkEnumerateInstanceLayerProperties(&num_instance_available_layers, instance_supported_layers.data()));

    uint32_t num_instance_available_extensions;
    VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_available_extensions, nullptr));
    std::vector<VkExtensionProperties> instance_supported_extensions(num_instance_available_extensions);
    VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_available_extensions, instance_supported_extensions.data()));

    std::vector<const char*> instance_required_layers;
    std::vector<const char*> instance_required_extensions;
    std::vector<const char*> instance_layers;
    std::vector<const char*> instance_extensions;

#ifdef VK_DEBUG
    instance_required_layers.push_back("VK_LAYER_KHRONOS_validation");
    instance_required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    instance_required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    instance_required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    for (auto it = instance_required_layers.begin(); it != instance_required_layers.end(); ++it)
    {
        if (is_layer_supported(*it, instance_supported_layers))
        {
            instance_layers.push_back(*it);
        }
    }

    for (auto it = instance_required_extensions.begin(); it != instance_required_extensions.end(); ++it)
    {
        if (is_extension_supported(*it, instance_supported_extensions))
        {
            instance_extensions.push_back(*it);
        }
    }

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pEngineName = "vulkan";
    app_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = nullptr;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = (uint32_t)instance_layers.size();
    instance_create_info.ppEnabledLayerNames = instance_layers.data();
    instance_create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
    instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

    VK_ASSERT(vkCreateInstance(&instance_create_info, nullptr, &ctx.instance));

    volkLoadInstance(ctx.instance);

#ifdef VK_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    messenger_create_info.pfnUserCallback = debug_utils_messenger_cb;
    VK_ASSERT(vkCreateDebugUtilsMessengerEXT(ctx.instance, &messenger_create_info, nullptr, &ctx.debug_messenger));
#endif

    // Selected physical device
    uint32_t num_gpus = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(ctx.instance, &num_gpus, nullptr));
    std::vector<VkPhysicalDevice> gpus(num_gpus);
    VK_ASSERT(vkEnumeratePhysicalDevices(ctx.instance, &num_gpus, gpus.data()));
    for (auto& g : gpus)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(g, &props);
        VK_LOGI("Found Vulkan GPU: %s\n", props.deviceName);
        VK_LOGI("API: %u.%u.%u\n",
                VK_VERSION_MAJOR(props.apiVersion),
                VK_VERSION_MINOR(props.apiVersion),
                VK_VERSION_PATCH(props.apiVersion));
        VK_LOGI("Driver: %u.%u.%u\n",
                VK_VERSION_MAJOR(props.driverVersion),
                VK_VERSION_MINOR(props.driverVersion),
                VK_VERSION_PATCH(props.driverVersion));
    }
    ctx.physical_device = gpus.front();

    ctx.properties2.pNext = &ctx.properties_1_1;
    ctx.properties_1_1.pNext = &ctx.properties_1_2;
    ctx.properties_1_2.pNext = &ctx.properties_1_3;
    void** properties_chain = &ctx.properties_1_3.pNext;

    ctx.features_1_3.dynamicRendering = true;
    ctx.features_1_3.synchronization2 = true;
    ctx.features_1_3.maintenance4 = true;
    ctx.features2.pNext = &ctx.features_1_1;
    ctx.features_1_1.pNext = &ctx.features_1_2;
    ctx.features_1_2.pNext = &ctx.features_1_3;
    void** features_chain = &ctx.features_1_3.pNext;

    uint32_t num_device_available_extensions = 0;
    VK_ASSERT(vkEnumerateDeviceExtensionProperties(ctx.physical_device, nullptr, &num_device_available_extensions, nullptr));
    std::vector<VkExtensionProperties> device_available_extensions(num_device_available_extensions);
    VK_ASSERT(vkEnumerateDeviceExtensionProperties(ctx.physical_device, nullptr, &num_device_available_extensions, device_available_extensions.data()));

    std::vector<const char*> device_extensions;
    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if (is_extension_supported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, device_available_extensions))
    {
        device_extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        *features_chain = &ctx.acceleration_structure_features;
        features_chain = &ctx.acceleration_structure_features.pNext;
        *properties_chain = &ctx.acceleration_structure_properties;
        properties_chain = &ctx.acceleration_structure_properties.pNext;

        if(is_extension_supported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, device_available_extensions))
        {
            device_extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            device_extensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
            *features_chain = &ctx.raytracing_features;
            features_chain = &ctx.raytracing_features.pNext;
            *properties_chain = &ctx.raytracing_properties;
            properties_chain = &ctx.raytracing_properties.pNext;
        }

        if(is_extension_supported(VK_KHR_RAY_QUERY_EXTENSION_NAME, device_available_extensions))
        {
            device_extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
            *features_chain = &ctx.raytracing_query_features;
            features_chain = &ctx.raytracing_query_features.pNext;
        }
    }

    vkGetPhysicalDeviceFeatures2(ctx.physical_device, &ctx.features2);
    vkGetPhysicalDeviceProperties2(ctx.physical_device, &ctx.properties2);

    if (ctx.raytracing_features.rayTracingPipeline == VK_TRUE &&
        ctx.raytracing_query_features.rayQuery == VK_TRUE &&
        ctx.acceleration_structure_features.accelerationStructure == VK_TRUE &&
        ctx.features_1_2.bufferDeviceAddress == VK_TRUE)
    {
        ctx.support_features |= EZ_FEATURE_RAYTRACING;
    }

    uint32_t num_queue_families;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.physical_device, &num_queue_families, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.physical_device, &num_queue_families, queue_family_properties.data());
    uint32_t graphics_family = -1;
    for (uint32_t i = 0; i < (uint32_t)queue_family_properties.size(); i++)
    {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_family = i;
            break;
        }
    }

    const float graphics_queue_prio = 0.0f;
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphics_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &graphics_queue_prio;

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &ctx.features2;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;

    VK_ASSERT(vkCreateDevice(ctx.physical_device, &device_create_info, nullptr, &ctx.device));

    vkGetDeviceQueue(ctx.device, graphics_family, 0, &ctx.queue);

    VkCommandPoolCreateInfo cmd_pool_create_info = {};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.queueFamilyIndex = graphics_family;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    VK_ASSERT(vkCreateCommandPool(ctx.device, &cmd_pool_create_info, nullptr, &ctx.cmd_pool));

    VkCommandBufferAllocateInfo cmd_alloc_info = {};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.commandBufferCount = 1;
    cmd_alloc_info.commandPool = ctx.cmd_pool;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VK_ASSERT(vkAllocateCommandBuffers(ctx.device, &cmd_alloc_info, &ctx.cmd));

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;
    VK_ASSERT(vkBeginCommandBuffer(ctx.cmd, &begin_info));

    init_descriptor_pool();

    // Initialize vma
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = ctx.physical_device;
    allocatorInfo.device = ctx.device;
    allocatorInfo.instance = ctx.instance;
    if (ctx.features_1_2.bufferDeviceAddress)
    {
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }
    VK_ASSERT(vmaCreateAllocator(&allocatorInfo, &ctx.allocator));
}

void ez_terminate()
{
    for (auto pipeline_iter : ctx.pipeline_cache)
    {
        ez_destroy_pipeline(pipeline_iter.second);
    }
    ctx.pipeline_cache.clear();
    destroy_descriptor_pool();
    clear_stage_buffer_pool();
    clear_res_mgr();
#ifdef VK_DEBUG
    if (ctx.debug_messenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(ctx.instance, ctx.debug_messenger, nullptr);
        ctx.debug_messenger = VK_NULL_HANDLE;
    }
#endif
    vmaDestroyAllocator(ctx.allocator);
    vkDestroyCommandPool(ctx.device, ctx.cmd_pool, nullptr);
    vkDestroyDevice(ctx.device, nullptr);
    vkDestroyInstance(ctx.instance, nullptr);
}

void ez_submit()
{
    vkEndCommandBuffer(ctx.cmd);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &ctx.cmd;
    if (ctx.acquire_semaphore != VK_NULL_HANDLE)
    {
        VkPipelineStageFlags submit_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &ctx.acquire_semaphore;
        submit_info.pWaitDstStageMask = &submit_stage_mask;
    }
    if (ctx.release_semaphore != VK_NULL_HANDLE)
    {
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &ctx.release_semaphore;
    }

    VK_ASSERT(vkQueueSubmit(ctx.queue, 1, &submit_info, VK_NULL_HANDLE));

    if (ctx.swapchain != VK_NULL_HANDLE)
    {
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &ctx.release_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &ctx.swapchain;
        present_info.pImageIndices = &ctx.image_index;

        VK_ASSERT(vkQueuePresentKHR(ctx.queue, &present_info));
    }

    VK_ASSERT(vkDeviceWaitIdle(ctx.device));

    VK_ASSERT(vkResetCommandPool(ctx.device, ctx.cmd_pool, 0));
    VK_ASSERT(vkResetDescriptorPool(ctx.device, ctx.descriptor_pool, 0));

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(ctx.cmd, &begin_info);

    clear_stage_buffer_pool();

    ctx.frame_count++;
    update_res_mgr(ctx.frame_count);

    ez_reset_pipeline_state();

    ctx.swapchain = VK_NULL_HANDLE;
    ctx.acquire_semaphore = VK_NULL_HANDLE;
    ctx.release_semaphore = VK_NULL_HANDLE;
}

VkCommandBuffer ez_cmd()
{
    return ctx.cmd;
}

VkDevice ez_device()
{
    return ctx.device;
}

void ez_flush()
{
    vkDeviceWaitIdle(ctx.device);
}

// Support features
bool ez_support_feature(EzFeature feature)
{
    return ctx.support_features & feature;
}

// Props
float ez_get_timestamp_period()
{
    return ctx.properties2.properties.limits.timestampPeriod;
}

uint32_t ez_get_shader_group_handle_size()
{
    return ctx.raytracing_properties.shaderGroupHandleSize;
}

uint32_t ez_get_shader_group_handle_alignment()
{
    return ctx.raytracing_properties.shaderGroupHandleAlignment;
}

uint32_t ez_get_shader_group_base_alignment()
{
    return ctx.raytracing_properties.shaderGroupBaseAlignment;
}

// Swapchain
void ez_create_swapchain(void* window, EzSwapchain& swapchain)
{
    swapchain = new EzSwapchain_T();
#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.pNext = nullptr;
    surface_create_info.flags = 0;
    surface_create_info.hinstance = ::GetModuleHandle(nullptr);
    surface_create_info.hwnd = (HWND)window;
    VK_ASSERT(vkCreateWin32SurfaceKHR(ctx.instance, &surface_create_info, nullptr, &swapchain->surface));
#endif

    VkSurfaceCapabilitiesKHR surface_caps;
    VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physical_device, swapchain->surface, &surface_caps));
    swapchain->width = surface_caps.currentExtent.width;
    swapchain->height = surface_caps.currentExtent.height;

    VkSwapchainCreateInfoKHR sc_create_info = {};
    sc_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sc_create_info.surface = swapchain->surface;
    sc_create_info.minImageCount = 2;
    sc_create_info.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    sc_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    sc_create_info.imageExtent.width = swapchain->width;
    sc_create_info.imageExtent.height = swapchain->height;
    sc_create_info.imageArrayLayers = 1;
    sc_create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    sc_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    sc_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sc_create_info.oldSwapchain = VK_NULL_HANDLE;

    VK_ASSERT(vkCreateSwapchainKHR(ctx.device, &sc_create_info, nullptr, &swapchain->handle));

    VK_ASSERT(vkGetSwapchainImagesKHR(ctx.device, swapchain->handle, &swapchain->image_count, nullptr));
    swapchain->images.resize(swapchain->image_count);
    VK_ASSERT(vkGetSwapchainImagesKHR(ctx.device, swapchain->handle, &swapchain->image_count, swapchain->images.data()));

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_ASSERT(vkCreateSemaphore(ctx.device, &semaphore_create_info, nullptr, &swapchain->acquire_semaphore));
    VK_ASSERT(vkCreateSemaphore(ctx.device, &semaphore_create_info, nullptr, &swapchain->release_semaphore));
}

void ez_destroy_swapchain(EzSwapchain swapchain)
{
    vkDestroySwapchainKHR(ctx.device, swapchain->handle, nullptr);
    vkDestroySurfaceKHR(ctx.instance, swapchain->surface, nullptr);
    vkDestroySemaphore(ctx.device, swapchain->acquire_semaphore, nullptr);
    vkDestroySemaphore(ctx.device, swapchain->release_semaphore, nullptr);
    delete swapchain;
}

EzSwapchainStatus ez_update_swapchain(EzSwapchain swapchain)
{
    VkSurfaceCapabilitiesKHR surface_caps;
    VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physical_device, swapchain->surface, &surface_caps));
    uint32_t new_width = surface_caps.currentExtent.width;
    uint32_t new_height = surface_caps.currentExtent.height;

    if (new_width == 0 || new_height == 0)
        return EzSwapchainStatus::NotReady;

    if (swapchain->width == new_width && swapchain->height == new_height)
        return EzSwapchainStatus::Ready;

    swapchain->width = new_width;
    swapchain->height = new_height;
    VkSwapchainKHR old_handle = swapchain->handle;
    VkSwapchainCreateInfoKHR sc_create_info = {};
    sc_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sc_create_info.surface = swapchain->surface;
    sc_create_info.minImageCount = 2;
    sc_create_info.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    sc_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    sc_create_info.imageExtent.width = swapchain->width;
    sc_create_info.imageExtent.height = swapchain->height;
    sc_create_info.imageArrayLayers = 1;
    sc_create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    sc_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    sc_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sc_create_info.oldSwapchain = old_handle;

    VK_ASSERT(vkCreateSwapchainKHR(ctx.device, &sc_create_info, nullptr, &swapchain->handle));

    VK_ASSERT(vkGetSwapchainImagesKHR(ctx.device, swapchain->handle, &swapchain->image_count, nullptr));
    swapchain->images.resize(swapchain->image_count);
    VK_ASSERT(vkGetSwapchainImagesKHR(ctx.device, swapchain->handle, &swapchain->image_count, swapchain->images.data()));

    vkDestroySwapchainKHR(ctx.device, old_handle, nullptr);

    return EzSwapchainStatus::Resized;
}

void ez_acquire_next_image(EzSwapchain swapchain)
{
    vkAcquireNextImageKHR(ctx.device, swapchain->handle, ~0ull, swapchain->acquire_semaphore, VK_NULL_HANDLE, &swapchain->image_index);
    swapchain->stage_mask = 0;
    swapchain->access_mask = 0;
    swapchain->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    ctx.image_index = swapchain->image_index;
    ctx.acquire_semaphore = swapchain->acquire_semaphore;
}

void ez_present(EzSwapchain swapchain)
{
    ctx.swapchain = swapchain->handle;
    ctx.release_semaphore = swapchain->release_semaphore;
}

// Buffer
static uint32_t select_memory_type(const VkPhysicalDeviceMemoryProperties& memory_properties, uint32_t memory_type_bits, VkMemoryPropertyFlags flags)
{
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
        if ((memory_type_bits & (1 << i)) != 0 && (memory_properties.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    return ~0u;
}

void ez_create_buffer(const EzBufferDesc& desc, EzBuffer& buffer)
{
    buffer = new EzBuffer_T();
    buffer->size = desc.size;
    buffer->memory_usage = desc.memory_usage;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = desc.size;
    buffer_info.usage = desc.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (ctx.features_1_2.bufferDeviceAddress)
    {
        buffer_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = desc.memory_usage;

    VK_ASSERT(vmaCreateBuffer(ctx.allocator, &buffer_info, &alloc_info, &buffer->handle, &buffer->allocation, nullptr));

    if (buffer_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo address_info = {};
        address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        address_info.buffer = buffer->handle;
        buffer->address = vkGetBufferDeviceAddress(ctx.device, &address_info);
    }
}

void ez_destroy_buffer(EzBuffer buffer)
{
    res_mgr.destroyer_buffers.emplace_back(std::make_pair(buffer->handle, buffer->allocation), ctx.frame_count);
    delete buffer;
}

void ez_map_memory(EzBuffer buffer, void** memory_ptr)
{
    vmaMapMemory(ctx.allocator, buffer->allocation, memory_ptr);
}

void ez_unmap_memory(EzBuffer buffer)
{
    vmaUnmapMemory(ctx.allocator, buffer->allocation);
}

void ez_clear_buffer(EzBuffer buffer, uint32_t size, uint32_t offset)
{
    vkCmdFillBuffer(ctx.cmd, buffer->handle, offset, size, 0);
}

void ez_copy_buffer(EzBuffer src_buffer, EzBuffer dst_buffer, VkBufferCopy range)
{
    vkCmdCopyBuffer(ctx.cmd, src_buffer->handle, dst_buffer->handle, 1, &range);
}

void ez_update_buffer(EzBuffer buffer, uint32_t size, uint32_t offset, void* data)
{
    EzStageAllocation alloc_info = ez_alloc_stage_buffer(size);
    void* memory_ptr = nullptr;
    ez_map_memory(alloc_info.buffer, &memory_ptr);
    memcpy((uint8_t*)memory_ptr + alloc_info.offset, data, size);
    ez_unmap_memory(alloc_info.buffer);

    VkBufferCopy range = {};
    range.size = size;
    range.srcOffset = alloc_info.offset;
    range.dstOffset = offset;
    ez_copy_buffer(alloc_info.buffer, buffer, range);
}

EzStageAllocation ez_alloc_stage_buffer(size_t size)
{
    const uint64_t free_space = stage_buffer_pool.size - stage_buffer_pool.offset;
    if (size > free_space || stage_buffer_pool.current_buffer == VK_NULL_HANDLE)
    {
        if (stage_buffer_pool.current_buffer != VK_NULL_HANDLE)
        {
            ez_destroy_buffer(stage_buffer_pool.current_buffer);
        }

        stage_buffer_pool.size = ez_align_to((stage_buffer_pool.size + size) * 2, 8);
        stage_buffer_pool.offset = 0;

        EzBufferDesc buffer_desc = {};
        buffer_desc.size = stage_buffer_pool.size;
        buffer_desc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_desc.memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
        ez_create_buffer(buffer_desc, stage_buffer_pool.current_buffer);
    }

    EzStageAllocation allocation;
    allocation.buffer = stage_buffer_pool.current_buffer;
    allocation.offset = stage_buffer_pool.offset;
    stage_buffer_pool.offset += ez_align_to(size, 8);
    return allocation;
}

// Texture
void ez_create_texture(const EzTextureDesc& desc, EzTexture& texture)
{
    texture = new EzTexture_T();
    texture->width = desc.width;
    texture->height = desc.height;
    texture->depth = desc.depth;
    texture->levels = desc.levels;
    texture->layers = desc.layers;
    texture->format = desc.format;
    texture->layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = desc.image_type;
    image_info.format = desc.format;
    image_info.extent.width = desc.width;
    image_info.extent.height = desc.height;
    image_info.extent.depth = desc.depth;
    image_info.mipLevels = desc.levels;
    image_info.arrayLayers = desc.layers;
    image_info.samples = desc.samples;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = nullptr;
    image_info.flags = 0;
    image_info.usage = desc.usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VK_ASSERT(vmaCreateImage(ctx.allocator, &image_info, &alloc_info, &texture->handle, &texture->allocation, nullptr));
}

void ez_destroy_texture(EzTexture texture)
{
    res_mgr.destroyer_images.emplace_back(std::make_pair(texture->handle, texture->allocation), ctx.frame_count);
    for (auto view : texture->views)
    {
        res_mgr.destroyer_imageviews.emplace_back(view.handle, ctx.frame_count);
    }
    delete texture;
}

int ez_create_texture_view(EzTexture texture, VkImageViewType view_type, VkImageAspectFlags aspect_mask,
                           uint32_t base_level, uint32_t level_count,
                           uint32_t base_layer, uint32_t layer_count)
{
    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.flags = 0;
    view_create_info.image = texture->handle;
    view_create_info.subresourceRange.aspectMask = aspect_mask;
    view_create_info.subresourceRange.baseArrayLayer = base_layer;
    view_create_info.subresourceRange.layerCount = layer_count;
    view_create_info.subresourceRange.baseMipLevel = base_level;
    view_create_info.subresourceRange.levelCount = level_count;
    view_create_info.format = texture->format;
    view_create_info.viewType = view_type;

    VkImageView image_view;
    VK_ASSERT(vkCreateImageView(ctx.device, &view_create_info, nullptr, &image_view));

    EzTextureView view{};
    view.handle = image_view;
    view.subresource_range = view_create_info.subresourceRange;
    texture->views.push_back(view);
    return int(texture->views.size()) - 1;
}

void ez_copy_image(EzTexture src_texture, EzTexture dst_texture, const VkImageCopy& region)
{
    vkCmdCopyImage(ctx.cmd, src_texture->handle, src_texture->layout, dst_texture->handle, dst_texture->layout, 1, &region);
}

void ez_copy_image(EzTexture src_texture, EzSwapchain dst_swapchain, const VkImageCopy& region)
{
    vkCmdCopyImage(ctx.cmd, src_texture->handle, src_texture->layout, dst_swapchain->images[dst_swapchain->image_index], dst_swapchain->layout, 1, &region);
}

void ez_copy_buffer_to_image(EzBuffer buffer, EzTexture texture, VkBufferImageCopy range)
{
    vkCmdCopyBufferToImage(ctx.cmd, buffer->handle, texture->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &range);
}

void ez_clear_color_image(EzTexture texture, int texture_view, float c[4])
{
    vkCmdClearColorImage(ctx.cmd, texture->handle, texture->layout, (const VkClearColorValue*)c, 1, &texture->views[texture_view].subresource_range);
}

void ez_update_image(EzTexture texture, VkBufferImageCopy range, void* data)
{
    uint32_t data_size = ez_get_format_stride(texture->format) * range.imageExtent.width * range.imageExtent.height * range.imageExtent.depth;
    EzStageAllocation alloc_info = ez_alloc_stage_buffer(data_size);

    void* memory_ptr = nullptr;
    ez_map_memory(alloc_info.buffer, &memory_ptr);
    memcpy((uint8_t*)memory_ptr + alloc_info.offset, data, data_size);
    ez_unmap_memory(alloc_info.buffer);

    range.bufferOffset = alloc_info.offset;
    ez_copy_buffer_to_image(alloc_info.buffer, texture, range);
}

void ez_create_sampler(const EzSamplerDesc& desc, EzSampler& sampler)
{
    sampler = new EzSampler_T();

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = desc.mag_filter;
    sampler_create_info.minFilter = desc.min_filter;
    sampler_create_info.mipmapMode = desc.mipmap_mode;
    sampler_create_info.addressModeU = desc.address_u;
    sampler_create_info.addressModeV = desc.address_v;
    sampler_create_info.addressModeW = desc.address_w;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_create_info.anisotropyEnable = desc.anisotropy_enable;
    sampler_create_info.maxAnisotropy = 0.0f;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = desc.compare_enable;
    sampler_create_info.compareOp = desc.compare_op;
    VK_ASSERT(vkCreateSampler(ctx.device, &sampler_create_info, nullptr, &sampler->handle));
}

void ez_destroy_sampler(EzSampler sampler)
{
    res_mgr.destroyer_samplers.emplace_back(std::make_pair(sampler->handle, ctx.frame_count));
    delete sampler;
}

// AccelerationStructure
void fill_acceleration_structure_geometries_info(const EzAccelerationStructureBuildInfo& build_info,
                                                 std::vector<uint32_t>& primitive_counts,
                                                 std::vector<VkAccelerationStructureGeometryKHR>& geometries,
                                                 std::vector<VkAccelerationStructureBuildRangeInfoKHR>& ranges)
{
    for(auto& triangles : build_info.geometry_set.triangles)
    {
        geometries.emplace_back();
        auto& geometry = geometries.back();
        geometry = {};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.flags = triangles.flags;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        geometry.geometry.triangles.indexType = triangles.index_type;
        geometry.geometry.triangles.maxVertex = triangles.vertex_count;
        geometry.geometry.triangles.vertexStride = triangles.vertex_stride;
        geometry.geometry.triangles.vertexFormat = triangles.vertex_format;
        geometry.geometry.triangles.vertexData.deviceAddress = triangles.vertex_buffer->address + triangles.vertex_offset;
        geometry.geometry.triangles.indexData.deviceAddress = triangles.index_buffer->address + triangles.index_offset;

        primitive_counts.emplace_back();
        uint32_t& primitive_count = primitive_counts.back();
        primitive_count = triangles.index_count / 3;

        ranges.emplace_back();
        auto& range = ranges.back();
        range.primitiveCount = triangles.index_count / 3;
        range.primitiveOffset = 0;
    }

    for(auto& instances : build_info.geometry_set.instances)
    {
        geometries.emplace_back();
        auto& geometry = geometries.back();
        geometry = {};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.flags = instances.flags;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        geometry.geometry.instances.arrayOfPointers = VK_FALSE;
        geometry.geometry.instances.data.deviceAddress = instances.instance_buffer->address;

        primitive_counts.emplace_back();
        uint32_t& primitive_count = primitive_counts.back();
        primitive_count = instances.count;

        ranges.emplace_back();
        auto& range = ranges.back();
        range = {};
        range.primitiveCount = instances.count;
        range.primitiveOffset = instances.offset;
    }
}

void ez_create_acceleration_structure(const EzAccelerationStructureBuildInfo& build_info, EzAccelerationStructure& as)
{
    as = new EzAccelerationStructure_T();

    std::vector<uint32_t> primitive_counts;
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> ranges;
    fill_acceleration_structure_geometries_info(build_info, primitive_counts, geometries, ranges);

    VkAccelerationStructureBuildGeometryInfoKHR vk_build_info{};
    vk_build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    vk_build_info.flags = build_info.flags;
    vk_build_info.type = build_info.type;
    vk_build_info.geometryCount = geometries.size();
    vk_build_info.pGeometries = geometries.data();

    VkAccelerationStructureBuildSizesInfoKHR sizes_info{};
    sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    vkGetAccelerationStructureBuildSizesKHR(
        ctx.device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &vk_build_info,
        primitive_counts.data(),
        &sizes_info
    );

    as->as_size = sizes_info.accelerationStructureSize;
    as->scratch_size = EZ_MAX(sizes_info.buildScratchSize, sizes_info.updateScratchSize);

    EzBufferDesc buffer_desc = {};
    buffer_desc.size = as->as_size;
    buffer_desc.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ez_create_buffer(buffer_desc, as->buffer);

    VkAccelerationStructureCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    create_info.type = vk_build_info.type;
    create_info.buffer = as->buffer->handle;
    create_info.size = as->as_size;
    VK_ASSERT(vkCreateAccelerationStructureKHR(
        ctx.device,
        &create_info,
        nullptr,
        &as->handle
    ));
}

void ez_destroy_acceleration_structure(EzAccelerationStructure as)
{
    ez_destroy_buffer(as->buffer);
    res_mgr.destroyer_acceleration_structures.emplace_back(as->handle, ctx.frame_count);
    delete as;
}

void ez_build_acceleration_structure(const EzAccelerationStructureBuildInfo& build_info, EzAccelerationStructure as)
{
    EzBuffer scratch_buffer;
    EzBufferDesc buffer_desc = {};
    buffer_desc.size = as->scratch_size;
    buffer_desc.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_desc.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ez_create_buffer(buffer_desc, scratch_buffer);

    std::vector<uint32_t> primitive_counts;
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> ranges;
    fill_acceleration_structure_geometries_info(build_info, primitive_counts, geometries, ranges);

    VkAccelerationStructureBuildGeometryInfoKHR vk_build_info{};
    vk_build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    vk_build_info.flags = build_info.flags;
    vk_build_info.type = build_info.type;
    vk_build_info.mode = build_info.mode;
    vk_build_info.geometryCount = geometries.size();
    vk_build_info.pGeometries = geometries.data();
    vk_build_info.srcAccelerationStructure = as->handle;
    vk_build_info.dstAccelerationStructure = as->handle;
    vk_build_info.scratchData.deviceAddress = scratch_buffer->address;

    VkAccelerationStructureBuildRangeInfoKHR* p_range_info = ranges.data();
    vkCmdBuildAccelerationStructuresKHR(
        ctx.cmd,
        1,
        &vk_build_info,
        &p_range_info
    );

    ez_destroy_buffer(scratch_buffer);
}

// Pipeline
static VkShaderStageFlagBits parse_shader_stage(SpvReflectShaderStageFlagBits reflect_shader_stage)
{
    switch (reflect_shader_stage)
    {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VkShaderStageFlagBits(0);
    }
}

void ez_create_shader(void* data, size_t size, EzShader& shader)
{
    shader = new EzShader_T();

    VkShaderModuleCreateInfo shader_create_info = {};
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.codeSize = size;
    shader_create_info.pCode = (const uint32_t*)data;
    VK_ASSERT(vkCreateShaderModule(ctx.device, &shader_create_info, nullptr, &shader->handle));

    // Parse shader
    SpvReflectResult reflect_result = spvReflectCreateShaderModule(shader_create_info.codeSize, shader_create_info.pCode, &shader->reflect);

    shader->stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->stage_info.module = shader->handle;
    shader->stage_info.pName = "main";
    shader->stage_info.stage = parse_shader_stage(shader->reflect.shader_stage);

    uint32_t binding_count = 0;
    reflect_result = spvReflectEnumerateDescriptorBindings(&shader->reflect, &binding_count, nullptr);

    std::vector<SpvReflectDescriptorBinding*> bindings(binding_count);
    reflect_result = spvReflectEnumerateDescriptorBindings(&shader->reflect, &binding_count, bindings.data());

    uint32_t push_count = 0;
    reflect_result = spvReflectEnumeratePushConstantBlocks(&shader->reflect, &push_count, nullptr);

    std::vector<SpvReflectBlockVariable*> pushconstants(push_count);
    reflect_result = spvReflectEnumeratePushConstantBlocks(&shader->reflect, &push_count, pushconstants.data());

    for (auto& x : pushconstants)
    {
        shader->pushconstants.size = x->size;
        shader->pushconstants.offset = x->offset;
        shader->pushconstants.stageFlags = shader->stage_info.stage;
    }

    for (auto& x : bindings)
    {
        VkDescriptorSetLayoutBinding descriptor = {};
        descriptor.stageFlags = shader->stage_info.stage;
        descriptor.binding = x->binding;
        descriptor.descriptorCount = x->count;
        descriptor.descriptorType = (VkDescriptorType)x->descriptor_type;
        shader->layout_bindings.push_back(descriptor);
    }
}

void ez_destroy_shader(EzShader shader)
{
    spvReflectDestroyShaderModule(&shader->reflect);
    res_mgr.destroyer_shadermodules.emplace_back(shader->handle, ctx.frame_count);
    delete shader;
}

uint32_t ez_get_format_stride(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
            return 16;

        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
            return 12;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
            return 8;

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return 4;

        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_SINT:
            return 2;

        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_SINT:
            return 1;

        default:
            break;
    }

    return 0;
}

void ez_create_pipeline_layout(EzPipeline& pipeline, const std::vector<EzShader>& shaders)
{
    auto insert_shader = [&](EzShader shader) {
        if (shader == VK_NULL_HANDLE)
            return;

        uint32_t i = 0;
        for (auto& x : shader->layout_bindings)
        {
            bool found = false;
            for (auto& y : pipeline->layout_bindings)
            {
                if (x.binding == y.binding)
                {
                    found = true;
                    y.stageFlags |= x.stageFlags;
                    break;
                }
            }

            if (!found)
            {
                pipeline->layout_bindings.push_back(x);
            }
            i++;
        }

        if (shader->pushconstants.size > 0)
        {
            pipeline->pushconstants.offset = EZ_MIN(pipeline->pushconstants.offset, shader->pushconstants.offset);
            pipeline->pushconstants.size = EZ_MAX(pipeline->pushconstants.size, shader->pushconstants.size);
            pipeline->pushconstants.stageFlags |= shader->pushconstants.stageFlags;
        }
    };

    for (size_t i = 0; i < shaders.size(); ++i)
    {
        insert_shader(shaders[i]);
    }

    std::vector<VkDescriptorSetLayout> set_layouts;
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pBindings = pipeline->layout_bindings.data();
    descriptor_set_layout_create_info.bindingCount = (uint32_t)pipeline->layout_bindings.size();
    VK_ASSERT(vkCreateDescriptorSetLayout(ctx.device, &descriptor_set_layout_create_info, nullptr, &pipeline->descriptor_set_layout));
    set_layouts.push_back(pipeline->descriptor_set_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pSetLayouts = set_layouts.data();
    pipeline_layout_create_info.setLayoutCount = (uint32_t)set_layouts.size();
    if (pipeline->pushconstants.size > 0)
    {
        pipeline_layout_create_info.pushConstantRangeCount = 1;
        pipeline_layout_create_info.pPushConstantRanges = &pipeline->pushconstants;
    }
    else
    {
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;
    }
    VK_ASSERT(vkCreatePipelineLayout(ctx.device, &pipeline_layout_create_info, nullptr, &pipeline->pipeline_layout));
}

void ez_create_graphics_pipeline(const EzPipelineState& pipeline_state, const EzRenderingInfo& rendering_info, EzPipeline& pipeline)
{
    pipeline = new EzPipeline_T();
    pipeline->bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::vector<EzShader> shaders;
    shaders.push_back(pipeline_state.vertex_shader);
    shaders.push_back(pipeline_state.fragment_shader);

    // Pipeline layout
    ez_create_pipeline_layout(pipeline, shaders);

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.layout = pipeline->pipeline_layout;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    // Shader
    uint32_t shader_stage_count = 0;
    VkPipelineShaderStageCreateInfo shader_stages[2] = {};
    for (size_t i = 0; i < shaders.size(); ++i)
    {
        shader_stages[shader_stage_count++] = shaders[i]->stage_info;
    }
    pipeline_info.stageCount = shader_stage_count;
    pipeline_info.pStages = shader_stages;

    // Input
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = pipeline_state.topology;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    pipeline_info.pInputAssemblyState = &input_assembly;

    std::vector<VkVertexInputBindingDescription> input_bindings;
    std::vector<VkVertexInputAttributeDescription> input_attributes;
    for (uint32_t i = 0; i < EZ_NUM_VERTEX_BUFFERS; i++)
    {
        if (pipeline_state.vertex_layout.vertex_binding_mask & (1 << i))
        {
            input_bindings.emplace_back();
            auto& input_binding = input_bindings.back();
            input_binding.binding = i;
            input_binding.inputRate = pipeline_state.vertex_layout.vertex_bindings[i].vertex_rate;
            input_binding.stride = pipeline_state.vertex_layout.vertex_bindings[i].vertex_stride;
        }
    }

    for (uint32_t j = 0; j < EZ_NUM_VERTEX_ATTRIBS; j++)
    {
        if (pipeline_state.vertex_layout.vertex_attrib_mask & (1 << j))
        {
            input_attributes.emplace_back();
            auto& input_attribute = input_attributes.back();
            input_attribute.location = j;
            input_attribute.binding = pipeline_state.vertex_layout.vertex_attribs[j].binding;
            input_attribute.format = pipeline_state.vertex_layout.vertex_attribs[j].format;
            input_attribute.offset = pipeline_state.vertex_layout.vertex_attribs[j].offset;
        }
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = input_bindings.size();
    vertex_input_info.pVertexBindingDescriptions = input_bindings.data();
    vertex_input_info.vertexAttributeDescriptionCount = input_attributes.size();
    vertex_input_info.pVertexAttributeDescriptions = input_attributes.data();
    pipeline_info.pVertexInputState = &vertex_input_info;

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_TRUE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = pipeline_state.fill_mode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = pipeline_state.cull_mode;
    rasterizer.frontFace = pipeline_state.front_face;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    pipeline_info.pRasterizationState = &rasterizer;

    // MSAA
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = pipeline_state.multisample_state.sample_shading ? VK_TRUE : VK_FALSE;
    multisampling.rasterizationSamples = pipeline_state.multisample_state.samples;
    multisampling.alphaToCoverageEnable = pipeline_state.multisample_state.alpha_to_coverage;
    multisampling.alphaToOneEnable = pipeline_state.multisample_state.alpha_to_one;
    pipeline_info.pMultisampleState = &multisampling;

    // Blend
    std::vector<VkPipelineColorBlendAttachmentState> blend_attachments{};
    for (uint32_t i = 0; i < rendering_info.colors.size(); ++i)
    {
        VkPipelineColorBlendAttachmentState attachment{};
        attachment.blendEnable = pipeline_state.blend_state.blend_enable ? VK_TRUE : VK_FALSE;
        attachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
        attachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
        attachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
        attachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
        attachment.srcColorBlendFactor = pipeline_state.blend_state.src_color;
        attachment.dstColorBlendFactor = pipeline_state.blend_state.dst_color;
        attachment.colorBlendOp = pipeline_state.blend_state.color_op;
        attachment.srcAlphaBlendFactor = pipeline_state.blend_state.src_alpha;
        attachment.dstAlphaBlendFactor = pipeline_state.blend_state.dst_alpha;
        attachment.alphaBlendOp = pipeline_state.blend_state.alpha_op;
        blend_attachments.push_back(attachment);
    }

    VkPipelineColorBlendStateCreateInfo blending_info = {};
    blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blending_info.logicOpEnable = VK_FALSE;
    blending_info.logicOp = VK_LOGIC_OP_COPY;
    blending_info.attachmentCount = blend_attachments.size();
    blending_info.pAttachments = blend_attachments.data();
    blending_info.blendConstants[0] = 1.0f;
    blending_info.blendConstants[1] = 1.0f;
    blending_info.blendConstants[2] = 1.0f;
    blending_info.blendConstants[3] = 1.0f;
    pipeline_info.pColorBlendState = &blending_info;

    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthstencil_info = {};
    depthstencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthstencil_info.depthTestEnable = pipeline_state.depth_state.depth_test ? VK_TRUE : VK_FALSE;
    depthstencil_info.depthWriteEnable = pipeline_state.depth_state.depth_write ? VK_TRUE : VK_FALSE;
    depthstencil_info.depthCompareOp = pipeline_state.depth_state.depth_func;
    depthstencil_info.depthBoundsTestEnable = VK_FALSE;

    depthstencil_info.stencilTestEnable = pipeline_state.stencil_state.stencil_test ? VK_TRUE : VK_FALSE;
    depthstencil_info.front.compareMask = pipeline_state.stencil_state.stencil_read_mask;
    depthstencil_info.front.writeMask = pipeline_state.stencil_state.stencil_write_mask;
    depthstencil_info.front.reference = 0;
    depthstencil_info.front.compareOp = pipeline_state.stencil_state.front_stencil_func;
    depthstencil_info.front.passOp = pipeline_state.stencil_state.front_stencil_pass_op;
    depthstencil_info.front.failOp = pipeline_state.stencil_state.front_stencil_fail_op;
    depthstencil_info.front.depthFailOp = pipeline_state.stencil_state.front_stencil_depth_fail_op;
    depthstencil_info.back.compareMask = pipeline_state.stencil_state.stencil_read_mask;
    depthstencil_info.back.writeMask = pipeline_state.stencil_state.stencil_write_mask;
    depthstencil_info.back.reference = 0;
    depthstencil_info.back.compareOp = pipeline_state.stencil_state.back_stencil_func;
    depthstencil_info.back.passOp = pipeline_state.stencil_state.back_stencil_pass_op;
    depthstencil_info.back.failOp = pipeline_state.stencil_state.back_stencil_fail_op;
    depthstencil_info.back.depthFailOp = pipeline_state.stencil_state.back_stencil_depth_fail_op;
    pipeline_info.pDepthStencilState = &depthstencil_info;

    // Tessellation
    VkPipelineTessellationStateCreateInfo tessellation_info = {};
    tessellation_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellation_info.patchControlPoints = 3;
    pipeline_info.pTessellationState = &tessellation_info;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = nullptr;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = nullptr;
    pipeline_info.pViewportState = &viewport_state;

    // Dynamic states
    VkDynamicState dynamic_states[5];
    dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;
    dynamic_states[2] = VK_DYNAMIC_STATE_DEPTH_BIAS;
    dynamic_states[3] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    dynamic_states[4] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.flags = 0;
    dynamic_state.dynamicStateCount = 5;
    dynamic_state.pDynamicStates = dynamic_states;
    pipeline_info.pDynamicState = &dynamic_state;

    // Renderpass layout
    std::vector<VkFormat> color_formats;
    VkFormat depth_format = VK_FORMAT_UNDEFINED;
    for (auto& x : rendering_info.colors)
    {
        color_formats.push_back(x.texture->format);
    }

    for (auto& x : rendering_info.depth)
    {
        depth_format = x.texture->format;
        break;
    }
    VkPipelineRenderingCreateInfo rendering_create_info = {};

    rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_create_info.colorAttachmentCount = (uint32_t)color_formats.size();
    rendering_create_info.pColorAttachmentFormats = color_formats.data();
    rendering_create_info.depthAttachmentFormat = depth_format;
    pipeline_info.pNext = &rendering_create_info;

    VK_ASSERT(vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline->handle));
}

void ez_create_compute_pipeline(const EzPipelineState& pipeline_state, EzPipeline& pipeline)
{
    pipeline = new EzPipeline_T();
    pipeline->bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;

    std::vector<EzShader> shaders;
    shaders.push_back(pipeline_state.compute_shader);
    ez_create_pipeline_layout(pipeline, shaders);

    VkComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.layout = pipeline->pipeline_layout;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.stage = pipeline_state.compute_shader->stage_info;
    VK_ASSERT(vkCreateComputePipelines(ctx.device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline->handle));
}

void ez_destroy_pipeline(EzPipeline pipeline)
{
    res_mgr.destroyer_pipelines.emplace_back(pipeline->handle, ctx.frame_count);
    res_mgr.destroyer_pipeline_layouts.emplace_back(pipeline->pipeline_layout, ctx.frame_count);
    res_mgr.destroyer_descriptor_set_layouts.emplace_back(pipeline->descriptor_set_layout, ctx.frame_count);
    delete pipeline;
}

std::size_t ez_get_compute_pipeline_hash(const EzPipelineState& pipeline_state)
{
    std::size_t hash = 0;
    hash_combine(hash, pipeline_state.compute_shader);
    return hash;
}

std::size_t ez_get_graphics_pipeline_hash(const EzPipelineState& pipeline_state, const EzRenderingInfo& rendering_info)
{
    std::size_t hash = 0;

    hash_combine(hash, pipeline_state.vertex_shader);
    hash_combine(hash, pipeline_state.fragment_shader);

    hash_combine(hash, pipeline_state.vertex_layout.vertex_binding_mask);
    for (uint32_t i = 0; i < EZ_NUM_VERTEX_BUFFERS; i++)
    {
        hash_combine(hash, pipeline_state.vertex_layout.vertex_bindings[i].vertex_stride);
        hash_combine(hash, pipeline_state.vertex_layout.vertex_bindings[i].vertex_rate);
    }

    hash_combine(hash, pipeline_state.vertex_layout.vertex_attrib_mask);
    for (uint32_t j = 0; j < EZ_NUM_VERTEX_ATTRIBS; j++)
    {
        hash_combine(hash, pipeline_state.vertex_layout.vertex_attribs[j].binding);
        hash_combine(hash, pipeline_state.vertex_layout.vertex_attribs[j].offset);
        hash_combine(hash, pipeline_state.vertex_layout.vertex_attribs[j].format);
    }

    hash_combine(hash, pipeline_state.blend_state.blend_enable);
    hash_combine(hash, pipeline_state.blend_state.src_color);
    hash_combine(hash, pipeline_state.blend_state.dst_color);
    hash_combine(hash, pipeline_state.blend_state.color_op);
    hash_combine(hash, pipeline_state.blend_state.src_alpha);
    hash_combine(hash, pipeline_state.blend_state.dst_alpha);
    hash_combine(hash, pipeline_state.blend_state.alpha_op);

    hash_combine(hash, pipeline_state.depth_state.depth_test);
    hash_combine(hash, pipeline_state.depth_state.depth_write);
    hash_combine(hash, pipeline_state.depth_state.depth_func);

    hash_combine(hash, pipeline_state.stencil_state.stencil_test);
    hash_combine(hash, pipeline_state.stencil_state.stencil_read_mask);
    hash_combine(hash, pipeline_state.stencil_state.stencil_write_mask);
    hash_combine(hash, pipeline_state.stencil_state.front_stencil_fail_op);
    hash_combine(hash, pipeline_state.stencil_state.front_stencil_depth_fail_op);
    hash_combine(hash, pipeline_state.stencil_state.front_stencil_pass_op);
    hash_combine(hash, pipeline_state.stencil_state.front_stencil_func);
    hash_combine(hash, pipeline_state.stencil_state.back_stencil_fail_op);
    hash_combine(hash, pipeline_state.stencil_state.back_stencil_depth_fail_op);
    hash_combine(hash, pipeline_state.stencil_state.back_stencil_pass_op);
    hash_combine(hash, pipeline_state.stencil_state.back_stencil_func);

    hash_combine(hash, pipeline_state.multisample_state.sample_shading);
    hash_combine(hash, pipeline_state.multisample_state.alpha_to_coverage);
    hash_combine(hash, pipeline_state.multisample_state.alpha_to_one);
    hash_combine(hash, pipeline_state.multisample_state.samples);

    hash_combine(hash, pipeline_state.fill_mode);
    hash_combine(hash, pipeline_state.topology);
    hash_combine(hash, pipeline_state.front_face);
    hash_combine(hash, pipeline_state.cull_mode);

    for (auto& x : rendering_info.colors)
    {
        hash_combine(hash, x.texture->format);
    }

    for (auto& x : rendering_info.depth)
    {
        hash_combine(hash, x.texture->format);
    }
    return hash;
}

void ez_flush_graphics_state()
{
    EzPipeline pipeline = VK_NULL_HANDLE;
    std::size_t hash = ez_get_graphics_pipeline_hash(ctx.pipeline_state, ctx.rendering_info);
    auto iter = ctx.pipeline_cache.find(hash);
    if (iter != ctx.pipeline_cache.end())
    {
        pipeline = iter->second;
    }
    else
    {
        ez_create_graphics_pipeline(ctx.pipeline_state, ctx.rendering_info, pipeline);
        ctx.pipeline_cache[hash] = pipeline;
    }
    if (ctx.pipeline != pipeline)
    {
        binding_table.dirty = true;
        ctx.pipeline = pipeline;
        vkCmdBindPipeline(ctx.cmd, pipeline->bind_point, pipeline->handle);
    }
    flush_binding_table();

    if (binding_table.pushconstants_size > 0)
    {
        binding_table.pushconstants_size = 0;
        vkCmdPushConstants(ctx.cmd,
                           ctx.pipeline->pipeline_layout,
                           ctx.pipeline->pushconstants.stageFlags,
                           0,
                           ctx.pipeline->pushconstants.size,
                           binding_table.pushconstants_data);
    }
}

void ez_flush_compute_state()
{
    EzPipeline pipeline = VK_NULL_HANDLE;
    std::size_t hash = ez_get_compute_pipeline_hash(ctx.pipeline_state);
    auto iter = ctx.pipeline_cache.find(hash);
    if (iter != ctx.pipeline_cache.end())
    {
        pipeline = iter->second;
    }
    else
    {
        ez_create_compute_pipeline(ctx.pipeline_state, pipeline);
        ctx.pipeline_cache[hash] = pipeline;
    }
    if (ctx.pipeline != pipeline)
    {
        binding_table.dirty = true;
        ctx.pipeline = pipeline;
        vkCmdBindPipeline(ctx.cmd, pipeline->bind_point, pipeline->handle);
    }
    flush_binding_table();

    if (binding_table.pushconstants_size > 0)
    {
        binding_table.pushconstants_size = 0;
        vkCmdPushConstants(ctx.cmd,
                           ctx.pipeline->pipeline_layout,
                           ctx.pipeline->pushconstants.stageFlags,
                           0,
                           ctx.pipeline->pushconstants.size,
                           binding_table.pushconstants_data);
    }
}

void ez_set_pipeline_state(const EzPipelineState& pipeline_state)
{
    ctx.pipeline_state = pipeline_state;
}

void ez_reset_pipeline_state()
{
    ctx.pipeline_state = {};
    ctx.pipeline = VK_NULL_HANDLE;
}

void ez_set_vertex_shader(EzShader shader)
{
    ctx.pipeline_state.vertex_shader = shader;
}

void ez_set_fragment_shader(EzShader shader)
{
    ctx.pipeline_state.fragment_shader = shader;
}

void ez_set_compute_shader(EzShader shader)
{
    ctx.pipeline_state.compute_shader = shader;
}

void ez_set_vertex_binding(uint32_t binding, uint32_t stride, VkVertexInputRate rate)
{
    ez_set_vertex_binding(ctx.pipeline_state.vertex_layout, binding, stride, rate);
}

void ez_set_vertex_attrib(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
    ez_set_vertex_attrib(ctx.pipeline_state.vertex_layout, binding, location, format, offset);
}

void ez_set_vertex_binding(EzVertexLayout& vertex_layout, uint32_t binding, uint32_t stride, VkVertexInputRate rate)
{
    vertex_layout.vertex_bindings[binding].vertex_stride = stride;
    vertex_layout.vertex_bindings[binding].vertex_rate = rate;
    vertex_layout.vertex_binding_mask |= 1 << binding;
}

void ez_set_vertex_attrib(EzVertexLayout& vertex_layout, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
    vertex_layout.vertex_attribs[location].binding = binding;
    vertex_layout.vertex_attribs[location].format = format;
    vertex_layout.vertex_attribs[location].offset = offset;
    vertex_layout.vertex_attrib_mask |= 1 << location;
}

void ez_set_vertex_layout(const EzVertexLayout& vertex_layout)
{
    ctx.pipeline_state.vertex_layout = vertex_layout;
}

void ez_set_blend_state(const EzBlendState& blend_state)
{
    ctx.pipeline_state.blend_state = blend_state;
}

void ez_set_depth_state(const EzDepthState& depth_state)
{
    ctx.pipeline_state.depth_state = depth_state;
}

void ez_set_stencil_state(const EzStencilState& stencil_state)
{
    ctx.pipeline_state.stencil_state = stencil_state;
}

void ez_set_multisample_state(const EzMultisampleState& multisample_state)
{
    ctx.pipeline_state.multisample_state = multisample_state;
}

void ez_set_polygon_mode(VkPolygonMode fill_mode)
{
    ctx.pipeline_state.fill_mode = fill_mode;
}

void ez_set_primitive_topology(VkPrimitiveTopology topology)
{
    ctx.pipeline_state.topology = topology;
}

void ez_set_front_face(VkFrontFace front_face)
{
    ctx.pipeline_state.front_face = front_face;
}

void ez_set_cull_mode(VkCullModeFlagBits cull_mode)
{
    ctx.pipeline_state.cull_mode = cull_mode;
}

void ez_begin_rendering(const EzRenderingInfo& rendering_info)
{
    ctx.rendering_info = rendering_info;
    std::vector<VkRenderingAttachmentInfo> color_attachments;
    for (auto& x : rendering_info.colors)
    {
        VkRenderingAttachmentInfo color_attachment = {};
        color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color_attachment.imageView = x.texture->views[x.texture_view].handle;
        color_attachment.imageLayout = x.texture->layout;
        color_attachment.loadOp = x.load_op;
        color_attachment.storeOp = x.store_op;
        color_attachment.clearValue = x.clear_value;
        if (x.resolve_texture)
        {
            color_attachment.resolveImageView = x.resolve_texture->views[x.resolve_texture_view].handle;
            color_attachment.resolveImageLayout = x.resolve_texture->layout;
            color_attachment.resolveMode = x.resolve_mode;
        }
        color_attachments.push_back(color_attachment);
    }

    std::vector<VkRenderingAttachmentInfo> depth_attachments;
    for (auto& x : rendering_info.depth)
    {
        VkRenderingAttachmentInfo depth_attachment = {};
        depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_attachment.imageView = x.texture->views[x.texture_view].handle;
        depth_attachment.imageLayout = x.texture->layout;
        depth_attachment.loadOp = x.load_op;
        depth_attachment.storeOp = x.store_op;
        depth_attachment.clearValue = x.clear_value;
        depth_attachments.push_back(depth_attachment);
        break;
    }

    VkRenderingInfo pass_info = {};
    pass_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    pass_info.renderArea.extent.width = rendering_info.width;
    pass_info.renderArea.extent.height = rendering_info.height;
    pass_info.layerCount = 1;
    pass_info.colorAttachmentCount = (uint32_t)color_attachments.size();
    pass_info.pColorAttachments = color_attachments.data();
    pass_info.pDepthAttachment = depth_attachments.data();

    vkCmdBeginRendering(ctx.cmd, &pass_info);
}

void ez_end_rendering()
{
    vkCmdEndRendering(ctx.cmd);
}

void ez_set_scissor(int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    VkRect2D scissor;
    scissor.extent.width = abs(right - left);
    scissor.extent.height = abs(top - bottom);
    scissor.offset.x = left;
    scissor.offset.y = top;
    vkCmdSetScissor(ctx.cmd, 0, 1, &scissor);
}

void ez_set_viewport(float x, float y, float w, float h, float min_depth, float max_depth)
{
    VkViewport viewport;
    viewport.x = x;
    viewport.y = y;
    viewport.width = w;
    viewport.height = h;
    viewport.minDepth = min_depth;
    viewport.maxDepth = max_depth;
    vkCmdSetViewport(ctx.cmd, 0, 1, &viewport);
}

void ez_bind_vertex_buffer(uint32_t binding, EzBuffer vertex_buffer, uint64_t offset)
{
    vkCmdBindVertexBuffers(ctx.cmd, binding, 1, &vertex_buffer->handle, &offset);
}

void ez_bind_vertex_buffers(uint32_t first_binding, uint32_t binding_count, EzBuffer* vertex_buffers, const uint64_t* offsets)
{
    VkBuffer vk_buffers[EZ_NUM_VERTEX_BUFFERS];
    VkDeviceSize vk_offsets[EZ_NUM_VERTEX_BUFFERS];
    for (uint32_t i = 0; i < binding_count; ++i)
    {
        vk_buffers[i] = vertex_buffers[i]->handle;
        if (offsets)
        {
            vk_offsets[i] = offsets[i];
        }
        else
        {
            vk_offsets[i] = 0;
        }
    }
    vkCmdBindVertexBuffers(ctx.cmd, first_binding, binding_count, vk_buffers, vk_offsets);
}

void ez_bind_index_buffer(EzBuffer index_buffer, VkIndexType type, uint64_t offset)
{
    vkCmdBindIndexBuffer(ctx.cmd, index_buffer->handle, offset, type);
}

void ez_bind_texture(uint32_t binding, EzTexture texture, int texture_view)
{
    binding_table.dirty = true;
    for (auto i = binding_table.bindings[binding].images.size(); i < 1; ++i)
    {
        binding_table.bindings[binding].images.emplace_back();
    }
    binding_table.bindings[binding].images[0].imageView = texture->views[texture_view].handle;
    binding_table.bindings[binding].images[0].imageLayout = texture->layout;
}

void ez_bind_texture_array(uint32_t binding, EzTexture texture, int texture_view, int array_idx)
{
    binding_table.dirty = true;
    for (auto i = binding_table.bindings[binding].images.size(); i < array_idx + 1; ++i)
    {
        binding_table.bindings[binding].images.emplace_back();
    }
    binding_table.bindings[binding].images[array_idx].imageView = texture->views[texture_view].handle;
    binding_table.bindings[binding].images[array_idx].imageLayout = texture->layout;
}

void ez_bind_buffer(uint32_t binding, EzBuffer buffer)
{
    binding_table.dirty = true;
    binding_table.bindings[binding].buffer.buffer = buffer->handle;
    binding_table.bindings[binding].buffer.offset = 0;
    binding_table.bindings[binding].buffer.range = buffer->size;
}

void ez_bind_buffer(uint32_t binding, EzBuffer buffer, uint64_t size, uint64_t offset)
{
    binding_table.dirty = true;
    binding_table.bindings[binding].buffer.buffer = buffer->handle;
    binding_table.bindings[binding].buffer.offset = offset;
    binding_table.bindings[binding].buffer.range = size;
}

void ez_bind_sampler(uint32_t binding, EzSampler sampler)
{
    binding_table.dirty = true;
    for (auto i = binding_table.bindings[binding].images.size(); i < 1; ++i)
    {
        binding_table.bindings[binding].images.emplace_back();
    }
    binding_table.bindings[binding].images[0].sampler = sampler->handle;
}

void ez_bind_sampler_array(uint32_t binding, EzSampler sampler, int array_idx)
{
    binding_table.dirty = true;
    for (auto i = binding_table.bindings[binding].images.size(); i < array_idx + 1; ++i)
    {
        binding_table.bindings[binding].images.emplace_back();
    }
    binding_table.bindings[binding].images[array_idx].sampler = sampler->handle;
}

void ez_push_constants(const void* data, uint32_t size, uint32_t offset)
{
    binding_table.pushconstants_size = size;
    memcpy(binding_table.pushconstants_data + offset, data, size);
}

void ez_draw(uint32_t vertex_count, uint32_t vertex_offset)
{
    ez_flush_graphics_state();
    vkCmdDraw(ctx.cmd, vertex_count, 1, vertex_offset, 0);
}

void ez_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t vertex_offset, uint32_t instance_offset)
{
    ez_flush_graphics_state();
    vkCmdDraw(ctx.cmd, vertex_count, instance_count, vertex_offset, instance_offset);
}

void ez_draw_indexed(uint32_t index_count, uint32_t index_offset, int32_t vertex_offset)
{
    ez_flush_graphics_state();
    vkCmdDrawIndexed(ctx.cmd, index_count, 1, index_offset, vertex_offset, 0);
}

void ez_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t index_offset, int32_t vertex_offset, uint32_t instance_offset)
{
    ez_flush_graphics_state();
    vkCmdDrawIndexed(ctx.cmd, index_count, instance_count, index_offset, vertex_offset, instance_offset);
}

void ez_draw_indirect(EzBuffer buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
{
    ez_flush_graphics_state();
    vkCmdDrawIndirect(ctx.cmd, buffer->handle, offset, draw_count, stride);
}

void ez_draw_indexed_indirect(EzBuffer buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
{
    ez_flush_graphics_state();
    vkCmdDrawIndexedIndirect(ctx.cmd, buffer->handle, offset, draw_count, stride);
}

void ez_dispatch(uint32_t thread_group_x, uint32_t thread_group_y, uint32_t thread_group_z)
{
    ez_flush_compute_state();
    vkCmdDispatch(ctx.cmd, thread_group_x, thread_group_y, thread_group_z);
}

void ez_dispatch_indirect(EzBuffer buffer, uint64_t offset)
{
    ez_flush_compute_state();
    vkCmdDispatchIndirect(ctx.cmd, buffer->handle, offset);
}

void ez_create_raytracing_pipeline(const EzRaytracingPipelineDesc& desc, EzPipeline& pipeline)
{
    pipeline = new EzPipeline_T();
    pipeline->bind_point = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

    ez_create_pipeline_layout(pipeline, desc.shaders);

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for(auto& shader : desc.shaders)
    {
        stages.push_back(shader->stage_info);
    }

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
    groups.reserve(desc.groups.size());
    for (auto& x : desc.groups)
    {
        groups.emplace_back();
        auto& group = groups.back();
        group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type = x.type;
        group.generalShader = x.general_shader;
        group.closestHitShader = x.closesthit_shader;
        group.anyHitShader = x.anyhit_shader;
        group.intersectionShader = x.intersection_shader;
    }

    VkRayTracingPipelineCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    create_info.flags = 0;
    create_info.maxPipelineRayRecursionDepth = desc.max_trace_recursion_depth;
    create_info.layout = pipeline->pipeline_layout;
    create_info.stageCount = (uint32_t)stages.size();
    create_info.pStages = stages.data();
    create_info.groupCount = (uint32_t)groups.size();
    create_info.pGroups = groups.data();
    VK_ASSERT(vkCreateRayTracingPipelinesKHR(
        ctx.device,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        1,
        &create_info,
        nullptr,
        &pipeline->handle
    ));
}

void ez_bind_raytracing_pipeline(EzPipeline pipeline)
{
    if (ctx.pipeline != pipeline)
    {
        binding_table.dirty = true;
        ctx.pipeline = pipeline;
        vkCmdBindPipeline(ctx.cmd, pipeline->bind_point, pipeline->handle);
    }
}

void ez_get_raytracing_group_handle(EzPipeline pipeline, uint32_t first_group, uint32_t group_count, size_t data_size, void* data)
{
    vkGetRayTracingShaderGroupHandlesKHR(ctx.device, pipeline->handle, first_group, group_count, data_size, data);
}

void ez_trace_rays(VkStridedDeviceAddressRegionKHR* raygen,
                   VkStridedDeviceAddressRegionKHR* miss,
                   VkStridedDeviceAddressRegionKHR* hit,
                   VkStridedDeviceAddressRegionKHR* callable,
                   uint32_t width, uint32_t height, uint32_t depth)
{
    flush_binding_table();

    if (binding_table.pushconstants_size > 0)
    {
        binding_table.pushconstants_size = 0;
        vkCmdPushConstants(ctx.cmd,
                           ctx.pipeline->pipeline_layout,
                           ctx.pipeline->pushconstants.stageFlags,
                           0,
                           ctx.pipeline->pushconstants.size,
                           binding_table.pushconstants_data);
    }

    vkCmdTraceRaysKHR(
        ctx.cmd,
        raygen,
        miss,
        hit,
        callable,
        width,
        height,
        depth
    );
}

// Barrier
VkImageMemoryBarrier2 ez_image_barrier(EzSwapchain swapchain,
                                       VkPipelineStageFlags2 stage_mask,
                                       VkAccessFlags2 access_mask,
                                       VkImageLayout layout,
                                       VkImageAspectFlags aspect_mask)
{
    VkImageMemoryBarrier2 barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = swapchain->stage_mask;
    barrier.srcAccessMask = swapchain->access_mask;
    barrier.dstStageMask = stage_mask;
    barrier.dstAccessMask = access_mask;
    barrier.oldLayout = swapchain->layout;
    barrier.newLayout = layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapchain->images[swapchain->image_index];
    barrier.subresourceRange.aspectMask = aspect_mask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    swapchain->stage_mask = stage_mask;
    swapchain->access_mask = access_mask;
    swapchain->layout = layout;
    return barrier;
}

VkImageMemoryBarrier2 ez_image_barrier(EzTexture texture,
                                       VkPipelineStageFlags2 stage_mask,
                                       VkAccessFlags2 access_mask,
                                       VkImageLayout layout,
                                       VkImageAspectFlags aspect_mask)
{
    VkImageMemoryBarrier2 barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = texture->stage_mask;
    barrier.srcAccessMask = texture->access_mask;
    barrier.dstStageMask = stage_mask;
    barrier.dstAccessMask = access_mask;
    barrier.oldLayout = texture->layout;
    barrier.newLayout = layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture->handle;
    barrier.subresourceRange.aspectMask = aspect_mask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    texture->stage_mask = stage_mask;
    texture->access_mask = access_mask;
    texture->layout = layout;
    return barrier;
}

VkBufferMemoryBarrier2 ez_buffer_barrier(EzBuffer buffer,
                                         VkPipelineStageFlags2 stage_mask,
                                         VkAccessFlags2 access_mask)
{
    VkBufferMemoryBarrier2 barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = buffer->stage_mask;
    barrier.srcAccessMask = buffer->access_mask;
    barrier.dstStageMask = stage_mask;
    barrier.dstAccessMask = access_mask;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer->handle;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    buffer->stage_mask = stage_mask;
    buffer->access_mask = access_mask;
    return barrier;
}

VkImageAspectFlags ez_get_aspect_mask(VkFormat format)
{
    VkImageAspectFlags result = 0;
    switch (format)
    {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        case VK_FORMAT_S8_UINT:
            result = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        default:
            result = VK_IMAGE_ASPECT_COLOR_BIT;
            break;
    }
    return result;
}

VkAccessFlags ez_get_access_flags(EzResourceState state)
{
    VkAccessFlags ret = 0;
    if (state & EZ_RESOURCE_STATE_COPY_SOURCE)
    {
        ret |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (state & EZ_RESOURCE_STATE_COPY_DEST)
    {
        ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (state & EZ_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
    {
        ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (state & EZ_RESOURCE_STATE_INDEX_BUFFER)
    {
        ret |= VK_ACCESS_INDEX_READ_BIT;
    }
    if (state & EZ_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (state & EZ_RESOURCE_STATE_INDIRECT_ARGUMENT)
    {
        ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if (state & EZ_RESOURCE_STATE_RENDERTARGET)
    {
        ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if (state & EZ_RESOURCE_STATE_DEPTH_WRITE)
    {
        ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    if ((state & EZ_RESOURCE_STATE_SHADER_RESOURCE) || (state & EZ_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
    {
        ret |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (state & EZ_RESOURCE_STATE_PRESENT)
    {
        //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR expected accessMask are VkAccessFlags2(0).
        //ret |= VK_ACCESS_MEMORY_READ_BIT;
    }

    return ret;
}

VkImageLayout ez_get_image_layout(EzResourceState state)
{
    if (state & EZ_RESOURCE_STATE_COPY_SOURCE)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (state & EZ_RESOURCE_STATE_COPY_DEST)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (state & EZ_RESOURCE_STATE_RENDERTARGET)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (state & EZ_RESOURCE_STATE_DEPTH_WRITE)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (state & EZ_RESOURCE_STATE_UNORDERED_ACCESS)
        return VK_IMAGE_LAYOUT_GENERAL;

    if ((state & EZ_RESOURCE_STATE_SHADER_RESOURCE) || (state & EZ_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (state & EZ_RESOURCE_STATE_PRESENT)
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (state == EZ_RESOURCE_STATE_COMMON)
        return VK_IMAGE_LAYOUT_GENERAL;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags ez_get_pipeline_stage_flags(VkAccessFlags access_flags)
{
    VkPipelineStageFlags flags = 0;

    if ((access_flags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

    if ((access_flags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
    {
        flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
        flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        if(ez_support_feature(EZ_FEATURE_RAYTRACING))
        {
            flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
        }
    }

    if ((access_flags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    if ((access_flags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if ((access_flags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    if ((access_flags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    if ((access_flags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

    if ((access_flags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_HOST_BIT;

    return flags;
}

VkImageMemoryBarrier2 ez_image_barrier(EzSwapchain swapchain, EzResourceState resource_state)
{
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageLayout image_layout = ez_get_image_layout(resource_state);
    VkAccessFlags access_flags = ez_get_access_flags(resource_state);
    VkPipelineStageFlags pipeline_stage_flags = ez_get_pipeline_stage_flags(access_flags);
    return ez_image_barrier(swapchain, pipeline_stage_flags, access_flags, image_layout, aspect_mask);
}

VkImageMemoryBarrier2 ez_image_barrier(EzTexture texture, EzResourceState resource_state)
{
    VkImageAspectFlags aspect_mask = ez_get_aspect_mask(texture->format);
    VkImageLayout image_layout = ez_get_image_layout(resource_state);
    VkAccessFlags access_flags = ez_get_access_flags(resource_state);
    VkPipelineStageFlags pipeline_stage_flags = ez_get_pipeline_stage_flags(access_flags);
    return ez_image_barrier(texture, pipeline_stage_flags, access_flags, image_layout, aspect_mask);
}

VkBufferMemoryBarrier2 ez_buffer_barrier(EzBuffer buffer, EzResourceState resource_state)
{
    VkAccessFlags access_flags = ez_get_access_flags(resource_state);
    VkPipelineStageFlags pipeline_stage_flags = ez_get_pipeline_stage_flags(access_flags);
    return ez_buffer_barrier(buffer, pipeline_stage_flags, access_flags);
}

VkMemoryBarrier2 ez_memory_barrier(VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
                                   VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
{
    VkMemoryBarrier2 barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstAccessMask = dst_access_mask;
    barrier.srcStageMask = src_stage_mask;
    barrier.dstStageMask = dst_stage_mask;
    return barrier;
}

void ez_pipeline_barrier(VkDependencyFlags dependency_flags,
                         size_t buffer_barrier_count, const VkBufferMemoryBarrier2* buffer_barriers,
                         size_t image_barrier_count, const VkImageMemoryBarrier2* image_barriers,
                         size_t memory_barrier_count, const VkMemoryBarrier2* memory_barriers)
{
    VkDependencyInfo dependency_info = {};
    dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency_info.dependencyFlags = dependency_flags;
    dependency_info.bufferMemoryBarrierCount = unsigned(buffer_barrier_count);
    dependency_info.pBufferMemoryBarriers = buffer_barriers;
    dependency_info.imageMemoryBarrierCount = unsigned(image_barrier_count);
    dependency_info.pImageMemoryBarriers = image_barriers;
    dependency_info.memoryBarrierCount = unsigned(memory_barrier_count);;
    dependency_info.pMemoryBarriers = memory_barriers;

    vkCmdPipelineBarrier2(ctx.cmd, &dependency_info);
}

void ez_create_query_pool(uint32_t query_count, VkQueryType type, EzQueryPool& query_pool)
{
    query_pool = new EzQueryPool_T();
    query_pool->type = type;
    query_pool->query_count = query_count;

    VkQueryPoolCreateInfo query_pool_create_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    query_pool_create_info.queryType = type;
    query_pool_create_info.queryCount = query_count;
    VK_ASSERT(vkCreateQueryPool(ctx.device, &query_pool_create_info, nullptr, &query_pool->handle));
}

void ez_destroy_query_pool(EzQueryPool query_pool)
{
    res_mgr.destroyer_query_pools.emplace_back(query_pool->handle, ctx.frame_count);
    delete query_pool;
}

void ez_reset_query_pool(EzQueryPool query_pool, uint32_t start_query, uint32_t query_count)
{
    vkCmdResetQueryPool(ctx.cmd, query_pool->handle, start_query, query_count);
}

void ez_write_timestamp(EzQueryPool query_pool, uint32_t query_index)
{
    vkCmdWriteTimestamp(ctx.cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool->handle, query_index);
}

void ez_get_query_pool_results(EzQueryPool query_pool,
                               uint32_t first_query,
                               uint32_t query_count,
                               uint32_t data_size,
                               void* data,
                               uint32_t stride,
                               VkQueryResultFlags flags)
{
    vkGetQueryPoolResults(ctx.device, query_pool->handle, first_query, query_count, data_size, data, stride, flags);
}

void ez_begin_debug_label(const char* label_name, const float color[4])
{
    VkDebugUtilsLabelEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = label_name;
    memcpy(label.color, color, 4 * sizeof(float));
    vkCmdBeginDebugUtilsLabelEXT(ctx.cmd, &label);
}

void ez_end_debug_label()
{
    vkCmdEndDebugUtilsLabelEXT(ctx.cmd);
}