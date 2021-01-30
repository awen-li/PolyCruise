
CC = clang 
LD = clang

SOURCE_PATH  = ./
INCLUDE_PATH = ../C/include
OBJ_PATH     = obj

ENV_LDFLAGS= -flto
ENV_CFLAGS = -emit-llvm
CFLAGS      = -D__DEBUG__ $(ENV_CFLAGS) -fPIC -O3 -I$(INCLUDE_PATH) -I/usr/include/python3.6

SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.c)
OBJ_FILES     = $(addprefix $(OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(SOURCE_FILES)))))

TARGET = PyDemo.so

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(LD) $(ENV_LDFLAGS) -fPIC -shared -L/usr/lib -lDemoTrace -o $@ $^
	
$(OBJ_PATH)/%.o: $(SOURCE_PATH)/%.c
	@if [ ! -d $(OBJ_PATH) ];then mkdir $(OBJ_PATH); fi
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_PATH) $(TARGET)
