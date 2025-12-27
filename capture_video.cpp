#include <syslog.h>
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <signal.h>
#include <memory.h>
#include <semaphore.h>

sem_t sem1, sem2, sem3;
int stop_flag = 0;


void* start_image_capture(void* args)
{
    int cpu = sched_getcpu();

    while(!stop_flag)
    {
        if (sem_wait(&sem1)!=0)
        {
            syslog(LOG_ERR, "Semaphore problem");
            break;
        }
        if (stop_flag) break;
        syslog(LOG_INFO, "Read Image Started on CPU %d", cpu);
    }

    syslog(LOG_INFO, "Read Image Completed");
    return NULL;


}

void* process_image(void* args)
{
    int cpu = sched_getcpu();
    while(!stop_flag)
    {
        if (sem_wait(&sem2)!=0)
        {
            syslog(LOG_ERR, "Semaphore problem");
            break;
        }
        if (stop_flag) break;
        syslog(LOG_INFO, "Process Image Started on CPU %d", cpu);
    }

    syslog(LOG_INFO, "Process Image Completed");
    return NULL;

}

void* write_image(void* args)
{
    int cpu = sched_getcpu();
    while(!stop_flag)
    {
        if (sem_wait(&sem3)!=0)
        {
            syslog(LOG_ERR, "Semaphore problem");
            break;
        }
        if (stop_flag) break;
        syslog(LOG_INFO, "Write Image Started on CPU %d", cpu);
    }

    syslog(LOG_INFO, "Write Image Completed");
    return NULL;
}

int counter = 0;

// Signal handler here
void scheduler(int sig)
{
    counter++;
    // 100ms/4 = 25HZ
    if (counter%4==0)
    {
        sem_post(&sem1);
    }
    if (counter%4==0)
    {
        sem_post(&sem2);
    }
    if (counter%4==0)
    {
        sem_post(&sem3);
    }
}

void program_interrupt(int sig)
{
    syslog(LOG_INFO, "Interrupt signal has been received");
    stop_flag = 1;
    sem_post(&sem1);
    sem_post(&sem2);
    sem_post(&sem3);
}


int main()
{
    pthread_t read_thread, process_thread, write_thread;
    pthread_attr_t attr[3];
    struct sched_param param[3];
    cpu_set_t cpu[3];

    syslog(LOG_INFO, "Start Capture Application");

    syslog(LOG_INFO, "System has %d cores and %d cores are available", get_nprocs_conf(), get_nprocs());

    if (sem_init(&sem1,0 ,0)!=0)
    {
        syslog(LOG_ERR, "Semafore cannot be created!!!");
        return 1;
    }
    if (sem_init(&sem2,0 ,0)!=0)
    {
        syslog(LOG_ERR, "Semafore cannot be created!!!");
        return 1;
    }
    if (sem_init(&sem2,0 ,0)!=0)
    {
        syslog(LOG_ERR, "Semafore cannot be created!!!");
        return 1;
    }

    int max_priority = sched_get_priority_max(SCHED_FIFO);

    for (int i=0; i<3; i++)
    {
        pthread_attr_init(&attr[i]);
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
        param[i].sched_priority = max_priority;
        pthread_attr_setschedparam(&attr[i], &param[i]);

        CPU_ZERO(&cpu[i]);
        CPU_SET(i, &cpu[i]);

        pthread_attr_setaffinity_np(&attr[i], sizeof(cpu_set_t), &cpu[i]);

        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);

    }


    if (pthread_create(&read_thread, &attr[0], start_image_capture, NULL) != 0)
    {
        syslog(LOG_ERR, "Failed to create read thread");
        return 1;
    }

    if (pthread_create(&process_thread, &attr[1], process_image, NULL) != 0)
    {
        syslog(LOG_ERR, "Failed to create read thread");
        return 1;
    }

    if (pthread_create(&write_thread, &attr[2], write_image, NULL) != 0)
    {
        syslog(LOG_ERR, "Failed to create read thread");
        return 1;
    }

    // Create a interrupt signal handler
    struct sigaction sa_in;
    memset(&sa_in, 0, sizeof(sa_in));
    sa_in.sa_handler = &program_interrupt;
    sigaction(SIGINT, &sa_in, NULL);
    sigaction(SIGTERM, &sa_in, NULL);

    // Create a signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &scheduler;
    sigaction(SIGALRM, &sa, NULL);
    
    // create a timer 100Hz
    struct itimerval timer;

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000; //10 ms
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000; //10 ms

    setitimer(ITIMER_REAL, &timer, NULL);

    pthread_join(read_thread, NULL);
    pthread_join(process_thread, NULL);
    pthread_join(write_thread, NULL);

    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);

    

    return 0;
}