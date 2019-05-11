#include "utility/p_allocator.h"
#include <iostream>
using namespace std;

// the file that store the information of allocator
const string P_ALLOCATOR_CATALOG_NAME = "p_allocator_catalog";
// a list storing the free leaves
const string P_ALLOCATOR_FREE_LIST = "free_list";

size_t mapped_len;

PAllocator *PAllocator::pAllocator = new PAllocator();

PAllocator *PAllocator::getAllocator()
{
    if (PAllocator::pAllocator == NULL)
    {
        PAllocator::pAllocator = new PAllocator();
    }
    return PAllocator::pAllocator;
}

/* data storing structure of allocator
   In the catalog file, the data structure is listed below
   | maxFileId(8 bytes) | freeNum = m | treeStartLeaf(the PPointer) |
   In freeList file:
   | freeList{(fId, offset)1,...(fId, offset)m} |
*/
PAllocator::PAllocator()
{
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in | ios::binary);
    ifstream freeListFile(freeListPath, ios::in | ios::binary);
    // judge if the catalog exists
    if (allocatorCatalog.is_open() && freeListFile.is_open())
    {
        // exist
        allocatorCatalog.read((char *)&maxFileId, sizeof(maxFileId));
        allocatorCatalog.read((char *)&freeNum, sizeof(freeNum));
        allocatorCatalog.read((char *)&startLeaf.fileId, sizeof(startLeaf.fileId));
        allocatorCatalog.read((char *)&startLeaf.offset, sizeof(startLeaf.offset));
        freeList.resize(freeNum);
        for (int i = 0; i < freeNum; i++)
        {
            freeListFile.read((char *)&(freeList[i].fileId), sizeof(freeList[i].fileId));
            freeListFile.read((char *)&(freeList[i].offset), sizeof(freeList[i].offset));
        }
        allocatorCatalog.close();
        freeListFile.close();
        // TODO
    }
    else
    {
        // not exist, create catalog and free_list file, then open.
        if (!allocatorCatalog.is_open())
        {
            ofstream f(allocatorCatalogPath, ios::out | ios::binary);
            if (!f)
                cout << "Fail in creating file.\n";
            else
            {
                maxFileId = 1;
                freeNum = 0;
                startLeaf.fileId = 1;
                startLeaf.offset = LEAF_GROUP_HEAD + (LEAF_GROUP_AMOUNT - 1) * calLeafSize();
                f.write((char *)&maxFileId, sizeof(maxFileId));
                f.write((char *)&freeNum, sizeof(freeNum));
                f.write((char *)&startLeaf, sizeof(startLeaf));
                f.close();
            }
        }
        else if (!freeListFile.is_open())
        {
            ofstream f(freeListPath, ios::out | ios::binary);
            if (!f)
                cout << "Fail in creating file.\n";
            else
                f.close();
        }
        // TODO
    }
    this->initFilePmemAddr();
}

PAllocator::~PAllocator()
{
    persistCatalog(); //在操作完毕后要把数据结构写入NVM,方便下次取用
    for (int i = 1; i < maxFileId; i++)
    {
        pmem_unmap(fId2PmAddr[i], mapped_len);
    }
    fId2PmAddr.clear();
    freeList.clear();
    maxFileId = 1;
    freeNum = 0;
    pAllocator = NULL;
    //由于该项目的单例模式中并无法将pAllocator delete所以只把它置为NULL
    // TODO
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr()
{
    int PMEM_LEN = LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(); //pmem应该分配大小,也就是leaf_group大小
    for (int i = 1; i < maxFileId; i++)
    {
        char *pmemaddr;
        int is_pmem;
        char ss[10];
        sprintf(ss, "%d", i); //数字转字符串构成数据文件名
        string data_path = DATA_DIR + ss;
        if ((pmemaddr = (char *)pmem_map_file(data_path.c_str(), PMEM_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == NULL)
        {
            perror("pmem_map_file");
            exit(1);
        }
        fId2PmAddr[i] = pmemaddr;           //保存虚拟地址跟fileId的映射
    }
    // TODO
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char *PAllocator::getLeafPmemAddr(PPointer p)
{
    // TODO
    if (ifLeafExist(p))
    {
        return (fId2PmAddr[p.fileId] + p.offset);
    }
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return
bool PAllocator::getLeaf(PPointer &p, char *&pmem_addr)
{
    // TODO
    if (freeNum >= 1 || newLeafGroup()) //没freeNum就分配页
    {
        p.offset = freeList[freeNum - 1].offset;
        p.fileId = freeList[freeNum - 1].fileId;

        char *temp = fId2PmAddr[p.fileId];

        temp[sizeof(uint64_t) + (p.offset - LEAF_GROUP_HEAD) / calLeafSize()] = 1; //将leaf_group中对应leaf的bitmap位置置为1,标志used, 这里计算bitmap位置的式子有点奇怪是因为offset内容是直接跟leaf在leaf_group位置挂钩的,要先算它是第几个leaf,再加上sizeof(uint64_t)
        ((uint64_t *)temp)[0] += 1;                                                //leaf_group usednum++
        pmem_addr = getLeafPmemAddr(p); //最终要的是叶的虚拟地址
        freeNum--;
        freeList.pop_back();
        return true;
    }
    return false;
}

bool PAllocator::ifLeafUsed(PPointer p)
{
    // TODO
    if (ifLeafExist(p))
    {
        char *temp = fId2PmAddr[p.fileId];
        return temp[sizeof(uint64_t) + (p.offset - LEAF_GROUP_HEAD) / calLeafSize()] == 1;
    }

    return false;
}

bool PAllocator::ifLeafFree(PPointer p)
{
    // TODO
    for (int i = 0; i < freeList.size(); i++)
        if (p == freeList[i])
            return true;

    return false;
}

// judge whether the leaf with specific PPointer exists.
bool PAllocator::ifLeafExist(PPointer p)
{
    // TODO
    return p.fileId > 0 && p.fileId < maxFileId && p.offset >= LEAF_GROUP_HEAD && (p.offset - LEAF_GROUP_HEAD) / calLeafSize() < LEAF_GROUP_AMOUNT && ((p.offset - LEAF_GROUP_HEAD) % calLeafSize() == 0);
    //判断fileId是否合法,offset是否合法,要注意offset合法应该是在leaf的初始位置
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p)
{
    // TODO
    if (ifLeafExist(p) && ifLeafUsed(p) && !ifLeafFree(p))
    {
        char *temp = fId2PmAddr[p.fileId];
        temp[sizeof(uint64_t) + (p.offset - LEAF_GROUP_HEAD) / calLeafSize()] = 0;
        ((uint64_t *)temp)[0] -= 1;
        freeList.push_back(p);
        freeNum++;
        return true;
    }
    return false;
}

bool PAllocator::persistCatalog()
{
    // TODO
    //将数据结构写入NVM
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ofstream allocatorCatalog(allocatorCatalogPath, ios::out | ios::binary);
    ofstream freeListFile(freeListPath, ios::out | ios::binary);
    allocatorCatalog.write((char *)&maxFileId, sizeof(maxFileId));
    allocatorCatalog.write((char *)&freeNum, sizeof(freeNum));
    allocatorCatalog.write((char *)&startLeaf.fileId, sizeof(startLeaf.fileId));
    allocatorCatalog.write((char *)&startLeaf.offset, sizeof(startLeaf.offset));
    for (int i = 0; i < freeNum; i++)
    {
        freeListFile.write((char *)&(freeList[i].fileId), sizeof(freeList[i].fileId));
        freeListFile.write((char *)&(freeList[i].offset), sizeof(freeList[i].offset));
    }
    allocatorCatalog.close();
    freeListFile.close();
    return false;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup
bool PAllocator::newLeafGroup()
{
    // TODO
    char ss[10];
    sprintf(ss, "%d", maxFileId);
    string newpath = DATA_DIR + ss;

    ofstream out(newpath, ios::binary | ios::out); //只是用来创建文件的,
    if (!out)
    {
        cout << "Cann't create new file." << endl;
        return false;
    }
    else
    {

        out.close();
        int PMEM_LEN = LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize();

        char *pmemaddr;
        int is_pmem;
        size_t mapped_len;
        if ((pmemaddr = (char *)pmem_map_file(newpath.c_str(), PMEM_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == NULL)
        {
            cout << "pmem_map_file" << endl;
            return false;
        }

        fId2PmAddr[maxFileId] = pmemaddr;
        pmem_memset(pmemaddr, 0, sizeof(Byte), mapped_len);
        pmem_persist(pmemaddr, mapped_len);
        freeNum += LEAF_GROUP_AMOUNT;
        for (int i = 0; i < LEAF_GROUP_AMOUNT; i++)
        {
            PPointer p;
            p.fileId = maxFileId;
            p.offset = LEAF_GROUP_HEAD + i * calLeafSize();
            freeList.push_back(p);
        }
        maxFileId++;
        return true;
    }
    return false;
}
