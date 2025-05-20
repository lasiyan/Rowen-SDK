if (NOT OPTION_INSTALL_FOR_DEVELOPER)
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/post_install/install.sh.in
        ${CMAKE_INSTALL_PREFIX}/install.sh
        IMMEDIATE @ONLY
    )
    execute_process(COMMAND chmod 777 "${CMAKE_INSTALL_PREFIX}/install.sh")

    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/post_install/uninstall.sh.in
        ${CMAKE_INSTALL_PREFIX}/uninstall.sh
        IMMEDIATE @ONLY
    )
    execute_process(COMMAND chmod 777 "${CMAKE_INSTALL_PREFIX}/uninstall.sh")
endif()
