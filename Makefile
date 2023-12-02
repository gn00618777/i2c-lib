TARGET = libi2c.so

DST_DEVPKG ?= .
DST_PKG ?= .
CC_OPT = -O3
CFLAGS = $(CC_OPT) -fPIC $(DEVPKG_CFLAGS)
CFLAGS += -I./include
OBJS := $(shell find ./ -name "*.c" | sed 's/\.cpp/\.o/g')
all: clean $(TARGET) install

$(TARGET):$(OBJS)
	$(CC) -shared $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo "clean"
	rm -rf *.o src/*.o $(TARGET)

install:
	@echo "Copy i2c library..."
	mkdir -p ${DST_DEVPKG}/lib
	mkdir -p ${DST_PKG}/lib
	cp -f $(TARGET) ${DST_DEVPKG}/lib
	cp -f $(TARGET) ${DST_PKG}/lib
	rm -f $(TARGET)

.PHONY: all clean install
