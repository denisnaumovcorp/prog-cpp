#include <stdio.h>

int main() {
    long long int sum = 0;
    int temp = 0;
    while(scanf("%d", &temp) != EOF) {
        sum += temp;
    }
    printf("%lld", sum);
}
