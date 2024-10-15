#include <stdio.h>
#include<unistd.h>

int main() {
    // Stores the string typed into the command line.
    printf("This program sleeps for 5 seconds.\n");
    sleep(5);
    return 0;
}