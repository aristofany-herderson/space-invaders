cmake_minimum_required(VERSION 3.21)
project(SpaceInvaders LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SFML 3 COMPONENTS Graphics Audio Window System QUIET)

if(NOT SFML_FOUND)
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.1.0
        GIT_SHALLOW ON
        SYSTEM)
    FetchContent_MakeAvailable(SFML)
endif()

set(GAME_SOURCES
    SpaceInvaders/Background.cpp
    SpaceInvaders/Bullet.cpp
    SpaceInvaders/Enemy.cpp
    SpaceInvaders/EnemyFactory.cpp
    SpaceInvaders/EnemyManager.cpp
    SpaceInvaders/Game.cpp
    SpaceInvaders/Particle.cpp
    SpaceInvaders/Player.cpp
    SpaceInvaders/RoundedRectShape.cpp
    SpaceInvaders/Shield.cpp
    SpaceInvaders/SoundManager.cpp
    SpaceInvaders/Spritegen.cpp
    SpaceInvaders/Storage.cpp)

add_executable(SpaceInvaders SpaceInvaders/main.cpp ${GAME_SOURCES})
target_include_directories(SpaceInvaders PRIVATE SpaceInvaders)
target_link_libraries(SpaceInvaders PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)

if(MSVC)
    target_compile_options(SpaceInvaders PRIVATE /W3 /utf-8)
else()
    target_compile_options(SpaceInvaders PRIVATE -Wall -Wextra)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/SpaceInvaders/assets)
    add_custom_command(TARGET SpaceInvaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/SpaceInvaders/assets
            $<TARGET_FILE_DIR:SpaceInvaders>/assets
        COMMENT "Copiando assets")
endif()
