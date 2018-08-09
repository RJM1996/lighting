
.PHONY:clean 
.PHONY:all

all:select insert

select:select.c
	gcc $^ -o $@ -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
insert:insert.c
	gcc $^ -o $@ -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -rf select insert

