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



int main(int argc, char *argv[]) {

printf("PID = %d\n", getpid());


int MQ1 = atoi(argv[1]);
int MQ2 = atoi(argv[2]);

    while (1) {
        // Wait until there's a process in the ready queue
        printf("Wating for process...\n");
        struct msg_process msg;
        msgrcv(MQ1, &msg, sizeof(msg), 0, 0);
        printf("Received process %d from ready queue\n", msg.process_index);
        // Send signal to start execution
        sleep(5);
        printf("Sending process %d with PID = %d to execution\n", msg.process_index, msg.process_pid);
        kill(msg.process_pid, SIGCONT);

        // Wait for message from MMU
        msgrcv(MQ2, &msg, sizeof(msg), 0, 0);
        // now check mytpe and conclude what has happened with the process
        if(msg.mtype == TYPE_1){
        // if it is a TYPE_1 message, add process back to ready queue, change the mtype to DEFAULT and send to MQ1
        msg.mtype = DEFAULT;
        msgsnd(MQ1, &msg, sizeof(msg), 0);
        }
        
        struct msqid_ds buf;
        msgctl(MQ1, IPC_STAT, &buf);
        int num = buf.msg_qnum;

        if(num == 0){

            // notify the master to exit
            

        }


    }


















}