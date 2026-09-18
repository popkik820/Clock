set(src_dirs
        TIME_SYNC
        DISPLAY
        MEMORY
        CL73T2
        KEY_CONTROL
        ALARM
        STOPWATCH
)

set(include_dirs
        DISPLAY
        TIME_SYNC
        MEMORY
        CL73T2
        KEY_CONTROL
        ALARM
        STOPWATCH
)

set(requires
        BSP
        esp_netif
        lwip
)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})