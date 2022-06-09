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
    for (int i = 0; i < argc; i++) {
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
                    break;
                }else if(strcmp(argv[i], "--show-pids") == 0){
                    // 存在显示pid
                    showPid = 1;
                    break;
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
        // printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]); // C 标准保证
    pstree(showPid, pidSort);
    return 0;
}

int pstree(int showPid, int pidSort){
    // 链表头尾结点结构 &初始化
    struct Node procList; 
    procList.head.next = &procList.tail;
    procList.tail.next = NULL;

    DIR *dir = opendir("/proc/"); // 记得closedir

    struct dirent *pdir; // 文件信息结构体
    int procNum = 0; // 进程的数量
    char state = ' '; // 用于存储进程状态
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
        node->next = procList.head.next;
        procList.head.next = node;
        
        if(freopen(dirstr, "r", stdin) != NULL){
            // 获取进程信息, 并存入结构体内
            scanf("Name:\t%s\nUmask:\t%s\nState:\t%c%s\nTgid:\t%d\nNgid:\t%d\nPid:\t%d\nPPid:\t%d", node->name, dirstr, &state, dirstr, &node->pid, &node->pid, &node->pid, &node->ppid);
            if(atoi(pdir->d_name) != node->pid || node->ppid == 0 || state == 'I' || strcmp(node->name, "sleep") == 0){
                // 阻止ppid为0的进程以及消失的进程
                // 阻止Idle进程
                // 阻止sleep进程
                procList.head.next = node->next; // 去除消失的进程结点
                if(node != NULL){
                    free(node); // 释放内存(可能导致释放两次)
                    node = NULL;
                }
            }else{
                procNum++;
            }
        }
        strcpy(dirstr, "/proc/");
        state = ' '; // 重置状态
    }
    printf("\nProcNum : %d\n\n", procNum);

    procNode* ptr = procList.head.next;
    while(ptr->next != &procList.tail){
        printf("%s\n", ptr->name);
        ptr = ptr->next;
    }

    if(pidSort == 1){
        printf("sorting ...\n");
    }
    printf("TREE!\n");

    // 关闭文件夹读写
    closedir(dir);
    return 0;
}

void showVersion(){
    printf("\n\tpstree version 0.1 (LEAUNY)  Copyright (C) 2022-2022 Hammer King\n\n");
}