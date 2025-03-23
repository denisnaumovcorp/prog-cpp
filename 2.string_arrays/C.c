#include <stdio.h>
#include <string.h>

int main() {
    char maxWord[256] = "";
    char word[256];
    unsigned int maxLength = 0;
    while(scanf("%255s", word) != EOF) {
        if (strlen(word) > maxLength || (strlen(word) == maxLength && strcmp(word, maxWord) < 0)) {
            maxLength = strlen(word);
            strcpy(maxWord, word);
        }
    }
    printf("%s\n%d\n", maxWord, maxLength);
}
