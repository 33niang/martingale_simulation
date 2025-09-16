#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // 引入 OpenMP 的头文件

// --- 函数声明 ---
int my_rand_thread_safe(unsigned int *seed);
int simulate_game_round_thread_safe(double initial_capital, double target_capital, double base_bet, unsigned int *seed_ptr);
void print_progress(double percentage);

// --- 主函数 ---
// --- 主函数 ---
int main() {
    // --- 模拟参数配置 ---
    double initial_capital = 10000.0; // 初始本金
    double base_bet = 1.0;          // 基础赌注

    // 【修复】对于大数模拟，必须使用 long long 类型
    // 数字后面的 LL 后缀是告诉编译器这是一个 long long 类型的常量
    long long num_simulations = 200000000LL;   // 模拟总次数 (二十亿)

    double target_capital = initial_capital * 2.0; // 目标金额

    // --- 结果统计变量 ---
    long long win_count = 0;
    long long loss_count = 0;

    // --- 进度条相关变量 ---
    long long completed_count = 0;
    // 更新间隔也使用 long long 以防万一
    long long update_interval = (num_simulations / 100) + 1;


    // 【注意】打印 long long 需要用 %lld
    printf("Starting Martingale Strategy Simulation...\n");
    printf("Total simulations to run: %lld\n", num_simulations);

    // 记录开始时间
    double start_time = omp_get_wtime();

    // --- 高质量随机数种子生成 ---
    int max_threads = omp_get_max_threads();
    unsigned int *seeds = (unsigned int*)malloc(max_threads * sizeof(unsigned int));
    srand((unsigned int)time(NULL));
    for (int i = 0; i < max_threads; i++) {
        seeds[i] = rand();
    }

    // --- 并行计算区域 ---
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        unsigned int my_seed = seeds[thread_id];

        #pragma omp for reduction(+:win_count, loss_count)
        // 【修复】将循环计数器 i 的类型也改为 long long
        for (long long i = 0; i < num_simulations; i++) {
            // 执行单次模拟
            if (simulate_game_round_thread_safe(initial_capital, target_capital, base_bet, &my_seed)) {
                win_count++;
            } else {
                loss_count++;
            }

            // --- 进度条更新逻辑 ---
            #pragma omp atomic
            completed_count++;

            if (thread_id == 0) {
                if (completed_count % update_interval == 0) {
                    print_progress((double)completed_count / num_simulations);
                }
            }
        }
    }

    free(seeds); // 释放内存

    print_progress(1.0);
    printf("\n");

    double end_time = omp_get_wtime();

    // --- 打印最终结果 ---
    printf("\n\nSimulation Finished!\n");
    printf("------------------------------------\n");
    printf("Time elapsed: %.2f seconds\n", end_time - start_time);
    printf("------------------------------------\n");
    printf("Results:\n");
    printf("Rounds Won: %lld\n", win_count);      // 【注意】打印 long long 需要用 %lld
    printf("Rounds Lost: %lld\n", loss_count);     // 【注意】打印 long long 需要用 %lld
    printf("Win Probability:   %.2f%%\n", (double)win_count / num_simulations * 100.0);
    printf("Loss Probability:  %.2f%%\n", (double)loss_count / num_simulations * 100.0);
    printf("------------------------------------\n");

    return 0;
}

// --- 函数定义 ---
void print_progress(double percentage) {
    int bar_width = 50;
    printf("\r[");
    int pos = bar_width * percentage;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %.0f %%", percentage * 100.0);
    fflush(stdout);
}

int my_rand_thread_safe(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return (int)(*seed);
}

int simulate_game_round_thread_safe(double initial_capital, double target_capital, double base_bet, unsigned int *seed_ptr) {
    double current_capital = initial_capital;
    double current_bet = base_bet;

    while (current_capital > 0 && current_capital < target_capital) {
        if (current_bet > current_capital) {
            current_bet = current_capital;
        }

        // 【最终修复】不再使用 % 2，而是比较数值大小，利用随机数的高位信息。
        // 这能提供一个更公平、更高质量的 50/50 概率判断。
        if (my_rand_thread_safe(seed_ptr) > (0x7fffffff / 2)) { // 赢了
            current_capital += current_bet;
            current_bet = base_bet;
        } else { // 输了
            current_capital -= current_bet;
            current_bet *= 2;
        }
    }
    return current_capital >= target_capital;
}
