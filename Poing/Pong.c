#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Poing.h"

static void error (const char *s) __attribute__ ((noreturn));

static void error(const char *s);

int main (int argc, char *argv[] __attribute__ ((unused)) ) {
    soc_token soc = init_soc;
    int res;
    char buff[BUFF_SIZE];
    soc_length len;

    /* Parse command line arguments : no arg */
    if (argc != 1) error("Syntax");

    /* Create socket */
    res = soc_open(&soc, udp_socket);
    if (res != SOC_OK) {
        perror("soc_open");
        error("Socket creation");
    }

    /* Connect to port */
    res = soc_link_service (soc, PORT_NAME);
    if (res != SOC_OK) {
        perror("soc_link_service");
        error("Connecting to port");
    }

    for (;;) {
        /* Receive */
        len = (soc_length)sizeof(buff);
        res = soc_receive (soc, buff, len, TRUE);
        if (res != SOC_OK) {
            perror("soc_receive");
            error("Receiving message");
        }
        len = res;

        /* Print */
        buff[len]='\0';
        printf("Received: >%s<\n", buff);

        /* Reply */
        strcpy (buff, "Pong");
        res = soc_send (soc, buff, (soc_length)strlen(buff));
        if (res != SOC_OK) {
            perror("soc_send");
            error("Sending reply");
        }
    }

    /* Close socket
    res = soc_close(&soc);
    if (res != SOC_OK) {
        perror("soc_close");
        error("Closing socket");
    }
    */

}

static void error(const char *s) {
    printf ("ERROR: %s\nUsage: Pong\n", s);
    exit(1);
}

