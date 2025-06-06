#include <syslog.h>
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>


void* start_image_capture(void* args)
{
    int cpu = sched_getcpu();

    syslog(LOG_INFO, "Read Image Started on CPU %d", cpu);

    syslog(LOG_INFO, "Read Image Completed");
    return NULL;


}

void* process_image(void* args)
{
    int cpu = sched_getcpu();
    syslog(LOG_INFO, "Process Image Started on CPU %d", cpu);

    syslog(LOG_INFO, "Process Image Completed");
    return NULL;

}

void* write_image(void* args)
{
    int cpu = sched_getcpu();
    syslog(LOG_INFO, "Write Image Started on CPU %d", cpu);

    syslog(LOG_INFO, "Write Image Completed");
    return NULL;
}



int main()
{
    pthread_t read_thread, process_thread, write_thread;
    pthread_attr_t attr[3];
    struct sched_param param[3];
    cpu_set_t cpu[3];

    syslog(LOG_INFO, "Start Capture Application");

    syslog(LOG_INFO, "System has %d cores and %d cores are available", get_nprocs_conf(), get_nprocs());

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

    pthread_join(read_thread, NULL);
    pthread_join(process_thread, NULL);
    pthread_join(write_thread, NULL);

    return 0;
}