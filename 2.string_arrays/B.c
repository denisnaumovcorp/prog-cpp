#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

struct stack {
    unsigned int size;
    unsigned int capacity;
    int *elements;
};

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
    initStack(&myStack);
    char input[10];
    while (scanf("%9s", input) != EOF) {
        if ((input[0] > 47 && input[0] < 58) || (input[0] == 45 && (input[1] > 47 && input[1] < 58))) {
            push(&myStack, atoi(input));
        } else if (input[0] == '+' || input[0] == '-' || input[0] == '*' || input[0] == '/') {
            int b = pop(&myStack);
            int a = pop(&myStack);
            switch (input[0]) {
                case '+':
                    push(&myStack, a + b);
                    break;
                case '-':
                    push(&myStack, a - b);
                    break;
                case '*':
                    push(&myStack, a * b);
                    break;
                case '/':
                    push(&myStack, a / b);
                    break;
            }
        }
    }
    printf("%d\n", pop(&myStack));
    free(myStack.elements);
}