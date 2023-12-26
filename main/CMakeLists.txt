set(srcs
    "main.cpp"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    REQUIRES
        esp_hw_support
        esp_event
        esp_lcd
        neopixel
        sx127x
)