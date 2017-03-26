#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#ifndef __HTTP_SERVER_UTILS_H_
    #include "http_server_utils.h"
#endif

#define BUFSIZE 500

/*typedef enum {
    put,
    post,
    get
} http_req_type_t;

typedef struct {
    char *req_page;
    int req_type;
} http_req_t;

typedef enum {
    text,
    css,
    audio,
    video,
    image
} http_data_t;

typedef struct {
    int code;
    int len;
    int type;
} http_header_t;



char *finput(char *path){
    FILE *fp = fopen(path, "r");
    if(fp == NULL){
        printf("ERROR: failed opening file '%s'\n", path);
        return NULL;
    }
    char c;
    int size = 1000;
    int i = 0;
    char *str = calloc(1 , sizeof(char) * size);
    char *tmpptr;
    while((c = fgetc(fp)) != EOF){
        str[i] = c;
        i++;
        if(size - i < 20){
            size += 100;
            tmpptr = realloc(str, size);
            if(tmpptr != NULL){
                str = tmpptr;
            }
            else {
                puts("FATAL ERROR: ran out of memory");
                exit(1);
            }
        }
    }
    str[i] = '\0';
    fclose(fp);
    str = realloc(str, strlen(str) + 1);
    return str;
}*/



int main(int argc, char *argv[]){
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[BUFSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char *path = NULL;
    char *rel_path = NULL;
    char *index_page = NULL;
    char *assets_path = getcwd(NULL, 0);
    assets_path = realloc(assets_path, (strlen(assets_path) + 8) * sizeof(char));
    strcat(assets_path, "/assets");
    portno = 443;

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-p") == 0){
            if(i + 1 <= argc){
                portno = strtol(argv[++i], NULL, 10);
                //i++;
                continue;
            }
            else {
                printf("ERROR: expected a port number after argument #%d '-p'\n", i);
                exit(1);
            }
        }
        else if(strcmp(argv[i], "--root-dir") == 0){
            if(i + 1 <= argc){
                path = calloc(1, strlen(argv[++i]) + 1);
                strcpy(path, argv[i]);
                int ret = chdir(path);
                if(ret != 0){
                    printf("FATAL ERROR: failed changing directory to %s\n", path);
                    exit(1);
                }
                continue;
            }
            else {
                printf("ERROR: expected a path after argument #%d '--root-dir'\n)", i);
                exit(1);
            }
        }
        else if(strcmp(argv[i], "--index-page") == 0){
            if(i + 1 <= argc){
                index_page = calloc(1, strlen(argv[++i]) + 1);
                strcpy(index_page, argv[i]);
                continue;
            }
            else {
                printf("ERROR: expected a path after argument #%d '--index-page'\n", i);
                exit(1);
            }
        }
        else {
            printf("FATAL ERROR: invalid argument #%d: '%s'\n", i, argv[i]);
            exit(1);
        }
    }

    puts("Starting SCSD");
    printf("    PORT : %d\n", portno);
    if(path != NULL){
        printf("ROOT-DIR : %s\n", path);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        error("ERROR opening socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }

    char *content = NULL;
    http_header_t header;
    char *str_header = NULL;
    char *reply = NULL;

    while(1){
        reply = NULL;
        str_header = NULL;
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            error("ERROR on accept");
        }
        bzero(buffer, BUFSIZE);
        n = read(newsockfd,buffer, BUFSIZE);
        if(n < 0){
            error("ERROR reading from socket");
        }
        puts(buffer);
        http_req_t req_header = http_parse_header(buffer);
        if(req_header.req_page == NULL && index_page != NULL){
            req_header.req_page = calloc(1, (strlen(index_page) + 1) * sizeof(char));
            strcpy(req_header.req_page, index_page);
        }
        else if(req_header.req_page == NULL && index_page == NULL){
            header.code = 404;
            header.len = 0;
            header.type = text;
            rel_path = calloc(1, (strlen(assets_path) + strlen("/html/error_404.html")) * sizeof(char));
            strcpy(rel_path, assets_path);
            strcat(rel_path, "/html/error_404.html");
            content = bfinput(rel_path);
            if(content != NULL){
                header.len = strlen(content);
                str_header = http_generate_header(header);
                reply = calloc(1, (strlen(str_header) + strlen(content) + 1) * sizeof(char));
                n = write(newsockfd, reply, strlen(reply) + 1);
            }
            else {
                str_header = http_generate_header(header);
                n = write(newsockfd, str_header, strlen(str_header) + 1);
            }
        }
        if(req_header.req_page != NULL){
            printf("requested file: %s\n", req_header.req_page);
            rel_path = calloc(1, sizeof(char) * ((strlen(req_header.req_page) + 3)));
            rel_path[0] = '.';
            if(req_header.req_page[0] != '/'){
                rel_path[1] = '/';
            }
            strcat(rel_path, req_header.req_page);
        }
        else {
            rel_path = calloc(1, sizeof(char) * 23);
            strcpy(rel_path, ".");
        }
        content = bfinput(rel_path);
        if(content != NULL){
            content = realloc(content, strlen(content) + 1);
            header.code = 200;
            header.len = strlen(content)/* + 1*/;
            header.type = text;
            strtok(req_header.req_page, ".");
            if(strcmp(strtok(NULL, "."), "css") == 0){
                header.type = css;
            }
            str_header = http_generate_header(header);
            puts(str_header);
            reply = calloc(1, strlen(str_header) + strlen(content) + 1);
            strcpy(reply, str_header);
            strcat(reply, content);
            reply = realloc(reply, strlen(reply) + 1);
            n = write(newsockfd, reply, strlen(reply) + 1);
            if(n < 0){
                error("ERROR writing to socket");
            }
        }
        else {
            header.code = 404;
            header.len = 0;
            header.type = text;
            rel_path = calloc(1, (strlen(assets_path) + strlen("/html/error_404.html") + 1) * sizeof(char));
            strcpy(rel_path, assets_path);
            strcat(rel_path, "/html/error_404.html");
            content = bfinput(rel_path);
            if(content != NULL){
                header.len = strlen(content);
                str_header = http_generate_header(header);
                reply = calloc(1, (strlen(str_header) + strlen(content) + 1) * sizeof(char));
                strcat(reply, str_header);
                strcat(reply, content);
                n = write(newsockfd, reply, strlen(reply) + 1);
            }
            else {
                str_header = http_generate_header(header);
                n = write(newsockfd, str_header, strlen(str_header) + 1);
            }
        }

        if(rel_path != NULL){
            free(rel_path);
        }
        if(content != NULL){
            free(content);
        }
        if(str_header != NULL){
            free(str_header);
        }
        if(reply != NULL){
            free(reply);
        }
        if(req_header.req_page != NULL){
            free(req_header.req_page);
        }
    }

    if(path != NULL){
        free(path);
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
