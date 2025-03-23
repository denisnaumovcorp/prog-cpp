#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct text {
    unsigned int size;
    unsigned int capacity;
    char **strings;
};

void initText(struct text *text) {
    text->size = 0;
    text->capacity = 1;
    text->strings = (char**)malloc(sizeof(char*) * text->capacity);
}

void addString(struct text *text, const char *string) {
    if (text->size == text->capacity) {
        text->capacity *= 2;
        char **temp = (char**)realloc(text->strings, sizeof(char*) * text->capacity);
        text->strings = temp;
    }
    text->strings[text->size] = (char*)malloc(strlen(string) + 1);
    strcpy(text->strings[text->size], string);
    text->size++;
}

void freeText(struct text *text) {
    for (unsigned int i = 0; i < text->size; i++) {
        free(text->strings[i]);
    }
    free(text->strings);
}

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int main() {
    char word[256];
    struct text myText;
    initText(&myText);
    unsigned int wordCount = 0;
    while (scanf("%255s", word) != EOF) {
        addString(&myText, word);
        wordCount++;
    }
    qsort(myText.strings, wordCount, sizeof(char *), compare);
    for (int i = 0; i < wordCount; i++) {
        printf("%s\n", myText.strings[i]);
    }
    freeText(&myText);
}
