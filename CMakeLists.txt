cmake_minimum_required(VERSION 3.15)
project(GameServer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiler options
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -Wall -Wextra -Wpedantic)
else()
    add_compile_options(-O2 -DNDEBUG)
endif()

# source files
set(SOURCES
    src/main.cpp
    src/Tetorio.cpp
    src/network/Server.cpp
    src/session/SessionManager.cpp
    src/room/RoomManager.cpp
    src/game/Board.cpp
    src/game/Bag.cpp
    src/game/Piece.cpp
)

# header files
set(HEADERS
    include/Tetorio.h
    include/network/Server.h
    include/network/ClientBuffer.h
    include/session/Session.h
    include/session/SessionManager.h
    include/room/Room.h
    include/room/RoomManager.h
    include/game/Board.h
    include/game/Bag.h
    include/game/Piece.h
)

# executable file
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# header directory
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# platform libraries
if(UNIX AND NOT APPLE)
    # linux
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
elseif(APPLE)
    # macOS: include POSIX socket
endif()

# installation settings
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
