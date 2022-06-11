#ifndef _PSTREE_H_
#define _PSTREE_H_

#define MAX_LEN 50

// 定义进程结点信息
typedef struct proc{
    int pid;
    int ppid;
    char name[MAX_LEN];
    struct proc *next;
    struct proc *nextLevel;
}procNode;

// 定义进程链表头尾结点
struct Node{
    procNode head;
    int number;
};

// 定义层级链表控制结构
// typedef struct treeLevel{
//     procNode head;
//     procNode tail;
//     struct treeLevel *nextLevel;
//     int number;
// }treeLevel;

void showVersion();
int pstree(int showPid, int pidSort);
void procTree(int showPid, int pidSort);
void formationTree(struct Node* ptr, procNode* currentProc);
void printTree(procNode* root, int deepth);
void printTreeV2(procNode* root, int showPid, int deepth);
struct Node* getProc();

#endif