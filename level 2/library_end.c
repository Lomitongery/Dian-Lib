#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 全局常量：保留7天的时间维度（无需动态调整）
#define DAYS 7

// 动态座位结构体：支持层、行、列的添加/删减
typedef struct {
    int floor_cnt;       // 当前实际层数（如5层、6层）
    int* row_cnt;        // 每行的实际行数：row_cnt[层] = 该层的总行数（如4行、5行）
    int** col_cnt;       // 每行列数：col_cnt[层][行] = 该行的总列数（如4列、6列）
    char*** seat;        // 动态三维指针：seat[层][行][列]，存储座位状态（'0'=空闲，用户首字符=已预约）
} Library;

// 初始化图书馆数据
bool init_library(Library lib[], int init_floor, int init_row_per_floor, int init_col_per_row);

// 文件读写
bool load_library(const char* filename, Library lib[]); // 读取文件数据
bool save_library(const char* filename, Library lib[]); // 保存数据到文件

// 处理输入
char* Read(); // 读取长度未知的字符串
char** split(char* input); // 分割读入的字符串 形成有效的字符串组

// 帮助函数
void Help();

// 判断转换
int judge(char** str, Library lib[]); // 检测指令类型
int judge_day(char* str); // 将星期转换为数字
char* Turn_day(int i); // 将数字转换为星期
int judge_fnum(char* str, int num); // 类型转换 字符串转为数字 针对层
int judge_snum(char* str, int num); // 类型转换 针对行列
int judge_name(char* str); // 检测用户名是否合法
int judge_state(char* str); // 判断指令是open还是close
int judge_reserve(char** str, Library lib[]); // 用于Admin的判断预约操作
bool judge_user(char* str); // 检测是否为用户
bool judge_admin(char* str); // 检测是否为管理员
bool judge_num(char* str); // 判断是否为合法数字

// 图书馆通用功能
void Login(char** user); // 登录
void Exit(char** user); // 登出
void Query(char** str, char** user, Library lib[]); // 查询某天某层预约

// 图书馆用户功能
void Reservation(char* user, Library lib[]); // 查询用户所有预约
void Reserve(char** str, char* user, Library lib[]); // 预定座位

// 图书馆管理员功能
void Clear(char* user, Library lib[]); // Admin清空所有预约 包括临时禁用/启用
void Adjust(char** str, char* user, Library lib[]); // 临时禁用/启用某一整天座位
void Adjust_floors(char** str, char* user, Library lib[]); // 临时禁用/启用某天某层座位
void Change(char** str, char* user, Library lib[]); // 永久改变地形
void Help_Reserve(char* user, Library lib[]); // 帮助预约
void Delete(char* user, Library lib[]); // 取消预约

int main() {
    Library Dian[DAYS];
    // if (!init_library(Dian, 5, 4, 4)) {
    //     printf("Failed to initialize library.\n");
    //     return 1;
    // }

    // 下载文件数据
    const char* filename = "library.txt";
    if (!load_library(filename, Dian)) {
        printf("Empty/Corrupted file detected. Reinitializing library data.\n");
        // 重新初始化
        if (!init_library(Dian, 5, 4, 4)) {
            printf("Failed to initialize library.\n");
            return 1;
        }

        if (save_library(filename, Dian)) {
            printf("Library data saved successfully.\n");
        }
        else printf("Failed to save library data.\n");
    }
    else printf("Library data loaded successfully.\n");

    char* help = "Help";
    printf("If you need help, you can entre %s.\n", help);

    char* user = NULL;
    char* input;
    while (1) {
        // 输入指令
        input = Read();
        if (input == NULL) {
            printf("Memory allocation failed.\n");
            continue;
        }

        // 分割输入进str
        char** str = split(input);
        if (str == NULL) {
            printf("Memory allocation failed.\n");
            continue;
        }

        /*
            0 错误/无效输入
            1 退出程序
            2 登录
            3 登出
            4 查询用户所有预约
            5 Admin清空所有预约
            6 查询某天某层预约
            7 预定座位
            8 寻求帮助
            9 临时禁用/启用某一整天座位
            10 临时禁用/启用某天某层座位
            11 永久改变地形
            12 帮助预约
            13 Admin取消预约
        */

        // 判断指令
        int status = judge(str, Dian);
        if (status == 0) printf("Invalid input.\n");
        if (status == 1) break;
        if (status == 2) Login(&user);
        if (status == 3) Exit(&user);
        if (status == 4) Reservation(user, Dian);
        if (status == 5) Clear(user, Dian);
        if (status == 6) Query(str, &user, Dian);
        if (status == 7) Reserve(str, user, Dian);
        if (status == 8) Help();
        if (status == 9) Adjust(str, user, Dian);
        if (status == 10) Adjust_floors(str, user, Dian);
        if (status == 11) Change(str, user, Dian);
        if (status == 12) Help_Reserve(user, Dian);
        if (status == 13) Delete(user, Dian);

        // 释放内存
        free(input);
        free(str);
    }

    // 保存数据到文件
    if (save_library(filename, Dian)) printf("Library data saved successfully.\n");
    else printf("Failed to save library data.\n");

    // 告知退出程序
    printf("Program exited.\n");

    return 0;
}

// 初始化图书馆数据
bool init_library(Library* lib, int init_floor, int init_row_per_floor, int init_col_per_row) {
    for (int d = 0; d < DAYS; d++) {
        lib[d].floor_cnt = init_floor;
        lib[d].row_cnt = (int*)malloc(init_floor * sizeof(int));
        if (lib[d].row_cnt == NULL) {
            perror("malloc");
            return false;
        }

        lib[d].col_cnt = (int**)malloc(init_floor * sizeof(int*));
        if (lib[d].col_cnt == NULL) {
            perror("malloc");
            free(lib[d].row_cnt);
            return false;
        }

        lib[d].seat = (char***)malloc(init_floor * sizeof(char**));
        if (lib[d].seat == NULL) {
            perror("malloc");
            free(lib[d].row_cnt);
            free(lib[d].col_cnt);
            return false;
        }

        for (int f = 0; f < lib[d].floor_cnt; f++) {
            lib[d].row_cnt[f] = init_row_per_floor;
            lib[d].col_cnt[f] = (int*)malloc(init_row_per_floor * sizeof(int));
            if (lib[d].col_cnt[f] == NULL) {
                perror("malloc");
                free(lib[d].row_cnt);
                free(lib[d].col_cnt);
                free(lib[d].seat);
                return false;
            }

            lib[d].seat[f] = (char**)malloc(init_row_per_floor * sizeof(char*));
            if (lib[d].seat[f] == NULL) {
                perror("malloc");
                free(lib[d].row_cnt);
                free(lib[d].col_cnt);
                free(lib[d].seat);
                return false;
            }

            for (int r = 0; r < lib[d].row_cnt[f]; r++) {
                lib[d].col_cnt[f][r] = init_col_per_row;
                lib[d].seat[f][r] = (char*)malloc(init_col_per_row * sizeof(char));
                if (lib[d].seat[f][r] == NULL) {
                    perror("malloc");
                    free(lib[d].row_cnt);
                    free(lib[d].col_cnt);
                    free(lib[d].seat);
                    return false;
                }

                for (int c = 0; c < lib[d].col_cnt[f][r]; c++) {
                    memset(lib[d].seat[f][r], '0', lib[d].col_cnt[f][r] * sizeof(char));
                }
            }
        }
    }

    // printf("%d floor * per floor %d row * per row %d column\n",
    //        init_floor, init_row_per_floor, init_col_per_row);
    return true;
}

// 文件读写
bool load_library(const char* filename, Library lib[]) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Failed to open file.\n");
        return false;
    }

    // 初始化

    // if (!init_library(lib, lib[0].floor_cnt, lib[0].row_cnt, 4)) {
    //      printf("Failed to initialize library.\n");
    //      return 1;
    // }

    // 遍历下载
    for (int d = 0; d < DAYS; d++) {
        for (int f = 0; f < lib[d].floor_cnt ; f++) {
            for (int r = 0; r < lib[d].row_cnt[f] ; r++) {
                for (int c = 0; c < lib[d].col_cnt[f][r] ; c++) {
                    int ch;
                    while ((ch = fgetc(fp)) == ' ') {
                        if (ch == EOF) {
                            printf("File read error.\n");
                            fclose(fp);
                            return false;
                        }
                    }
                    lib[d].seat[f][r][c] = (char)ch;
                }
                int ch;
                while ((ch = fgetc(fp)) != '\n') {
                    if (ch == EOF) {
                        printf("File read error.\n");
                        fclose(fp);
                        return false;
                    }
                }
            }
            int ch;
            while ((ch = fgetc(fp)) != '\n') {
                if (ch == EOF) {
                    printf("File read error.\n");
                    fclose(fp);
                    return false;
                }
            }
        }
        int ch;
        while ((ch = fgetc(fp)) != '\n') {
            if (ch == EOF) {
                printf("File read error.\n");
                fclose(fp);
                return false;
            }
        }
    }
    fclose(fp);
    return true;
}
bool save_library(const char* filename, Library lib[]) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Failed to open file.\n");
        return false;
    }
    for (int d = 0; d < DAYS; d++) {
        //printf("Day %d\n", i + 1);
        for (int f = 0; f < lib[d].floor_cnt ; f++) {
            //printf("Level %d\n", l + 1);
            for (int r = 0; r < lib[d].row_cnt[f] ; r++) {
                for (int c = 0; c < lib[d].col_cnt[f][r] ; c++) {
                    fprintf(fp, "%c ", lib[d].seat[f][r][c]);
                }
                fprintf(fp, "\n");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "===\n");
    }
    fclose(fp);
    return true;
}

// 处理输入
char* Read() {
    int size = 128;
    char* str = (char*)malloc(size * sizeof(char));
    if (str == NULL) {
        perror("malloc");
        return NULL;
    }
    int cnt = 0;
    int read;

    while ((read = getchar()) != '\n' && cnt < size) {
        str[cnt++] = read;

        if (cnt >= size) { // 内存拓展
            size *= 2;
            char* smp = (char*)realloc(str, size * sizeof(char));
            if (smp == NULL) {
                perror("realloc");
                free(str); // 记得释放原来的内存
                return NULL;
            }
            str = smp;
        }
    }
    str[cnt] = '\0';
    return str;
}
char** split(char* input) {
    int cnt = 0;
    int cap = 10; // 字符串组长度
    char** ans = (char**)malloc((cap + 1) * sizeof(char*));
    if (ans == NULL) {
        perror("malloc");
        return NULL;
    }

    char* start = input, * end; // 首指针和尾指针

    while ((end = strchr(start, ' ')) != NULL) { //易错写成input 修改input后就已经被截开了
        // end找到第一个空格位置 将其改为'\0'
        *end = '\0';
        if (start == end) { // 处理多个空格
            start += 1;
            continue;
        }

        ans[cnt++] = start;
        start = end + 1;

        if (cnt >= cap) { //内存拓展
            cnt *= 2;
            char** tmp = (char**)realloc(ans, cap * sizeof(char*));
            if (tmp == NULL) {
                perror("realloc");
                free(ans);
                return NULL;
            }
            ans = tmp;
        }
    }

    if (start != NULL) ans[cnt++] = start;
    ans[cnt] = NULL;

    return ans;
}

// 帮助函数
void Help() {
    printf("The orders of Dian's Library :\n"
            "Quit\n"
            "Login\n"
            "Exit\n"
            "Reservation\n"
            "Reserve (day) Floor (floor_num) Seat (row_num) (column_num)\n"
            "(day) Floor (floor_num)\n"
            "Clear (only Admin)\n"
            );
    return;
}

// 总判断
int judge(char** str, Library lib[]) {
    int len = 0;
    while (str[len] != NULL) len++; // 用于算出字符串组长度
    // 针对单词指令
    if (len == 1) {
        if (_stricmp(str[0], "Quit") == 0) return 1;
        if (_stricmp(str[0], "Login") == 0) return 2;
        if (_stricmp(str[0], "Exit") == 0) return 3;
        if (_stricmp(str[0], "Reservation") == 0) return 4;
        if (_stricmp(str[0], "Clear") == 0) return 5;
        if (_stricmp(str[0], "Help") == 0) return 8;
        else if (_stricmp(str[0], "Reserve") == 0) return 12;
        else if (_stricmp(str[0], "Delete") == 0) return 13;
        return 0;
    }

    // 针对三词指令 即查询某天某层
    if (len == 3) {
        int cmp1 = judge_day(str[0]);
        if (cmp1 == 0) return 0;

        int cmp2 = _stricmp(str[1], "Floor");
        int cmp3 = judge_fnum(str[2], lib[cmp1-1].floor_cnt);
        if (!cmp2 && cmp3) return 6;
        return 0;
    }

    // 针对四词指令 即临时禁用/启用座位
    if (len == 4) {
        int cmp1 = _stricmp(str[0], "Adjust");
        int cmp2 = _stricmp(str[1], "Day");
        int cmp3 = judge_day(str[2]);
        int cmp4 = judge_state(str[3]);
        if (!cmp1 && !cmp2 && cmp3 && cmp4) return 9;
        return 0;
    }

    // 针对六词指令 即临时禁用/启用某天某层座位
    if (len == 6) {
        int cmp1 = _stricmp(str[0], "Adjust");
        int cmp2 = _stricmp(str[1], "Day");
        int cmp3 = judge_day(str[2]);
        int cmp4 = _stricmp(str[3], "Floor");
        int cmp5 = judge_fnum(str[4], lib[cmp3-1].floor_cnt);
        int cmp6 = judge_state(str[5]);
        if (!cmp1 && !cmp2 && cmp3 && !cmp4 && cmp5 && cmp6) return 10;
        else return 0;
    }

    // 针对七词指令 即预定座位
    else if (len == 7) {
        int cmp1 = _stricmp(str[0], "Reserve");
        int cmp2 = judge_day(str[1]); // 天
        if (cmp1 || cmp2 == 0) return 0;

        int cmp3 = _stricmp(str[2], "Floor");
        int cmp4 = judge_fnum(str[3], lib[cmp2-1].floor_cnt); // 层
        if (cmp3 || cmp4 == 0) return 0;

        int cmp5 = _stricmp(str[4], "Seat");
        int cmp6 = judge_snum(str[5], lib[cmp2-1].row_cnt[cmp4-1]); // 行
        if (cmp5 || cmp6 == 0) return 0;

        int cmp7 = judge_snum(str[6], lib[cmp2-1].col_cnt[cmp4-1][cmp6-1]); // 列

        if (cmp7) return 7;
        else return 0;
    }

    // 针对八词指令 即永久改变地形
    else if (len == 8) {
        int cmp1 = _stricmp(str[0], "Change");
        int cmp2 = _stricmp(str[1], "Floor");
        int cmp3 = judge_num(str[2]);
        int cmp4 = _stricmp(str[3], "Row");
        int cmp5 = judge_num(str[4]);
        int cmp6 = _stricmp(str[5], "Column");
        int cmp7 = judge_num(str[6]);
        int cmp8 = _stricmp(str[7], "Forever");
        if (!cmp1 && !cmp2 && cmp3 && !cmp4 && cmp5 && !cmp6 && cmp7 && !cmp8) return 11;
        else return 0;
    }
    else return 0;
}

// 判断转换
int judge_day(char* str) {
    if (!_stricmp(str, "Monday")) return 1;
    else if (!_stricmp(str, "Tuesday")) return 2;
    else if (!_stricmp(str, "Wednesday")) return 3;
    else if (!_stricmp(str, "Thursday")) return 4;
    else if (!_stricmp(str, "Friday")) return 5;
    else if (!_stricmp(str, "Saturday")) return 6;
    else if (!_stricmp(str, "Sunday")) return 7;
    else return 0;
}
char* Turn_day(int i) {
    char* ans = NULL;
    if (i == 0) ans = "Monday";
    if (i == 1) ans = "Tuesday";
    if (i == 2) ans = "Wednesday";
    if (i == 3) ans = "Thursday";
    if (i == 4) ans = "Friday";
    if (i == 5) ans = "Saturday";
    if (i == 6) ans = "Sunday";
    return ans;
}
int judge_fnum(char* str, int num) {
    int f = atoi(str);
    if (f >= 1 && f <= num) return f;
    else return 0;
}
int judge_snum(char* str, int num) {
    int n = atoi(str);
    if (n >= 1 && n <= num) return n;
    else return 0;
}
int judge_name(char* str) {
    int len = strlen(str);
    if (len == 1 && str[0] >= 'A' && str[0] <= 'Z') { // 是普通用户
        return 1;
    }
    else if (len == 5 && !_stricmp(str, "Admin")) { // 是管理员
        return 1;
    }
    return 0; // 用户名非法
}
int judge_state(char* str) {
    if (!_stricmp(str, "Open")) return 1;
    if (!_stricmp(str, "Close")) return 2;
    return 0;
}
int judge_reserve(char** str, Library lib[]) {
    int len = 0;
    while (str[len] != NULL) len++;
    if (len == 6) {
        int cmp1 = judge_day(str[0]);
        int cmp2 = _stricmp(str[1], "Floor");
        int cmp3 = judge_fnum(str[2], lib[cmp1-1].floor_cnt);
        int cmp4 = _stricmp(str[3], "Seat");
        int cmp5 = judge_snum(str[4], lib[cmp1-1].row_cnt[cmp3-1]);
        int cmp6 = judge_snum(str[5], lib[cmp1-1].col_cnt[cmp3-1][cmp5-1]);
        if (cmp1 && !cmp2 && cmp3 && !cmp4 && cmp5 && cmp6) return 1;
        else return 0;
    }
    else return 0;
}
bool judge_user(char* str) {
    if (str == NULL) return false;
    return true;
}
bool judge_admin(char* str){
    if (!_stricmp(str, "Admin")) return true;
    return false;
}
bool judge_num(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i]<'0'||str[i]>'9') return false;
        if (i == 0 && str[i] == '0') return false;
    }
    return true;
}

// 图书馆通用功能
void Login(char** user) {
    if(*user!=NULL) {
        printf("Program exited: %s\nPlease enter 'Exit' to log out, then log in again with your account.\n", *user);
        return;
    }

    printf("Please enter your username: ");
    char *name = Read();
    if (!_stricmp(name, "back")) {
        printf("Login aborted.\n");
        return;
    }

    if (judge_name(name) == 0) {
        printf("Invalid username. Please log in again. Enter 'Back' to abort login.\n");
        Login(user);
        return;
    }
    else {
        *user = name;
        printf("Login successful: %s\n", *user);
    }
    return;
}
void Exit(char** user) {
    if (*user == NULL) {
        printf("Already logged out.\n");
        return;
    }
    printf("Logged out successfully: %s\n", *user);
    *user = NULL;
}
void Query(char** str, char** user, Library lib[]) {
    if (!judge_user(*str)) {
        printf("Please log in. (Currently logged out)\n");
        return;
    }
    int day = judge_day(str[0]);
    int floor = *str[2] - '0';

    //printf("%d \n", lib[day-1].row_cnt[floor-1]);
    printf("%s\n", *user);

    for (int r = 0; r < lib[day-1].row_cnt[floor-1]; r++) {
        for (int c = 0; c < lib[day-1].col_cnt[floor-1][r]; c++) {
            if (lib[day - 1].seat[floor - 1][r][c] == '1') printf("3 ");
            else if (lib[day - 1].seat[floor - 1][r][c] == '0') printf("0 ");
            else {
                if (!_stricmp(*user, "Admin")) {
                    printf("%c ", lib[day - 1].seat[floor - 1][r][c]);
                }
                else {
                    if (lib[day - 1].seat[floor - 1][r][c] == **user) printf("2 ");
                    else printf("1 ");
                }
            }
        }
        printf("\n");
    }
}

// 图书馆用户功能
void Reservation(char* user, Library lib[]) {
    if (!judge_user(user)){
        printf("Please log in. (Currently logged out)\n");
        return;
    }
    if (judge_admin(user)) {
        printf("Administrators cannot make reservations.\n");
        return;
    }
    bool bp = false;
    for (int d = 0; d < DAYS; d++) {
        for (int f = 0; f < lib[d].floor_cnt ; f++) {
            for (int r = 0; r < lib[d].row_cnt[f] ; r++) {
                for (int c = 0; c < lib[d].col_cnt[f][r] ; c++) {
                    if (lib[d].seat[f][r][c] == *user) {
                        printf("%s Floor %d Seat %d %d\n", Turn_day(d), f+1, r+1, c+1);
                        bp = true;
                    }
                }
            }
        }
    }
    if (!bp) printf("You have no reservations.\n");
}
void Reserve(char** str, char* user, Library lib[]) {
    if (!judge_user(user)){
        printf("Please log in. (Currently logged out)\n");
        return;
    }
    if (judge_admin(user)) {
        printf("Administrators cannot make reservations.\n");
        return;
    }
    int day = judge_day(str[1]);
    int floor = *str[3] - '0';
    int row = *str[5] - '0';
    int column = *str[6] - '0';

    if (lib[day - 1].seat[floor - 1][row - 1][column - 1] == '1') {
        printf("This seat is already forbidden.\n");
        return;
    }
    if (lib[day - 1].seat[floor - 1][row - 1][column - 1] != '0') {
        printf("This seat is already reserved.\n");
        return;
    }
    else {
        lib[day - 1].seat[floor - 1][row - 1][column - 1] = *user;
        printf("Reservation successful.\n");
    }
}

// 图书馆管理员功能
void Clear(char* user, Library lib[]) {
    if (!judge_user(user)){
        printf("Please log in. (Currently logged out)\n");
        return;
    }
    if (!judge_admin(user)) {
        printf("Access denied. Administrator rights required.\n");
        return;
    }
    for (int d = 0; d < DAYS; d++) {
        for (int f = 0; f < lib[d].floor_cnt ; f++) {
            for (int r = 0; r < lib[d].row_cnt[f] ; r++) {
                for (int c = 0; c < lib[d].col_cnt[f][r] ; c++)  {
                    lib[d].seat[f][r][c] = '0';
                }
            }
        }
    }
    printf("All reservations cleared.\n");
}
void Adjust(char** str, char* user, Library lib[]) {
    if (!judge_user(user)){
        printf("Please log in. (Currently logged out)\n");
        return;
    }
    if (!judge_admin(user)) {
        printf("Access denied. Administrator rights required.\n");
        return;
    }
    int day = judge_day(str[2]);
    int state = judge_state(str[3]);
    for (int f = 0; f < day; f++) {
        for (int r = 0; r < lib[day-1].row_cnt[f] ; r++) {
            for (int c = 0; c < lib[day-1].col_cnt[f][r] ; c++) {
                if (state == 1) lib[day-1].seat[f][r][c] = '0';
                else if (state == 2) lib[day-1].seat[f][r][c] = '1';
            }
        }
    }
    if (state == 1) printf("Temporarily open all seats for %s.\n", Turn_day(day-1));
    else if (state == 2) printf("Temporarily disable all seats for %s.\n", Turn_day(day-1));
}
void Adjust_floors(char** str, char* user, Library lib[]) {
    if (!judge_user(user)){
        printf("Please log in. (Currently logged out)\n");
        return;
    }
    if (!judge_admin(user)) {
        printf("Access denied. Administrator rights required.\n");
        return;
    }
    int day = judge_day(str[2]);
    int floor = judge_fnum(str[4], lib[day-1].floor_cnt);
    int state = judge_state(str[5]);
    for (int r = 0; r < lib[day-1].row_cnt[floor-1]; r++) {
        for (int c = 0; c < lib[day-1].col_cnt[floor-1][r] ; c++) {
            if (state == 1) lib[day-1].seat[floor-1][r][c] = '0';
            else if (state == 2) lib[day-1].seat[floor-1][r][c] = '1';
        }
    }
    if (state == 1) printf("Temporarily open all seats for %s Floor %d.\n", Turn_day(day-1), floor);
    else if (state == 2) printf("Temporarily disable all seats for %s Floor %d.\n", Turn_day(day-1), floor);

}
void Change(char** str, char* user, Library lib[]) {
    int floor = atoi(str[2]);
    int row = atoi(str[4]);
    int column = atoi(str[6]);
    init_library(lib, floor, row, column);
    printf("Successfully change.\n");
}
void Help_Reserve(char* user, Library lib[]) {
    if (!judge_admin(user)) {
        printf("Access denied. Administrator rights required.\n");
        return;
    }
	printf("Please enter the reservation account: ");

	char* name = Read();
	if (!_stricmp(name, "Back")) {
		printf("Abandon this operation.\n");
	    free(name);
		return;
	}

	if (judge_name(name) == 0) {
	    char* back = "Back";
		printf("The username is invalid. Please re-enter. Type %s to abandon this operation.\n", back);
		Help_Reserve(user, lib);
	    free(name);
		return;
	}
	printf("Please enter the reserved time, floor and seat.\n");
	char* input = Read();
	if (input == NULL) {
		printf("Failed to allocate memory.\n");
	    free(name);
		return;
	}
	//printf("%s\n", input);
	char** stri = split(input);
	if (stri == NULL) {
		printf("Failed to allocate memory.\n");
	    free(name);
	    free(input);
		return;
	}

	int status = judge_reserve(stri, lib);
	if (status == 0) {
		printf("Error. Please Reserve again\n");
	    free(name);
	    free(input);
		return;
	}
	else {
		int day = judge_day(stri[0]);
		int floor = *stri[2] - '0';
		int row = *stri[4] - '0';
		int column = *stri[5] - '0';

	    if (lib[day - 1].seat[floor - 1][row - 1][column - 1] == '1') {
	        printf("This seat is already forbidden.\n");
	        return;
	    }
	    if (lib[day - 1].seat[floor - 1][row - 1][column - 1] != '0') {
	        printf("This seat is already reserved.\n");
	        return;
	    }
	    else {
	        lib[day - 1].seat[floor - 1][row - 1][column - 1] = *name;
	        printf("Reservation successful.\n");
	    }
	    free(name);
	    free(input);
		return;
	}
}
void Delete(char* user, Library lib[]) {
    if (!judge_admin(user)) {
        printf("Access denied. Administrator rights required.\n");
        return;
    }
	printf("Please enter the reservation account to be deleted: ");

    char* name = Read();
    if (!_stricmp(name, "Back")) {
        printf("Abandon this operation.\n");
        free(name);
        return;
    }

    if (judge_name(name) == 0) {
        char* back = "Back";
        printf("The username is invalid. Please re-enter. Type %s to abandon this operation.\n", back);
        Help_Reserve(user, lib);
        free(name);
        return;
    }

    Reservation(name, lib);
    printf("Please enter the reserved time, floor and seat\n");
    char* input = Read();
    if (input == NULL) {
        printf("Failed to allocate memory.\n");
        free(name);
        free(input);
        return;
    }

	//printf("%s\n", input);
	char** stri = split(input);
	if (stri == NULL) {
		printf("Failed to allocate memory.\n");
	    free(name);
	    free(input);
		return;
	}

	int status = judge_reserve(stri, lib);
	if (status == 0) {
		printf("Error. Please Delete again.\n");
	    free(name);
	    free(input);
		return;
	}
	else {
		int day = judge_day(stri[0]);
		int floor = *stri[2] - '0';
		int row = *stri[4] - '0';
		int column = *stri[5] - '0';

		if (lib[day - 1].seat[floor - 1][row - 1][column - 1] == '0') {
			printf("This seat has no reservation. Deletion is invalid.\n");
		}
	    else if (lib[day - 1].seat[floor - 1][row - 1][column - 1] == '1') {
	        printf("This seat is forbidden. Deletion is invalid.\n");
	    }
		else if(lib[day - 1].seat[floor - 1][row - 1][column - 1] == *name){
			lib[day - 1].seat[floor - 1][row - 1][column - 1] = '0';
			printf("The reservation has been successfully deleted.\n");
		}
		else {
			printf("The reserved object is not the specified account. Deletion is invalid.\n");
		}
	    free(name);
	    free(input);
		return;
	}
}
