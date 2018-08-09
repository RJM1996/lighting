
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include </usr/include/mysql/mysql.h>

void instert_data(char *name, char *sex, char *phone)
{
    printf("<html>");
    printf("<meta charset=utf-8>");
    MYSQL *mysql_fd = mysql_init(NULL);
    if(mysql_real_connect(mysql_fd, 
                          "127.0.0.1", "root", "4321", "http", 
                          3306, NULL, 0) == NULL)
    {
        printf("connect failed!\n");
        return;
    }
    printf("connect mysql success!\n");

    mysql_set_character_set(mysql_fd, "utf8");
    
    char sql[1024];

    sprintf(sql, "INSERT INTO user (name, sex, phone) VALUES (\"%s\", \"%s\", \"%s\")", name, sex, phone);

    const char* charset = mysql_character_set_name(mysql_fd);
    printf("<字符集: %s><br>", charset);
    printf("<h4>插入语句: %s</h4>", sql);

    if(mysql_query(mysql_fd, sql) != 0)
    {
        printf("<h3>插入失败 !!!</h3>\n");
    }
    else
    {
        printf("<h3>插入成功 !!!</h3>\n");
    }
    printf("</html>");
    mysql_close(mysql_fd);
}

int main()
{
    char data[1024];
    if(getenv("METHOD"))
    {
        if(strcasecmp("GET", getenv("METHOD")) == 0)
        {
            strcpy(data, getenv("QUERY_STRING"));
        }
        else
        {
            int content_length = atoi(getenv("CONTENT_LENGTH"));
            int i = 0;
            for(; i < content_length; i++)
            {
                read(0, data + i, 1);
            }
            data[i] = 0;
        }
    }

    //printf("参数: %s", data);

    //name=""&sex=""&phone=""
    char *name;
    char *sex;
    char *phone;

    strtok(data, "=&");
    name = strtok(NULL, "=&");
    strtok(NULL, "=&");
    sex = strtok(NULL, "=&");
    strtok(NULL, "=&");
    phone = strtok(NULL, "=&");

    instert_data(name, sex, phone);
    return 0;
}
