cmake_minimum_required(VERSION 3.10)
project(DragonEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(dragon_engineV2.24 src/main.cpp src/chessBoard.cpp src/moveEngine.cpp src/logger.cpp src/PieceSquare.cpp src/Perft.cpp src/NNUE.cpp src/Evaluator.cpp)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
