
SOURCE_PATH = source
HEADER_PATH = include
OBJ_PATH    = obj

CUR_CFLAGS  = $(CFLAGS) -I$(HEADER_PATH) -I$(INCLUDE)

SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.c)
OBJ_FILES    = $(addprefix $(OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(SOURCE_FILES)))))

TARGET = libdeint-overflow.so

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(LD) -o $@ $^ $(LDFLAGS)
	
$(OBJ_PATH)/%.o: $(SOURCE_PATH)/%.c
	@if [ ! -d $(OBJ_PATH) ];then mkdir $(OBJ_PATH); fi
	$(CC) $(CUR_CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_PATH) $(TARGET)

