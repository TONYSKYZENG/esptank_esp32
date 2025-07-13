set(srcs "ble_spp_server_demo.c" "motor.c" "sound.c")

if(CONFIG_EXAMPLE_ENABLE_RF_TESTING_CONFIGURATION_COMMAND)
list(APPEND srcs
     "rf_tesing_configuration_cmd.c")
endif()
set(COMPONENT_EMBED_TXTFILES idel.mp3 mg.mp3 music.mp3 cannon.mp3)
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    EMBED_FILES "engine.mp3" "mg.mp3" "idel.mp3" "cannon.mp3" "music.mp3")
