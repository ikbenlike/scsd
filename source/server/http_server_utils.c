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

void error(char *msg){
    perror(msg);
    exit(1);
}

char *finput(char *dest, char *path, int size){
    int i = 0;
    char c;
    FILE *fp = fopen(path, "r");
    if(fp == NULL){
        perror("fopen");
        return NULL;
    }
    while((c= fgetc(fp)) != EOF){
        dest[i] = c;
        if(i >= size){
            size += 20;
            dest = realloc(dest, size);
        }
        if(dest[i] == EOF){
            dest[i] = '\0';
            break;
        }
        i++;
    }
    fclose(fp);
    dest[i] = '\0';
    return dest;
}

char *bfinput(char *path){
    struct stat *st = calloc(1, sizeof(struct stat));
    stat(path, st);
    int size = st->st_size;
    free(st);
    printf("File '%s' size: %d\n", path, size);
    FILE *fp = fopen(path, "r");
    if(fp == NULL){
        perror("fopen");
        return NULL;
    }
    char *buf = calloc(1, size * (sizeof(char) + 1));
    int i = 0;
    for(i = 0; i <= size; i++){
        buf[i] = fgetc(fp);
        if(buf[i] == EOF){
            buf[i] = '\0';
            break;
        }
    }
    printf("File '%s' len: %d\n", path, i);
    fclose(fp);
    return buf;
}

http_req_t http_parse_header(char *header){
    puts("parsing header");
    //puts(header);
    http_req_t req_header;
    int header_len = strlen(header) + 1;
    char *part = strtok(header, " ");
    //puts(part);
    while(part){
        if(strcmp(part, "GET") == 0){
            req_header.req_type = get;
            part = strtok(NULL, " ");
            if(part != NULL){
                if(strcmp(part, "/") != 0){
                    req_header.req_page = calloc(1, strlen(part) + 1);
                    strcpy(req_header.req_page, part);
                    continue;
                }
                else {
                    req_header.req_page = NULL;
                    continue;
                }
            }
        }
        else if(strcmp(part, "PUT") == 0){
            req_header.req_type = put;
            if(part != NULL){
                req_header.req_page = calloc(1, strlen(part) + 1);
                strcpy(req_header.req_page, part);
                continue;
            }
        }
        else if(strcmp(part, "POST") == 0){
            req_header.req_type = post;
            if(part != NULL){
                req_header.req_page = calloc(1, strlen(part) + 1);
                strcpy(req_header.req_page, part);
                continue;
            }
        }

        part = strtok(NULL, " ");
    }

    return req_header;
}

char *http_generate_header(http_header_t header_data){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    //printf("%s\n", asctime(tm));
    char *header_date = calloc(1, 32 * sizeof(char));
    sprintf(header_date, "Date: %s", asctime(tm));
    //free(tm);
    char* std_header = calloc(1, 72 * sizeof(char));
    sprintf(std_header, "HTTP/1.1 %d OK\nServer: SCSDv0.0\nAccept-Ranges: none\nConnection: Close\n", header_data.code);
    char* content_t;
    char* len = calloc(1, 38 * sizeof(char));
    char* header = NULL;
    if(header_data.type == text){
        content_t = "Content-Type: text/html\n";
    }
    if(header_data.type == css){
        content_t = "Content-Type: text/css\n";
    }
    else if(header_data.type == image){
        content_t = "Content-Type: image\n";
    }
    else if(header_data.type == video){
        content_t = "Content-Type: video\n";
    }
    else if(header_data.type == audio){
        content_t = "Content-Type: audio\n";
    }
    sprintf(len, "Content-Length: %d\n\n", header_data.len);
    header = calloc(1, sizeof(char) * (strlen(std_header) + strlen(content_t) + strlen(len) + strlen(header_date) + 1));
    strcat(header, std_header);
    strcat(header, header_date);
    strcat(header, content_t);
    strcat(header, len);
    free(header_date);
    free(len);
    header = realloc(header, strlen(header) + 1);
    return header;
}
