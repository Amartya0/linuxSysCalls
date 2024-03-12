// Include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Define constants
#define MAX_QUEUE_SIZE 10
#define MAX_PRODUCER_THREADS 10
#define MAX_CONSUMER_THREADS 10
#define MAX_SLEEP_TIME 10

// Define data structures

// Structure to represent a circular queue
typedef struct
{
    int buffer[MAX_QUEUE_SIZE];                 // Array to hold the queue elements
    int front, rear;                            // Indices to track front and rear of the queue
    bool isFull;                                // Flag to indicate whether the queue is full
    pthread_mutex_t synchronizer;               // Mutex for synchronization
    pthread_cond_t enough_spaces, enough_items; // Condition variables for synchronization
} circular_queue;

// Structure to hold information about threads
typedef struct
{
    int id;              // Thread ID
    pthread_t thread_id; // Pthread ID
    bool is_active;      // Flag to indicate whether the thread is active
} thread_information;

// Function prototypes
void initialize_queue();
void enqueue();
int dequeue();
int number_of_items();

void initialize_thread_information();
void create_producer_thread();
void create_consumer_thread();

void *manager(void *arg);
void *producer(void *arg);
void *consumer(void *arg);

void delete_producer_thread();
void delete_consumer_thread();

void show_status();
void clean_up();

// Global variables
circular_queue queue;
thread_information producer_threads[MAX_PRODUCER_THREADS];
thread_information consumer_threads[MAX_CONSUMER_THREADS];
int producer_count = 0, consumer_count = 0;
pthread_t managerThread;

// Main function
void main()
{
    initialize_queue();
    initialize_thread_information();

    // Create manager thread
    pthread_create(&managerThread, NULL, manager, NULL);
    printf("Manager thread created.\n");

    // Wait for manager thread to finish
    pthread_join(managerThread, NULL);

    // Exit program
    exit(0);
}

// Manager thread function
void *manager(void *arg)
{
    printf("Welcome to manager thread.\n");
    char choice;
    do
    {
        // Display menu options
        printf("Menu:\n");
        printf("  1. Add Producer.\n");
        printf("  2. Add Consumer.\n");
        printf("  3. Delete Producer.\n");
        printf("  4. Delete Consumer.\n");
        printf("  5. Show Queue Status.\n");
        printf("  6. Show thread status.\n");
        printf("  7. Clear All Threads, Resources and Exit.\n");
        printf("  Enter your choice: ");
        scanf(" %c", &choice);

        // Perform actions based on user's choice
        switch (choice)
        {
        case '1':
            create_producer_thread();
            break;
        case '2':
            create_consumer_thread();
            break;
        case '3':
            delete_producer_thread();
            break;
        case '4':
            delete_consumer_thread();
            break;
        case '5':
            show_status();
            break;
        case '6':
            clean_up();
            printf("Exiting manager thread.\n");
            pthread_exit(0);
            break;
        default:
            break;
        }
    } while (choice != '6');

    pthread_exit(0);
}

// Producer thread function
void *producer(void *arg)
{
    // Get thread ID
    int id = *(int *)arg;
    while (true)
    {
        producer_threads[id].is_active = true; // Mark thread as active
        printf("\n");
        srand(time(NULL));
        int item = rand() % (MAX_QUEUE_SIZE - 1) + 1;     // Generate random number of items to produce
        pthread_mutex_lock(&queue.synchronizer);          // Lock the mutex
        while (MAX_QUEUE_SIZE - number_of_items() < item) // check if enough space in the queue
        {
            printf("\t\t\t\t\tProducer %d waiting for %d spaces.\n", id, item);
            pthread_cond_wait(&queue.enough_spaces, &queue.synchronizer); // Wait until there is enough space in the queue
        }
        for (int i = 0; i < item; i++)
        {
            int data = rand() % 100; // Generate random data
            enqueue(data);           // Enqueue the data
            printf("\t\t\t\t\tProducer %d produced item %d.\n", id, data);
        }
        pthread_cond_broadcast(&queue.enough_items); // Signal all waiting consumer threads
        pthread_mutex_unlock(&queue.synchronizer);   // Unlock the mutex
        if (producer_count > 1)
        {
            producer_threads[id].is_active = false; // Mark thread as inactive
        }
        sleep(rand() % MAX_SLEEP_TIME + 1); // Sleep for random amount of time
    }
}

// Consumer thread function
void *consumer(void *arg)
{
    int id = *(int *)arg;
    while (true)
    {
        consumer_threads[id].is_active = true;
        printf("\n");
        srand(time(NULL));
        int item = rand() % (MAX_QUEUE_SIZE - 1) + 1;
        pthread_mutex_lock(&queue.synchronizer);
        while (number_of_items() < item)
        {
            printf("\t\t\t\t\t\t\t\t\t\tConsumer %d waiting for %d items.\n", id, item);
            pthread_cond_wait(&queue.enough_items, &queue.synchronizer);
        }
        for (int i = 0; i < item; i++)
        {
            int data = dequeue();
            printf("\t\t\t\t\t\t\t\t\t\tConsumer %d consumed item %d.\n", id, data);
        }
        pthread_cond_broadcast(&queue.enough_spaces);
        pthread_mutex_unlock(&queue.synchronizer);
        if (consumer_count > 1)
        {
            consumer_threads[id].is_active = false;
        }
        sleep(rand() % MAX_SLEEP_TIME + 1);
    }
}

// Function to create a producer thread
void create_producer_thread()
{
    if (producer_count < MAX_PRODUCER_THREADS)
    {
        for (int i = 0; i < MAX_PRODUCER_THREADS; i++)
        {
            if (producer_threads[i].id == -1)
            {
                producer_threads[i].id = i + 1;
                pthread_create(&producer_threads[i].thread_id, NULL, producer, &producer_threads[i].id);
                producer_threads[i].is_active = true;
                producer_count++;
                printf("Producer thread %d created.\n", producer_threads[i].id);
                break;
            }
        }
    }
    else
    {
        printf("Maximum producer threads reached.\n");
    }
}

// Function to create a consumer thread
void create_consumer_thread()
{
    if (consumer_count < MAX_CONSUMER_THREADS)
    {
        for (int i = 0; i < MAX_CONSUMER_THREADS; i++)
        {
            if (consumer_threads[i].id == -1)
            {
                consumer_threads[i].id = i + 1;
                pthread_create(&consumer_threads[i].thread_id, NULL, consumer, &consumer_threads[i].id);
                consumer_threads[i].is_active = true;
                consumer_count++;
                printf("Consumer thread %d created.\n", consumer_threads[i].id);
                break;
            }
        }
    }
    else
    {
        printf("Maximum consumer threads reached.\n");
    }
}

// Function to delete a producer thread
void delete_producer_thread()
{
    if (producer_count > 0)
    {
        bool delete_flag = false;
        for (int i = 0; i < MAX_PRODUCER_THREADS; i++)
        {
            if (producer_threads[i].id != -1 && producer_threads[i].is_active == false)
            {
                delete_flag = true;
                pthread_cancel(producer_threads[i].thread_id);
                int temp = producer_threads[i].id;
                producer_threads[i].id = -1;
                producer_threads[i].is_active = false;
                producer_threads[i].thread_id = -1;
                producer_count--;
                printf("Producer thread %d deleted.\n", temp);

                return;
            }
        }
        if (delete_flag == false)
        {
            printf("All producer threads are active.\n");
        }
    }
    else
    {
        printf("No producer threads to delete.\n");
    }
}

// Function to delete a consumer thread
void delete_consumer_thread()
{
    if (consumer_count > 0)
    {
        bool delete_flag = false;
        for (int i = 0; i < MAX_CONSUMER_THREADS; i++)
        {
            if (consumer_threads[i].id != -1 && consumer_threads[i].is_active == false)
            {
                delete_flag = true;
                pthread_cancel(consumer_threads[i].thread_id);
                int temp = consumer_threads[i].id;
                consumer_threads[i].id = -1;
                consumer_threads[i].is_active = false;
                consumer_threads[i].thread_id = -1;
                consumer_count--;
                printf("Consumer thread %d deleted.\n", temp);
                return;
            }
        }
        if (delete_flag == false)
        {
            printf("All consumer threads are active.\n");
        }
    }
    else
    {
        printf("No consumer threads to delete.\n");
    }
}

// Function to initialize thread information
void initialize_thread_information()
{
    for (int i = 0; i < MAX_PRODUCER_THREADS; i++)
    {
        producer_threads[i].id = -1;
        producer_threads[i].thread_id = -1;
        producer_threads[i].is_active = false;
    }
    for (int i = 0; i < MAX_CONSUMER_THREADS; i++)
    {
        consumer_threads[i].id = -1;
        consumer_threads[i].thread_id = -1;
        consumer_threads[i].is_active = false;
    }
}

// Function to initialize the circular queue
void initialize_queue()
{
    queue.front = 0;
    queue.rear = 0;
    queue.isFull = false;
    pthread_mutex_init(&queue.synchronizer, NULL);
    pthread_cond_init(&queue.enough_spaces, NULL);
    pthread_cond_init(&queue.enough_items, NULL);
}

// Function to enqueue an item into the circular queue
void enqueue(int item)
{
    queue.buffer[queue.rear] = item;
    queue.rear = (queue.rear + 1) % MAX_QUEUE_SIZE;
    if (queue.rear == queue.front)
    {
        queue.isFull = true;
    }
}

// Function to dequeue an item from the circular queue
int dequeue()
{
    int item = queue.buffer[queue.front];
    queue.front = (queue.front + 1) % MAX_QUEUE_SIZE;
    queue.isFull = false;
    return item;
}

// Function to get the number of items in the circular queue
int number_of_items()
{
    int result;
    if (queue.isFull)
    {
        result = MAX_QUEUE_SIZE;
    }
    else
    {
        result = (queue.rear - queue.front + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE;
    }
    // printf("Number of items in queue: %d\n", result);
    return result;
}

// Function to display the status of the circular queue and threads
void show_status()
{
    printf("\n");
    printf("Queue Status:\n");
    printf("  Front: %d\n", queue.front);
    printf("  Rear: %d\n", queue.rear);
    printf("  Number of items: %d\n", number_of_items());
    printf("  Producer threads: %d\n", producer_count);
    for (int i = 0; i < MAX_PRODUCER_THREADS; i++)
    {
        if (producer_threads[i].id != -1)
        {
            printf("    Producer %d: %ld: %s\n", producer_threads[i].id, producer_threads[i].thread_id, producer_threads[i].is_active ? "Active" : "Inactive");
        }
    }
    printf("  Consumer threads: %d\n", consumer_count);
    for (int i = 0; i < MAX_CONSUMER_THREADS; i++)
    {
        if (consumer_threads[i].id != -1)
        {
            printf("    Consumer %d: %ld: %s\n", consumer_threads[i].id, consumer_threads[i].thread_id, consumer_threads[i].is_active ? "Active" : "Inactive");
        }
    }
    printf("\n");
}

// Function to clean up resources and exit the program
void clean_up()
{
    // Cancel all producer threads and clear thread information
    for (int i = 0; i < MAX_PRODUCER_THREADS; i++)
    {
        if (producer_threads[i].id != -1)
        {
            pthread_cancel(producer_threads[i].thread_id);
            producer_threads[i].id = -1;
            producer_threads[i].is_active = false;
            producer_threads[i].thread_id = -1;
            producer_count--;
        }
    }
    // Cancel all consumer threads and clear thread information
    for (int i = 0; i < MAX_CONSUMER_THREADS; i++)
    {
        if (consumer_threads[i].id != -1)
        {
            pthread_cancel(consumer_threads[i].thread_id);
            consumer_threads[i].id = -1;
            consumer_threads[i].is_active = false;
            consumer_threads[i].thread_id = -1;
            consumer_count--;
        }
    }
    // Destroy mutex and condition variables
    pthread_mutex_destroy(&queue.synchronizer);
    pthread_cond_destroy(&queue.enough_spaces);
    pthread_cond_destroy(&queue.enough_items);
    printf("All threads and resources cleared.\n");
}
