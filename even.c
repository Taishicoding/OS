#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
/*Taishi Morgan A1904976*/
/*August 20*/
/*Overriding default signal behaviors*/

/*Function Explanton: This signal is sent when terminal collection
is lost, throughout the ouch function, it is overridden to provide the "Ouch" output*/
void ouch(int sig) {
    printf("Ouch!");
    fflush(stdout);
}
/*Function Explanation: This signal is the Signal Interupt signal
usually operated through CTRL+C, it's output is overrided, to print Yeah*/
void yeah(int sig) {
    printf("Yeah!");
    fflush(stdout);  
}
int main(int argc, char *argv[]) {
    /*Converting command prompt input into integer*/
    int n = atoi(argv[1]);
    /*Overiding the SIGHUP (terminal cancel) to the Ouch function*/
    signal(SIGHUP, ouch);
    /*Overiding the SIGINT(Interupt in signal) to the Yeah function*/
    signal(SIGINT, yeah);
    for (int i = 0; i < n; i++) {
        printf("%d\n", i * 2);
        fflush(stdout);  
        sleep(5); 
    }
    return 0;
}/*?*/
