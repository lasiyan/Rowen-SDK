# JetPack version table
set(JETPACK_VERSION_TABLE 
    # L4T Version       JetPack Version      Version Number
    "36.3"              "6.0"               "600"
    "36.2"              "6.0_dev"           "600"
    "35.5.0"            "5.1.3"             "513"
    "35.4.1"            "5.1.2"             "512"
    "35.3.1"            "5.1.1"             "511"
    "35.2.1"            "5.1"               "510"
    "35.1"              "5.0.2"             "502"
    "34.1.1"            "5.0.1_dev"         "501"
    "34.1"              "5.0_dev"           "500"
    "32.7.4"            "4.6.4"             "464"
    "32.7.3"            "4.6.3"             "463"
    "32.7.2"            "4.6.2"             "462"
    "32.7.1"            "4.6.1"             "461"
    "32.6.1"            "4.6"               "460"
    "32.5.1"            "4.5.1"             "451"
    "32.5"              "4.5"               "450"
    "32.4.4"            "4.4.1"             "441"
    "32.4.3"            "4.4"               "440"
    "32.4.2"            "4.4_dev"           "440"
    "32.3.1"            "4.3"               "430"
    "32.2.1"            "4.2.3"             "423"
    "32.2.1"            "4.2.2"             "422"
    "32.2"              "4.2.1"             "421"
    "32.1"              "4.2"               "420"
    "31.1"              "4.1.1_dev"         "411"
    "28.5"              "3.3.4"             "334"
    "28.4"              "3.3.3"             "333"
    "28.3.2"            "3.3.2"             "332"
    "28.3.1"            "3.3.1"             "331"
    "28.2"              "3.3"               "330"
    "28.2"              "3.2.1"             "321"
)

# JetPack version finder
function(jetpack_version_configure VERSION_PATH OUTPUT_LIST)
    if (NOT EXISTS ${VERSION_PATH})
        return()
    endif()
    
    # if not return, read the version file
    file(READ ${VERSION_PATH} jet_version_string_full)
    string(REGEX REPLACE "\n" "" jet_version_string_full ${jet_version_string_full})
    message(DEBUG "Jetson Revision : ${jet_version_string_full}")

    # Parse Jetpack major & minor version
    set(jet_l4t_major "")
    set(jet_l4t_minor "")
    string(REGEX REPLACE ".*R([0-9]+).*" "\\1" jet_l4t_major ${jet_version_string_full})
    string(REGEX REPLACE ".*REVISION: ([0-9]+(\\.[0-9]+)?).*" "\\1" jet_l4t_minor ${jet_version_string_full})

    # Find match version
    set(jet_l4t_version ${jet_l4t_major}.${jet_l4t_minor})
    set(jetpack_version "")
    set(jetpack_number  "")

    list(LENGTH JETPACK_VERSION_TABLE length)
    math(EXPR elem_count "${length} / 3")

    foreach(i RANGE 0 ${elem_count})
        math(EXPR l4t_version_idx "${i} * 3")
        math(EXPR jet_version_idx "${l4t_version_idx} + 1")
        math(EXPR jet_number_idx  "${l4t_version_idx} + 2")

        # if l4t_version_idx + 2 GREATER length, break
        if (jet_number_idx GREATER_EQUAL length)
            break()
        endif()

        list(GET JETPACK_VERSION_TABLE ${l4t_version_idx} key)
        list(GET JETPACK_VERSION_TABLE ${jet_version_idx} jetpack_version)
        list(GET JETPACK_VERSION_TABLE ${jet_number_idx}  jetpack_number)

        # Get values
        if (${jet_l4t_version} STREQUAL ${key})
            set(jetpack_version ${jetpack_version})
            set(jetpack_number  ${jetpack_number})
            break()
        endif()
    endforeach()

    # Result check
    if (jetpack_version AND jetpack_number)
        message(STATUS "JetPack Version : ${jetpack_version} (${jetpack_number}, ${jet_l4t_version})")

        # Set Define : Jetson
        set(jetpack_defines "${${OUTPUT_LIST}}") # backup previous defines
        list(APPEND jetpack_defines __platform_jetson__)

        # Set Define : JetPack Version
        string(REGEX MATCH "^[0-9]+" JETPACK_MAJOR_VERSION ${jetpack_version})
        if (JETPACK_MAJOR_VERSION EQUAL 2)
            list(APPEND jetpack_defines __platform_jetpack2__)
        elseif (JETPACK_MAJOR_VERSION EQUAL 3)
            list(APPEND jetpack_defines __platform_jetpack3__)
        elseif (JETPACK_MAJOR_VERSION EQUAL 4)
            list(APPEND jetpack_defines __platform_jetpack4__)
        elseif (JETPACK_MAJOR_VERSION EQUAL 5)
            list(APPEND jetpack_defines __platform_jetpack5__)
        elseif (JETPACK_MAJOR_VERSION EQUAL 6)
            list(APPEND jetpack_defines __platform_jetpack6__)
        else()
            message(STATUS "Unknown JetPack version")
        endif()

        # Set Define : JetPack Number
        list(APPEND jetpack_defines __platform_jetpack_version__=${jetpack_number})
        list(APPEND jetpack_defines __platform_jetpack_version_string__="${jetpack_version}")

        # Return
        set(${OUTPUT_LIST} "${jetpack_defines}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "[Fatal Error] JetPack version not found : ${jet_l4t_major}.${jet_l4t_minor}")
    endif()
endfunction()
