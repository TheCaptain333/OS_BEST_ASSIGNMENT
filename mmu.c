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
#include <string.h>

int timestamp = 0;


void PageFaultHandler(int *FFL, int f, PageTable *PT, int page_number)
{

    for (int i = 0; i < f; i++)
    {

        if (FFL[i] == TRUE)
        {

            FFL[i] = FALSE;
            PT->arr[page_number].frame_number = i;
            PT->arr[page_number].valid = TRUE;
        }
    }

    int LRU_time = 1e9;
    int LRU_page = -1;
    int m_i = PT->given_len;

    for (int i = 0; i < m_i; i++)
    {

        if (PT->arr[i].valid == TRUE && PT->arr[i].last_used_time < LRU_time)
        {
            LRU_time = PT->arr[i].last_used_time;
            LRU_page = i;
        }

    }

    PT->arr[page_number].frame_number = PT->arr[LRU_page].frame_number;
    PT->arr[page_number].valid = TRUE;
    PT->arr[LRU_page].frame_number = -1;
    PT->arr[LRU_page].valid = FALSE;

}

int main(int argc, char *argv[])
{

    int MQ2, MQ3, SM1, SM2, f, k;
    MQ2 = atoi(argv[1]); // message queue to send to scheduler
    MQ3 = atoi(argv[2]); // message queue to receive from processes
    SM1 = atoi(argv[3]); // Process Tables
    SM2 = atoi(argv[4]); // Free Frame List
    f = atoi(argv[5]);   // Total number of frames
    k = atoi(argv[6]);   // Number of processes
    int *FFL = (int *)shmat(SM2, NULL, 0);

    struct msg_frame msg1;


    int metric[k][3];
    memset(metric,0,sizeof(metric));

    // first column represents number of page faults
    // second column represents number of invalid page references
    // third column represents total number of references



    while (TRUE)
    {
        msgrcv(MQ3, &msg1, sizeof(msg1), 0, 0);
        int process_index = msg1.process_index;
        int *shm_arr = (int *)shmat(SM1, NULL, 0); // shared memory segment for page table
        int shmid = shm_arr[process_index];

        PageTable *PT = (PageTable *)shmat(shmid, NULL, 0);
        int m = PT->max_len;
        int m_i = PT->given_len;
        PageTableEntry *arr = PT->arr;
        int page_number = msg1.page_number;

        printf("Page Reference- process %d page number %d\n", process_index, page_number);
        metric[process_index][2]++;


        if (page_number >= m_i)
        {
            
            
            // segmentation fault
            printf("\tInvalid page reference - process %d page number %d\n", process_index, page_number);
            metric[process_index][1]++;
            
            // will send -2 as frame number to process
            msg1.frame_number = -2;
            msgsnd(MQ3, &msg1, sizeof(msg1), 0);

            
            // will send TYPE_2 message to scheduler
            struct msg_process msg2;
            msg2.mtype = TYPE_2;
            msg2.process_index = process_index;
            msg2.process_pid = -1;
            msgsnd(MQ2, &msg2, sizeof(msg2), 0);
        }

        else if(page_number == -9){

            // process has completed execution
            // release all allocated frames
            for(int pg_no = 0; pg_no < PT->given_len; pg_no++){

                FFL[PT->arr[pg_no].frame_number]= TRUE;
                PT->arr[pg_no].valid = FALSE;
                PT->arr[pg_no].frame_number = -1;
                PT->arr[pg_no].last_used_time = 0;
            }

            // will send TYPE_2 message to scheduler
            struct msg_process msg2;
            msg2.mtype = TYPE_2;
            msg2.process_index = process_index;
            msg2.process_pid = -1;
            msgsnd(MQ2, &msg2, sizeof(msg2), 0);

        }


        else if (arr[page_number].valid == FALSE)
        {

            // page fault
            printf("\tPage fault - process %d page number %d\n", process_index, page_number);
            metric[process_index][0]++; 

            msg1.frame_number = -1;
            msgsnd(MQ3, &msg1, sizeof(msg1), 0);
            PageFaultHandler(FFL, f, PT, page_number);
            // will send TYPE_1 message to scheduler
            struct msg_process msg2;
            msg2.mtype = TYPE_1;
            msg2.process_index = process_index;
            msg2.process_pid = -1;
            msgsnd(MQ2, &msg2, sizeof(msg2), 0);
        }

        else
        {

            // page is present in memory
            // update frame number in message
            msg1.frame_number = arr[page_number].frame_number;
            // send response back
            msgsnd(MQ3, &msg1, sizeof(msg1), 0);
        }

        for (int i = 0; i < m_i; i++)
        {

            if (i == page_number)
                arr[i].last_used_time = 0;

            else
                arr[i].last_used_time--;
        }

        timestamp++;
    }
}