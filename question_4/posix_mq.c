 #define _GNU_SOURCE
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <signal.h>
#define SNDRCV_MQ "/send_receive_mq"
#define MAX_MSG_SIZE (128)

struct mq_attr mq_attr;
static char canned_msg[] = "this is a test, and only a test, in the event of a real emergency, you would be instructed ...";

void *receiver(void *parameter)
{
  mqd_t mymq;
  char buffer[MAX_MSG_SIZE];
  int prio;
  int nbytes;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0664, &mq_attr);

  if(mymq == -1)
    perror("mq_open");

  /* read oldest, highest priority msg from the message queue */
  if((nbytes = mq_receive(mymq, buffer, MAX_MSG_SIZE, &prio)) == -1)
  {
    perror("mq_receive");
  }
  else
  {
    buffer[nbytes] = '\0';
    printf("receive: msg %s received with priority = %d, length = %d\n",
           buffer, prio, nbytes);
  }
    
}

void *sender(void *parameter)
{
  mqd_t mymq;
  int prio;
  int nbytes;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_RDWR| O_CREAT, 0664, &mq_attr);

  if(mymq == -1)
    perror("mq_open");
    printf("messade size%d",sizeof(canned_msg));

  /* send message with priority=30 */
  if((nbytes = mq_send(mymq, canned_msg, sizeof(canned_msg), 30)) == -1)
  {
    perror("mq_send");
  }
  else
  {
    printf("send: message successfully sent\n");
  }
}
  
  int main()
  {
    int ret;
    
    cpu_set_t cpuset;
    pthread_t sender_thread,reciever_thread;
    int max_priority,min_priority;
    max_priority=sched_get_priority_max(SCHED_FIFO);
    min_priority=sched_get_priority_min(SCHED_FIFO);
    pthread_attr_t receiver_attr,sender_attr;
    struct sched_param receiver_param,sender_param;
    CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);

  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 10;
  mq_attr.mq_msgsize = MAX_MSG_SIZE;
  mq_attr.mq_flags = 0;
  mq_attr.mq_curmsgs = 0;

 ret = pthread_attr_init(&receiver_attr);
  //For Receiving
  ret = pthread_attr_setinheritsched(&receiver_attr, PTHREAD_EXPLICIT_SCHED);
  ret = pthread_attr_setschedpolicy(&receiver_attr, SCHED_FIFO);
  receiver_param.sched_priority = min_priority;
  pthread_attr_setschedparam(&receiver_attr, &receiver_param);
  ret = pthread_attr_setaffinity_np(&receiver_attr, sizeof(cpu_set_t), &cpuset);

  //initialize  with default atrribute
  ret = pthread_attr_init(&sender_attr);
  //For Sending
  ret = pthread_attr_setinheritsched(&sender_attr, PTHREAD_EXPLICIT_SCHED);
  ret = pthread_attr_setschedpolicy(&sender_attr, SCHED_FIFO);
  sender_param.sched_priority = max_priority;
  pthread_attr_setschedparam(&sender_attr, &sender_param);
  ret = pthread_attr_setaffinity_np(&sender_attr, sizeof(cpu_set_t), &cpuset);

  if((ret=pthread_create(&sender_thread, &sender_attr, sender, NULL)) != 0)
  {
    perror("\n\rFailed to createSender Thread\n\r");
    perror("pthread_create");
    printf("ret=%d\n", ret);
  }  
  else
  {
    printf("\n\rSender Thread Created with id=%d\n\r", ret);
  }

  if((ret=pthread_create(&reciever_thread, &receiver_attr, receiver, NULL)) != 0)
{
perror("\n\r Failed Making Reciever Thread\n\r");
printf("ret=%d\n", ret);
}
  else
{
printf("\n\r Receiver Thread Created with ret=%d\n\r", ret);
}
  printf("pthread join send\n");
  pthread_join(sender_thread, NULL);

  printf("pthread join receive\n");
  pthread_join(reciever_thread, NULL);
  }