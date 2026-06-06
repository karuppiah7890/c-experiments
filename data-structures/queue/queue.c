#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct node
{
    int value;
    struct node *next;
};

struct queue
{
    struct node *head;
    struct node *tail;
    int size;
};

bool is_queue_empty(struct queue *q)
{
    return q->size == 0;
}

void enqueue(struct queue *q, int value)
{
    struct node *node = malloc(sizeof(struct node));

    node->value = value;
    node->next = NULL;

    if (q->tail == NULL)
    {
        // here head MUST also be NULL actually
        q->head = node;
        q->tail = q->head;
        q->size = 1;
        return;
    }

    q->tail->next = node;
    q->tail = q->tail->next; // same as: q->tail = node;
    q->size++;
}

int dequeue(struct queue *q)
{
    if (q->head == NULL)
    {
        // here tail MUST also be NULL actually
        printf("head is null. nothing to dequeue!");
        return -1;
    }

    if (q->head == q->tail)
    {
        int value = q->head->value;
        free(q->head);
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
        return value;
    }

    struct node *top = q->head;
    q->head = q->head->next;
    int value = top->value;
    top->next = NULL;
    free(top);
    q->size--;
    return value;
}

void print(struct queue *q)
{
    printf("\n\nqueue details:\n");
    if (q->head == NULL)
    {
        printf("head is null\n");

        if (q->tail == NULL)
        {
            printf("tail is also null as expected\n");
        }
        else
        {
            printf("tail is NOT null! ERROR!\n");
        }

        return;
    }

    printf("head is not null\n");

    if (q->tail == NULL)
    {
        printf("tail is null! Error!\n");
        return;
    }

    struct node *start = q->head;

    do
    {
        printf("%d,", start->value);
        start = start->next;
    } while (start != NULL);

    printf("\nfinished printing queue details\n\n");
}

int main()
{
    struct queue *q = malloc(sizeof(struct queue));

    printf("checking queue before enqueueing\n");

    if (q->head != NULL)
    {
        printf("expected queue head to be null but it wasn't null! ERROR!\n");
        return 1;
    }

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    enqueue(q, 5);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    enqueue(q, 6);

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    enqueue(q, 7);

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    printf("checking queue details after enqueueing\n");

    if (q->head == NULL)
    {
        printf("expected queue head to not be null but it was null! ERROR!\n");
        return 1;
    }

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    printf("dequeued value: %d\n", dequeue(q)); // 5

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    printf("dequeued value: %d\n", dequeue(q)); // 6

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    printf("dequeued value: %d\n", dequeue(q)); // 7

    print(q);

    printf("queue size: %d. is queue empty? %d\n\n", q->size, is_queue_empty(q));

    printf("checking queue details after dequeueing\n");

    if (q->head != NULL)
    {
        printf("expected queue head to be null but it wasn't null! ERROR!\n");
        return 1;
    }

    return 0;
}
