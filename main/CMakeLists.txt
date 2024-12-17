set(srcs
    "main.cpp"
    "gui.cpp"
    "wifi.cpp"
    "lvgl-touch-xpt2046.cpp"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    REQUIRES
        esp_hw_support
        esp_event
        esp_lcd
        esp_adc
        neopixel
        display
        lvgl
        esp_wifi
        nvs_flash
        wifi_provisioning
)