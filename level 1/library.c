//
// Created by Lenovo on 2025/9/20.
//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// 固定层数直接初始化
#define L 5  // 层
#define R 4  // 行
#define C 4  // 列

// 手写结构体 用char型保存座位状态
typedef struct {
	char seat[L][R][C];
}Library;

// 处理输入
char* Read(); // 读取长度未知的字符串
char** split(char* input); // 分割读入的字符串 形成有效的字符串组

// 文件读写
bool save_library(const char* filename, Library Dian[]); // 保存数据进入文件
bool load_library(const char* filename, Library Dian[]); // 读取文件数据

// 图书馆功能
void Login(char** user); // 登录
void Exit(char** user); // 登出
void Reservation(char** user, Library Dian[]); // 查询用户所有预约
void Clear(char** user, Library Dian[]); // Admin清空所有预约
void Query(char** str, char** user, Library Dian[]); // 查询某天某层预约
void Reserve(char** str, char* user, Library Dian[]); // 预定座位

// 判断转换
int judge_day(char* str); // 将星期转换为数字
char* Turn_day(int i); // 将数字转换为星期
int judge_fnum(char* str); // 类型转换 字符串转为数字 针对层
int judge_snum(char* str); // 类型转换 针对行列
int judge_name(char* str); // 检测用户名是否合法
int judge(char** str); // 检测指令类型

int main()
{
	Library Dian[7];
	const char* filename = "library.txt";
	if (!load_library(filename, Dian)) {
		printf("Empty/Corrupted file detected. Reinitializing library data.\n");
		memset(Dian, '0', 7 * sizeof(Library));
		if (save_library(filename, Dian)) {
			printf("Library data saved successfully.\n");
		}
		else printf("Failed to save library data.\n");
	}
	else printf("Library data loaded successfully.\n");
	//初始化座位状态
	/*for (int i = 0; i < 7; i++) {
		memset(Dian[i].seat, 0, sizeof(Dian[i].seat));
	}*/

	//检查是否正常初始化
	/*{
		for (int i = 0; i < 7; i++) {
			printf("Day %d\n", i + 1);
			for (int l = 0; l < 5; l++) {
				printf("Level %d\n", l + 1);
				for (int j = 0; j < 4; j++) {
					for (int k = 0; k < 4; k++) {
						printf("%d ", Dian[i].seat[l][j][k]);
					}
					printf("\n");
				}
				printf("\n");
			}


		}
	}*/
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
		*/

		// 判断指令
		int status = judge(str);

		if (status == 0) printf("Invalid input.\n");
		if (status == 1) break;
		if (status == 2) Login(&user);
		if (status == 3) Exit(&user);
		if (status == 4) Reservation(&user, Dian);
		if (status == 5) Clear(&user, Dian);
		if (status == 6) Query(str, &user, Dian);
		if (status == 7) Reserve(str, user, Dian);

		// 释放内存
		free(input);
		free(str);
	}

	if (save_library(filename, Dian)) printf("Library data saved successfully.\n");
	else printf("Failed to save library data.\n");

	printf("Program exited.\n");

	return 0;
}

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

void Reservation(char** user, Library Dian[]) {
	if (*user == NULL) {
		printf("Please log in. (Currently logged out)\n");
		return;
	}
	if (!_stricmp(*user, "Admin")) {
		printf("Administrators cannot make reservations.\n");
		return;
	}
	bool bp = 0;
	for (int i = 0; i < 7; i++) {
		for (int l = 0; l < 5; l++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					if (Dian[i].seat[l][j][k] == **user) {
						printf("%s Floor %d Seat %d %d\n", Turn_day(i), l+1, j+1, k+1);
						bp = 1;
					}
				}
			}
		}
	}
	if (!bp) printf("You have no reservations.\n");
}

void Clear(char** user, Library Dian[]) {
	if (*user == NULL) {
		printf("Please log in. (Currently logged out)\n");
		return;
	}
	if (_stricmp(*user, "Admin") != 0) {
		printf("Access denied. Administrator rights required.\n");
		return;
	}
	for (int i = 0; i < 7; i++) {
		for (int l = 0; l < 5; l++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					Dian[i].seat[l][j][k] = '0';
				}
			}
		}
	}
	printf("All reservations cleared.\n");
}

void Query(char** str, char** user, Library Dian[]) {
	if (*user == NULL) {
		printf("Please log in. (Currently logged out)\n");
		return;
	}
	int day = judge_day(str[0]);
	int floor = *str[2] - '0';
	//printf("%d %s %d\n", day, str[2], floor);

	printf("%s\n", *user);
	if (!_stricmp(*user, "Admin")) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				if (Dian[day - 1].seat[floor - 1][j][k] == '0') printf("0 ");
				else printf("%c ", Dian[day - 1].seat[floor - 1][j][k]);
			}
			printf("\n");
		}
	}

	else {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				if (Dian[day - 1].seat[floor - 1][j][k] == '0') printf("0 ");
				else if (Dian[day - 1].seat[floor - 1][j][k] == **user) printf("2 ");
				else printf("1 ");
			}
			printf("\n");
		}
	}
}

void Reserve(char** str, char* user, Library Dian[]) {
	if (user == NULL) {
		printf("Please log in. (Currently logged out)\n");
		return;
	}
	if (!_stricmp(user, "Admin")) {
		printf("Administrators cannot make reservations.\n");
		return;
	}
	int day = judge_day(str[1]);
	int floor = *str[3] - '0';
	int row = *str[5] - '0';
	int column = *str[6] - '0';

	if (Dian[day - 1].seat[floor - 1][row - 1][column - 1] != '0') {
		printf("This seat is already reserved.\n");
		return;
	}
	else {
		Dian[day - 1].seat[floor - 1][row - 1][column - 1] = *user;
		printf("Reservation successful.\n");
	}
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

int judge(char** str) {
	int len = 0;
	while (str[len] != NULL) len++; // 用于算出字符串组长度

	if (len == 1) {
		if (_stricmp(str[0], "Quit") == 0) return 1;
		else if (_stricmp(str[0], "Login") == 0) return 2;
		else if (_stricmp(str[0], "Exit") == 0) return 3;
		else if (_stricmp(str[0], "Reservation") == 0) return 4;
		else if (_stricmp(str[0], "Clear") == 0) return 5;
		else return 0;
	}
	else if (len == 3) {
		int cmp1 = judge_day(str[0]);
		int cmp2 = _stricmp(str[1], "Floor");
		int cmp3 = judge_fnum(str[2]);
		if (cmp1 && !cmp2 && cmp3) return 6;
		else return 0;
	}
	else if (len == 7) {
		int cmp1 = _stricmp(str[0], "Reserve");
		int cmp2 = judge_day(str[1]);
		int cmp3 = _stricmp(str[2], "Floor");
		int cmp4 = judge_fnum(str[3]);
		int cmp5 = _stricmp(str[4], "Seat");
		int cmp6 = judge_snum(str[5]);
		int cmp7 = judge_snum(str[6]);
		if (!cmp1 && cmp2 && !cmp3 && cmp4 && !cmp5 && cmp6 && cmp7) return 7;
		else return 0;
	}
	else return 0;
}

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

int judge_fnum(char* str) {
	if (!strcmp(str, "1")) return 1;
	else if (!strcmp(str, "2")) return 2;
	else if (!strcmp(str, "3")) return 3;
	else if (!strcmp(str, "4")) return 4;
	else if (!strcmp(str, "5")) return 5;
	else return 0;
}

int judge_snum(char* str) {
	if (!strcmp(str, "1")) return 1;
	else if (!strcmp(str, "2")) return 2;
	else if (!strcmp(str, "3")) return 3;
	else if (!strcmp(str, "4")) return 4;
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

bool save_library(const char* filename, Library Dian[]) {
	FILE* fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("文件打开失败\n");
		return false;
	}
	for (int i = 0; i < 7; i++) {
		//printf("Day %d\n", i + 1);
		for (int l = 0; l < 5; l++) {
			//printf("Level %d\n", l + 1);
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					fprintf(fp, "%c ", Dian[i].seat[l][j][k]);
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

bool load_library(const char* filename, Library Dian[]) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Failed to open file.\n");
		return false;
	}
	memset(Dian, '0', sizeof(Library) * 7);

	for (int i = 0; i < 7; i++) {
		//printf("Day %d\n", i + 1);
		for (int l = 0; l < 5; l++) {
			//printf("Level %d\n", l + 1);
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					int ch;
					while ((ch = fgetc(fp)) == ' ') {
						if (ch == EOF) {
							printf("File read error.\n");
							fclose(fp);
							return false;
						}
					}
					Dian[i].seat[l][j][k] = (char)ch;
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