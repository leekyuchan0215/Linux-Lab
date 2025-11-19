/*
 * notify_client.c
 * 기능: D-Bus 서버에 메서드를 호출하여 알림을 보내는 클라이언트
 * 사용법: ./notify_client "메시지 내용" [긴급도숫자]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>

#define SERVICE_NAME "com.university.lab.Notify"
#define OBJECT_PATH  "/com/university/lab/Notify"
#define INTERFACE_NAME "com.university.lab.Notify.Manager"

int main(int argc, char *argv[]) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    const char *response_msg;
    const char *send_msg = "기본 테스트 메시지";
    int level = 1;
    int r;

    // 인자 처리
    if (argc > 1) send_msg = argv[1];
    if (argc > 2) level = atoi(argv[2]);

    // 1. 사용자 세션 버스 연결
    r = sd_bus_default_user(&bus);
    if (r < 0) {
        fprintf(stderr, "[클라이언트] 버스 연결 실패: %s\n", strerror(-r));
        goto finish;
    }

    printf("[클라이언트] 서버(%s)에 메시지 전송 시도...\n", SERVICE_NAME);

    // 2. 메서드 호출 (동기 방식)
    // - 서비스이름, 객체경로, 인터페이스, 메서드명
    // - 인자 타입: "si" (String, Int)
    r = sd_bus_call_method(bus,
                           SERVICE_NAME,
                           OBJECT_PATH,
                           INTERFACE_NAME,
                           "SendNotification",
                           &error,      // 에러 담을 구조체
                           &m,          // 응답 메시지 담을 포인터
                           "si",        // 인자 시그니처
                           send_msg,    // 인자 1 (메시지)
                           level);      // 인자 2 (긴급도)

    if (r < 0) {
        fprintf(stderr, "[클라이언트] 메서드 호출 실패: %s\n", error.message);
        goto finish;
    }

    // 3. 서버로부터 받은 응답 파싱 (s: 문자열)
    r = sd_bus_message_read(m, "s", &response_msg);
    if (r < 0) {
        fprintf(stderr, "[클라이언트] 응답 파싱 실패: %s\n", strerror(-r));
        goto finish;
    }

    printf("[클라이언트] 서버 응답: %s\n", response_msg);

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
