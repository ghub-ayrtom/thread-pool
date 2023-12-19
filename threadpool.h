#ifndef _THREAD_POOL_
#define _THREAD_POOL_

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

// Рабочая область пула потоков
void *worker(void *args);

// Выполняет (вызывает) задачу (функцию)
void execute(void (*function)(void *function_args), void *data);

// Инициализирует пул потоков
void pool_init(void);

// Отправляет задачу function с её данными data в пул (очередь) потоков
int pool_submit(void (*function)(void *function_args), void *data);

// Завершает работу пула потоков
void pool_shutdown(void);

#endif
