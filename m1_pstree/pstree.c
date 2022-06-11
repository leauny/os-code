#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h> // 读取文件夹
#include "pstree.h"

int main(int argc, char *argv[]) {
    int pidSort = 0;
    int showPid = 0;
    for (int i = 1; i < argc; i++) {
        assert(argv[i]); // C 标准保证
        if(argv[i][0] == '-'){
            // 先判断第一个参数是不是-, 然后开始接下来判断是短命令还是长命令, 接着分别查看是否存在V, p, n三个参数
            if(argv[i][1] != '\0' && argv[i][1] == '-'){
                // 长命令
                if(strcmp(argv[i],"--version") == 0){
                    showVersion();
                    return 0;
                }else if(strcmp(argv[i],"--numeric-sort") == 0){
                    // 存在排序
                    pidSort = 1;
                }else if(strcmp(argv[i], "--show-pids") == 0){
                    // 存在显示pid
                    showPid = 1;
                }
            }else{
                for(int j = 1; argv[i][j] != '\0'; j++){
                    switch (argv[i][j]){
                        case 'V' :{
                            // 一旦出现Version参数, 则直接结束
                            showVersion();
                            return 0;
                        }
                        case 'p' :{
                            showPid = 1;
                            break;
                        }
                        case 'n' :{
                            pidSort = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    assert(!argv[argc]); // C 标准保证
    pstree(showPid, pidSort);
    return 0;
}

void printprocList(procNode* ptr){
    // 打印进程名称列表
    while(ptr != NULL){
        printf("%6d - %d (%s)\n", ptr->ppid, ptr->pid, ptr->name);
        ptr = ptr->next;
    }
    printf("\n");
}


int pstree(int showPid, int pidSort){

    struct Node* procList = getProc();
    
    procNode* ptr = &procList->head; // 方便连接移走的链表结点
    procNode* root = (procNode*)malloc(sizeof(procNode));
    strcpy(root->name, "root"); root->next=NULL; root->nextLevel=NULL; root->pid=0; root->ppid=-1;

    // printprocList(procList->head.next);

    formationTree(procList, root);

    // printprocList(root->nextLevel);

    // printTree(root->nextLevel, 0);

    printTreeV2(root->nextLevel, showPid, 0);

    return 0;
}

void printTreeV2(procNode* root, int showPid, int deepth){
    while(root != NULL){
        for(int i = deepth; i > 0; i--){
            printf("\t");
        }
        printf("%s", root->name);
        if(showPid == 1){
            printf("(%d)", root->pid);
        }
        printf("\n");
        
        if(root->nextLevel != NULL){
            printTreeV2(root->nextLevel, showPid, deepth + 1);
        }
        root = root->next;
    }
}

void printTree(procNode* root, int deepth){
    if(root == NULL){
        return;
    }

    procNode* ptr = root;

    while(ptr != NULL){
        for(int i = deepth; i > 0; i--){
            printf("\t");
        }
        printf("%s(%d)\n", ptr->name, ptr->pid);
        
        if(ptr->nextLevel != NULL){
            printTree(ptr->nextLevel, deepth + 1);
        }
        ptr = ptr->next;
    }
}

void formationTree(struct Node* procList, procNode* currentProc){
    // 如果没有待处理的, 则退出
    procNode* ptr = &procList->head;
    procNode* cptr = currentProc;

    procNode* newCurrentProc = NULL;

    while(ptr->next != NULL){ 
        /*
        // bug: 在找孩子时被拐走
        if(currentProc->pid == ptr->next->ppid){
            // 找到当前结点的孩子
            printf("+ %s(%d)\n", ptr->next->name, ptr->next->pid);
            newCurrentProc = ptr->next; //　保存当前节点地址
            // currentProc->next = newCurrentProc;

            ptr->next = ptr->next->next; // 连接主链
            newCurrentProc->next = NULL;
            
            // 当前匹配结点加入兄弟组, 匹配结点的nextLevel进行递归查找
            currentProc->nextLevel = newCurrentProc;
            formationTree(procList, newCurrentProc);
        }else if(currentProc->ppid == ptr->next->ppid){
            // 找到兄弟
            printf("- %s(%d)\n", ptr->next->name, ptr->next->pid);
            currentProc->next = ptr->next;
            currentProc = currentProc->next; // 移动current防止丢失信息
            ptr->next = ptr->next->next; // 连接主链
            currentProc->next = NULL;
        }
        else{
            printf("\t^ %s(%d)\n", ptr->next->name, ptr->next->pid);
            ptr = ptr->next;
        }*/

        if(currentProc->ppid == ptr->next->ppid){
            // 先保存所有的同级结点, 再进入每一个的子节点找孩子
            currentProc->next = ptr->next;
            currentProc = currentProc->next; // 移动current防止丢失信息
            ptr->next = ptr->next->next; // 连接主链
            currentProc->next = NULL;
        }else{
            ptr = ptr->next;
        }
    }

    while(cptr != NULL){

        ptr = &procList->head;

        while(ptr->next != NULL){
            if(cptr->pid == ptr->next->ppid){
                // 找到当前结点的孩子
                cptr->nextLevel = ptr->next; //　保存当前节点地址
                ptr->next = ptr->next->next; // 连接主链
                cptr->nextLevel->next = NULL;
                formationTree(procList, cptr->nextLevel);
            }else{
                ptr = ptr->next;
            }
        }
        cptr = cptr->next;
    }
}

struct Node* getProc(){
    /*
        获取进程的信息, 保存到struct结构体内
    */

    // 链表头尾结点结构 &初始化
    struct Node *procList = (struct Node*)malloc(sizeof(struct Node)); 
    procList->head.next = NULL;// &procList->tail;
    // procList->tail.next = NULL;
    procNode *tailptr = &procList->head; // 尾插法指针

    DIR *dir = opendir("/proc/"); // 记得closedir

    struct dirent *pdir; // 文件信息结构体
    int procNum = 0; // 进程的数量
    char state = ' '; // 用于存储进程状态
    int umask = 0;
    char dirstr[30] = "/proc/"; // 用来存储目录信息
    while((pdir = readdir(dir)) != NULL){
        if(strcmp(pdir->d_name,".")==0 || strcmp(pdir->d_name,"..")==0){    ///current dir OR parrent dir
            continue;
        }
        if(!(pdir->d_name[0] >= '0' && pdir->d_name[0] <= '9')){
            continue;// 如果非数字, 则继续   
        }

        strcat(dirstr, pdir->d_name);
        strcat(dirstr, "/status");
        
        // 创建结点并连接
        procNode* node = (procNode*) malloc(sizeof(procNode));
        
        if(freopen(dirstr, "r", stdin) != NULL){
            // 获取进程信息, 并存入结构体内
            scanf("Name: %[^\n] Umask: %d State: %c %[^\n] Tgid: %d Ngid: %d Pid: %d PPid: %d", node->name, &umask, &state, dirstr, &node->pid, &node->pid, &node->pid, &node->ppid);

            /*
                阻止部分进程会导致pstree不完整
                if((atoi(pdir->d_name) != node->pid || node->ppid == 0 || state == 'I' || strcmp(node->name, "sleep") == 0 || umask != 22) && node->pid != 1)
            */
            if((atoi(pdir->d_name) != node->pid || node->ppid == 0 || state == 'I') && node->pid != 1){ 
                // 阻止ppid为0的进程以及消失的进程
                // 阻止Idle进程
                // 阻止sleep进程
                // 阻止umask非0022的进程
                free(node); // 释放内存
                node = NULL; // 不置为null可能导致释放两次
            }else{
                procNum++;
                tailptr->next = node;
                node->nextLevel = NULL;
                node->next = NULL;
                tailptr = tailptr->next; // 只有完全获取proc信息后才移动尾指针, 否则会导致出现冗余错误信息
            }
        }
        strcpy(dirstr, "/proc/");
        state = ' '; // 重置状态
    }

    procList->number = procNum; // 写入进程总数

    // 关闭文件夹读写
    closedir(dir);

    return procList;
}

void showVersion(){
    fprintf(stderr,"\n\tpstree version 0.1 (LEAUNY)  Copyright (C) 2022-2022 Hammer King\n\n");
}
