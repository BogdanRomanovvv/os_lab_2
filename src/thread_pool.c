#include "timsort.h" // Заголовочный файл с определениями структур и функций
#include <pthread.h> // pthread_create, pthread_join, pthread_mutex_*
#include <stdlib.h>  // malloc, free

/* ========== Управление пулом потоков ========== */

// Функция инициализации контекста пула потоков
// pool - указатель на структуру пула потоков
// ctx - указатель на контекст сортировки (массив, прогоны, настройки)
void thread_pool_init(thread_pool_context_t *pool, sort_context_t *ctx)
{
    pool->ctx = ctx;                        // Сохраняем указатель на контекст сортировки
    pool->next_run_index = 0;               // Инициализируем индекс следующего прогона для обработки
    pthread_mutex_init(&pool->mutex, NULL); // Инициализируем мьютекс для синхронизации доступа
}

// Функция очистки ресурсов пула потоков
// pool - указатель на структуру пула потоков
void thread_pool_cleanup(thread_pool_context_t *pool)
{
    pthread_mutex_destroy(&pool->mutex); // Уничтожаем мьютекс и освобождаем связанные ресурсы
}

/* ========== Рабочий поток ========== */

// Функция рабочего потока для параллельной сортировки прогонов
// arg - указатель на структуру пула потоков (приводится к thread_pool_context_t*)
// Каждый поток забирает очередной прогон из общей очереди и сортирует его
static void *worker_thread(void *arg)
{
    // Приводим аргумент к типу указателя на контекст пула
    thread_pool_context_t *pool = (thread_pool_context_t *)arg;

    for (;;) // Бесконечный цикл обработки прогонов
    {
        pthread_mutex_lock(&pool->mutex); // Блокируем мьютекс для безопасного доступа к shared data
        size_t i = pool->next_run_index;  // Читаем индекс следующего прогона для обработки

        if (i >= pool->ctx->runs.size) // Если все прогоны уже розданы
        {
            pthread_mutex_unlock(&pool->mutex); // Разблокируем мьютекс
            break;                              // Завершаем цикл (поток завершает работу)
        }

        pool->next_run_index++;             // Увеличиваем индекс (резервируем прогон для текущего потока)
        pthread_mutex_unlock(&pool->mutex); // Разблокируем мьютекс (другие потоки могут забрать свой прогон)

        // Сортируем зарезервированный прогон (вне критической секции - параллельно)
        sort_single_run(pool->ctx, i);
    }

    return NULL; // Возвращаем NULL при завершении потока
}

/* ========== Параллельная сортировка прогонов ========== */

// Функция запуска параллельной сортировки всех прогонов
// pool - указатель на контекст пула потоков
// Создаёт рабочие потоки, распределяет прогоны между ними и ожидает завершения
void thread_pool_sort_runs(thread_pool_context_t *pool)
{
    // Определяем количество потоков (берём из настроек)
    size_t thcount = pool->ctx->max_threads;

    // Оптимизация: не создаём больше потоков, чем есть прогонов
    if (thcount > pool->ctx->runs.size)
        thcount = pool->ctx->runs.size ? pool->ctx->runs.size : 1;

    // Выделяем память для массива дескрипторов потоков (thread IDs)
    pthread_t *threads = malloc(thcount * sizeof(pthread_t));

    // Создание рабочих потоков
    for (size_t t = 0; t < thcount; ++t)
        // Создаём поток: threads[t] - ID, worker_thread - функция, pool - аргумент
        pthread_create(&threads[t], NULL, worker_thread, pool);

    // Ожидание завершения всех потоков (барьер синхронизации)
    for (size_t t = 0; t < thcount; ++t)
        pthread_join(threads[t], NULL); // Блокируется до завершения потока threads[t]

    free(threads); // Освобождаем память массива дескрипторов
}
