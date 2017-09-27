OBJ_NAME=dbproxy
EXE=lib$(OBJ_NAME).so
CC=g++ -std=c++0x

#编译选项

EL_LIBS_FLAG=-Wl,-rpath,$(EL_LIB_PATH)
FLAG=-Wall -fPIC -MMD -shared -DDEBUG -g -O0 -o

#包含的头文件
INCLUDES=-I/usr/include/libxml2/ -I$(EL_LIB_INCLUDE)/include -I$(AS_INCLUDE)/include -I../share
#-I$(SHARE_PATH)

LIBS=-L/usr/lib64 -L/usr/lib -L$(EL_LIB_PATH)

LINK=-lxml2 -lel $(EL_LIBS_FLAG)

#编译的代码文件
LOCAL_SRCS =$(wildcard ./*.cpp)
OTHER_SRCS =

$(EXE):
	rm -f $(EXE)
	$(CC) -ldl -lrt $(LOCAL_SRCS) $(OTHER_SRCS) $(INCLUDES) $(LINK) $(LIBS) $(FLAG) $@ 
	mkdir -p $(EL_LIB_PATH)
#	cp $(EXE) ../bin/
	cp $(EXE) $(EL_LIB_PATH)
#	cp $(EL_LIB_PATH)async-server.exe $(SERVER_HOME)$(OBJ_NAME)/bin/$(OBJ_NAME).exe
	rm -f $(EXE)
	rm -f lib$(OBJ_NAME).d
clean:
	rm -f $(EXE)
