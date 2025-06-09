#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()

using namespace std;

// ѧ���ṹ��
struct Student {
    string name;
    string wakeUpTime;
    string sleepTime;
    int tidy;
    int social;
};

// �ж��ļ��Ƿ����
bool fileExists(const string& filePath) {
    ifstream file(filePath.c_str());
    return file.good();
}

// �ж��ļ��Ƿ�Ϊ��
bool isFileEmpty(const string& filePath) {
    ifstream file(filePath.c_str());
    return file.peek() == ifstream::traits_type::eof();
}

// ��ȡ�����ʱ��
string getRandomWakeUpTime() {
    const string options[] = {"6:00", "6:30", "7:00", "7:30"};
    return options[rand() % 4];
}

// ��ȡ���˯��ʱ��
string getRandomSleepTime() {
    const string options[] = {"22:00", "22:30", "23:00", "23:30", "00:00"};
    return options[rand() % 5];
}

// ��ȡ 1~5 ���������
int getRandomRating() {
    return rand() % 5 + 1;
}

// ����ָ�����ȵĴ�д��ĸ�ַ���
string generateRandomName(int length) {
    string name;
    for (int i = 0; i < length; ++i) {
        char ch = 'A' + (rand() % 26);  // A-Z
        name += ch;
    }
    return name;
}

// ������� 2~4 ����ĸ������
string generateRandomStudentName() {
    int length = 2 + (rand() % 3);  // ���� 2, 3 �� 4
    return generateRandomName(length);
}

// д�� CSV �ļ����Զ��ж��Ƿ�Ҫд���ͷ��
void writeRandomStudentsToCSV(const string& filePath, int numEntries) {
    ofstream file(filePath.c_str(), ios::app); // ׷��ģʽ��

    if (!file.is_open()) {
        cerr << "�޷����ļ�: " << filePath << endl;
        return;
    }

    // ����ǿ��ļ���д���ͷ
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
    cout << "�ɹ�д��/׷�� " << numEntries << " ��ѧ����¼�� " << filePath << endl;
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // ��ʼ���������

    string filePath = "../data.csv"; // �ļ�·��
    int numEntries = 0;

    // ����������Ŀ����1~256��
    while (true) {
        cout << "������Ҫ���ɵ�ѧ��������1~256����";
        cin >> numEntries;

        if (numEntries >= 1 && numEntries <= 256) {
            break;
        }
        cout << "������Ч�������� 1~256 ֮���������\n";
    }

    writeRandomStudentsToCSV(filePath, numEntries);

    cout << numEntries << " ��ѧ����Ϣ��������ϡ�" << endl;

    return 0;
}
