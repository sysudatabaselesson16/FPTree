#include <leveldb/db.h>
#include <string>

#define KEY_LEN 8
#define VALUE_LEN 8
using namespace std;

const string workload = "../workloads";

const string load = workload + "220w-rw-50-50-load.txt"; // TODO: the workload_load filename
const string run  = workload + "220w-rw-50-50-run.txt"; // TODO: the workload_run filename

const string filePath = "";

const int READ_WRITE_NUM = 2200000; // TODO: how many operations

int main()
{        
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::WriteOptions write_options;
    // TODO: open and initial the levelDB
    leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(s.ok());
    uint64_t inserted = 0, queried = 0, t = 0;
    uint64_t* key = new uint64_t[2200000]; // the key and value are same
    bool* ifInsert = new bool[2200000]; // the operation is insertion or not
	FILE *ycsb_load, *ycsb_run; // the files that store the ycsb operations
	char *buf = NULL;
	size_t len = 0;
    struct timespec start, finish; // use to caculate the time
    double single_time; // single operation time

    printf("Load phase begins \n");
    // TODO: read the ycsb_load and store
    ycsb_load = fopen(load, "r"); 
    char op[7];//load the operations
    if (ycsb_load == NULL) return 0;
    /*int flag = 0, file_row = 0;
    while(!feof(ycsb_load)){ 
    flag = fgetc(ycsb_load);
    if(flag == '\n')
      file_row++;
    }
    file_row += 1;*/
    for (t = 0; t < 2200000; t++) {
        fscanf(ycsb_load, "%s %llu", op, &key[t]);
        if (strcmp(op,"INSERT") == 0){
        ifInsert[t] = true;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: load the workload in LevelDB
    for (t = 0; t < 2200000; t++) {//record the number of insert
        if (ifInsert[t]) {
	    inserted++;
	}
    //write operation
    leveldb::Status s = db->Put(write_options, leveldb::Slice((char*)&key[t], KEY_LEN), leveldb::Slice((char*)&key[t], VALUE_LEN));
    if (!s.ok()) printf("put failed!");
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 + (finish.tv_nsec - start.tv_nsec);

    printf("Load phase finishes: %d items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n", single_time / inserted);

	int operation_num = 2200000;
    inserted = 0;		

    // TODO:read the ycsb_run and store
    ycsb_run = fopen(run, "r"); 
    char op1[7];
    if (ycsb_run == NULL) return 0;
    for (t = 0; t < 2200000; t++) {
        ifInsert[t] = false;
        fscanf(ycsb_run, "%s %llu", op1, &key[t]);
        if (strcmp(op1,"INSERT") == 0){
        ifInsert[t] = true;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: operate the levelDB
    string val;//the par of read operation
    for (t = 0; t < 2200000; t++) {
        if (ifInsert[t]) {
	    inserted++;
	    leveldb::Status s1 = db->Put(write_options, leveldb::Slice((char*)&key[t], KEY_LEN), leveldb::Slice((char*)&key[t], VALUE_LEN));
            if (!s1.ok()) printf("Put failed!");
		}
        else {
	    leveldb::Status s2 = db->Get(leveldb::ReadOptions(), leveldb::Slice((char*)&key[t], KEY_LEN), &val);
	    if (!s2.ok()) printf("Get failed!");
        }
    }
	clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %d/%d items are inserted/searched\n", operation_num - inserted, inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);	
    return 0;
}
