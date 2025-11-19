/*
 * notify_server.c
 * 기능: D-Bus를 통해 클라이언트로부터 알림 메시지와 긴급도를 수신하는 서버
 * 라이브러리: libsystemd (sd-bus)
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <systemd/sd-bus.h>

#define SERVICE_NAME "com.university.lab.Notify"
#define OBJECT_PATH  "/com/university/lab/Notify"
#define INTERFACE_NAME "com.university.lab.Notify.Manager"

// 메서드 콜백 함수: 클라이언트가 "SendNotification" 메서드를 호출하면 실행됨
static int method_send_notification(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
    const char *message;
    int level;
    char response[128];

    // 1. 인자 파싱 (s: 문자열, i: 정수)
    // 클라이언트로부터 "메시지(string)"와 "긴급도(int)"를 읽음
    int r = sd_bus_message_read(m, "si", &message, &level);
    if (r < 0) {
        fprintf(stderr, "[서버] 인자 파싱 실패: %s\n", strerror(-r));
        return r;
    }

    // 2. 로직 처리 (화면 출력)
    printf("\n=== [서버] 알림 수신 ===\n");
    printf("내용: %s\n", message);
    printf("긴급도: %d ", level);
    
    if (level >= 5) printf("(매우 긴급!)\n");
    else printf("(일반)\n");
    printf("========================\n");

    // 3. 클라이언트에게 응답 전송
    sprintf(response, "알림이 접수되었습니다. (처리 ID: %d)", rand() % 1000);
    
    // 결과 리턴 (s: 문자열)
    return sd_bus_reply_method_return(m, "s", response);
}

// VTable 정의: D-Bus 인터페이스 명세
static const sd_bus_vtable notify_vtable[] = {
    SD_BUS_VTABLE_START(0),
    
    // 메서드 등록: 이름 "SendNotification", 입력 "si", 출력 "s"
    SD_BUS_METHOD_WITH_ARGS("SendNotification",
                            SD_BUS_ARGS("s", message, "i", level),
                            SD_BUS_ARGS("s", reply),
                            method_send_notification,
                            SD_BUS_VTABLE_UNPRIVILEGED),
                            
    SD_BUS_VTABLE_END
};

int main(int argc, char *argv[]) {
    sd_bus_slot *slot = NULL;
    sd_bus *bus = NULL;
    int r;

    // 1. 사용자 세션 버스에 연결
    r = sd_bus_default_user(&bus);
    if (r < 0) {
        fprintf(stderr, "[서버] 버스 연결 실패: %s\n", strerror(-r));
        goto finish;
    }

    // 2. 객체와 인터페이스 등록 (VTable 연결)
    r = sd_bus_add_object_vtable(bus,
                                 &slot,
                                 OBJECT_PATH,
                                 INTERFACE_NAME,
                                 notify_vtable,
                                 NULL);
    if (r < 0) {
        fprintf(stderr, "[서버] 객체 등록 실패: %s\n", strerror(-r));
        goto finish;
    }

    // 3. 서비스 이름 요청 (잘 알려진 이름 등록)
    r = sd_bus_request_name(bus, SERVICE_NAME, 0);
    if (r < 0) {
        fprintf(stderr, "[서버] 서비스 이름 획득 실패: %s\n", strerror(-r));
        goto finish;
    }

    printf("[서버] 서비스 시작: %s\n", SERVICE_NAME);
    printf("[서버] 클라이언트 요청 대기 중...\n");

    // 4. 이벤트 루프
    while (1) {
        // 버스 이벤트 처리
        r = sd_bus_process(bus, NULL);
        if (r < 0) {
            fprintf(stderr, "[서버] 버스 처리 오류: %s\n", strerror(-r));
            goto finish;
        }

        // 처리할 작업이 없으면 대기
        if (r == 0) {
            r = sd_bus_wait(bus, (uint64_t) -1);
            if (r < 0) {
                fprintf(stderr, "[서버] 대기 오류: %s\n", strerror(-r));
                goto finish;
            }
        }
    }

finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);
    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
