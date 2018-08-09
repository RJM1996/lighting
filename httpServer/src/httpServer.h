
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "url.h"

#define MAX_SIZE 1024*4

// 状态行 空行
const char* status_line = "HTTP/1.0 200 OK\r\n";
const char* blank_line = "\r\n";

int get_line(int sock, char line[], int size);
void status_response(int sock, int status_code);
int echo_www(int sock, const char* resource_path, int size);
void handle_hander(int sock);
static void* handle_request(void* arg);
int startUp(int port);
