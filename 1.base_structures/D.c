#include <stdio.h>

int main() {
    unsigned short int sec, min, hour, day, month, year;
    unsigned long long int t, delta, deltaDays;
    unsigned short int months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    scanf("%hu:%hu:%hu %hu-%hu-%hu", &hour, &min, &sec, &day, &month, &year);
    scanf("%llu", &t);
    delta = sec + min * 60 + hour * 3600 + t;
    sec = delta % 60;
    delta /= 60;
    min = delta % 60;
    delta /= 60;
    hour = delta % 24;
    deltaDays = delta / 24;
    while (deltaDays > 0) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            months[1] = 29;
        } else {
            months[1] = 28;
        }
        if (deltaDays < months[month - 1] - day + 1) {
            day += deltaDays;
            deltaDays = 0;
        } else {
            deltaDays -= months[month - 1] - day + 1;
            month++;
            day = 1;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
    }
    printf("%02hu:%02hu:%02hu %02hu-%02hu-%04hu", hour, min, sec, day, month, year);
}
