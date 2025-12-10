#include "timsort.h"
#include <stdlib.h>
#include <string.h>

/* ========== Алгоритмы сортировки ========== */

void insertion_sort(int32_t *arr, size_t left, size_t right)
{
    for (size_t i = left + 1; i < right; ++i)
    {
        int32_t key = arr[i];
        size_t j = i;
        while (j > left && arr[j - 1] > key)
        {
            arr[j] = arr[j - 1];
            --j;
        }
        arr[j] = key;
    }
}

int cmp_int(const void *a, const void *b)
{
    int32_t va = *(const int32_t *)a;
    int32_t vb = *(const int32_t *)b;
    return (va > vb) - (va < vb);
}

/* ========== Слияние отсортированных диапазонов ========== */

void merge_ranges(int32_t *arr, size_t l, size_t m, size_t r, int32_t *tmp)
{
    size_t i = l, j = m, k = 0;

    while (i < m && j < r)
    {
        if (arr[i] <= arr[j])
            tmp[k++] = arr[i++];
        else
            tmp[k++] = arr[j++];
    }

    while (i < m)
        tmp[k++] = arr[i++];
    while (j < r)
        tmp[k++] = arr[j++];

    memcpy(arr + l, tmp, (r - l) * sizeof(int32_t));
}

/* ========== Сортировка одного прогона ========== */

void sort_single_run(sort_context_t *ctx, size_t run_index)
{
    size_t s = ctx->runs.data[run_index].start;
    size_t len = ctx->runs.data[run_index].len;

    if (len <= 32)
    {
        insertion_sort(ctx->array, s, s + len);
    }
    else
    {
        qsort(ctx->array + s, len, sizeof(int32_t), cmp_int);
    }
}

/* ========== Слияние всех прогонов ========== */

void merge_all_runs(sort_context_t *ctx)
{
    size_t current_runs = ctx->runs.size;

    while (current_runs > 1)
    {
        size_t write_idx = 0;

        // Слияние смежных пар прогонов
        for (size_t j = 0; j + 1 < current_runs; j += 2)
        {
            size_t s1 = ctx->runs.data[j].start;
            size_t l1 = ctx->runs.data[j].len;
            size_t s2 = ctx->runs.data[j + 1].start;
            size_t l2 = ctx->runs.data[j + 1].len;

            merge_ranges(ctx->array, s1, s2, s2 + l2, ctx->temp_buffer);

            ctx->runs.data[write_idx].start = s1;
            ctx->runs.data[write_idx].len = l1 + l2;
            write_idx++;
        }

        // Если осталось нечетное количество прогонов
        if (current_runs % 2 == 1)
        {
            ctx->runs.data[write_idx++] = ctx->runs.data[current_runs - 1];
        }

        current_runs = write_idx;
        ctx->runs.size = current_runs;
    }
}
