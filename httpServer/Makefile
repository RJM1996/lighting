ROOT=$(shell pwd)

.PHONY:clean

.PHONY:all

all:httpServer cgi

httpServer:
	cd $(ROOT)/src/; make clean; make; cd -
cgi:
	cd $(ROOT)/webroot/cgi/mysql; make clean; make; cd -
	cd $(ROOT)/webroot/cgi/cal; make clean; make; cd -
	cd $(ROOT)/webroot/cgi/date; make clean; make; cd -
	cd $(ROOT)/webroot/cgi/tcp_client; make clean; make; cd -

clean:
	rm -f httpServer core.*
