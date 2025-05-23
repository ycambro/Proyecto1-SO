#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void run_client(const char* host, int port);

int main(int argc, char* argv[]) {
    const char* host = NULL;
    int port = 5000;

    int opt;
    while ((opt = getopt(argc, argv, "m:p:")) != -1) {
        switch (opt) {
            case 'm': host = optarg; break;
            case 'p': port = atoi(optarg); break;
            default:
                fprintf(stderr,
                    "uso:\n"
                    "  animar -m host [-p puerto]    # monitor\n");
                return EXIT_FAILURE;
        }
    }

    if (!host) {
        fprintf(stderr, "Debes especificar el host con -m\n");
        return EXIT_FAILURE;
    }

    run_client(host, port);
    return EXIT_SUCCESS;
}