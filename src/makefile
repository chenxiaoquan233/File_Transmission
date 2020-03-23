bin_path = ../bin/
obj_path = $(bin_path)obj/
serv_obj_path = $(obj_path)Server/
clit_obj_path = $(obj_path)Client/
src_path = src/
serv_src_path = $(src_path)Server/
clit_src_path = $(src_path)Client/
serv_obj = $(serv_obj_path)main.o $(serv_obj_path)Server.o
clit_obj = $(clit_obj_path)main.o $(clit_obj_path)Client.o

all: $(obj_path) $(serv_obj_path) $(clit_obj_path) $(bin_path)Server-linux $(bin_path)Client-linux
	make clean

$(bin_path)Server-linux: $(serv_obj)
	g++ -g -o $@ $(serv_obj) -lpthread

$(bin_path)Client-linux: $(clit_obj)
	g++ -g -o $@ $(serv_obj) -lpthread

$(serv_obj_path)main.o: $(serv_src_path)main.cpp
	g++ -g -c $^ -o $@

$(serv_obj_path)Server.o: $(serv_src_path)Server.cpp
	g++ -g -c $^ -o $@

$(clit_obj_path)main.o: $(clit_src_path)main.cpp
	g++ -g -c $^ -o $@

$(clit_obj_path)Client.o: $(clit_src_path)Client.cpp
	g++ -g -c $^ -o $@

$(obj_path):
	mkdir $@

$(serv_obj_path):
	mkdir $@

$(clit_obj_path):
	mkdir $@

clean:
	rm -rf ../bin/obj/