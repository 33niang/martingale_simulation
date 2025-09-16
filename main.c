#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // 引入 OpenMP 的头文件

// --- 函数声明 ---

// 线程安全的伪随机数生成器
int my_rand_thread_safe(unsigned int *seed);
// 模拟单次马丁格尔策略游戏
int simulate_game_round_thread_safe(double initial_capital, double target_capital, double base_bet, unsigned int *seed_ptr);
// 打印进度条的函数
void print_progress(double percentage);


// --- 主函数 ---

int main() {
    // --- 模拟参数配置 ---
    double initial_capital = 10000.0; // 初始本金
    double base_bet = 1.0;          // 基础赌注
    int num_simulations = 200000000;   // 模拟总次数
    double target_capital = initial_capital * 2.0; // 目标金额

    // --- 结果统计变量 ---
    int win_count = 0;
    int loss_count = 0;

    // --- 进度条相关变量 ---
    // 使用 long long 防止多线程下数值溢出，并且声明为原子操作对象
    long long completed_count = 0;
    // 每完成总任务的 1% 就更新一次进度条，避免过于频繁的I/O操作影响性能
    // 加 1 是为了防止 num_simulations 太小时 update_interval 变成 0
    int update_interval = (num_simulations / 100) + 1;


    printf("Starting Martingale Strategy Simulation...\n");
    printf("Total simulations to run: %d\n", num_simulations);

    // 记录开始时间
    double start_time = omp_get_wtime();

    // --- 并行计算区域 ---
    // #pragma omp parallel 指令开启并行区域，所有线程都会执行这部分代码
    #pragma omp parallel
    {
        // 为每个线程创建独立的随机数种子，防止多线程冲突
        unsigned int my_seed = (unsigned int)time(NULL) ^ omp_get_thread_num();

        // #pragma omp for 指令将 for 循环的迭代任务分配给不同的线程
        // reduction(+:win_count, loss_count) 会为每个线程创建私有的 win_count 和 loss_count,
        // 循环结束后，再将所有线程的私有计数值安全地加到全局变量上。
        #pragma omp for reduction(+:win_count, loss_count)
        for (int i = 0; i < num_simulations; i++) {
            // 执行单次模拟
            if (simulate_game_round_thread_safe(initial_capital, target_capital, base_bet, &my_seed)) {
                // 这里线程操作的是自己的私有 win_count, 所以不会有冲突
                win_count++;
            } else {
                loss_count++;
            }

            // --- 进度条更新逻辑 ---
            // 使用 #pragma omp atomic 来确保 completed_count 的增加是线程安全的
            // 这可以防止多个线程同时写入造成数据竞争
            #pragma omp atomic
            completed_count++;

            // 为了防止所有线程都去打印进度条导致输出混乱，我们只让主线程(线程号为0)来负责打印
            if (omp_get_thread_num() == 0) {
                // 只有当完成的次数是更新间隔的整数倍时，才更新进度条
                if (completed_count % update_interval == 0) {
                    print_progress((double)completed_count / num_simulations);
                }
            }
        }
    }

    // 循环结束后，确保进度条显示100%
    print_progress(1.0);
    printf("\n"); // 进度条结束后换行

    // 记录结束时间
    double end_time = omp_get_wtime();

    // --- 打印最终结果 ---
    printf("\n\nSimulation Finished!\n");
    printf("------------------------------------\n");
    printf("Time elapsed: %.2f seconds\n", end_time - start_time);
    printf("------------------------------------\n");
    printf("Results:\n");
    printf("Rounds Won: %d\n", win_count);
    printf("Rounds Lost: %d\n", loss_count);
    printf("Win Probability:   %.2f%%\n", (double)win_count / num_simulations * 100.0);
    printf("Loss Probability:  %.2f%%\n", (double)loss_count / num_simulations * 100.0);
    printf("------------------------------------\n");

    return 0;
}


// --- 函数定义 ---

/**
 * @brief 打印进度条
 * @param percentage 完成百分比, 范围从 0.0 到 1.0
 */
void print_progress(double percentage) {
    int bar_width = 50; // 进度条的宽度
    printf("\r["); // \r 是回车符，让光标回到行首，实现原地更新的效果
    int pos = bar_width * percentage;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %.0f %%", percentage * 100.0);
    fflush(stdout); // 刷新标准输出缓冲区，确保进度条能立即显示出来
}

/**
 * @brief 一个简单的、线程安全的伪随机数生成器 (用于替代 rand_r)
 * @param seed 指向线程专属的随机数种子的指针
 * @return 一个伪随机整数
 */
int my_rand_thread_safe(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return (int)(*seed);
}

/**
 * @brief 模拟一次完整的马丁格尔策略游戏 (使用新的线程安全随机函数)
 * @param initial_capital 初始本金
 * @param target_capital  目标金额
 * @param base_bet        基础赌注
 * @param seed_ptr        指向该线程专属的随机数种子的指针
 * @return 1 代表成功，0 代表失败
 */
int simulate_game_round_thread_safe(double initial_capital, double target_capital, double base_bet, unsigned int *seed_ptr) {
    double current_capital = initial_capital;
    double current_bet = base_bet;

    while (current_capital > 0 && current_capital < target_capital) {
        if (current_bet > current_capital) {
            current_bet = current_capital;
        }

        if (my_rand_thread_safe(seed_ptr) % 2 == 1) { // 赢了
            current_capital += current_bet;
            current_bet = base_bet;
        } else { // 输了
            current_capital -= current_bet;
            current_bet *= 2;
        }
    }
    return current_capital >= target_capital;
}
