/*
 * 임베디드 프로젝트
 * 멜로디 암기 게임 (Melody Memorize Game)
 *
 * 게임 방법:
 * 1. 닉네임을 입력하여 게임을 시작합니다.
 * 2. 멜로디를 주의 깊게 듣고, 기억해 둡니다.
 * 3. 들었던 멜로디를 음에 해당하는 버튼을 눌러 똑같이 연주합니다.
 * 4. 2~3 과정이 반복됩니다. 단계가 올라갈 때마다 새로운 음이 하나씩 멜로디의 끝부분에 추가됩니다.
 * 5. 잘못된 버튼을 누르거나 중도 포기를 할 경우 점수가 표시되며, 게임은 끝이 납니다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#define MAX_STAGE 100 // 이 상수를 변경해 최대 스테이지 수를 설정할 수 있습니다

int melody[MAX_STAGE + 1] = {};
char led_buf[9] = {};
char push_buf[100] = {};
char seg_buf[7] = {};
int vb = 0;
int score = 0;

int convert_to_melody(int num)
{
    int arr[8] = {0, 0, 0, 0, 12, 7, 4, 0};
    return arr[num];
}

int convert_to_led(int num)
{
    if (num == 12)
        return 4;
    else if (num == 7)
        return 5;
    else if (num == 4)
        return 6;
    else
        return 7;
}

void flash_led(int no, int led)
{
    int i;

    for (i = 0; i < 8; i++)
        led_buf[i] = 0;
    if (no != -1)
        led_buf[no] = 1;

    write(led, &led_buf, 8);
}

void play_melody(int stage, int led, int piezo)
{
    int i, j;

    int random = (int)(rand() % 4) + 4;
    melody[stage] = convert_to_melody(random);

    for (i = 1; i <= stage; i++)
    {
        flash_led(convert_to_led(melody[i]), led);
        write(piezo, &melody[i], 4);
    }
}

int guess_melody(int stage, int led, int piezo, int btn)
{
    int i, j;

    for (i = 1; i <= stage; i++)
    {
        read(btn, push_buf, 100);
        if (!strcmp(push_buf, "Down"))
        {
            if (melody[i] == 0)
            {
                score++;
                flash_led(7, led);
                write(piezo, &melody[i], 4);
            }
            else
                return 2;
        }
        else if (!strcmp(push_buf, "Left"))
        {
            if (melody[i] == 4)
            {
                score++;
                flash_led(6, led);
                write(piezo, &melody[i], 4);
            }
            else
                return 2;
        }
        else if (!strcmp(push_buf, "Right"))
        {
            if (melody[i] == 7)
            {
                score++;
                flash_led(5, led);
                write(piezo, &melody[i], 4);
            }
            else
                return 2;
        }
        else if (!strcmp(push_buf, "Up"))
        {

            if (melody[i] == 12)
            {
                score++;
                flash_led(4, led);
                write(piezo, &melody[i], 4);
            }
            else
                return 2;
        }
        else
            return -1;
    }

    return 1;
}

void print_result(int score, int vibe, int seg)
{
    int i;

    if (score > 999999)
        score = 999999;

    vb = 1;
    write(vibe, &vb, 1);
    sleep(1);
    vb = 0;
    write(vibe, &vb, 1);

    for (i = 0; i < 6; i++)
        seg_buf[i] = 0;

    for (i = 5; i >= 0; i--)
    {
        if (score > 0)
        {
            seg_buf[i] = score % 10;
            score /= 10;
        }
        else
            break;
    }

    for (i = 0; i < 300; i++)
        write(seg, seg_buf, 6);
}

int main(int argc, char *argv[])
{
    int led = open("/dev/sm9s5422_led", O_WRONLY);
    int piezo = open("/dev/sm9s5422_piezo", O_WRONLY);
    int seg = open("/dev/sm9s5422_segment", O_RDWR | O_SYNC);
    int btn = open("/dev/sm9s5422_interrupt", O_RDWR);
    int vibe = open("/dev/sm9s5422_perivib", O_WRONLY);

    char username[41];
    int i, j, k;
    int result;

    srand((unsigned int)time(NULL));

    if (led < 0 || piezo < 0 || seg < 0 || btn < 0 || vibe < 0)
    {
        printf("드라이버를 불러오지 못 했습니다. 모든 드라이버를 빌드하신 후, 다시 시도해 주세요.\n");
        printf("게임을 종료합니다.");
        exit(1);
    }

    printf("┏━━━━━━━━━━━━━━┓\n");
    printf("┃  ☞  MEMORIZE GAME  ☜    ┃\n");
    printf("┗━━━━━━━━━━━━━━┛\n");
    printf("닉네임을 입력하면 게임을 시작합니다(20자 이내) ☞ ");
    scanf("%s", &username);
    printf("『%s』 님 환영합니다.\n", username);

    for (i = 1; i <= MAX_STAGE; i++)
    {
        printf("\n\n【 STAGE %d 】\n\n", i);
        printf("★ 3초 후 멜로디를 들려드립니다. 준비하세요!\n");
        sleep(3);

        printf("♬ 멜로디가 재생되고 있습니다. 주의 깊게 들으세요!\n");
        play_melody(i, led, piezo);
        flash_led(-1, led);

        printf("▶ 이제, 들려드린 멜로디를 기억해서 다시 연주해 보세요!\n");
        printf("   【도: ↓ 버튼】 【미: ← 버튼】 【솔: → 버튼】\n");
        printf("   【높은 도: ↑ 버튼】 【포기: 가운데 버튼】\n");
        result = guess_melody(i, led, piezo, btn);
        flash_led(-1, led);

        if (result == 1)
            printf("♥ 모든 멜로디를 맞추셨습니다!\n");
        else if (result == 2)
        {
            printf("▼ 아쉽게도, 틀렸습니다!\n");
            break;
        }
        else
        {
            printf("▼ 게임을 포기했습니다!\n");
            break;
        }
    }

    printf("\n\n┏━━━━━━━━━━━━━━┓\n");
    printf("┃  ☞  MEMORIZE GAME  ☜    ┃\n");
    printf("┗━━━━━━━━━━━━━━┛\n");
    printf("게임 끝!\n");
    printf("『%s』 님의 점수는 『%d』점 입니다!\n", username, score);

    print_result(score, vibe, seg);

    close(led);
    close(piezo);
    close(seg);
    close(btn);
    close(vibe);

    return 0;
}
