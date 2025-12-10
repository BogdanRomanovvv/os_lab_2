#include "timsort.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// Функция инициализации контекста сортировки
// ctx - указатель на структуру контекста
// array - указатель на массив для сортировки
// size - размер массива
// threads - количество потоков для параллельной сортировки
void sort_context_init(sort_context_t *ctx, int32_t *array, size_t size, size_t threads)
{
    ctx->array = array;                                // Сохраняем указатель на массив
    ctx->array_size = size;                            // Сохраняем размер массива
    ctx->max_threads = threads > 0 ? threads : 1;      // Устанавливаем количество потоков (минимум 1)
    ctx->temp_buffer = malloc(size * sizeof(int32_t)); // Выделяем временный буфер для слияния
    runs_init(&ctx->runs);                             // Инициализируем структуру для хранения прогонов
}

// Функция очистки ресурсов контекста сортировки
// ctx - указатель на структуру контекста
void sort_context_cleanup(sort_context_t *ctx)
{
    if (ctx->temp_buffer) // Проверяем, был ли выделен временный буфер
    {
        free(ctx->temp_buffer);  // Освобождаем память временного буфера
        ctx->temp_buffer = NULL; // Обнуляем указатель для безопасности
    }
    runs_free(&ctx->runs); // Освобождаем память структуры прогонов
}

/* ========== Основная функция параллельной сортировки ========== */

// Основная функция выполнения параллельной сортировки TimSort
// ctx - указатель на контекст сортировки с массивом и настройками
void parallel_timsort(sort_context_t *ctx)
{
    struct timeval t1, t2, t3; // Переменные для замера времени выполнения этапов

    // 1. Обнаружение прогонов в массиве
    gettimeofday(&t1, NULL); // Засекаем время начала обнаружения прогонов
    detect_runs(ctx);        // Находим естественные упорядоченные последовательности
    gettimeofday(&t2, NULL); // Засекаем время окончания

    size_t initial_runs = ctx->runs.size; // Сохраняем количество найденных прогонов
    // Вычисляем время в миллисекундах: (секунды * 1000) + (микросекунды / 1000)
    long long detect_ms = (t2.tv_sec - t1.tv_sec) * 1000LL + (t2.tv_usec - t1.tv_usec) / 1000LL;
    printf("DEBUG: Обнаружено прогонов: %zu (время: %lld мс)\n", initial_runs, detect_ms);

    // 2. Параллельная сортировка каждого прогона
    thread_pool_context_t pool;   // Структура для управления пулом потоков
    thread_pool_init(&pool, ctx); // Инициализируем пул потоков
    gettimeofday(&t1, NULL);      // Засекаем время начала параллельной сортировки
    thread_pool_sort_runs(&pool); // Распределяем прогоны между потоками и сортируем параллельно
    gettimeofday(&t2, NULL);      // Засекаем время окончания
    thread_pool_cleanup(&pool);   // Завершаем потоки и освобождаем ресурсы пула

    // Вычисляем время параллельной сортировки в миллисекундах
    long long sort_ms = (t2.tv_sec - t1.tv_sec) * 1000LL + (t2.tv_usec - t1.tv_usec) / 1000LL;
    printf("DEBUG: Параллельная сортировка прогонов: %lld мс\n", sort_ms);

    // 3. Последовательное слияние всех прогонов
    gettimeofday(&t1, NULL); // Засекаем время начала слияния
    merge_all_runs(ctx);     // Объединяем все отсортированные прогоны в один массив
    gettimeofday(&t3, NULL); // Засекаем время окончания

    // Вычисляем время слияния в миллисекундах
    long long merge_ms = (t3.tv_sec - t1.tv_sec) * 1000LL + (t3.tv_usec - t1.tv_usec) / 1000LL;
    printf("DEBUG: Слияние прогонов: %lld мс\n", merge_ms);
}

/* ========== Парсинг аргументов командной строки ========== */

// Структура для хранения параметров программы
typedef struct
{
    size_t array_size;  // Размер массива для сортировки
    size_t num_threads; // Количество потоков для параллельной обработки
    unsigned int seed;  // Seed для генератора случайных чисел
} program_args_t;

// Функция парсинга аргументов командной строки
// argc - количество аргументов
// argv - массив строк с аргументами
// args - указатель на структуру для сохранения параметров
static void parse_args(int argc, char **argv, program_args_t *args)
{
    // Значения по умолчанию
    args->array_size = 100000;             // Размер массива: 100000 элементов
    args->num_threads = 1;                 // Количество потоков: 1 (последовательная сортировка)
    args->seed = (unsigned int)time(NULL); // Seed: текущее время (случайная генерация)

    int opt; // Переменная для хранения очередного параметра
    // Цикл обработки параметров командной строки
    // getopt возвращает -1 когда все параметры обработаны
    while ((opt = getopt(argc, argv, "n:t:s:")) != -1)
    {
        switch (opt) // Обрабатываем каждый параметр
        {
        case 'n': // Параметр -n (размер массива)
            // strtoul преобразует строку в unsigned long (десятичная система)
            args->array_size = (size_t)strtoul(optarg, NULL, 10);
            break;
        case 't': // Параметр -t (количество потоков)
            args->num_threads = (size_t)strtoul(optarg, NULL, 10);
            if (args->num_threads < 1) // Гарантируем минимум 1 поток
                args->num_threads = 1;
            break;
        case 's': // Параметр -s (seed для генератора)
            args->seed = (unsigned int)strtoul(optarg, NULL, 10);
            break;
        }
    }
}

/* ========== Главная функция ========== */

// Точка входа в программу
// argc - количество аргументов командной строки
// argv - массив строк с аргументами
int main(int argc, char **argv)
{
    // Парсинг аргументов
    program_args_t args;           // Создаем структуру для параметров
    parse_args(argc, argv, &args); // Разбираем аргументы командной строки

    // Выделение памяти для массива
    // malloc выделяет memory: размер_массива * размер_int32_t байт
    int32_t *array = malloc(args.array_size * sizeof(int32_t));
    if (!array) // Проверяем успешность выделения памяти
    {
        putstr("Ошибка: не удалось выделить память\n");
        return 1; // Завершаем программу с кодом ошибки
    }

    // Генерация случайных данных
    // Заполняем массив случайными числами с заданным seed
    generate_random_array(array, args.array_size, args.seed);

    // Инициализация контекста сортировки
    sort_context_t ctx; // Создаем структуру контекста
    // Инициализируем контекст: передаем массив, размер и количество потоков
    sort_context_init(&ctx, array, args.array_size, args.num_threads);

    // Измерение времени выполнения
    struct timeval tv_start, tv_end; // Структуры для хранения времени
    gettimeofday(&tv_start, NULL);   // Засекаем время начала сортировки

    // Выполнение параллельной сортировки
    parallel_timsort(&ctx); // Запускаем основной алгоритм TimSort

    gettimeofday(&tv_end, NULL); // Засекаем время окончания сортировки
    // Вычисляем общее время выполнения в миллисекундах
    long long elapsed_ms = (tv_end.tv_sec - tv_start.tv_sec) * 1000LL +
                           (tv_end.tv_usec - tv_start.tv_usec) / 1000LL;

    // Вывод статистики
    // Выводим: размер массива, количество потоков, количество прогонов, время выполнения
    print_stats(args.array_size, args.num_threads, ctx.runs.size, elapsed_ms);

    // Проверка корректности сортировки
    // verify_sorted проверяет, что каждый элемент <= следующего
    if (!verify_sorted(array, args.array_size))
    {
        putstr("ОШИБКА: массив не отсортирован!\n");
    }

    // Очистка ресурсов
    sort_context_cleanup(&ctx); // Освобождаем ресурсы контекста (буферы, прогоны)
    free(array);                // Освобождаем память массива

    return 0; // Успешное завершение программы
}
