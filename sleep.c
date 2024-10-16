#include <stdio.h>
#include<unistd.h>

int main() {
    sleep(100);
    printf("I was asleep for 100 seconds, just woke up!\n");
    return 0;
}