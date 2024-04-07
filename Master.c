#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "master.h"

int k, m, f;
float p = 0.1; // probability of segmentation fault

// segmentation fault is when the process is trying to access a page it doesn't have access to
// page fault is when process is trying to access a page but it is not currently in memory

int * getReferenceString(int str_len, int max_pages, float prob) {

    int * arr = (int * ) malloc(str_len * sizeof(int));

    for (int i = 0; i < str_len; i++) {

        arr[i] = rand() % max_pages;
        if ((float) random() / RAND_MAX < p) {
            int offset = random(); // Generate a random offset
            arr[i] += offset;
        }

    }
    
    
    return arr;
    
}

int main() {
    int shmid, msgid;
    srand(time(NULL));

    // k keys required for page table of each of the k processes
    // one more key required for Free Frame List
    // 3 more keys required for 3 message queues

    printf("Enter the number of processes(k) :");
    scanf("%d", & k);
    printf("Enter the maximum number of pages required per process(m) :");
    scanf("%d", & m);
    printf("Enter the total number of frames(f) :");
    scanf("%d", & f);


    key_t keys[k+4];
    for (int i = 0; i < k + 4; i++) keys[i] = ftok(".", i);

    PageTable* PT[k];

    // create a shared memory segment for each page table entry

    key_t key = ftok(".",k+4);
    int PT_shmid = shmget(key, k*sizeof(int), 0666 | IPC_CREAT); // SM1
    int * process_shmid = (int* )shmat(PT_shmid,0,0);
    for (int i = 0; i < k; i++) {

        // PT[i].n = m;
        // shmid = process_shmid[i] = shmget(keys[i], m * sizeof(PageTableEntry), 0666 | IPC_CREAT);
        // PT[i].arr = (PageTableEntry * ) shmat(shmid, 0, 0);

        process_shmid[i] = shmget(keys[i], 2*sizeof(int) + sizeof(PageTableEntry*), 0666 | IPC_CREAT);
        PT[i] = (PageTable*) shmat(process_shmid[i], 0, 0);
        PT[i]->given_len = m;
        PT[i]->arr = (PageTableEntry*) malloc(m * sizeof(PageTableEntry));

        for (int j = 0; j < m; j++) {
            PT[i]->arr[j].frame_number = -1;
            PT[i]->arr[j].valid = FALSE;
            PT[i]->arr[j].last_used_time = 0;
        }

    }

    int FFL_shmid; //SM2
    shmid = FFL_shmid = shmget(keys[k], f * sizeof(int), 0666 | IPC_CREAT);
    int * FFL = (int * ) shmat(shmid, 0, 0);

    for (int j = 0; j < f; j++) FFL[j] = TRUE;

    int msgid1 = msgget(keys[k + 1], IPC_CREAT | 0666); //MQ1
    int msgid2 = msgget(keys[k + 2], IPC_CREAT | 0666); //MQ2
    int msgid3 = msgget(keys[k + 3], IPC_CREAT | 0666); //MQ3

    char MQ1[32], MQ2[32], MQ3[32], SM1[32], SM2[32], frames[32], kprocs[32];
    sprintf(MQ1, "%d", msgid1);
    sprintf(MQ2, "%d", msgid2);
    sprintf(MQ3, "%d", msgid3);
    sprintf(SM1, "%d", PT_shmid);
    sprintf(SM2, "%d", FFL_shmid);
    sprintf(frames, "%d", f);
    sprintf(kprocs, "%d", k);


    int pages_per_table[k];

    if (fork() == 0) {
    // child process to execute scheduler using exec
	// pass MQ1 and MQ2 to scheduler
    execlp("./sched","./sched",MQ1,MQ2,NULL);
    } else {
        if (fork() == 0) {

            // child process to execute MMU using exec
            // pass MQ2,MQ3,SM1,SM2 to MMU
            // execlp("./mmu","./mmu",MQ2,MQ3,SM1,SM2,frames,NULL);
            execlp("xterm", "xterm", "-T","MMU", "-e", "./mmu",MQ2,MQ3,SM1,SM2,frames,kprocs,NULL);
        } else {

            for (int i = 0; i < k; i++) {
                int process_pid = fork();
                if (process_pid == 0) {

                    // child process to execute p_i using exec
                    // generate random m_i between 1 and m and assign it to pages_per_table
                    int m_i = (rand() % m) + 1;
                    pages_per_table[i] = m_i;
                    PT[i]->max_len = m_i;
                    // generate reference string
                    referenceString R_i;
                    R_i.len = (rand() % (9 * m_i)) + 2 * m_i;
                    R_i.arr = getReferenceString(R_i.len, m_i, p);
                    
                    // send index i,MQ1, MQ3 and reference string to the process
                    // number of arguments is 4 + length of reference string

                    int len = R_i.len+5;

                    char* arg_list[len];
                    sprintf(arg_list[0], "%s", "./process");
                    sprintf(arg_list[1], "%d", i);
                    sprintf(arg_list[2], "%s", MQ1);
                    sprintf(arg_list[3], "%s", MQ3);

                    for(int j=1;j < R_i.len;j++){
                        sprintf(arg_list[j+3], "%d", R_i.arr[j]);
                    }

                    arg_list[len-1] = NULL;
					execvp("./process",arg_list);

                }

                else {

                    usleep(250000);
                    
                }

            }

        }

    }

    // wait till scheduler notifies all processes are done	

    while(TRUE){

        
    }
    // terminate scheduler
    // terminate MMU

    return 0;
}
