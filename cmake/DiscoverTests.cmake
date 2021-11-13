set(DISCOVER_TESTS_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")

function(target_discover_tests target)
    cmake_parse_arguments(DISCOVER_TESTS
            # options
            ""
            # oneValueArgs
            "TEST_PREFIX;TEST_SOURCE_DIR"
            # multiValueArgs
            ""
            ${ARGN}
            )

    if (NOT DISCOVER_TESTS_TEST_PREFIX)
        set(DISCOVER_TESTS_TEST_PREFIX "${target}::")
    endif ()
    if (NOT DISCOVER_TESTS_TEST_SOURCE_DIR)
        set(DISCOVER_TESTS_TEST_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif ()
    if (NOT IS_ABSOLUTE ${DISCOVER_TESTS_TEST_SOURCE_DIR})
        set(DISCOVER_TESTS_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${DISCOVER_TESTS_TEST_SOURCE_DIR}")
    endif ()

    set(DISCOVER_TESTS_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    get_property(DISCOVER_TESTS_CROSSCOMPILING_EMULATOR
            TARGET ${target}
            PROPERTY CROSSCOMPILING_EMULATOR
            )
    set(DISCOVER_TESTS_DISCOVERY_TIMEOUT 5)
    set(DISCOVER_TESTS_TEST_LIST ${target}_TESTS)

    set(counter 0)
    get_property(has_counter
            TARGET ${target}
            PROPERTY CTEST_DISCOVERED_TEST_COUNTER
            SET
            )
    if (has_counter)
        get_property(counter
                TARGET ${target}
                PROPERTY CTEST_DISCOVERED_TEST_COUNTER
                )
    endif ()
    math(EXPR counter "${counter} + 1")
    set_property(
            TARGET ${target}
            PROPERTY CTEST_DISCOVERED_TEST_COUNTER
            ${counter}
    )

    set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${target}-${counter}")
    get_property(GENERATOR_IS_MULTI_CONFIG
            GLOBAL
            PROPERTY GENERATOR_IS_MULTI_CONFIG
            )
    if (NOT GENERATOR_IS_MULTI_CONFIG)
        set(ctest_tests_file "${ctest_file_base}.tests.cmake")
    else ()
        set(ctest_tests_file "${ctest_file_base}.tests.$<CONFIG>.cmake")
    endif ()
    string(CONCAT ctest_include_content
            "if (NOT EXISTS \"$<TARGET_FILE:${target}>\")" "\n"
            "    add_test(${target}_NOT_BUILT ${target}_NOT_BUILT)" "\n"
            "    return()" "\n"
            "endif ()" "\n"
            "if (NOT EXISTS \"${ctest_tests_file}\"" "\n"
            "        OR NOT \"${ctest_tests_file}\" IS_NEWER_THAN \"$<TARGET_FILE:${target}>\"" "\n"
            "        OR NOT \"${ctest_tests_file}\" IS_NEWER_THAN \"${DISCOVER_TESTS_SCRIPT}\")" "\n"
            "    include(\"${DISCOVER_TESTS_SCRIPT}\")" "\n"
            "    discover_tests(" "\n"
            "            TEST_TARGET" " [[" "${target}" "]]" "\n"
            "            TEST_PREFIX" " [[" "${DISCOVER_TESTS_TEST_PREFIX}" "]]" "\n"
            "            CTEST_FILE" " [[" "${ctest_tests_file}" "]]" "\n"
            "            TEST_SOURCE_DIR" " [[" "${DISCOVER_TESTS_TEST_SOURCE_DIR}" "]]" "\n"
            "            TEST_WORKING_DIR" " [[" "${DISCOVER_TESTS_WORKING_DIRECTORY}" "]]" "\n"
            "            TEST_EXECUTOR" " [[" "${CMAKE_CURRENT_LIST_DIR}/cmake/testrunner.sh" "]]" "\n"
            "            TEST_EMULATOR" " [[" "${DISCOVER_TESTS_CROSSCOMPILING_EMULATOR}" "]]" "\n"
            "            TEST_EXECUTABLE" " [[" "$<TARGET_FILE:${target}>" "]]" "\n"
            "            TEST_DISCOVERY_TIMEOUT" " [[" "${DISCOVER_TESTS_DISCOVERY_TIMEOUT}" "]]" "\n"
            "            TEST_LIST" " [[" "${DISCOVER_TESTS_TEST_LIST}" "]]" "\n"
            "    )" "\n"
            "endif ()" "\n"
            "include(\"${ctest_tests_file}\")" "\n"
            )
    if (NOT GENERATOR_IS_MULTI_CONFIG)
        file(GENERATE OUTPUT "${ctest_file_base}.include.cmake" CONTENT "${ctest_include_content}")
    else ()
        file(WRITE "${ctest_file_base}.include.cmake" "include(\"${ctest_file_base}.include.\${CTEST_CONFIGURATION_TYPE}.cmake\")")
        foreach (_config ${CMAKE_CONFIGURATION_TYPES})
            file(GENERATE OUTPUT "${ctest_file_base}.include.${_config}.cmake" CONTENT "${ctest_include_content}" CONDITION $<CONFIG:${_config}>)
        endforeach ()
    endif ()
    set_property(
            DIRECTORY
            APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_file_base}.include.cmake"
    )
endfunction()

function(discover_tests)
    cmake_parse_arguments(DISCOVER_TESTS
            # options
            ""
            # oneValueArgs
            "TEST_TARGET;TEST_PREFIX;CTEST_FILE;TEST_SOURCE_DIR;TEST_WORKING_DIR;TEST_EXECUTOR;TEST_EMULATOR;TEST_EXECUTABLE;TEST_DISCOVERY_TIMEOUT;TEST_LIST"
            # multiValueArgs
            ""
            ${ARGN}
            )

    if (NOT EXISTS "${DISCOVER_TESTS_TEST_EXECUTABLE}")
        message(FATAL_ERROR
                "Error running test executable" "\n"
                "  Path: '${DISCOVER_TESTS_TEST_EXECUTABLE}'" "\n"
                )
    endif ()

    execute_process(
            COMMAND ${DISCOVER_TESTS_TEST_EMULATOR} "${DISCOVER_TESTS_TEST_EXECUTABLE}"
            WORKING_DIRECTORY "${DISCOVER_TESTS_TEST_WORKING_DIR}"
            TIMEOUT ${DISCOVER_TESTS_TEST_DISCOVERY_TIMEOUT}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE output
    )

    if (NOT ${result} EQUAL 0)
        string(REPLACE "\n" "\n    " output "${output}")
        message(FATAL_ERROR
                "Error running test executable" "\n"
                "  Path: '${DISCOVER_TESTS_TEST_EXECUTABLE}'" "\n"
                "  Result: ${result}" "\n"
                "  Output:" "\n"
                "    ${output}" "\n"
                )
    endif ()

    set(script_file "${DISCOVER_TESTS_CTEST_FILE}")
    set(script_write_mode WRITE)
    set(script_buffer)
    function(script_append NAME)
        set(_args "")
        foreach (_arg ${ARGN})
            if (_arg MATCHES "[^-./:a-zA-Z0-9_]")
                string(APPEND _args " [==[${_arg}]==]")
            else ()
                string(APPEND _args " ${_arg}")
            endif ()
        endforeach ()
        string(SUBSTRING "${_args}" 1 -1 _args)
        string(APPEND script_buffer "${NAME}(${_args})\n")
        string(LENGTH "${script_buffer}" script_buffer_length)
        if (${script_buffer_length} GREATER "4096")
            script_flush()
        endif ()
        set(script_write_mode "${script_write_mode}" PARENT_SCOPE)
        set(script_buffer "${script_buffer}" PARENT_SCOPE)
    endfunction()
    function(script_flush)
        file(${script_write_mode} "${script_file}" "${script_buffer}")
        set(script_write_mode APPEND PARENT_SCOPE)
        set(script_buffer "" PARENT_SCOPE)
    endfunction()

    set(tests)
    set(tests_buffer)
    macro(tests_append testname)
        list(APPEND tests_buffer "${testname}")
        list(LENGTH tests_buffer tests_buffer_length)
        if (${tests_buffer_length} GREATER "250")
            tests_flush()
        endif ()
        set(tests "${tests}" PARENT_SCOPE)
        set(tests_buffer "${tests_buffer}" PARENT_SCOPE)
    endmacro()
    macro(tests_flush)
        list(APPEND tests "${tests_buffer}")
        set(tests "${tests}" PARENT_SCOPE)
        set(tests_buffer "" PARENT_SCOPE)
    endmacro()

    string(REPLACE [[;]] [[\\;]] output "${output}")
    string(REPLACE "\n" [[;]] output "${output}")
    list(SORT output)
    foreach (tuple ${output})
        list(GET tuple 0 name)
        list(GET tuple 1 mode)
        list(GET tuple 2 file)
        list(GET tuple 3 id)
        string(REGEX REPLACE " +" "" testid "${name}")
        string(REPLACE [[\]] [[\\]] testid "${testid}")
        string(REPLACE [[;]] [[\;]] testid "${testid}")
        string(REPLACE [[$]] [[\$]] testid "${testid}")

        set(testname "${DISCOVER_TESTS_TEST_PREFIX}${testid} (${mode}, ${id})")

        if (mode MATCHES "^C$")
            set(target "${DISCOVER_TESTS_TEST_TARGET}")
            file(WRITE "${DISCOVER_TESTS_TEST_WORKING_DIR}/compiletests/${testid}/CMakeLists.txt" "cmake_minimum_required(VERSION 3.19)
project(CompileTests)
add_subdirectory(../../.. build)
target_compile_definitions(${target} PRIVATE __TEST_COMPILE __TEST_${id}=1)
")

            set(dir compiletests/${testid})
            script_append(add_test "${testname}"
                    ctest --build-and-test ${dir} ${dir}
                    --build-generator "Unix Makefiles"
                    --build-target "${target}"
                    --test-command true
                    )
            script_append(set_tests_properties "${testname}" PROPERTIES WILL_FAIL TRUE)

            tests_append("${testname}")
        elseif (mode MATCHES "^R$")
            script_append(add_test "${testname}"
                    ${DISCOVER_TESTS_TEST_EXECUTOR} "${DISCOVER_TESTS_TEST_SOURCE_DIR}" "${testid}"
                    --
                    ${DISCOVER_TESTS_TEST_EMULATOR} "${DISCOVER_TESTS_TEST_EXECUTABLE}" "${testid}"
                    )
            script_append(set_tests_properties "${testname}" PROPERTIES TIMEOUT 10)
            if (testid MATCHES "^DISABLED_")
                script_append(set_tests_properties "${testname}" PROPERTIES DISABLED TRUE)
            endif ()
            script_append(set_tests_properties "${testname}" PROPERTIES WORKING_DIRECTORY "${DISCOVER_TESTS_TEST_WORKING_DIR}")
            script_append(set_tests_properties "${testname}" PROPERTIES SKIP_REGULAR_EXPRESSION "\\\\[  SKIPPED \\\\]")

            tests_append("${testname}")
        endif ()
    endforeach ()

    tests_flush()
    script_append(set ${DISCOVER_TESTS_TEST_LIST} ${tests})
    script_flush()
endfunction()
