#include "fptree/fptree.h"
#include <algorithm>
#include <queue>

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
    this->keys = new Key[d * 2 + 1];         //先分配一个足够大的空间方便后面直接使用
    this->childrens = new Node *[d * 2 + 2]; //同上
}

// delete the InnerNode
InnerNode::~InnerNode()
{
    // DONIG
    delete this->childrens;
    delete this->keys;
}

// binary search the first key in the innernode larger than input key
int InnerNode::findIndex(const Key &k)
{
    // TODO
    //二分查找,结点key>k的在它的左孩子找,key<=k在右孩子找
    int from = 0, to = nKeys - 1;
    if (k < this->keys[0])
        return 0;
    if (k >= this->keys[to])
        return to + 1;
    while (from <= to)
    {
        int mid = (from + to) / 2;
        if (keys[to - 1] < k)
            return to;
        if (keys[mid] < k)
            from = mid;
        else if (keys[mid] > k)
            to = mid;
        else if (keys[mid] == k)
            return mid + 1;
    }
}

// insert the node that is assumed not full
// insert format:
// ======================
// | key | node pointer |
// ======================
// WARNING: can not insert when it has no entry
void InnerNode::insertNonFull(const Key &k, Node *const &node)
{
    // DOING
    if (nChild == 0)
    {
        this->childrens[nChild++] = node;
        return;
    }
    KeyNode *newChild = NULL;
    int temp = findIndex(k);
    for (int i = this->nKeys; i > temp; i--)
    {
        keys[i] = keys[i - 1];
        childrens[i + 1] = childrens[i];
    }
    this->nKeys++;
    this->nChild++;
    this->keys[temp] = k;
    this->childrens[temp + 1] = node;
    return;
}

// insert func
// return value is not NULL if split, returning the new child and a key to insert
KeyNode *InnerNode::insert(const Key &k, const Value &v)
{
    KeyNode *newChild = NULL;
    // 1.insertion to the first leaf(oenly one laf)
    if (this->isRoot && this->nKeys == 0)
    {
        // DOING
        if (nChild == 0)
        {
            LeafNode *temp = new LeafNode(this->tree);
            this->childrens[nChild++] = (Node *)temp;
        }
        newChild = this->childrens[nChild - 1]->insert(k, v);
        if (newChild != NULL)
        {
            this->keys[nKeys++] = newChild->key;
            this->childrens[nChild++] = newChild->node;
        }
        return newChild;
    }
    // 2.recursive insertion
    // DOING
    int temp = findIndex(k);
    newChild = this->childrens[temp]->insert(k, v);
    if (newChild != NULL)
        insertNonFull(newChild->key, newChild->node);
    newChild = NULL;
    if (nKeys == 2 * this->degree + 1)
    {
        if (this->isRoot)
        {
            this->isRoot = false;
            newChild = split();
            InnerNode *temp = new InnerNode(this->degree, this->tree, true);
            temp->insertNonFull(k, (Node *)this);
            temp->insertNonFull(newChild->key, newChild->node);
            this->tree->changeRoot(temp);
        }
        else
        {
            newChild = split();
        }
    }

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
        if (nChild == 0)
        {
            this->childrens[this->nChild++] = leaf.node;
        }
        else
        {
            this->childrens[this->nChild++] = leaf.node;
            this->keys[this->nKeys++] = leaf.key;
        }
        return newChild;
    }
    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    // TODO
    if (!this->childrens[this->nChild - 1]->ifLeaf())
    {
        InnerNode *kk = (InnerNode *)this->childrens[this->nChild - 1];
        newChild = kk->insertLeaf(leaf);
        if (newChild != NULL)
        {
            this->childrens[this->nChild++] = newChild->node;
            this->keys[this->nKeys++] = newChild->key;
        }
    }
    else
    {
        this->childrens[this->nChild++] = leaf.node;
        this->keys[this->nKeys++] = leaf.key;
    }
    newChild = NULL;
    if (nKeys == 2 * this->degree + 1)
    {
        if (this->isRoot)
        {
            this->isRoot = false;
            newChild = split();
            InnerNode *temp = new InnerNode(this->degree, this->tree, true);
            temp->insertNonFull(leaf.key, (Node *)this);
            temp->insertNonFull(newChild->key, newChild->node);
            this->tree->changeRoot(temp);
        }
        else
        {
            newChild = split();
        }
    }

    // next level is leaf, insert to childrens array
    // TODO

    return newChild;
}

KeyNode *InnerNode::split()
{
    KeyNode *newChild = new KeyNode();
    // right half entries of old node to the new node, others to the old node.
    // DOING
    InnerNode *temp = new InnerNode(this->degree, this->tree, this->isRoot);
    newChild->key = this->keys[this->degree];
    newChild->node = (Node *)temp;
    this->nKeys = this->degree;
    this->nChild = this->degree + 1;
    temp->nKeys = 0;
    temp->nChild = 0;
    for (int i = 0; i <= this->degree; i++)
    {
        temp->insertNonFull(this->keys[i + this->degree], this->childrens[i + this->degree + 1]);
    }
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
    if (nChild == 1) // 如果只有一个叶子节点(根节点才有的情况)
    {
        bool ifD = false;
        ifRemove = this->remove(k, 0, this, ifD); // 传递是否删除成功
        if (ifD)
        {
            childrens[0] = NULL;
            nChild--;
        }
    }
    else
    {
        bool ifD = false;
        int temp = findIndex(k);
        ifRemove = this->remove(k, temp, this, ifD); // 传递是否删除成功
        if (ifD)                                     // 若孩子节点需要删除
        {
            if (temp == nKeys - 1)
            {
                removeChild(temp - 1, temp);
            }
            else
            {
                removeChild(temp, temp);
            }
            if (nKeys < degree && !isRoot)
            {
                InnerNode *lb;
                InnerNode *rb;
                this->getBrother(temp, parent, lb, rb);
                if (parent->getIsRoot() && parent->getChildNum() == 2)
                {
                    if (rb != NULL)
                    {
                        mergeParentRight(parent, rb);
                        delete rb;
                        rb = NULL;
                        ifDelete = true;
                    }
                    if (lb != NULL)
                    {
                        mergeParentLeft(parent, lb);
                        delete lb;
                        lb = NULL;
                        ifDelete = true;
                    }
                }
                else
                {
                    if (rb != NULL && rb->getKeyNum() > degree)
                    {
                        this->redistributeRight(index, rb, parent);
                    }
                    else if (lb != NULL && lb->getKeyNum() > degree)
                    {
                        this->redistributeLeft(index - 1, lb, parent);
                    }
                    else if (rb != NULL)
                    {
                        this->mergeRight(rb, parent->getKey(index));
                        ifDelete = true;
                    }
                    else if (lb != NULL)
                    {
                        this->mergeLeft(lb, parent->getKey(index - 1));
                        ifDelete = true;
                    }
                }
            }
        }
    }

    // recursive remove
    // TODO
    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int &index, InnerNode *const &parent, InnerNode *&leftBro, InnerNode *&rightBro)
{
    // TODO
    if (index != 0) // 如果不是最左边
        leftBro = (InnerNode *)parent->getChild(index - 1);
    else
        leftBro = NULL;

    if (index < parent->getChildNum() - 1) // 如果不是最右边
        rightBro = (InnerNode *)parent->getChild(index + 1);
    else
        rightBro = NULL;
}

// merge this node, its parent and left brother(parent is root)
void InnerNode::mergeParentLeft(InnerNode *const &parent, InnerNode *const &leftBro)
{
    // TODO
    int i;
    for (i = this->nKeys + leftBro->nKeys + 1; i >= leftBro->nKeys + 2; i--)
        parent->keys[i] = this->keys[i - leftBro->nKeys - 2];
    parent->keys[i--] = parent->keys[0];
    for (; i >= 2; i--)
        parent->keys[i] = leftBro->keys[i - 1];
    parent->keys[0] = leftBro->keys[0];
    parent->nKeys = this->nKeys + leftBro->nKeys + 2;
    for (i = this->nChild + leftBro->nChild; i >= leftBro->nChild + 1; i--)
        parent->childrens[i] = this->childrens[i - leftBro->nChild - 1];
    for (; i >= 2; i--)
        parent->childrens[i] = leftBro->childrens[i - 1];
    parent->childrens[0] = leftBro->childrens[0];
    parent->nChild = this->nChild + leftBro->nChild + 1;
}

// merge this node, its parent and right brother(parent is root)
void InnerNode::mergeParentRight(InnerNode *const &parent, InnerNode *const &rightBro)
{
    // TODO
    //mergeParentRight将当前结点和右兄弟merge到根结点,同时保留当前结点到根节点的对应位置,使得最终该结点可以被delete
    int i;
    for (i = this->nKeys + rightBro->nKeys + 1; i >= this->nKeys + 2; i--)
        parent->keys[i] = rightBro->keys[i - this->nKeys - 2];
    parent->keys[i--] = parent->keys[0];
    for (; i >= 1; i--)
        parent->keys[i] = this->keys[i - 1];
    parent->nKeys = this->nKeys + rightBro->nKeys + 2;
    for (i = this->nChild + rightBro->nChild; i >= this->nChild + 1; i--)
        parent->childrens[i] = rightBro->childrens[i - this->nChild - 1];
    for (; i >= 1; i--)
        parent->childrens[i] = this->childrens[i - 1];
    parent->nChild = this->nChild + rightBro->nChild + 1;
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int &index, InnerNode *const &leftBro, InnerNode *const &parent)
{
    // TODO
    for (int i = this->nKeys; i > 0; i--)
        this->keys[i] = this->keys[i - 1];
    this->keys[0] = parent->keys[index];
    parent->keys[index] = leftBro->keys[--leftBro->nKeys];
    for (int i = this->nChild; i > 0; i--)
        this->childrens[i] = this->childrens[i - 1];
    this->childrens[0] = leftBro->childrens[--leftBro->nChild];
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int &index, InnerNode *const &rightBro, InnerNode *const &parent)
{
    // TODO
    this->keys[this->nKeys++] = parent->keys[index];
    this->childrens[this->nChild++] = rightBro->children[0];
    parent->keys[index] = rightBro->keys[0];
    rightBro->nKeys--;
    rightBro->nChild--;
    for (int i = 0; i < rightBro->nKeys; i++)
        rightBro->keys[i] = rightBro->keys[i + 1];
    for (int i = 0; i < rightBro->nChild; i++)
        rightBro->childrens[i] = rightBro->childrens[i + 1];
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode *const &leftBro, const Key &k)
{
    // TODO
    leftBro->keys[leftBro->nKeys] = k;
    int i;
    for (i = leftBro->nKeys + 1; i < leftBro->nKeys + this->nKeys + 1; i++)
        leftBro->keys[i] = this->keys[i - leftBro->nKeys - 1];
    for (i = leftBro->nChild; i < leftBro->nChild + this->nChild; i++)
        leftBro->childrens[i] = this->childrens[i - leftBro->nChild];
    leftBro->nKeys += this->nKeys + 1;
    leftBro->nChild += this->nChild;
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode *const &rightBro, const Key &k)
{
    // TODO
    int i;
    for (i = this->nKeys + rightBro->nKeys; i > this->nKeys; i--)
        rightBro->keys[i] = rightBro->keys[i - this->nKeys - 1];
    rightBro->keys[i--] = k;
    for (; i >= 0; i--)
        rightBro->keys[i] = this->keys[i];
    for (i = this->nChild + rightBro->nChild - 1; i >= this->nChild; i--)
        rightBro->childrens[i] = rightBro->childrens[i - this->nChild];
    for (; i >= 0; i--)
        rightBro->childrens[i] = this->childrens[i];
    rightBro->nKeys += this->nKeys + 1;
    rightBro->nChild += this->nChild;
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int &keyIdx, const int &childIdx)
{
    // TODO
    delete this->childrens[childIdx];
    for (int i = keyIdx; i < this->nKeys - 1; i++)
        this->keys[i] = this->keys[i + 1];
    for (int i = childIdx; i < this->nChild - 1; i++)
        this->childrens[i] = this->childrens[i + 1];
    this->nChild--;
    this->nKeys--;
}

// update the target entry, return true if the update succeed.
bool InnerNode::update(const Key &k, const Value &v)
{
    // TODO
    int pos = findIndex(k);
    if (this->childrens[pos] != NULL)
    {
        return this->childrens[pos]->update(k, v);
    }
    return false;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key &k)
{
    // TODO
    InnerNode *temp = this;
    while (1)
    {
        int idx = temp->findIndex(k);    //idx是接下来查找的孩子序号
        if (temp->getChild(idx) == NULL) //不存在返回MAX_VALUE
            return MAX_VALUE;
        if (temp->getChild(idx)->ifLeaf()) //存在且是叶子结点,在叶子结点中找
            return temp->getChild(idx)->find(k);
        else
            temp = (InnerNode *)temp->getChild(idx); //存在且不是叶子结点,递归往下找
    }
    return MAX_VALUE;
}

// get the children node of this InnerNode
Node *InnerNode::getChild(const int &idx)
{
    // TODO
    if (idx < 0 || idx >= this->nChild)
        return NULL;
    return this->childrens[idx];
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
    this->degree = LEAF_DEGREE; //叶子结点的degree是固定好的LEAF_DEGREE
    this->isLeaf = true;
    if (!PAllocator::getAllocator()->getLeaf(this->pPointer, this->pmem_addr)) //分配叶子的NVM地址
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
    this->degree = LEAF_DEGREE;
    this->isLeaf = true;
    this->pPointer = p;
    if ((this->pmem_addr = PAllocator::getAllocator()->getLeafPmemAddr(p)) == NULL) //根据p找到对应NVM地址
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
    int mapped_len = calLeafSize();
    pmem_unmap(this->pmem_addr, mapped_len);
}

// insert an entry into the leaf, need to split it if it is full
KeyNode *LeafNode::insert(const Key &k, const Value &v)
{
    KeyNode *newChild = NULL;
    // TODO
    this->insertNonFull(k, v);       //先插入再判定满不满
    if (this->n == this->degree * 2) //满则拆
        newChild = this->split();

    return newChild;
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key &k, const Value &v)
{
    // TODO
    int i = findFirstZero(); //先找到空槽位
    this->bitmap[i / 8] |= (1 << (7 - i % 8));
    this->kv[i].k = k;
    this->kv[i].v = v;
    this->fingerprints[i] = keyHash(k);
    n++;
}

// split the leaf node
KeyNode *LeafNode::split()
{
    KeyNode *newChild = new KeyNode();
    // TODO
    Key mid_key = findSplitKey();

    LeafNode *temp = new LeafNode(this->tree);
    int i;
    for (i = 0; this->kv[i].k < mid_key; i++)
    {
        this->fingerprints[i] = keyHash(this->kv[i].k); //前面findSplitKey对keyvalue进行qsort使fingerprints与key不对应,需重算fingerprint
    }
    int tn = this->n;
    for (; i < tn; i++)
    {
        temp->insertNonFull(this->kv[i].k, this->kv[i].v);
        this->bitmap[i / 8] &= !(1 << (7 - i % 8)); //将原来的bitmap相应位置设为空槽
        this->n--;
    }
    this->next = temp;
    temp->prev = this;
    newChild->key = mid_key; //midKey是分出来结点的第一个key值
    newChild->node = temp;
    PPointer pp;
    pp.fileId = this->pNext[0].fileId;
    pp.offset = this->pNext[0].offset;
    this->pNext[0] = temp->pPointer; //重构叶子结点的链表
    temp->pNext[0] = pp;
    return newChild;
}

// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find

static int cmp(const void *a, const void *b)
{
    return ((KeyValue *)a)->k > ((KeyValue *)b)->k;
}

Key LeafNode::findSplitKey()
{
    Key midKey = 0;
    // TODO
    qsort(kv, n, sizeof(KeyValue), cmp);
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
    Byte fgp = keyHash(k);
    for (int i = 0; i < this->degree * 2; i++)
    {
        if (this->getBit(i) && this->fingerprints[i] == fgp && this->getKey(i) == k)
        {
            this->bitmap[i / 8] &= !(1 << (7 - i % 8)));
            this->n--;
            if (this->n == 0)
            {
                this->prev->pNext = this->pNext;
                this->pNext = NULL;
                this->pmem_addr = NULL;
                this->prev->next = this->next;
                PAllocator::getAllocator()->freeLeaf(((LeafNode *)(parent->getChild(index)))->getPPointer());
                ifRemove = true;
                ifDelete = true;
            }
        }
    }
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key &k, const Value &v)
{
    bool ifUpdate = false;
    // TODO
    for (int i = 0; i < this->degree * 2; i++)
    {
        if (this->getBit(i) && this->getKey(i) == k)
        {
            this->kv[i].v = v;
            if (this->getValue(i) == v)
                ifUpdate = true;
        }
    }
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key &k)
{
    // TODO
    Byte fgp = keyHash(k);
    for (int i = 0; i < this->degree * 2; i++)
        if (this->getBit(i) && this->fingerprints[i] == fgp && this->getKey(i) == k) // 先找有空位的再找fingerprints再确定key是否确实相同
            return this->getValue(i);
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero()
{
    // TODO
    int i;
    for (i = 0; i < this->degree * 2; i++)
        if (!this->getBit(i))
            return i;
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
    PAllocator::getAllocator();
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
    if (root != NULL)
    {
        queue<Node *> q;
        q.push(root);
        while (!q.empty())
        {
            Node *temp = q.front();
            temp->printNode();
            q.pop();
            if (!temp->ifLeaf())
            {
                InnerNode *ti = (InnerNode *)temp;
                for (int i = 0; i < ti->getChildNum() i++)
                    q.push(ti->getChild(i));
            }
        }
    }
}

// bulkLoading the leaf files and reload the tree
// need to traverse leaves chain
// if no tree is reloaded, return FALSE
// need to call the PALlocator
bool FPTree::bulkLoading()
{
    // TODO
    if (PAllocator::getAllocator()->getMaxFileId() == 1)
        return false;
    PPointer temp = PAllocator::getAllocator()->getStartPointer();
    while (temp.fileId != 0)
    {
        LeafNode *leaf1 = new LeafNode(temp, this);
        KeyNode leaf;
        leaf.key = leaf1->getKey(0);
        leaf.node = (Node *)leaf1;
        this->root->insertLeaf(leaf);
        temp = leaf1->pNext[0];
    }
    return false;
}
