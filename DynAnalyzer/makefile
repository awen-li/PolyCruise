
CC = clang 
LD = clang
AR = ar -crv
SOURCE_PATH  = source
INCLUDE_PATH = include
OBJ_PATH     = obj

#CFLAGS     = -fPIC -O3 -I$(INCLUDE_PATH) -D__DEBUG__ 
CFLAGS      = -O3 -I$(INCLUDE_PATH)
LD_CFLAGS   = -lpthread -ldl 

SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.c)
OBJ_FILES    = $(addprefix $(OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(SOURCE_FILES)))))

TARGET = difaEngine

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(LD) -o $@ $^ $(LD_CFLAGS)
	
$(OBJ_PATH)/%.o: $(SOURCE_PATH)/%.c
	@if [ ! -d $(OBJ_PATH) ];then mkdir $(OBJ_PATH); fi
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_PATH) $(TARGET)
