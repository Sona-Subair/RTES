#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>


pthread_mutex_t mutex;
pthread_t thread1, thread2;

double random_angle();
double random_value();
void *get_state(void* thread_parameter);
void *update_state(void* thread_parameter);

struct state
{
    double X, Y, Z, Roll, Pitch, Yaw;
    struct timespec time;
};

int main()
{
    int ret = 0;
    struct state state_t;

    openlog("rtes-log", LOG_PID | LOG_ERR, LOG_USER);
    setlogmask(LOG_UPTO(LOG_DEBUG));
    if ((ret = pthread_mutex_init(&mutex, NULL)) != 0)
    {
        syslog(LOG_USER, "%s", strerror(errno));
        closelog();
        exit(1);
    }
    if ((ret = pthread_create(&thread1, NULL, update_state,(void*)&state_t)) != 0)
    {
        syslog(LOG_USER, "%s", strerror(errno));
        closelog();
        exit(1);
    }

    if ((ret = pthread_create(&thread2, NULL, get_state,(void*)&state_t)) != 0)
    {
        syslog(LOG_USER, "%s", strerror(errno));
        closelog();
        exit(1);
    }

    if ((ret = pthread_join(thread1, NULL) != 0))
    {
        syslog(LOG_USER, "%s", strerror(errno));
        closelog();
        exit(1);
    }
    if ((ret = pthread_join(thread2, NULL) != 0))
    {
        syslog(LOG_USER, "%s", strerror(errno));
        closelog();
        exit(1);
    }

    if((ret = pthread_mutex_destroy(&mutex)) != 0)
    {
        syslog(LOG_USER, "%s", strerror(errno));
        closelog();
        exit(1);
    }     

    closelog();
    return (0);
}

void *get_state(void* thread_parameter)
{
    while(1){
        sleep(5);
        int ret=0;
        struct state *state_t= (struct state*)thread_parameter;
            if((ret = pthread_mutex_lock(&mutex)) != 0)
            {
                syslog(LOG_USER, "%s", strerror(errno));
                exit(1);
            }
            syslog(LOG_USER, "Read: X-axis:%2f, Y-axis:%2f, Z-axis:%2f, Roll:%2f, Pitch:%2f, Yaw:%2f updated on timestamp:[%ld seconds:%ld nanoseconds]",state_t->X,state_t->Y,state_t->Z,state_t->Roll,state_t->Pitch,state_t->Yaw,state_t->time.tv_sec,state_t->time.tv_nsec);
            if((ret = pthread_mutex_unlock(&mutex)) != 0)
            {
                syslog(LOG_USER, "%s", strerror(errno));
                exit(1);
            }
    }
    pthread_exit(NULL);
}

void *update_state(void* thread_parameter)
{ 
    struct state *state_t= (struct state*)thread_parameter;
    int ret = 0;
    while(1){
        sleep(5);
        if((ret = pthread_mutex_lock(&mutex)) != 0)
        {
            syslog(LOG_USER, "%s", strerror(errno));
            exit(1);
        }
        state_t->X=random_value();
        state_t->Y=random_value();
        state_t->Z=random_value();
        state_t->Roll=random_angle();
        state_t->Pitch=random_angle();
        state_t->Yaw=random_angle();
        clock_gettime(CLOCK_REALTIME, &state_t->time);
        syslog(LOG_USER, "Updated:Timestamp:[%ld seconds:%ld nanoseconds] X-axis : %2f, Y-axis: %2f, Z-axis: %2f, Roll:%2f, Pitch:%2f, Yaw:%2f",state_t->time.tv_sec,state_t->time.tv_nsec,state_t->X,state_t->Y,state_t->Z,state_t->Roll,state_t->Pitch,state_t->Yaw);
        if((ret = pthread_mutex_unlock(&mutex)) != 0)
        {
            syslog(LOG_USER, "%s", strerror(errno));
            exit(1);
        }
    }
    pthread_exit(NULL);
}

double random_angle()
{
    return ((((double)rand()/(double)RAND_MAX) * 180) - 90);
}

double random_value()
{
    return ((((double)rand()/(double)RAND_MAX) * 10));
}