set(src_dirs
            IIC
            SPI
            WIFI
            UART
            KEY
            DS3231
            MAX7219
            24CXX
            TIMER)

set(include_dirs
            IIC
            SPI
            WIFI
            UART
            KEY
            DS3231
            MAX7219
            24CXX
            TIMER)

set(requires
            driver
            esp_wifi
            esp_netif
            esp_event
            nvs_flash
            esp_timer)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)