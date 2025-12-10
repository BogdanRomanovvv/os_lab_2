#ifndef TIMSORT_H
#define TIMSORT_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

/* ========== Структуры данных ========== */

// Дескриптор прогона (последовательности элементов)
typedef struct
{
    size_t start; // Начальный индекс прогона
    size_t len;   // Длина прогона
} run_t;

// Динамический массив прогонов
typedef struct
{
    run_t *data; // Указатель на массив прогонов
    size_t size; // Текущее количество прогонов
    size_t cap;  // Вместимость массива
} runs_array_t;

// Контекст сортировки (вместо глобальных переменных)
typedef struct
{
    int32_t *array;       // Массив для сортировки
    size_t array_size;    // Размер массива
    runs_array_t runs;    // Массив прогонов
    size_t max_threads;   // Максимальное количество потоков
    int32_t *temp_buffer; // Временный буфер для слияния
} sort_context_t;

// Контекст пула потоков
typedef struct
{
    sort_context_t *ctx;   // Указатель на контекст сортировки
    size_t next_run_index; // Следующий прогон для обработки
    pthread_mutex_t mutex; // Мьютекс для синхронизации
} thread_pool_context_t;

/* ========== Функции для работы с прогонами (runs.c) ========== */

// Инициализация массива прогонов
void runs_init(runs_array_t *r);

// Добавление прогона в массив
void runs_push(runs_array_t *r, run_t v);

// Освобождение памяти массива прогонов
void runs_free(runs_array_t *r);

// Вычисление минимальной длины прогона (как в TimSort)
size_t min_run_length(size_t n);

// Обнаружение и создание прогонов в массиве
void detect_runs(sort_context_t *ctx);

/* ========== Функции сортировки (sort.c) ========== */

// Сортировка вставками
void insertion_sort(int32_t *arr, size_t left, size_t right);

// Функция сравнения для qsort
int cmp_int(const void *a, const void *b);

// Слияние двух отсортированных диапазонов
void merge_ranges(int32_t *arr, size_t l, size_t m, size_t r, int32_t *tmp);

// Сортировка одного прогона
void sort_single_run(sort_context_t *ctx, size_t run_index);

// Слияние всех прогонов в один отсортированный массив
void merge_all_runs(sort_context_t *ctx);

/* ========== Функции пула потоков (thread_pool.c) ========== */

// Инициализация контекста пула потоков
void thread_pool_init(thread_pool_context_t *pool, sort_context_t *ctx);

// Параллельная сортировка всех прогонов
void thread_pool_sort_runs(thread_pool_context_t *pool);

// Очистка контекста пула потоков
void thread_pool_cleanup(thread_pool_context_t *pool);

/* ========== Вспомогательные функции (utils.c) ========== */

// Вывод строки на стандартный вывод
void putstr(const char *s);

// Вывод статистики сортировки
void print_stats(size_t n, size_t threads, size_t runs, long long elapsed_ms);

// Проверка корректности сортировки
int verify_sorted(int32_t *array, size_t size);

// Генерация случайного массива
void generate_random_array(int32_t *array, size_t size, unsigned int seed);

/* ========== Основная функция сортировки ========== */

// Инициализация контекста сортировки
void sort_context_init(sort_context_t *ctx, int32_t *array, size_t size, size_t threads);

// Выполнение параллельной TimSort
void parallel_timsort(sort_context_t *ctx);

// Очистка контекста сортировки
void sort_context_cleanup(sort_context_t *ctx);

#endif // TIMSORT_H
