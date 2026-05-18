function(OctalEngine_AddGame NAME)
    if("${NAME}" STREQUAL "")
        message(FATAL_ERROR "OctalEngine_AddGame requires a target name.")
    endif()

    if(ARGC LESS 2)
        message(FATAL_ERROR "OctalEngine_AddGame(${NAME}) requires at least one source file.")
    endif()

    add_executable(${NAME} ${ARGN})

    target_compile_features(${NAME} PRIVATE cxx_std_20)

    if(TARGET OctalEngine::OctalEnginePlatform)
        target_link_libraries(${NAME} PRIVATE OctalEngine::OctalEnginePlatform)
    elseif(TARGET OctalEnginePlatform)
        target_link_libraries(${NAME} PRIVATE OctalEnginePlatform)
    elseif(TARGET OctalEngine::OctalEngine)
        target_link_libraries(${NAME} PRIVATE OctalEngine::OctalEngine)
    elseif(TARGET OctalEngine)
        target_link_libraries(${NAME} PRIVATE OctalEngine)
    else()
        message(FATAL_ERROR "OctalEngine targets were not found. Call find_package(OctalEngine REQUIRED) first.")
    endif()

    if(TARGET OctalEngine::OctalEngineRenderer)
        target_link_libraries(${NAME} PRIVATE OctalEngine::OctalEngineRenderer)
    elseif(TARGET OctalEngineRenderer)
        target_link_libraries(${NAME} PRIVATE OctalEngineRenderer)
    endif()

    if(TARGET OctalEngine::OctalEngineJobs)
        target_link_libraries(${NAME} PRIVATE OctalEngine::OctalEngineJobs)
    elseif(TARGET OctalEngineJobs)
        target_link_libraries(${NAME} PRIVATE OctalEngineJobs)
    endif()

    if(WIN32 AND DEFINED OctalEngine_RUNTIME_DLL AND EXISTS "${OctalEngine_RUNTIME_DLL}")
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OctalEngine_RUNTIME_DLL}"
                "$<TARGET_FILE_DIR:${NAME}>"
        )
    endif()
endfunction()
