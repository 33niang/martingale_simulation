# C语言 & OpenMP 并行模拟马丁格-尔策略 (Martingale Strategy Parallel Simulator)

这是一个使用C语言和OpenMP构建的高性能并行计算程序，用于模拟经典的马丁格尔（Martingale）赌博策略。通过大规模的蒙特卡洛模拟，本项目旨在探究在拥有有限资本的情况下，马丁格尔策略的长期胜率和破产风险。

**[查看模拟结果 (View Simulation Results)](RESULTS.md)**

## ✨ 主要功能

* **高性能**：基于C语言实现，并针对计算密集型任务进行了优化。
* **并行计算**：利用 OpenMP 实现了多核并行处理，能够充分利用现代CPU的多核性能，极大缩短大规模模拟的计算时间。
* **线程安全**：内置了线程安全的伪随机数生成器，确保在并行环境下的数据准确性。
* **动态进度条**：在控制台实时显示模拟进度，提供了良好的用户交互体验。

## ⚠️ 重要限制：模拟次数上限

由于本项目中的核心计数变量（如 `num_simulations`）使用的是 `int` 数据类型，其数值大小会受到32位有符号整数的限制。

这意味着**最大模拟次数不能超过 `2,147,483,647`** (约二十一亿)。

如果需要进行更大规模（超过二十一亿次）的模拟，您需要手动将 `main.c` 文件中的相关变量类型从 `int` 修改为 `long long`。

## 🚀 如何编译和运行

### 方式一：使用 IDE (推荐 CLion)

1.  克隆本仓库到本地。
2.  使用 CLion 打开项目文件夹。
3.  CLion 会自动加载 `CMakeLists.txt` 并配置好项目。
4.  为了获得最佳性能，请将右上角的构建配置从 `Debug` 切换到 `Release`。
5.  点击绿色的 ▶️ “运行”按钮即可。

### 方式二：使用命令行 (GCC/Clang)

1.  克隆本仓库并进入项目目录。
2.  使用以下命令进行编译（开启最高优化 `-O3` 并启用 OpenMP `-fopenmp`）：
    ```bash
    gcc -O3 -fopenmp main.c -o martingale_simulator
    ```
3.  运行生成的可执行文件：
    ```bash
    # Windows
    .\martingale_simulator.exe
    # Linux / macOS
    ./martingale_simulator
    ```

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE) 授权。
