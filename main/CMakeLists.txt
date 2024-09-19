# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES bt )
set(COMPONENT_PRIV_REQUIRES esp_driver_spi esp_driver_gpio nvs_flash)

set(COMPONENT_SRCS "asm330lhh_reg.c" "asm330lhh_cda.c" "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "" )

register_component()
