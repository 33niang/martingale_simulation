# 设置 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.21)

# 定义项目名称和所用语言 (C)
project(martingale_simulation C)

# 设置 C 语言的标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加可执行文件，源文件是 main.c
add_executable(martingale_simulation main.c
        main.c)

# 查找 OpenMP 包
find_package(OpenMP)

# 如果找到了 OpenMP，就将其链接到你的项目中
if(OpenMP_FOUND)
    target_link_libraries(martingale_simulation PUBLIC OpenMP::OpenMP_C)
endif()

# 保险措施：在部分 MinGW 工具链下，显式添加 -fopenmp 到编译与链接选项
# 若 OpenMP::OpenMP_C 已正确传播，这里不会造成冲突
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(martingale_simulation PRIVATE -O2 -fopenmp)
    target_link_options(martingale_simulation PRIVATE -fopenmp)
endif()

# Windows/MinGW 下避免运行“单文件编译”，请通过 CMake 目标运行该可执行文件
