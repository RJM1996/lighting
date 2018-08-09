
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

// cgi 程序
// 计算器
void my_cal(char* arg)
{
    printf("<html>");
    printf("<meta charset=utf-8>");
    printf("<h1>hello, cal !</h1>");
    int num1 = 0;
    int num2 = 0;
    // int sscanf(const char *str, const char *format, ...);
    sscanf(arg, "firstdata=%d&lastdata=%d", &num1, &num2);
    printf("<h3>%d + %d = %d</h3>", num1, num2, num1 + num2);
    printf("<h3>%d - %d = %d</h3>", num1, num2, num1 - num2);
    printf("<h3>%d * %d = %d</h3>", num1, num2, num1 * num2);
    if(num2 == 0)
    {
        printf("<h3>%d / %d = -1</h3>", num1, num2);
        printf("<h3>%d %% %d = -1</h3>", num1, num2);
        printf("<h3>除数不能为 0 !!!</h3>");
    }
    else
    {
        printf("<h3>%d / %d = %d</h3>", num1, num2, num1 / num2);
        printf("<h3>%d %% %d = %d</h3>", num1, num2, num1 % num2);
    }
    printf("</html>");
}


int main()
{
    char* method = NULL;
    char* query_string = NULL;
    char* arg = NULL;
    int content_len = -1;
    char buf[1024];
    if(getenv("METHOD") != NULL)
    {
        method = getenv("METHOD");
        if(strcasecmp(method, "GET") == 0)
        {
            if(getenv("QUERY_STRING") != NULL)
            {
                query_string = getenv("QUERY_STRING");
                arg = query_string;
            }
        }
        else
        {
            // post
            if(getenv("CONTENT_LENGTH") != NULL)
            {
                content_len = atoi(getenv("CONTENT_LENGTH"));
                int i = 0;
                for(; i < content_len; i++)
                {
                    read(0, &buf[i], 1);
                }
                buf[i] = '\0';
                arg = buf;
            }
        }
    }
    my_cal(arg);
    return 0;
}
