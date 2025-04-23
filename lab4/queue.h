#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


typedef struct queue_node {
    int tag;
    int last_visited_time;                 // 缓存行的标记
    struct queue_node *next; // 指向下一个节点
} QueueNode;

typedef struct queue {
    QueueNode *front; // 队首
    QueueNode *rear;  // 队尾
    int size;         // 当前队列大小
} Queue;

typedef struct set_node{
    Queue* a1;
    Queue* am;
}setnode;
Queue *create_queue();
void enqueue(Queue *q, int tag);
int dequeue(Queue *q);
int find_and_update(Queue *q,int tag,int timecount);
void lru_replace(Queue *q,int tag,int timecount);
int find_and_remove(Queue *q, int tag);