idf_component_register(SRCS "adc.c" "bt_server.c" "motor.c" "sound.c" "adc.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "engine.mp3" "mg.mp3" "idel.mp3" "cannon.mp3" "music.mp3")
#spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
