#include <stdio.h>

int main() {
    char *a = "hello";

    printf("%c\n", a[5]);   // 输出的是 '\0'，通常看不到任何字符
    printf("%c\n", a[4]);   // 输出 'o'

    return 0;
}
