#include "assert.h"
#include "threadpool.h"
#include "pthread.h"
#include "semaphore.h"
#include "stdbool.h"

// Структура задачи (функции), которая должна быть выполнена потоком из пула
typedef struct 
{
    void (*function)(void *function_args); // Задача
    void *data; // Её данные
} task;

task task_queue[QUEUE_SIZE];
int last_added_task_index = 0;

pthread_t threads[NUMBER_OF_THREADS];
pthread_mutex_t mutex; // = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;

bool work_is_done = false;

void Pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int result_code = pthread_mutex_lock(mutex);
    assert(result_code == 0);
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    int result_code = pthread_mutex_unlock(mutex);
    assert(result_code == 0);
}

// Добавляет задачу в очередь на выполнение пулом потоков
int enqueue(task enqueuing_task)
{
    if (last_added_task_index < QUEUE_SIZE)
    {
        // Критическая секция

        Pthread_mutex_lock(&mutex);

        while (task_queue[last_added_task_index].function != NULL && task_queue[last_added_task_index].data != NULL && last_added_task_index < QUEUE_SIZE)
        {
            ++last_added_task_index;
        }

        task_queue[last_added_task_index] = enqueuing_task;
        sem_post(&semaphore); // Уведомляем один из ожидающих потоков, что работа была передана в пул на выполнение (стала доступной)

        Pthread_mutex_unlock(&mutex);

        //

        return 0;
    }

    return 1;
}

// Извлекает (удаляет) задачу из очереди на выполнение пулом потоков и передаёт её на выполнение одному из них
task dequeue() 
{
    // Критическая секция

    Pthread_mutex_lock(&mutex);

    while (task_queue[last_added_task_index].function == NULL && task_queue[last_added_task_index].data == NULL && last_added_task_index > 0)
    {
        --last_added_task_index;
    }

    task dequeuing_task = task_queue[last_added_task_index];

    task_queue[last_added_task_index].function = NULL;
    task_queue[last_added_task_index].data = NULL;

    if (last_added_task_index > 0)
    {
        --last_added_task_index;
    }
    else
    {
        work_is_done = true;
    }

    Pthread_mutex_unlock(&mutex);

    //

    return dequeuing_task;
}

void *worker(void *args)
{
    while (!work_is_done)
    {
        // Поток ожидает доступную работу
        sem_wait(&semaphore);

        task execute_task = dequeue();

        if (execute_task.data == NULL)
        {
            work_is_done = true;
            break;
        }

        execute(execute_task.function, execute_task.data);
    }

    // Завершаем работу вызвавшего данную функцию потока
    pthread_exit(0);
}

void execute(void (*function)(void *function_args), void *data)
{
    (*function)(data);
}

void pool_init(void)
{
    int return_code = pthread_mutex_init(&mutex, NULL);
    assert(return_code == 0); // 0 - success, 1 - error

    // 0 - семафор будет разделяться между потоками внутри одного процесса, 0 - его начальное значение
    sem_init(&semaphore, 0, 0);
    
    for (int thread = 0; thread < NUMBER_OF_THREADS; ++thread)
    {
        // threads[thread] - один из трёх потоков, worker - функция, которую они все будут выполнять
        pthread_create(&threads[thread], NULL, worker, NULL);
    }
}

int pool_submit(void (*function)(void *function_args), void *data)
{
    task task;

    task.function = function;
    task.data = data;

    // 0 - задача была успешно добавлена в очередь на выполнение пулом потоков, 1 - произошла ошибка при добавлении (очередь заполнена)
    if (enqueue(task) == 1)
    {
        return 1;
    }

    return 0;
}

void pool_shutdown(void)
{
    for (int thread = 0; thread < NUMBER_OF_THREADS; ++thread)
    {
        // Отменяем все ожидающие потоки
        sem_post(&semaphore);
    }
    
    for (int thread = 0; thread < NUMBER_OF_THREADS; ++thread)
    {
        // Ждём завершения работы каждого потока thread
        pthread_join(threads[thread], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&semaphore);
}
