#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
    int a;
    int b;
} Data;

// Function for the addition thread
void *addition(void *arg)
{
    Data *data = (Data *)arg;
    // printf("%s", sec_arg);
    int sum = data->a + data->b;
    printf("Addition: %d + %d = %d\n", data->a, data->b, sum);
    return NULL;
}

// Function for the subtraction thread
void *subtraction(void *arg)
{
    Data *data = (Data *)arg;
    int diff = data->a - data->b;
    printf("Subtraction: %d - %d = %d\n", data->a, data->b, diff);
    return NULL;
}

int main()
{
    pthread_t thread1, thread2;
    Data data = {10, 5}; // Example values
    char *head = "hello!";
    // Create threads
    pthread_create(&thread1, NULL, addition, &data);
    pthread_create(&thread2, NULL, subtraction, &data);

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}