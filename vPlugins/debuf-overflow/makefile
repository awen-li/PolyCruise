
CC = clang 
LD = clang

SOURCE_PATH = source
HEADER_PATH = include
OBJ_PATH    = obj

CFLAGS      = -D__DEBUG__ -fPIC -g -I$(HEADER_PATH) -I$(INCLUDE)

SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.c)
OBJ_FILES    = $(addprefix $(OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(SOURCE_FILES)))))

TARGET = libdebuf-overflow.so

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(LD) -fPIC -shared $(PLUGIN_LINK) -o $@ $^
	
$(OBJ_PATH)/%.o: $(SOURCE_PATH)/%.c
	@if [ ! -d $(OBJ_PATH) ];then mkdir $(OBJ_PATH); fi
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_PATH) $(TARGET)
