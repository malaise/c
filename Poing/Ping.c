#include "Poing.h"

static void error (const char *s);

int main (int argc, char *argv[]) {
    soc_token soc = init_soc;
    int res;
    char buff[BUFF_SIZE];
    soc_length len;
    boolean ping_lan;

    /* Parse command line arguments : host_name */
    if (argc != 3) error("Syntax");

    /* Syntax is Ping lan <lan_name> or Ping host <host_name> */
    ping_lan = false;
    if (strcmp(argv[1], "host") == 0) {
      ping_lan = false;
    } else if (strcmp(argv[1], "lan") == 0) {
      ping_lan = true;
    } else {
      error("Syntax");
    }

    /* Create socket */
    res = soc_open(&soc, udp_socket);
    if (res != SOC_OK) {
        perror("soc_open");
        error("Socket creation");
    }
   
    /* Set destination */
    res = soc_set_dest_name_service(soc, argv[2], ping_lan, PORT_NAME);
    if (res != SOC_OK) {
        perror("soc_set_dest_service");
        error("Setting destination");
    }

    /* Connect to port */
    res = soc_link_service (soc, PORT_NAME); 
    if (res != SOC_OK) {
        perror("soc_link_service");
        error("Connecting to port");
    }

    /* Send */
    strcpy (buff, "Ping");
    res = soc_send (soc, buff, (soc_length)strlen(buff));
    if (res != SOC_OK) {
        perror("soc_send");
        error("Sending message");
    }

    /* Receive */
    len = (soc_length)sizeof(buff);
    res = soc_receive (soc, buff, len, FALSE);
    if (res != SOC_OK) {
        perror("soc_receive");
        error("receiving message");
    }
    len = res;

    /* Print */
    buff[len]='\0';
    printf("Received: >%s<\n", buff);

    /* Close socket */
    res = soc_close(&soc);
    if (res != SOC_OK) {
        perror("soc_close");
        error("Closing socket");
    }
    exit(0);
}

static void error (const char *s) {
    printf ("ERROR: %s\nUsage: Ping host <host_name>  or  Ping lan <lan_name> \n", s);
    exit(1);
}

