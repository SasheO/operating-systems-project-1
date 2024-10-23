/***************************************************************************************
*    Title: sleep.c
*    Author: Mezisashe Ojuba
*    Date: 23 Oct 2024
*    Code version: 1.0
*    Availability: https://github.com/SasheO/operating-systems-project-1
*
***************************************************************************************/

#include <stdio.h>
#include<unistd.h>

int main() {
    sleep(100);
    printf("I was asleep for 100 seconds, just woke up!\n");
    return 0;
}