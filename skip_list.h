#include<iostream>
#include<string>
#include<set>
#include<time.h>
#include<climits> 

using std::string;
using std::cout;
using std::endl;



// 模板结构体 Less 用于比较两个对象的大小
template<typename T>
struct Less {
    bool operator () (const T & a , const T & b) const {
        return a < b;
    }
};

// 模板类 skip_list 表示一个键值对的跳跃表
// 参数 K 和 V 表示键和值的类型
// 参数 Comp 默认为 Less<K>，表示键的比较函数
template<typename K, typename V,typename Comp = Less<K>>
class skip_list {
private:
    // 跳跃表节点结构体
    typedef struct skip_list_node {
        int level;               // 节点的层数
        const K key;             // 键的值（const 类型，表示不可变）
        V value;                 // 值的值
        skip_list_node** forward; // 指向下一层节点的指针数组

        // 默认构造函数，初始化 key、value、level 和 forward
        skip_list_node() :key{ 0 }, value{ 0 }, level{ 0 }, forward{0} {}

        // 构造函数，初始化 key、value、level 和 forward，并设置下一层节点
        skip_list_node(K k, V v, int l, skip_list_node* nxt = nullptr) :key(k), value(v), level(l) {
            forward = new skip_list_node * [level + 1];
            for (int i = 0; i <= level; ++i) forward[i] = nxt;
        }

        // 析构函数，释放 forward 指针数组内存
        ~skip_list_node() { delete[] forward; }
    } node;


    // 初始化函数 init，用于初始化跳跃表
    void init() {
        // 随机数种子
        srand((uint32_t)time(NULL));

        // 初始化层数和长度
        level = length = 0;

        // 分配头结点的 forward 数组，并将所有元素指向尾结点
        head->forward = new node * [MAXL + 1];
        for (int i = 0; i <= MAXL; i++) {
            head->forward[i] = tail;
        }  
    }

    // 随机生成节点的层数
    int randomLevel() {
        int lv = 1; 
        // 使用位运算和随机数生成器生成新的层数
        while ((rand() & S) < PS) {
            ++lv;
        }
        // 返回新的层数，但不超过最大层数 MAXL
        return MAXL > lv ? lv : MAXL;
    }

    int level;                   // 跳表的最大层数
    int length;                  // 跳表的长度
    static const int MAXL = 32;  // 跳表的最大层数
    static const int P = 4;      // P 和 S 是用于随机生成新节点层数的常数
    static const int S = 0xFFFF; // P 和 S 是用于随机生成新节点层数的常数
    static const int PS = S / P; // P 和 S 是用于随机生成新节点层数的常数
    static const int INVALID = INT_MAX; // 表示无效的键值
    node* head, * tail;          // 指向跳表头尾的指针
    Comp less;                   // 用于比较键值的函数对象

    // 在跳表中查找给定 key 对应的节点，并返回该节点的指针
    // 如果找到，则返回对应节点的指针
    // 如果未找到，则返回空指针
    // 在查找过程中，会同时记录每一层的最后一个小于 key 的节点，用于插入新节点时使用
    node* find(const K& key, node** update) {
        node* p = head; // 从头结点开始查找
        for (int i = level; i >= 0; i--) { // 从高到低逐层查找
            // 在当前层中，如果存在某个节点的 key 值小于给定的 key 值，就向前移动
            while (p->forward[i] != tail && less(p->forward[i]->key, key)) {
                p = p->forward[i];
            }
                update[i] = p; // 记录下每一层的最后一个小于 key 的节点
        }
        p = p->forward[0]; // 最终在底层找到了节点，返回其指针
        return p;
    }

public:
    struct Iter {
        node* p;  // 指向节点的指针

        // 默认构造函数，将指针 p 初始化为 nullptr
        Iter() : p(nullptr) {};

        // 构造函数，将指针 p 初始化为 rhs
        Iter(node* rhs) : p(rhs) {}

        // 重载 -> 运算符，返回指针 p
        node* operator ->()const { return (p);}

        // 重载 * 运算符，返回指针 p 所指向的节点
        node& operator *() const { return *p;}

        // 重载 == 运算符，判断两个迭代器指向的节点是否相同
        bool operator == (const Iter& rhs) { return rhs.p == p;}

        // 重载 != 运算符，判断两个迭代器指向的节点是否不同
        bool operator != (const Iter& rhs) {return !(rhs.p == p);}

        // 重载前置 ++ 运算符，将指针 p 移动到下一个节点
        void operator ++() {p = p->forward[0];}

        // 重载后置 ++ 运算符，将指针 p 移动到下一个节点，并返回原始值
        void operator ++(int) { p = p->forward[0]; }
    };

    // 调用 init 函数来初始化头节点的指针数组和其他成员变量
    skip_list() : head(new node()), tail(new node()), less{Comp()} {
        init();    
    }
    skip_list(Comp _less) : head(new node()), tail(new node()),  less{_less} {
        init();
    }

    void insert(const K& key, const V& value) {
        node * update[MAXL + 1];    // update 数组用于保存每层插入位置的前驱节点指针
        node* p = find(key,update); // 查找待插入节点的位置，并将每层插入位置的前驱节点指针保存在 update 数组中
        if (p->key == key) {       // 如果待插入节点已存在，则更新节点的 value 值
            p->value = value;
            return;
        }
        int lv = randomLevel();    // 生成一个随机的节点层数
        if (lv > level) {          // 如果节点层数比当前 skip list 的最大层数还要大，则更新 skip list 的最大层数
            lv = ++level;
            update[lv] = head;
        }
        // 创建新节点，并将其插入每一层
        node * newNode = new node(key, value, lv);
        for (int i = lv; i >= 0; --i) {
            p = update[i];
            newNode->forward[i] = p->forward[i];
            p->forward[i] = newNode;
        }
        ++length;                  // 插入完成后更新 skip list 的长度
    }


    bool erase(const K& key) {  // 删除键为 key 的节点，返回删除是否成功
        node* update[MAXL + 1];  // 定义指针数组 update，存储查找过程中访问的各层节点指针
        node* p = find(key, update);  // 查找键为 key 的节点，同时更新 update 数组
        if (p->key != key) return false;  // 如果查找到的节点的键不是 key，则删除失败
        for (int i = 0; i <= p->level; ++i) {  // 遍历删除节点的各层前驱节点
            update[i]->forward[i] = p->forward[i];  // 将前驱节点的指针指向删除节点的后继节点
        }
        delete p;  // 释放删除节点的内存空间
        while (level > 0 && head->forward[level] == tail) --level;  // 更新跳表的最大层数 level
        --length;  // 更新跳表的节点数量 length
        return true;  // 删除成功
    }

    // 查找给定键 key 是否在跳表中存在
    // 参数 key: 要查找的键值
    // 返回值: Iter 类型，表示迭代器，指向对应的节点，如果未找到则返回指向尾节点的迭代器
    Iter find(const K&key) {
        // 创建更新数组，用于存储查找过程中经过的节点
        node* update[MAXL + 1];
        // 调用 find 函数进行查找，并将更新数组传入，返回值为最接近要查找的节点（小于等于 key）的节点指针
        node* p = find(key, update);
        // 如果找到了尾节点，表示 key 大于跳表中所有键，返回指向尾节点的迭代器
        if (p == tail) return tail;
        // 如果找到的节点的键不等于要查找的键，则返回指向尾节点的迭代器
        if (p->key != key) return tail;
        // 返回指向找到的节点的迭代器
        return Iter(p);
    }

    // 判断跳表中是否存在键值为 key 的节点，若存在返回 true，否则返回 false
    bool count(const K& key) {
        node* update[MAXL + 1];
        node* p = find(key, update); // 查找键值为 key 的节点
        if (p == tail) return false; // 如果找到的节点是尾节点，则说明跳表中不存在键值为 key 的节点，返回 false
        return key == p->key; // 否则比较找到的节点的键值和 key 是否相等，相等则说明存在，返回 true，否则不存在，返回 false
    }

    Iter end() {
        return Iter(tail);
    }   

    Iter begin() {
        return Iter(head->forward[0]);
    }
};