#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>    // 引入 OpenMP 的头文件

/*
 * =================================================================================
 * 程序内置两种“运行模式”和三种“下注模式”
 * =================================================================================
 *
 * I. 运行模式 (Run Modes): 决定程序做什么。
 * - 模式 1: 基准测试 (Benchmark) -> 并行运行海量模拟，输出最终统计概率。
 * - 模式 2: 详细追踪 (Detailed Trace) -> 只运行一次，但打印每一步的详细过程。
 *
 * II. 下注模式 (Betting Modes): 决定资金不足时如何行动。
 * - 模式 0: 宽松/All-In 模式 -> 用剩余全部资金下注。
 * - 模式 1: 严格模式 -> 直接判定本轮游戏失败。
 * - 模式 2: Reset-to-1 模式 -> 下注额重置为基础赌注。
 *
 * 程序启动后会通过菜单让您依次选择这两种模式。
 * =================================================================================
 */

// --- 函数声明 ---
int my_rand_thread_safe(unsigned int *seed);
int simulate_game_round_thread_safe(double initial_capital, double target_capital, double base_bet, int betting_mode, unsigned int *seed_ptr);
void simulate_detailed_run(double initial_capital, double target_capital, double base_bet, int betting_mode);
void print_progress(double percentage);

// --- 主函数 ---
int main() {
    int run_mode = 0;
    int betting_mode = 0;

    // --- 用户交互菜单 (英文界面) ---
    // 菜单部分，用于让用户选择运行模式
    printf("Please select a Run Mode:\n");
    printf("  1: Benchmark Mode (High-performance parallel simulation)\n");
    printf("  2: Detailed Trace Mode (Step-by-step for a single game)\n");
    printf("Enter your choice (1 or 2): ");
    if (scanf("%d", &run_mode) != 1 || (run_mode != 1 && run_mode != 2)) {
        fprintf(stderr, "\nInvalid input. Please run the program again and enter 1 or 2.\n");
        return 1;
    }

    // 菜单部分，用于让用户选择下注模式
    printf("\nPlease select a Betting Mode:\n");
    printf("  0: Loose/All-in Mode (Bet all remaining capital)\n");
    printf("  1: Strict Mode (Fail immediately if capital is insufficient)\n");
    printf("  2: Reset-to-1 Mode (Reset bet to base value)\n");
    printf("Enter your choice (0, 1, or 2): ");
    if (scanf("%d", &betting_mode) != 1 || (betting_mode < 0 || betting_mode > 2)) {
        fprintf(stderr, "\nInvalid input. Please run the program again and enter 0, 1, or 2.\n");
        return 1;
    }

    // --- 模拟参数配置 ---
    double initial_capital = 10000.0; // 初始本金
    double base_bet = 1.0;          // 基础赌注
    double target_capital = initial_capital * 2.0; // 目标金额

    // ==================== 根据运行模式执行对应代码 ====================
    if (run_mode == 1) { // 运行模式一：大规模并行基准测试
        long long num_simulations = 2000000LL; // 模拟总次数
        long long win_count = 0, loss_count = 0, completed_count = 0;
        long long update_interval = (num_simulations / 100) + 1;

        int max_threads = omp_get_max_threads();
        unsigned int *seeds = (unsigned int*)malloc(max_threads * sizeof(unsigned int));
        srand((unsigned int)time(NULL));
        for (int i = 0; i < max_threads; i++) seeds[i] = rand();

        // 运行信息输出 (英文)
        printf("\nRun Mode 1: Benchmark | Betting Mode: %d\n", betting_mode);
        printf("Total simulations to run: %lld\n", num_simulations);
        double start_time = omp_get_wtime();

        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            unsigned int my_seed = seeds[thread_id];
            #pragma omp for reduction(+:win_count, loss_count)
            for (long long i = 0; i < num_simulations; i++) {
                if (simulate_game_round_thread_safe(initial_capital, target_capital, base_bet, betting_mode, &my_seed)) {
                    win_count++;
                } else {
                    loss_count++;
                }
                #pragma omp atomic
                completed_count++;
                if (thread_id == 0 && (completed_count % update_interval == 0)) {
                    print_progress((double)completed_count / num_simulations);
                }
            }
        }
        free(seeds);
        print_progress(1.0);
        printf("\n");
        double end_time = omp_get_wtime();

        // 结果输出 (英文)
        printf("\n\nSimulation Finished!\n");
        printf("------------------------------------\n");
        printf("Time elapsed: %.2f seconds\n", end_time - start_time);
        printf("------------------------------------\n");
        printf("Results:\n");
        printf("Rounds Won: %lld\n", win_count);
        printf("Rounds Lost: %lld\n", loss_count);
        printf("Win Probability:   %.2f%%\n", (double)win_count / num_simulations * 100.0);
        printf("Loss Probability:  %.2f%%\n", (double)loss_count / num_simulations * 100.0);
        printf("------------------------------------\n");
    }
    else if (run_mode == 2) { // 运行模式二：单次详细追踪
        // 运行信息输出 (英文)
        printf("\nRun Mode 2: Detailed Trace | Betting Mode: %d\n", betting_mode);
        printf("------------------------------------\n");
        simulate_detailed_run(initial_capital, target_capital, base_bet, betting_mode);
        printf("------------------------------------\n");
    }

    return 0; // 程序正常结束
}

// --- 函数定义 ---

/**
 * @brief 模式二专用：带详细打印的单次模拟
 */
void simulate_detailed_run(double initial_capital, double target_capital, double base_bet, int betting_mode) {
    double current_capital = initial_capital;
    double current_bet = base_bet;
    int round = 1;
    unsigned int seed = (unsigned int)time(NULL);

    // 详细追踪的表头 (英文)
    printf("Initial Capital: %.2f, Target: %.2f, Base Bet: %.2f\n\n", initial_capital, target_capital, base_bet);

    while (current_capital > 0 && current_capital < target_capital) {
        printf("Round %-4d | Capital: %-10.2f | ", round, current_capital);

        if (current_bet > current_capital) {
            switch(betting_mode) {
                case 1: // 严格模式
                    printf("Insufficient capital for bet of %.2f. Declaring failure.\n", current_bet);
                    current_capital = 0;
                    continue;
                case 2: // Reset-to-1 模式
                    printf("Insufficient capital. Bet reset to %.2f | ", base_bet);
                    current_bet = base_bet;
                    break;
                case 0: // 宽松/All-in 模式
                default:
                    printf("Insufficient capital. Going all-in with remaining %.2f | ", current_capital);
                    current_bet = current_capital;
                    break;
            }
        }

        printf("Betting: %-10.2f | ", current_bet);

        if (my_rand_thread_safe(&seed) > (0x7fffffff / 2)) {
            current_capital += current_bet;
            current_bet = base_bet;
            printf("Result: WIN   | New Capital: %.2f\n", current_capital);
        } else {
            current_capital -= current_bet;
            current_bet *= 2;
            printf("Result: LOSE  | New Capital: %.2f\n", current_capital);
        }
        round++;
    }

    // 详细追踪的最终结果 (英文)
    printf("\nGame Over.\n");
    if (current_capital >= target_capital) {
        printf("Final Result: SUCCESS! Reached target capital.\n");
    } else {
        printf("Final Result: BANKRUPT! Capital is zero.\n");
    }
}

/**
 * @brief 模拟一次完整的马丁格尔策略游戏（模式一专用，无打印）
 * @param betting_mode 下注模式 (0: All-in, 1: 严格, 2: Reset-to-1)
 */
int simulate_game_round_thread_safe(double initial_capital, double target_capital, double base_bet, int betting_mode, unsigned int *seed_ptr) {
    double current_capital = initial_capital;
    double current_bet = base_bet;

    while (current_capital > 0 && current_capital < target_capital) {
        if (current_bet > current_capital) {
            switch(betting_mode) {
                case 1: return 0; // 严格模式: 失败
                case 2: current_bet = base_bet; break; // Reset-to-1 模式
                case 0: default: current_bet = current_capital; break; // 宽松/All-in 模式
            }
        }

        if (my_rand_thread_safe(seed_ptr) > (0x7fffffff / 2)) {
            current_capital += current_bet;
            current_bet = base_bet;
        } else {
            current_capital -= current_bet;
            current_bet *= 2;
        }
    }
    return current_capital >= target_capital;
}

// ... (print_progress 和 my_rand_thread_safe 函数保持不变，进度条输出已改为英文) ...
void print_progress(double percentage) {
    int bar_width = 50;
    printf("\r[");
    int pos = bar_width * percentage;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("="); else if (i == pos) printf(">"); else printf(" ");
    }
    printf("] %.0f %% Complete", percentage * 100.0);
    fflush(stdout);
}

int my_rand_thread_safe(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return (int)(*seed);
}
