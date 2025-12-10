# Makefile для параллельной сортировки TimSort
# Лабораторная работа №2 по ОС

CC = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
LDFLAGS = -pthread

TARGET = main
SOURCES = main_new.c runs.c sort.c thread_pool.c utils.c
OBJECTS = $(SOURCES:.c=.o)

# Основная цель
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Сборка объектных файлов
%.o: %.c timsort.h
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJECTS) $(TARGET)

# Пересборка
rebuild: clean all

# Тестовый запуск
test: $(TARGET)
	@echo "=== Тест 1: 10000 элементов, 1 поток ==="
	./$(TARGET) -n 10000 -t 1
	@echo ""
	@echo "=== Тест 2: 10000 элементов, 4 потока ==="
	./$(TARGET) -n 10000 -t 4
	@echo ""
	@echo "=== Тест 3: 100000 элементов, 4 потока ==="
	./$(TARGET) -n 100000 -t 4

# Быстрый тест производительности
benchmark: $(TARGET)
	@echo "=== Бенчмарк: различное количество потоков ==="
	@echo "n=1000000, threads=1:"
	@./$(TARGET) -n 1000000 -t 1
	@echo "n=1000000, threads=2:"
	@./$(TARGET) -n 1000000 -t 2
	@echo "n=1000000, threads=4:"
	@./$(TARGET) -n 1000000 -t 4
	@echo "n=1000000, threads=8:"
	@./$(TARGET) -n 1000000 -t 8

.PHONY: all clean rebuild test benchmark
