#include "fptree/fptree.h"
#include <leveldb/db.h>
#include <string>

#define KEY_LEN 8
#define VALUE_LEN 8
using namespace std;

const string workload = "../workloads"; // TODO: the workload folder filepath

const string load = workload + "220w-rw-50-50-load.txt"; // TODO: the workload_load filename
const string run  = workload + "220w-rw-50-50-run.txt"; // TODO: the workload_run filename

const int READ_WRITE_NUM = 2200000; // TODO: amount of operations

int main()
{        
    FPTree fptree(1028);
    uint64_t inserted = 0, queried = 0, t = 0;
    uint64_t* key = new uint64_t[2200000];
    uint64_t* temp = new uint64_t[2200000];
    bool* ifInsert = new bool[2200000];
	FILE *ycsb, *ycsb_read;
	char *buf = NULL;
	size_t len = 0;
    struct timespec start, finish;
    double single_time;

    printf("===================FPtreeDB===================\n");
    printf("Load phase begins \n");

    // TODO: read the ycsb_load
    ycsb = fopen("../workloads/220w-rw-50-50-load.txt", "r");
    char op[7];//load the operations
    if (ycsb == NULL) return 0;
    for (t = 0; t < 2200000; t++) {
        fscanf(ycsb, "%s %llu", op, &key[t]);
        if (strcmp(op,"INSERT") == 0){
            ifInsert[t] = true;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: load the workload in the fptree
    for (t = 0; t < 2200000; t++) {
        fptree.insert(key[t], key[t]);
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 + (finish.tv_nsec - start.tv_nsec);
    printf("Load phase finishes: %d items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n", single_time / inserted);

	printf("Run phase begins\n");

	int operation_num = 0;
    inserted = 0;		
    // TODO: read the ycsb_run
    ycsb_read = fopen("../workloads/220w-rw-50-50-run.txt", "r"); 
    char op1[7];
    if (ycsb_read == NULL) return 0;
    for (t = 0; t < 2200000; t++) {
        ifInsert[t] = false;
        fscanf(ycsb_read, "%s %llu", op1, &key[t]);
        if (strcmp(op1,"INSERT") == 0){
        ifInsert[t] = true;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: operate the fptree
    for (t = 0; t < 2200000; t++) {
        if (ifInsert[t]) {
	    inserted++;
	    fptree.insert(key[t], key[t]);
	}
        else {
            temp[t] = fptree.find(key[t]);
        }
    }
	clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %d/%d items are inserted/searched\n", inserted, operation_num - inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);	
    
    // LevelDB
    printf("===================LevelDB====================\n");
    const string filePath = ""; // data storing folder(NVM)

    memset(key, 0, 2200000);
    memset(ifInsert, 0, 2200000);

    leveldb::DB* db;
    leveldb::Options options;
    leveldb::WriteOptions write_options;
    // TODO: initial the levelDB
    leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(s.ok());
    inserted = 0;
    printf("Load phase begins \n");
    // TODO: read the ycsb_read
    ycsb = fopen("../workloads/220w-rw-50-50-load.txt", "r");
    char op2[7];//load the operations
    if (ycsb == NULL) return 0;
    for (t = 0; t < 2200000; t++) {
        fscanf(ycsb, "%s %llu", op2, &key[t]);
        if (strcmp(op2,"INSERT") == 0){
            ifInsert[t] = true;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    // TODO: load the levelDB
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

	printf("Run phase begin\n");
	operation_num = 0;
    inserted = 0;		
    // TODO: read the ycsb_run
    ycsb_read = fopen("../workloads/220w-rw-50-50-run.txt", "r"); 
    char op3[7];
    if (ycsb_read == NULL) return 0;
    for (t = 0; t < 2200000; t++) {
        ifInsert[t] = false;
        fscanf(ycsb_read, "%s %llu", op3, &key[t]);
        if (strcmp(op3,"INSERT") == 0){
        ifInsert[t] = true;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: run the workload_run in levelDB
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
    fclose(ycsb_read);
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %d/%d items are inserted/searched\n", inserted, operation_num - inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);	
    return 0;
}
