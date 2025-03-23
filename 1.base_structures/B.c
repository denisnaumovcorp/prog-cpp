#include <stdio.h>

int main() {
    double a = 0, b = 0;
    char c = 0;
    scanf("%lf%c%lf", &a, &c, &b);
    switch (c) {
        case '+':
            printf("%.6f", a + b);
            break;
        case '-':
            printf("%.6f", a - b);
            break;
        case '*':
            printf("%.6f", a * b);
            break;
        case '/':
            printf("%.6f", a / b);
            break;
        default:
            printf("Wrong expression");
            break;
    }
}