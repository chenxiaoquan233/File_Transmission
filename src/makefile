# build path
bin_path = ../bin/

# source path
src_path = src/
serv_src_path = $(src_path)Server/
clit_src_path = $(src_path)Client/
zlib_src_path = $(src_path)zlib/

# obj path
obj_path = $(bin_path)obj/
serv_obj_path = $(obj_path)Server/
clit_obj_path = $(obj_path)Client/
zlib_obj_path = $(obj_path)zlib/

#source files
base_src = $(shell find $(src_path) -maxdepth 1 -name '*.cpp')
zlib_src = $(shell find $(zlib_src_path) -maxdepth 1 -name '*.cpp')
serv_src = $(shell find $(serv_src_path) -maxdepth 1 -name '*.cpp')
clit_src = $(shell find $(clit_src_path) -maxdepth 1 -name '*.cpp')

#objs
base_obj = $(addprefix $(obj_path)/,$(notdir $(base_src:%.cpp=%.o)))
zlib_obj = $(addprefix $(zlib_obj_path)/,$(notdir $(zlib_src:%.cpp=%.o)))

serv_obj = $(addprefix $(serv_obj_path)/,$(notdir $(serv_src:%.cpp=%.o)))
clit_obj = $(addprefix $(clit_obj_path)/,$(notdir $(clit_src:%.cpp=%.o)))
serv_obj += $(base_obj)
serv_obj += $(zlib_obj)
clit_obj += $(base_obj)
clit_obj += $(zlib_obj)

# targets
.PHONY: all clean

all:
	make Server
	make Clinet
	make clean


Server: $(bin_path)Server-linux

Client: $(bin_path)Client-linux

$(bin_path)Server-linux: $(serv_obj)
	g++ -g -o $@ $(serv_obj) -lpthread

$(bin_path)Client-linux: $(clit_obj)
	g++ -g -o $@ $(clit_obj) -lpthread

# objs
$(obj_path)%.o: $(src_path)%.cpp $(obj_path)
	g++ -g -c $< -o $@

$(zlib_obj_path)%.o: $(zlib_src_path)%.cpp $(zlib_obj_path)
	g++ -g -c $< -o $@

$(serv_obj_path)%.o: $(serv_src_path)%.cpp $(serv_obj_path)
	g++ -g -c $< -o $@

$(clit_obj_path)%.o: $(clit_src_path)%.cpp $(clit_obj_path)
	g++ -g -c $< -o $@

$(obj_path):
	mkdir $@

$(serv_obj_path): $(obj_path)
	mkdir $@

$(clit_obj_path): $(obj_path)
	mkdir $@

clean:
	rm -rf ../bin/obj/