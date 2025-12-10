#include "timsort.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ========== Функции вывода ========== */

void putstr(const char *s)
{
    printf("%s", s);
}

void print_stats(size_t n, size_t threads, size_t runs, long long elapsed_ms)
{
    printf("n=%zu threads=%zu runs_after_merge=%zu elapsed_ms=%lld\n",
           n, threads, runs, elapsed_ms);
}

/* ========== Проверка сортировки ========== */

int verify_sorted(int32_t *array, size_t size)
{
    for (size_t k = 1; k < size; ++k)
    {
        if (array[k - 1] > array[k])
            return 0;
    }
    return 1;
}

/* ========== Генерация случайного массива ========== */

void generate_random_array(int32_t *array, size_t size, unsigned int seed)
{
    srand(seed);
    for (size_t i = 0; i < size; ++i)
        array[i] = (int32_t)((rand() << 1) ^ rand());
}
