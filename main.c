/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Thiago Kenji Okada
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "ftp.h"


int main (int argc, char **argv) {
    int listenfd;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t  n;

    if (argc != 2) {
        fprintf(stderr,"usage: %s PORT\n",argv[0]);
        fprintf(stderr,"Run FTP server in port PORT\n");
        exit(EXIT_FAILURE);
    }

    if ((listenfd = create_listener(INADDR_ANY, atoi(argv[1]), 1)) == -1) {
        perror("create_listener");
        exit(EXIT_FAILURE);
    }

    printf("YAFTPd is running in port %s\n",argv[1]);

    for (;;) {
        if ((CONN_FD = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if ((childpid = fork()) == 0) { // Child proccess
            printf("Succesful connection at %s. New child PID: %d\n", get_socket_ip(CONN_FD), getpid());
            close(listenfd);

            /* When the user connects show info message about server version */ 
            char* msg = version_info();
            write(CONN_FD, msg, strlen(msg));

            while ((n=read(CONN_FD, recvline, MAXLINE)) > 0) {
                recvline[n]=0;
                printf("PID %d SEND: ", getpid());
                if ((fputs(recvline,stdout)) == EOF) {
                    perror("fputs");
                    exit(EXIT_FAILURE);
                }
                int result = parse_command(recvline);
                if(result == 1) {
                    // If parse_command returns 1, we should exit the current
                    // proccess
                    exit(EXIT_SUCCESS);
                } else if (result == -1) {
                    // In case of error too, but we should inform the system.
                    perror("parse_command");
                    exit(EXIT_FAILURE);
                } else {
                    // If parse_command returns 0, we should continue the
                    // process.
                    continue;
                }
            }

            printf("Finished connection with child PID: %d\n", getpid());

        } else { // Parent proccess
            close(CONN_FD);
        }
    }
    exit(EXIT_SUCCESS);
}
