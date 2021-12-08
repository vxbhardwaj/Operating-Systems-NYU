#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];
//1st line addition
pthread_mutex_t lock[NUM_BUCKETS]; //declare a lock


typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];

void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
    int i = key % NUM_BUCKETS;
    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
    if (!e) panic("No memory to allocate bucket!");
    //2nd line addtion
    pthread_mutex_lock(&lock[i]);
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;
    //3rd line addition
    pthread_mutex_unlock(&lock[i]);
}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key) return b;
    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;

    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        insert(keys[key], tid);
    }

    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;

    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        if (retrieve(keys[key]) == NULL) lost++;
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);

    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;

    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

    srandom(time(NULL));
    for (i = 0; i < NUM_KEYS; i++)
        keys[i] = random();

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }
    //4th and 5th line addition
    for (i=0; i<NUM_BUCKETS; i++){
        pthread_mutex_init(&lock[i], NULL);
    }
    // Insert keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }
    
    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();
    
    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);
    
    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }

    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();

    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}

// Q1: Keys are lost during insertion due to the Race condition. So, locking during insertion is needed to ensure keys aren’t lost. No such race condition happens during retrieval. 
// There is an overhead to adding mutex with additional line of code for mutex, resulting in more time for the mutex code to run. I have saved the changed code under parallel_mutex.c. Verified that I can now run multiple threads without losing any keys. I didn’t add lock and unlock for retrieval task, only added it for the insertion task because insertion experiences the race condition, retrieval doesn’t.

// Q2: Averages time for parallel_mutex.c and parallel_spin.c for insertion and deletion for 8 threads for 10 iterations is as follows:
// Insertion 0.096 seconds and retrieval 8.86 seconds with parallel_mutex.c.
// Insertion 0.069 seconds and retrieval 7.65 seconds with parallel_spin.c.
// For different thread values, observations vary.
 
// Time difference between mutex and spin can be attributed to the reason that mutex method causes threads to relinquish CPU allocation if threads can’t acquire mutex. Spin ensures that the thread keeps spinning while looping again till it gets the lock. 
// Spin is not necessarily faster than mutex. Spin is faster in certain situation such as critical region is small or when we have multiple cores. If those are smaller spins, they will quickly acquire resources, and the process is faster. Therefore, in those situations, the spin is faster. So, spin, by never sleeping, and always checking can be faster, especially because the duration of lock being held is usually very small. 
// But that is not this case then spins are slow because instead of sleeping and waiting for resources to be available, it will keep "spinning" and racking up the execution time until it acquires the resources.The mutex can be faster here because for spinlock, it enters a busy-waiting until it gets the resource and uses up the processor, but for mutex, process goes in waiting state and does not use the processor while waiting. 
// No keys are lost using mutex and spin due to lock being present for the insertion step, preventing the race condition. As per question 1, we know that retrieval step doesn’t have a key loss anyways.

// Q3: No, only insertion requires it, so no code change needed. It is because I didn’t add a lock for retrieval of an item from the hash table for Q1 either. So, multiple retrieval operations can run in parallel as-is.

// Q4: I have edited the code to ensure that the insertions run parallelly while using multiple threads while employing locking. It is achieved by ensuring that locking only applies when the value of “i” is the same as key % NUM_BUCKETS. For a race condition, if we try accessing the same slot, we acquire the lock first making the thread wait.


