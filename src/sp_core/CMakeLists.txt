add_library(sp_core STATIC
        sp_core.h
        sp_core.cpp
        sp_log.h
        sp_event.h
        sp_event.cpp
        sp_module.h
        sp_module.cpp
        sp_application.h
        sp_application.cpp)
target_link_libraries(spark PUBLIC sp_core)