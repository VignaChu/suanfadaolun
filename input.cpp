#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits> // 用于 numeric_limits

using namespace std;

// 学生结构体
struct Student {
    string name;
    string wakeUpTime;
    string sleepTime;
    int tidy;
    int social;
};

// 显示起床时间选项
void showWakeUpOptions() {
    cout << "起床时间选项：" << endl;
    cout << "1. 6:00" << endl;
    cout << "2. 6:30" << endl;
    cout << "3. 7:00" << endl;
    cout << "4. 7:30" << endl;
}

// 显示睡觉时间选项
void showSleepOptions() {
    cout << "睡觉时间选项：" << endl;
    cout << "1. 22:00（10点）" << endl;
    cout << "2. 22:30（10点半）" << endl;
    cout << "3. 23:00（11点）" << endl;
    cout << "4. 23:30（11点半）" << endl;
    cout << "5. 00:00 及以后" << endl;
}

// 根据选择获取对应的时间字符串
string getWakeUpTime(int choice) {
    switch (choice) {
        case 1: return "6:00";
        case 2: return "6:30";
        case 3: return "7:00";
        case 4: return "7:30";
        default: return "";
    }
}

string getSleepTime(int choice) {
    switch (choice) {
        case 1: return "22:00";
        case 2: return "22:30";
        case 3: return "23:00";
        case 4: return "23:30";
        case 5: return "00:00";
        default: return "";
    }
}

// 获取 1~5 的评分（封装输入校验逻辑）
int getRating(const string& prompt) {
    int rating;
    while (true) {
        cout << prompt;
        cin >> rating;
        if (cin.fail()) {
            cin.clear(); // 清除错误状态
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略非法输入
            cout << "请输入数字！" << endl;
        } else if (rating >= 1 && rating <= 5) {
            break;
        } else {
            cout << "请输入 1~5 的数字。" << endl;
        }
    }
    return rating;
}

// 录入单个学生信息
void inputStudent(Student& s) {
    // 清空缓冲区，避免 getline 被干扰
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "请输入学生姓名：";
    getline(cin, s.name); // 完整读取姓名（包括空格）

    int choice;

    showWakeUpOptions();
    cout << "请选择起床时间（输入数字 1~4）：";
    cin >> choice;
    s.wakeUpTime = getWakeUpTime(choice);

    showSleepOptions();
    cout << "请选择睡觉时间（输入数字 1~5）：";
    cin >> choice;
    s.sleepTime = getSleepTime(choice);

    s.tidy = getRating("爱干净程度（1-非常不喜欢，5-非常喜欢）：");
    s.social = getRating("社交倾向（1-非常不喜欢，5-非常喜欢）：");
}

// 检查文件是否存在且非空
bool fileExistsWithContent(const string& filename) {
    ifstream file(filename.c_str());
    return file.good();
}

// 写入 CSV 文件（追加模式）
void writeCSV(const vector<Student>& students) {
    ofstream file("../data.csv", ios::app); // 追加模式
    if (!file.is_open()) {
        cerr << "无法打开文件 data.csv 进行写入！" << endl;
        return;
    }

    // 如果文件是空的，写入表头
    if (!fileExistsWithContent("../data.csv")) {
        file << "Name,WakeUpTime,SleepTime,Tidy,Social\n";
    }

    // 写入学生数据
    for (vector<Student>::const_iterator it = students.begin(); it != students.end(); ++it) {
        const Student& s = *it;
        file << s.name <<"," 
             << s.wakeUpTime << ","
             << s.sleepTime << ","
             << s.tidy << ","
             << s.social << "\n";
    }

    file.close();
    cout << "学生信息已成功追加保存到 data.csv" << endl;
}

int main() {
    vector<Student> students;
    char addMore = 'y';

    cout << "=== 宿舍分配系统 - 学生信息录入(按下回车开始) ===" << endl;

    while (addMore == 'y' || addMore == 'Y') {
        Student s;
        inputStudent(s);
        students.push_back(s);

        cout << "是否继续录入？(y/n): ";
        cin >> addMore;
    }

    writeCSV(students);

    cout << "录入完成。你可以运行室友匹配程序来生成宿舍分配结果。" << endl;

    return 0;
}
