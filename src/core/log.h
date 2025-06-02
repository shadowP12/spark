#pragma once

#define LOGE(...)                                 \
    do                                               \
    {                                                \
        fprintf(stderr, "[LOG_ERROR]: " __VA_ARGS__); \
        fflush(stderr);                              \
    } while (false)

#define LOGW(...)                                \
    do                                              \
    {                                               \
        fprintf(stderr, "[LOG_WARN]: " __VA_ARGS__); \
        fflush(stderr);                             \
    } while (false)

#define LOGI(...)                                \
    do                                              \
    {                                               \
        fprintf(stderr, "[LOG_INFO]: " __VA_ARGS__); \
        fflush(stderr);                             \
    } while (false)