cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(chess_engine src/main.cpp src/chessBoard.cpp src/moveEngine.cpp)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
