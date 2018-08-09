#include "date.h"

void Date_cal(char* arg)
{
    printf("<html>");
    printf("<meta charset=utf-8>");
    printf("<h2>Hello Date Cal</h2>");
    size_t year = 0;
    size_t month = 0;
    size_t day = 0;
    int num = 0;
    // year=2018&month=7&day=19&nums=100
    sscanf(arg, "year=%lu&month=%lu&day=%lu&num=%d", &year, &month, &day, &num); 
    Date d1(year, month, day);
    Date d2 = d1 + num;
    printf("<h4>%lu - %lu - %lu after %d days is</h4>", year, month, day, num);
    d2.Display();
    printf("</html>");
}
