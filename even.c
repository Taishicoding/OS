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
}
/*Function Explanation: This signal is the Signal Interupt signal
usually operated through CTRL+C, it's output is overrided, to print Yeah*/
void yeah(int sig) {
    printf("Yeah!");
}
int main(int argc, char *argv[]) {
    /*Converting command prompt input into integer*/
    int n = atoi(argv[1]);
    /*Setting Up single handlers after signal handling error*/
    struct sigaction sa_hup = {0};
    sa_hup.sa_handler = ouch;
    sa_hup.sa_flags = SA_RESTART; 
    sigaction(SIGHUP, &sa_hup, NULL);
    /*Setting Up single handlers after signal handling error*/
    struct sigaction sa_int = {0};
    sa_int.sa_handler = yeah;
    sa_int.sa_flags = SA_RESTART; 
    sigaction(SIGINT, &sa_int, NULL);
    /*Overiding the SIGHUP (terminal cancel) to the Ouch function*/
    signal(SIGHUP, ouch);
    /*Overiding the SIGINT(Interupt in signal) to the Yeah function*/
    signal(SIGINT, yeah);
    for (int i = 0; i < n; i++) {
        printf("%d\n", i * 2);
        sleep(5); 
    }
    return 0;
}/*?*/
