#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "master.h"
#include <signal.h>

int main(int argc, char *argv[])
{
    printf("PID = %d\n", getpid());
    int i = atoi(argv[1]);
    int MQ1 = atoi(argv[2]);
    int MQ3 = atoi(argv[3]);
    int ref_shmid = atoi(argv[4]);

    char output_name[50]={0};
    char error_name[50]={0};
    sprintf(output_name,"process_%d_output.txt",i);
    sprintf(error_name,"process_%d_error.txt",i);

    // FILE* fp1 = freopen(output_name,"w",stdout);
    // FILE* fp2 = freopen(error_name,"w",stderr);



    struct msg_process msg;
    msg.mtype = DEFAULT;
    msg.process_index = i;
    msg.process_pid = getpid();
    
    msgsnd(MQ1, &msg, sizeof(msg), 0); // add process i to ready queue
    printf("Sent process %d with PID = %d to ready queue\n", i, getpid());
    pause();
    printf("Successfully woke up again\n");

    sleep(5);
    printf("PID = %d\n", getpid());
    referenceString* pt = (referenceString*)shmat(ref_shmid, NULL, 0);
    int num = pt->len;
    int referenceString[num];

    printf("num = %d\n", num);

    printf("Reference string : ");
    for(int i=0;i<num;i++) {
        int temp;
        temp = referenceString[i] = pt->arr[i];
        printf("%d ",temp);
    
    }

    printf("\n");
    struct msg_frame msg1;
    for(int i=0;i<num;i++){
        msg1.mtype = DEFAULT;
        msg1.page_number = referenceString[i];
        msg1.frame_number = -1;
        msg1.process_index = i;

        printf("Sent page number %d to MMU\n", msg1.page_number);
        msgsnd(MQ3, &msg1, sizeof(msg1), 0);
        usleep(1000);
        msgrcv(MQ3, &msg1, sizeof(msg1), 0, 0);

        if(msg1.frame_number == -2){

        // segmentation fault
        exit(0);

        }

        else if(msg1.frame_number == -1){

        // page fault
        int current_page = referenceString[i];
        kill(getpid(),SIGTSTP); // Send SIGTSTP to pause the process

        }

        // parsing the current frame number

    }


    msg1.mtype = DEFAULT;
    msg1.page_number = -9;
    msg1.frame_number = -1;
    msg1.process_index = i;
    msgsnd(MQ3, &msg1, sizeof(msg1), 0);


    // fclose(fp1);
    // fclose(fp2);

    return 0;
}