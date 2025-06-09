#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits> // ���� numeric_limits

using namespace std;

// ѧ���ṹ��
struct Student {
    string name;
    string wakeUpTime;
    string sleepTime;
    int tidy;
    int social;
};

// ��ʾ��ʱ��ѡ��
void showWakeUpOptions() {
    cout << "��ʱ��ѡ�" << endl;
    cout << "1. 6:00" << endl;
    cout << "2. 6:30" << endl;
    cout << "3. 7:00" << endl;
    cout << "4. 7:30" << endl;
}

// ��ʾ˯��ʱ��ѡ��
void showSleepOptions() {
    cout << "˯��ʱ��ѡ�" << endl;
    cout << "1. 22:00��10�㣩" << endl;
    cout << "2. 22:30��10��룩" << endl;
    cout << "3. 23:00��11�㣩" << endl;
    cout << "4. 23:30��11��룩" << endl;
    cout << "5. 00:00 ���Ժ�" << endl;
}

// ����ѡ���ȡ��Ӧ��ʱ���ַ���
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

// ��ȡ 1~5 �����֣���װ����У���߼���
int getRating(const string& prompt) {
    int rating;
    while (true) {
        cout << prompt;
        cin >> rating;
        if (cin.fail()) {
            cin.clear(); // �������״̬
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ���ԷǷ�����
            cout << "���������֣�" << endl;
        } else if (rating >= 1 && rating <= 5) {
            break;
        } else {
            cout << "������ 1~5 �����֡�" << endl;
        }
    }
    return rating;
}

// ¼�뵥��ѧ����Ϣ
void inputStudent(Student& s) {
    // ��ջ����������� getline ������
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "������ѧ��������";
    getline(cin, s.name); // ������ȡ�����������ո�

    int choice;

    showWakeUpOptions();
    cout << "��ѡ����ʱ�䣨�������� 1~4����";
    cin >> choice;
    s.wakeUpTime = getWakeUpTime(choice);

    showSleepOptions();
    cout << "��ѡ��˯��ʱ�䣨�������� 1~5����";
    cin >> choice;
    s.sleepTime = getSleepTime(choice);

    s.tidy = getRating("���ɾ��̶ȣ�1-�ǳ���ϲ����5-�ǳ�ϲ������");
    s.social = getRating("�罻����1-�ǳ���ϲ����5-�ǳ�ϲ������");
}

// ����ļ��Ƿ�����ҷǿ�
bool fileExistsWithContent(const string& filename) {
    ifstream file(filename.c_str());
    return file.good();
}

// д�� CSV �ļ���׷��ģʽ��
void writeCSV(const vector<Student>& students) {
    ofstream file("../data.csv", ios::app); // ׷��ģʽ
    if (!file.is_open()) {
        cerr << "�޷����ļ� data.csv ����д�룡" << endl;
        return;
    }

    // ����ļ��ǿյģ�д���ͷ
    if (!fileExistsWithContent("../data.csv")) {
        file << "Name,WakeUpTime,SleepTime,Tidy,Social\n";
    }

    // д��ѧ������
    for (vector<Student>::const_iterator it = students.begin(); it != students.end(); ++it) {
        const Student& s = *it;
        file << s.name <<"," 
             << s.wakeUpTime << ","
             << s.sleepTime << ","
             << s.tidy << ","
             << s.social << "\n";
    }

    file.close();
    cout << "ѧ����Ϣ�ѳɹ�׷�ӱ��浽 data.csv" << endl;
}

int main() {
    vector<Student> students;
    char addMore = 'y';

    cout << "=== �������ϵͳ - ѧ����Ϣ¼��(���»س���ʼ) ===" << endl;

    while (addMore == 'y' || addMore == 'Y') {
        Student s;
        inputStudent(s);
        students.push_back(s);

        cout << "�Ƿ����¼�룿(y/n): ";
        cin >> addMore;
    }

    writeCSV(students);

    cout << "¼����ɡ��������������ƥ����������������������" << endl;

    return 0;
}
