#include "queue.h"

Queue *create_queue() {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue *q, int tag) {
    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    new_node->tag = tag;
    new_node->last_visited_time=-1;
    new_node->next = NULL;

    if (!q->rear) {
        q->front = q->rear = new_node;
    } else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
}

int dequeue(Queue *q) {
    if (!q->front) return -1; // 队列为空

    QueueNode *temp = q->front;
    int tag = temp->tag;
    q->front = q->front->next;

    if (!q->front)
        q->rear = NULL;

    free(temp);
    q->size--;
    return tag;
}

int find_and_update(Queue *q,int tag,int timecount)
{
    QueueNode *cur=q->front;
    while(cur)
    {
        if(cur->tag==tag)
        {
            cur->last_visited_time=timecount;
            return 1;
        }
        cur=cur->next;
    }
    return 0;
}

void lru_replace(Queue *q,int tag,int timecount)
{
    QueueNode *cur=q->front;
    int mintime=cur->last_visited_time;
    int minpos=cur->tag;
    while(cur)
    {
        if(mintime>cur->last_visited_time)
        {
            mintime=cur->last_visited_time;
            minpos=cur->tag;
        }
        cur=cur->next;
    }
    cur=q->front;
    while(cur)
    {
        if(minpos==cur->tag)
        {
            cur->tag=tag;
            cur->last_visited_time=timecount;
        }
        cur=cur->next;
    }
    return;
}
int find_and_remove(Queue *q, int tag) {
    QueueNode *prev = NULL, *curr = q->front;

    while (curr) {
        if (curr->tag == tag) {
            // 如果找到，移除节点
            if (prev)
                prev->next = curr->next;
            else
                q->front = curr->next;

            if (!curr->next)
                q->rear = prev;

            free(curr);
            q->size--;
            return 1; // 找到并移除
        }
        prev = curr;
        curr = curr->next;
    }
    return 0; // 未找到
}