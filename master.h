#define TRUE 1
#define FALSE 0
#define DEFAULT 1
#define TYPE_1 100
#define TYPE_2 200


typedef struct {

    int frame_number;
    int valid;
    int last_used_time;

}
PageTableEntry;

typedef struct {
    int max_len;
    int given_len;
    PageTableEntry * arr;

}
PageTable;

typedef struct {

    int len;
    int * arr;

}
referenceString;

struct msgbuf {
    long mtype;
    int process_index;
};

struct msg_process{
    long mtype;
    int process_index;
    int process_pid;

};

struct msg_frame{
    long mtype;
    int page_number;
    int frame_number;
    int process_index;

};


