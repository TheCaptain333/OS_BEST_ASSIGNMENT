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
    
    int i = atoi(argv[1]);
    int MQ1 = atoi(argv[2]);
    int MQ3 = atoi(argv[3]);

    struct msg_process msg;
    msg.mtype = DEFAULT;
    msg.process_index = i;
    msg.process_pid = getpid();
    msgsnd(MQ1, &msg, sizeof(msg), 0); // add process i to ready queue
    kill(getpid(),SIGTSTP); // Send SIGTSTP to pause the process

    int j = 4;
    int num = argc - j;

    int referenceString[num];

    while (j < argc)
    {
        referenceString[j - 4] = atoi(argv[j]);
        j++;
    }

    struct msg_frame msg1;
    for(int i=0;i<num;i++){
        msg1.mtype = DEFAULT;
        msg1.page_number = referenceString[i];
        msg1.frame_number = -1;
        msg1.process_index = i;
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




    return 0;
}