#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct mq_attr mq_attr;

#define SNDRCV_MQ "/send_receive_mq"
#define MSG_SIZE (8)

static char imagebuff[4096];

pthread_attr_t receiver_attr, sender_attr;
struct sched_param receiver_param, sender_param;

void *receiver(void *parameter)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr; 
  int prio;
  int nbytes;
  int count = 0;
  int id;
  static mqd_t mymq;
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0644, &mq_attr);
  if(mymq==-1){
    perror("mq_open");
  }
 
  while(1) {

    /* read oldest, highest priority msg from the message queue */

    printf("Reading %ld bytes\n", sizeof(void *)+sizeof(int));
  
    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == -1)
/*
    if((nbytes = mq_receive(mymq, (void *)&buffptr, (size_t)sizeof(void *), &prio)) == ERROR)
*/
    {
      perror("mq_receive");
    }
    else
    {
      memcpy(&buffptr, buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      printf("receive: ptr msg 0x%p received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);

      printf("contents of ptr = \n%s\n", (char *)buffptr);

      free(buffptr);

      printf("heap space memory freed\n");

    }
    sleep(10);
    
  }

}

void *sender(void *parameter)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio;
  int nbytes;
  int id = 999;
  static mqd_t mymq;
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0644, &mq_attr);
  if(mymq==-1){
    perror("mq_open");
  }
  while(1) {

    /* send malloc'd message with priority=30 */

    buffptr = (void *)malloc(sizeof(imagebuff));
    strcpy(buffptr, imagebuff);
    printf("Message to send = %s\n", (char *)buffptr);

    printf("Sending %ld bytes\n", sizeof(void *)+sizeof(int));

    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

    if((nbytes = mq_send(mymq,  (const char *)&buffer, (size_t)(sizeof(void *)+sizeof(int)), 30)) == -1)
    {
      perror("mq_send");
    }
    else
    {
      printf("send: message ptr 0x%p successfully sent\n", buffptr);
    }
sleep(10);
  }
  
}

int main(){
    int ret;
    pthread_t sender_thread,reciever_thread;
    int max_priority,min_priority;
    max_priority=sched_get_priority_max(SCHED_FIFO);
    min_priority=sched_get_priority_min(SCHED_FIFO);

  int i, j;
  char pixel = 'A';
 
  for(i=0;i<4096;i+=64) {
    pixel = 'A';
    for(j=i;j<i+64;j++) {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';

  printf("buffer =\n%s", imagebuff);

    /* setup common message q attributes */
  /* note that VxWorks does not deal with permissions? */


  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);

  mq_attr.mq_flags = 0;
mq_attr.mq_curmsgs = 0;

 ret = pthread_attr_init(&receiver_attr);
  //For Receiving
  ret = pthread_attr_setinheritsched(&receiver_attr, PTHREAD_EXPLICIT_SCHED);
  ret = pthread_attr_setschedpolicy(&receiver_attr, SCHED_FIFO);
  receiver_param.sched_priority = min_priority;
  pthread_attr_setschedparam(&receiver_attr, &receiver_param);

  //initialize  with default atrribute
  ret = pthread_attr_init(&sender_attr);
  //For Sending
  ret = pthread_attr_setinheritsched(&sender_attr, PTHREAD_EXPLICIT_SCHED);
  ret = pthread_attr_setschedpolicy(&sender_attr, SCHED_FIFO);
  sender_param.sched_priority = max_priority;
  pthread_attr_setschedparam(&sender_attr, &sender_param);

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
printf("\n\r Receiver Thread Created with rc=%d\n\r", ret);
}
  printf("pthread join send\n");
  pthread_join(sender_thread, NULL);

  printf("pthread join receive\n");
  pthread_join(reciever_thread, NULL);
}