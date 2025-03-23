#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct stack {
    unsigned int size;
    unsigned int capacity;
    int *elements;
};

struct log {
    unsigned int size;
    unsigned int capacity;
    char **messages;
};

void initLog(struct log *myLog) {
    myLog->size = 0;
    myLog->capacity = 1;
    myLog->messages = (char**)malloc(sizeof(char*) * myLog->capacity);
}

void logMessage(struct log *myLog, const char *message) {
    if (myLog->size == myLog->capacity) {
        myLog->capacity *= 2;
        char **temp = (char**)realloc(myLog->messages, sizeof(char*) * myLog->capacity);
        myLog->messages = temp;
    }
    myLog->messages[myLog->size] = (char*)malloc(strlen(message) + 1);
    strcpy(myLog->messages[myLog->size], message);
    myLog->size++;
}

int logSize(struct log *myLog) {
    return myLog->size;
}

void freeLog(struct log *myLog) {
    for (int i = 0; i < myLog->size; i++) {
        free(myLog->messages[i]);
    }
    free(myLog->messages);
}

void initStack(struct stack *myStack) {
    myStack->size = 0;
    myStack->capacity = 1;
    myStack->elements = (int*)malloc(sizeof(int));
}


void push(struct stack *myStack, int n) {
    if (myStack->size == myStack->capacity) {
        myStack->capacity = myStack->capacity == 0 ? 1 : myStack->capacity * 2;
        myStack->elements = (int*)realloc(myStack->elements, sizeof(int) * myStack->capacity);
    }
    myStack->elements[myStack->size] = n;
    myStack->size++;
}

int back(struct stack *myStack) {
    return myStack->elements[myStack->size - 1];
}

int pop(struct stack *myStack) {
    int answer = myStack->elements[myStack->size - 1];
    myStack->size--;
    return answer;
}

int size(struct stack *myStack) {
    return myStack->size;
}

void clear(struct stack *myStack) {
    free(myStack->elements);
    myStack->elements = (int*)malloc(sizeof(int));
    myStack->size = 0;
    myStack->capacity = 1;
}

int main() {
    struct stack myStack;
    struct log myLog;
    char command[10], temp[20];
    int number, n;
    unsigned short int i;
    
    initStack(&myStack);
    initLog(&myLog);
    scanf("%d", &n);
    
    for (i = 0; i < n; i++) {
        scanf("%9s", command);
        if (strcmp(command, "push") == 0) {
            scanf("%d", &number);
            push(&myStack, number);
            logMessage(&myLog, "ok");
        } else if (strcmp(command, "pop") == 0) {
            sprintf(temp, "%d", pop(&myStack));
            logMessage(&myLog, temp);
        } else if (strcmp(command, "back") == 0) {
            sprintf(temp, "%d", back(&myStack));
            logMessage(&myLog, temp);
        } else if (strcmp(command, "size") == 0) {
            sprintf(temp, "%d", size(&myStack));
            logMessage(&myLog, temp);
        } else if (strcmp(command, "clear") == 0) {
            clear(&myStack);
            logMessage(&myLog, "ok");
        } else if (strcmp(command, "exit") == 0) {
            logMessage(&myLog, "bye");
            break;
        }
    }
    for (i = 0; i < logSize(&myLog); i++) {
        printf("%s\n", myLog.messages[i]);
    }
    freeLog(&myLog);
    free(myStack.elements);
}
