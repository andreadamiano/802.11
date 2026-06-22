idf_component_register(SRCS "active_scanning.c" "utils/frames.c" "utils/802.11.c"
                    INCLUDE_DIRS "." "utils"
                    REQUIRES esp_wifi
                    REQUIRES nvs_flash)
