#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()

using namespace std;

// 学生结构体
struct Student {
    string name;
    string wakeUpTime;
    string sleepTime;
    int tidy;
    int social;
};

// 判断文件是否存在
bool fileExists(const string& filePath) {
    ifstream file(filePath.c_str());
    return file.good();
}

// 判断文件是否为空
bool isFileEmpty(const string& filePath) {
    ifstream file(filePath.c_str());
    return file.peek() == ifstream::traits_type::eof();
}

// 获取随机起床时间
string getRandomWakeUpTime() {
    const string options[] = {"6:00", "6:30", "7:00", "7:30"};
    return options[rand() % 4];
}

// 获取随机睡觉时间
string getRandomSleepTime() {
    const string options[] = {"22:00", "22:30", "23:00", "23:30", "00:00"};
    return options[rand() % 5];
}

// 获取 1~5 的随机评分
int getRandomRating() {
    return rand() % 5 + 1;
}

// 生成指定长度的大写字母字符串
string generateRandomName(int length) {
    string name;
    for (int i = 0; i < length; ++i) {
        char ch = 'A' + (rand() % 26);  // A-Z
        name += ch;
    }
    return name;
}

// 随机生成 2~4 个字母的名字
string generateRandomStudentName() {
    int length = 2 + (rand() % 3);  // 生成 2, 3 或 4
    return generateRandomName(length);
}

// 写入 CSV 文件（自动判断是否要写入表头）
void writeRandomStudentsToCSV(const string& filePath, int numEntries) {
    ofstream file(filePath.c_str(), ios::app); // 追加模式打开

    if (!file.is_open()) {
        cerr << "无法打开文件: " << filePath << endl;
        return;
    }

    // 如果是空文件，写入表头
    if (isFileEmpty(filePath)) {
        file << "Name,WakeUpTime,SleepTime,Tidy,Social\n";
    }

    for (int i = 0; i < numEntries; ++i) {
        Student s;
        s.name = generateRandomStudentName();
        s.wakeUpTime = getRandomWakeUpTime();
        s.sleepTime = getRandomSleepTime();
        s.tidy = getRandomRating();
        s.social = getRandomRating();

        file << s.name << ","
             << s.wakeUpTime << ","
             << s.sleepTime << ","
             << s.tidy << ","
             << s.social << "\n";
    }

    file.close();
    cout << "成功写入/追加 " << numEntries << " 条学生记录到 " << filePath << endl;
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // 初始化随机种子

    string filePath = "../data.csv"; // 文件路径
    int numEntries = 0;

    // 输入数据条目数（1~256）
    while (true) {
        cout << "请输入要生成的学生人数（1~256）：";
        cin >> numEntries;

        if (numEntries >= 1 && numEntries <= 256) {
            break;
        }
        cout << "输入无效，请输入 1~256 之间的整数。\n";
    }

    writeRandomStudentsToCSV(filePath, numEntries);

    cout << numEntries << " 条学生信息已生成完毕。" << endl;

    return 0;
}
