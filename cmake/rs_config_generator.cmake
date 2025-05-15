set(RS_CONFIG_GENERATOR_FILE "${CMAKE_CURRENT_LIST_DIR}/rs_config_generator.h.in")

function(rs_config_generator CONFIG_FLAGS)
    foreach(def IN LISTS ${CONFIG_FLAGS})
        string(FIND ${def} "=" pos)

        # Format : __platform_xxxx__
        if (pos EQUAL -1)
            list(APPEND RESULT "/* ${def} */")
            list(APPEND RESULT "#ifndef ${def}")
            list(APPEND RESULT "#define ${def}")
            list(APPEND RESULT "#endif")
            list(APPEND RESULT "") # newline
            add_compile_definitions(${def})
        # Format : __platform_xxxx__=yyyy
        else()
            string(SUBSTRING ${def} 0 ${pos} def_name)
            math(EXPR vpos "${pos} + 1")
            string(SUBSTRING ${def} ${vpos} -1 def_val)

            # Make system.h
            list(APPEND RESULT "/* ${def_name} */")
            list(APPEND RESULT "#ifndef ${def_name}")
            list(APPEND RESULT "#define ${def_name}\t\t(${def_val})")
            list(APPEND RESULT "#endif")
            list(APPEND RESULT "") # newline
            add_compile_definitions(${def_name}=${def_val})
        endif()
    endforeach()

    # If result is not empty
    if (RESULT)
        list(JOIN RESULT "\n" RS_CONFIG_FLAGS_OUTPUT)

        configure_file(
            ${RS_CONFIG_GENERATOR_FILE}
            ${CMAKE_INSTALL_PREFIX}/${RSDK_INSTALL_INCLUDEDIR}/${PROJECT_NAME}_config.h
        )
    endif()
endfunction()
