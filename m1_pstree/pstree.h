#ifndef _PSTREE_H_
#define _PSTREE_H_

#define MAX_LEN 50

void showVersion();
int pstree(int showPid, int pidSort);
void procTree(int showPid, int pidSort);
struct Node* getProc();

// 定义进程结点信息
typedef struct proc{
    int pid;
    int ppid;
    char name[MAX_LEN];
    struct proc *next;
}procNode;

// 定义进程链表头尾结点
struct Node{
    procNode head;
    procNode tail;
};

#endif