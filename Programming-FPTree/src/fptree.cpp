#include "fptree/fptree.h"

using namespace std;

// Initial the new InnerNode
InnerNode::InnerNode(const int &d, FPTree *const &t, bool _isRoot)
{
    // DOING
    this->tree = t;
    this->degree = d;
    this->isLeaf = false;
    this->isRoot = _isRoot;
    this->nKeys = 0;
    this->nChild = 0;
    this->keys = NULL;
    this->childrens = NULL;
}

// delete the InnerNode
InnerNode::~InnerNode()
{
    // DONIG
}

// binary search the first key in the innernode larger than input key
int InnerNode::findIndex(const Key &k)
{
    // TODO
    return 0;
}

// insert the node that is assumed not full
// insert format:
// ======================
// | key | node pointer |
// ======================
// WARNING: can not insert when it has no entry
void InnerNode::insertNonFull(const Key &k, Node *const &node)
{
    // TODO
}

// insert func
// return value is not NULL if split, returning the new child and a key to insert
KeyNode *InnerNode::insert(const Key &k, const Value &v)
{
    KeyNode *newChild = NULL;

    // 1.insertion to the first leaf(only one leaf)
    if (this->isRoot && this->nKeys == 0)
    {
        // TODO
        return newChild;
    }

    // 2.recursive insertion
    // TODO
    return newChild;
}

// ensure that the leaves inserted are ordered
// used by the bulkLoading func
// inserted data: | minKey of leaf | LeafNode* |
KeyNode *InnerNode::insertLeaf(const KeyNode &leaf)
{
    KeyNode *newChild = NULL;
    // first and second leaf insertion into the tree
    if (this->isRoot && this->nKeys == 0)
    {
        // TODO
        return newChild;
    }

    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    // TODO

    // next level is leaf, insert to childrens array
    // TODO

    return newChild;
}

KeyNode *InnerNode::split()
{
    KeyNode *newChild = new KeyNode();
    // right half entries of old node to the new node, others to the old node.
    // TODO

    return newChild;
}

// remove the target entry
// return TRUE if the children node is deleted after removement.
// the InnerNode need to be redistributed or merged after deleting one of its children node.
bool InnerNode::remove(const Key &k, const int &index, InnerNode *const &parent, bool &ifDelete)
{
    bool ifRemove = false;
    // only have one leaf
    // TODO

    // recursive remove
    // TODO
    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int &index, InnerNode *const &parent, InnerNode *&leftBro, InnerNode *&rightBro)
{
    // TODO
}

// merge this node, its parent and left brother(parent is root)
void InnerNode::mergeParentLeft(InnerNode *const &parent, InnerNode *const &leftBro)
{
    // TODO
}

// merge this node, its parent and right brother(parent is root)
void InnerNode::mergeParentRight(InnerNode *const &parent, InnerNode *const &rightBro)
{
    // TODO
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int &index, InnerNode *const &leftBro, InnerNode *const &parent)
{
    // TODO
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int &index, InnerNode *const &rightBro, InnerNode *const &parent)
{
    // TODO
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode *const &leftBro, const Key &k)
{
    // TODO
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode *const &rightBro, const Key &k)
{
    // TODO
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int &keyIdx, const int &childIdx)
{
    // TODO
}

// update the target entry, return true if the update succeed.
bool InnerNode::update(const Key &k, const Value &v)
{
    // TODO
    return false;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key &k)
{
    // TODO
    return MAX_VALUE;
}

// get the children node of this InnerNode
Node *InnerNode::getChild(const int &idx)
{
    // TODO
    return NULL;
}

// get the key of this InnerNode
Key InnerNode::getKey(const int &idx)
{
    if (idx < this->nKeys)
    {
        return this->keys[idx];
    }
    else
    {
        return MAX_KEY;
    }
}

// print the InnerNode
void InnerNode::printNode()
{
    cout << "||#|";
    for (int i = 0; i < this->nKeys; i++)
    {
        cout << " " << this->keys[i] << " |#|";
    }
    cout << "|"
         << "    ";
}

// print the LeafNode
void LeafNode::printNode()
{
    cout << "||";
    for (int i = 0; i < 2 * this->degree; i++)
    {
        if (this->getBit(i))
        {
            cout << " " << this->kv[i].k << " : " << this->kv[i].v << " |";
        }
    }
    cout << "|"
         << " ====>> ";
}

// new a empty leaf and set the valuable of the LeafNode
LeafNode::LeafNode(FPTree *t)
{
    // TODO
    this->tree = t;
    this->degree = t->degree;
    this->isLeaf = false;
    if (!pAllocator->getLeaf(this->pPointer, this->pmem_addr))
    {
        perror("cannot allocate a new lean\n");
        exit(1);
    }
    int tn = LEAF_DEGREE * 2;
    int bitArrNum = (tn + 7) / 8;
    this->bitmap = (Byte *)this->pmem_addr; //根据calLeafSize函数知道各部分在Leaf中的位置
    this->pNext = (PPointer *)(this->pmem_addr + bitArrNum);
    this->fingerprints = (Byte *)(this->pmem_addr + bitArrNum + sizeof(PPointer));
    this->kv = (KeyValue *)(this->pmem_addr + bitArrNum + sizeof(PPointer) + tn * sizeof(Byte));
    this->n = 0;
    this->prev = NULL;
    this->next = NULL;
    char ss[10];
    sprintf(ss, "%d", this->pPointer.fileId);
    this->filePath = DATA_DIR + ss;
    this->bitmapSize = (this->degree * 2 + 7) / 8;
}

// reload the leaf with the specific Persistent Pointer
// need to call the PAllocator
LeafNode::LeafNode(PPointer p, FPTree *t)
{
    // TODO
    this->tree = t;
    this->degree = t->degree;
    this->isLeaf = false;
    this->pPointer = p;
    if ((this->pmem_addr = getLeafPmemAddr(p)) == NULL)
    {
        perror("PPointer not valid\n");
        exit(1);
    }
    int tn = LEAF_DEGREE * 2;
    int bitArrNum = (tn + 7) / 8;
    this->bitmap = (Byte *)this->pmem_addr; //根据calLeafSize函数知道各部分在Leaf中的位置
    this->pNext = (PPointer *)(this->pmem_addr + bitArrNum);
    this->fingerprints = (Byte *)(this->pmem_addr + bitArrNum + sizeof(PPointer));
    this->kv = (KeyValue *)(this->pmem_addr + bitArrNum + sizeof(PPointer) + tn * sizeof(Byte));
    this->n = 0;
    this->prev = NULL;
    this->next = NULL;
    char ss[10];
    sprintf(ss, "%d", this->pPointer.fileId);
    this->filePath = DATA_DIR + ss;
    this->bitmapSize = (this->degree * 2 + 7) / 8;
}

LeafNode::~LeafNode()
{
    // TODO
    mapped_len = calLeafSize();
    pmem_unmap(this->pmem_addr, mapped_len);
}

// insert an entry into the leaf, need to split it if it is full
KeyNode *LeafNode::insert(const Key &k, const Value &v)
{
    KeyNode *newChild = NULL;
    // TODO
    if (this->n == this->degree * 2 - 1)
        newChild = this->split();
    this->insertNonFull(k, v);
    return newChild;
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key &k, const Value &v)
{
    // TODO
    int i;
    for (i = 0; i < this->degree * 2; i++)
        if (!this->getBit(i))
            break;
    this->bitmap[i / 8] |= (1 << (7 - i % 8));
    this->kv[i].k = k;
    this->kv[i].v = v;
    n++;
}

// split the leaf node
KeyNode *LeafNode::split()
{
    KeyNode *newChild = new KeyNode();
    // TODO
    Key mid_key = findSplitKey();

    LeafNode *temp = LeafNode();
    for (int i = 0; this->kv[i].k < mid_key; i++)
    {
        temp->insertNonFull(this->kv[i].k, this->kv[i].v);
        this->bitmap[i / 8] &= !(1 << (7 - i % 8));
        this->n--;
    }
    this->next = temp;
    temp->prev = this;
    newChild->key = mid_key;
    newChild->node = temp;
    return newChild;
}

// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find
Key LeafNode::findSplitKey()
{
    Key midKey = 0;
    // TODO
    sort(kv, kv + n, (const KeyValue &kv1, const KeyValue &kv2)[] { return kv1.k < kv2.k; });
    midKey = kv[n / 2].k;
    return midKey;
}

// get the targte bit in bitmap
// TIPS: bit operation
int LeafNode::getBit(const int &idx)
{
    // TODO
    return (this->bitmap[idx / 8] & (1 << (7 - idx % 8))) > 0;
    // return 0;
}

Key LeafNode::getKey(const int &idx)
{
    return this->kv[idx].k;
}

Value LeafNode::getValue(const int &idx)
{
    return this->kv[idx].v;
}

PPointer LeafNode::getPPointer()
{
    return this->pPointer;
}

// remove an entry from the leaf
// if it has no entry after removement return TRUE to indicate outer func to delete this leaf.
// need to call PAllocator to set this leaf free and reuse it
bool LeafNode::remove(const Key &k, const int &index, InnerNode *const &parent, bool &ifDelete)
{
    bool ifRemove = false;
    // TODO
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key &k, const Value &v)
{
    bool ifUpdate = false;
    // TODO
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key &k)
{
    // TODO
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero()
{
    // TODO
    return -1;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist()
{
    // TODO
    pmem_persist(pmem_addr, calLeafSize());
}

// call by the ~FPTree(), delete the whole tree
void FPTree::recursiveDelete(Node *n)
{
    if (n->isLeaf)
    {
        delete n;
    }
    else
    {
        for (int i = 0; i < ((InnerNode *)n)->nChild; i++)
        {
            recursiveDelete(((InnerNode *)n)->childrens[i]);
        }
        delete n;
    }
}

FPTree::FPTree(uint64_t t_degree)
{
    FPTree *temp = this;
    this->root = new InnerNode(t_degree, temp, true);
    this->degree = t_degree;
    bulkLoading();
}

FPTree::~FPTree()
{
    recursiveDelete(this->root);
}

// get the root node of the tree
InnerNode *FPTree::getRoot()
{
    return this->root;
}

// change the root of the tree
void FPTree::changeRoot(InnerNode *newRoot)
{
    this->root = newRoot;
}

void FPTree::insert(Key k, Value v)
{
    if (root != NULL)
    {
        root->insert(k, v);
    }
}

bool FPTree::remove(Key k)
{
    if (root != NULL)
    {
        bool ifDelete = false;
        InnerNode *temp = NULL;
        return root->remove(k, -1, temp, ifDelete);
    }
    return false;
}

bool FPTree::update(Key k, Value v)
{
    if (root != NULL)
    {
        return root->update(k, v);
    }
    return false;
}

Value FPTree::find(Key k)
{
    if (root != NULL)
    {
        return root->find(k);
    }
}

// call the InnerNode and LeafNode print func to print the whole tree
// TIPS: use Queue
void FPTree::printTree()
{
    // TODO
}

// bulkLoading the leaf files and reload the tree
// need to traverse leaves chain
// if no tree is reloaded, return FALSE
// need to call the PALlocator
bool FPTree::bulkLoading()
{
    // TODO
    return false;
}
