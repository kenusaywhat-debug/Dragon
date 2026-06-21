cmake_minimum_required(VERSION 3.10)
project(DragonEngine)

#set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(dragon_engineV2.15 src/main.cpp src/chessBoard.cpp src/moveEngine.cpp src/logger.cpp src/PieceSquare.cpp src/Perft.cpp)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
