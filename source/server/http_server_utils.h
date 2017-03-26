#ifndef __HTTP_SERVER_UTILS_H_
    #define __HTTP_SERVER_UTILS_H_
#endif


typedef enum {
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


void error(char *msg);
char *finput(char *dest, char *path, int size);
char *bfinput(char *path);
http_req_t http_parse_header(char *header);
char *http_generate_header(http_header_t header_data);
