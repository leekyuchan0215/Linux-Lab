# ==== 설정 ====
CC      = gcc
CFLAGS  = -Iinclude -Wall
AR      = ar
ARFLAGS = rcs

SRC_DIR = src
INC_DIR = include
LIB_DIR = lib
APP_DIR = app

# 연산 함수 오브젝트들은 lib/ 밑에 생성
OBJS = $(LIB_DIR)/add.o $(LIB_DIR)/sub.o $(LIB_DIR)/mul.o $(LIB_DIR)/div.o

STATIC_LIB = $(LIB_DIR)/liboperations.a
SHARED_LIB = $(LIB_DIR)/liboperations.so

APP_STATIC = $(APP_DIR)/main_static
APP_SHARED = $(APP_DIR)/main_shared
APP_DYN    = $(APP_DIR)/dyn_main

.PHONY: all static shared apps clean

# 한 번에 전부
all: $(APP_STATIC) $(APP_SHARED) $(APP_DYN)

# ----- 오브젝트 규칙 (PIC로 만들어 .so에 재사용) -----
$(LIB_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/cal.h
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

# ----- 정적 라이브러리 -----
$(STATIC_LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# ----- 공유 라이브러리 -----
$(SHARED_LIB): $(OBJS)
	$(CC) -shared -o $@ $^

# ----- 실행파일 (정적 링크) -----
$(APP_STATIC): $(APP_DIR)/main.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -loperations -o $@

# ----- 실행파일 (공유 링크) -----
$(APP_SHARED): $(APP_DIR)/main.c $(SHARED_LIB)
	# rpath: 실행 시 app/ 기준 ../lib 에서 .so 찾기
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -loperations -Wl,-rpath='$$ORIGIN/../lib' -o $@

# ----- 실행파일 (동적 로딩 dlopen) -----
$(APP_DYN): $(APP_DIR)/dyn_main.c $(SHARED_LIB)
	$(CC) $(APP_DIR)/dyn_main.c -ldl -o $@

# 편의 타겟
static: $(APP_STATIC)
shared: $(APP_SHARED)
apps:   $(APP_STATIC) $(APP_SHARED) $(APP_DYN)

clean:
	rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a $(LIB_DIR)/*.so \
	      $(APP_STATIC) $(APP_SHARED) $(APP_DYN)

