#include "stdio.h"
#include "threadpool.h"
#include "unistd.h"

struct data
{
    int a;
    int b;
};

void add(void *data)
{
    sleep(1); // Секунды

    struct data *task_data;
    task_data = (struct data*)data;
    
    printf("I add two values %d and %d, result is %d.\n", task_data->a, task_data->b, task_data->a + task_data->b);
}

int main(void)
{
    struct data tasks_data[QUEUE_SIZE];
    int a = 1, b = 2;

    pool_init();

    for (int i = 0; i < QUEUE_SIZE; ++i)
    {
        tasks_data[i].a = a;
        tasks_data[i].b = b;

        if (pool_submit(&add, &tasks_data[i]) == 1)
        {
            printf("[WARNING] The queue of tasks to be executed by the thread pool is full!\n");
        }

        a += 2, b += 2;
    }

    pool_shutdown();

    return 0;
}
