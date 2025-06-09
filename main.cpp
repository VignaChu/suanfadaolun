#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <cctype> // ���� tolower ����

using namespace std;

// ѧ���ṹ��
struct Student {
    string name;
    int wakeUpMin;   // ��ʱ�䣨���ӣ�
    int sleepMin;    // ˯��ʱ�䣨���ӣ�
    int tidy;        // ���ɾ��̶ȣ�1~5��
    int social;      // �罻����1~5��
};

// ʱ���ַ���ת���������� "6:00" -> 360��
int timeToMinutes(const string& timeStr) {
    int hour = 0, minute = 0;
    char colon;
    stringstream ss(timeStr);
    if (ss >> hour >> colon >> minute) {
        return hour * 60 + minute;
    }
    return -1; // ��ʽ���󷵻� -1
}

// ʱ���ʽ���������� 0:00 ��ʾΪ 24:00
string formatTime(int minutes) {
    if (minutes < 0) return "δ֪";

    int hour = minutes / 60;
    int min = minutes % 60;

    // �� 0:00 ��ʾΪ 24:00
    if (hour == 0 && min == 0) {
        return "24:00";
    }

    return string(1, '0' + (hour / 10 % 10)) + string(1, '0' + (hour % 10)) + ":" +
           string(1, '0' + (min / 10 % 10)) + string(1, '0' + (min % 10));
}

// ��ȡ CSV ����
vector<Student> readStudentsFromCSV(const string& filename) {
    vector<Student> students;
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cerr << "�����޷����ļ� " << filename << endl;
        return students;
    }

    string line;
    getline(file, line);  // ������ͷ

    while (getline(file, line)) {
        stringstream ss(line);
        Student s;

        string nameField;
        if (ss.peek() == '"') {
            ss.ignore(1);
            getline(ss, nameField, '"');
        } else {
            getline(ss, nameField, ',');
        }
        s.name = nameField;

        string wakeTime;
        getline(ss, wakeTime, ',');

        if (wakeTime.empty()) {
            s.wakeUpMin = -1;
        } else {
            s.wakeUpMin = timeToMinutes(wakeTime);
        }

        string sleepTime;
        getline(ss, sleepTime, ',');

        if (sleepTime.empty()) {
            s.sleepMin = -1;
        } else {
            s.sleepMin = timeToMinutes(sleepTime);
        }

        ss >> s.tidy;
        ss.ignore(1000, ',');

        ss >> s.social;

        students.push_back(s);
    }

    file.close();
    return students;
}

// ������ + ��Ȩ��һ��
vector<vector<double> > buildFeatureVectors(const vector<Student>& students) {
    const double w_wake = 2.0;
    const double w_sleep = 2.0;
    const double w_tidy = 1.0;
    const double w_social = 1.0;

    int n = students.size();

    vector<vector<double> > features(n, vector<double>(4));

    // �������Сֵ�����ڹ�һ����
    int minWake = 9999, maxWake = 0;
    int minSleep = 9999, maxSleep = 0;
    for (int i = 0; i < n; ++i) {
        if (students[i].wakeUpMin >= 0 && students[i].wakeUpMin < minWake) minWake = students[i].wakeUpMin;
        if (students[i].wakeUpMin >= 0 && students[i].wakeUpMin > maxWake) maxWake = students[i].wakeUpMin;
        if (students[i].sleepMin >= 0 && students[i].sleepMin < minSleep) minSleep = students[i].sleepMin;
        if (students[i].sleepMin >= 0 && students[i].sleepMin > maxSleep) maxSleep = students[i].sleepMin;
    }

    // ��ֹ�������
    double wakeRange = maxWake - minWake;
    double sleepRange = maxSleep - minSleep;
    if (wakeRange < 1e-6) wakeRange = 1;
    if (sleepRange < 1e-6) sleepRange = 1;

    // ����������������һ��
    for (int i = 0; i < n; ++i) {
        double wakeNorm = (double)(students[i].wakeUpMin >= 0 ? students[i].wakeUpMin - minWake : 0) / wakeRange;
        double sleepNorm = (double)(students[i].sleepMin >= 0 ? students[i].sleepMin - minSleep : 0) / sleepRange;
        double tidyNorm = (double)(students[i].tidy - 1) / 4;
        double socialNorm = (double)(students[i].social - 1) / 4;

        features[i][0] = wakeNorm * w_wake;
        features[i][1] = sleepNorm * w_sleep;
        features[i][2] = tidyNorm * w_tidy;
        features[i][3] = socialNorm * w_social;

        // �ٴε�λ��
        double length = sqrt(
            features[i][0]*features[i][0] +
            features[i][1]*features[i][1] +
            features[i][2]*features[i][2] +
            features[i][3]*features[i][3]
        );
        if (length > 1e-6) {
            for (int j = 0; j < 4; ++j)
                features[i][j] /= length;
        }
    }

    return features;
}

// �����������ƶȾ���
vector<vector<double> > computeCosineSimilarity(const vector<vector<double> >& features) {
    int n = features.size();
    vector<vector<double> > simMatrix(n, vector<double>(n));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int k = 0; k < 4; ++k)
                dot += features[i][k] * features[j][k];
            simMatrix[i][j] = dot;
        }
    }

    return simMatrix;
}

// �������������
vector<vector<int> > matchDormsGreedyClustering(
    const vector<Student>& students,
    const vector<vector<double> >& simMatrix,
    int roomSize) {

    int n = students.size();
    vector<bool> assigned(n, false); // �Ƿ��ѱ�����
    vector<vector<int> > dorms;

    // �����������������ǲ��������������
    int dormCount = n / roomSize;
    int remaining = n % roomSize;
    
    dorms.resize(dormCount + (remaining > 0 ? 1 : 0));
    
    for (int d = 0; d < dormCount; d++) {
        dorms[d].resize(roomSize, -1);
    }
    
    if (remaining > 0) {
        dorms[dormCount].resize(remaining, -1);
    }

    for (int d = 0; d < dormCount; d++) {
        double maxSim = -1e9;
        int i1 = -1, i2 = -1;

        for (int i = 0; i < n; i++) {
            if (assigned[i]) continue;
            for (int j = i + 1; j < n; j++) {
                if (assigned[j]) continue;
                if (simMatrix[i][j] > maxSim) {
                    maxSim = simMatrix[i][j];
                    i1 = i;
                    i2 = j;
                }
            }
        }

        if (i1 == -1 || i2 == -1) break;

        dorms[d][0] = i1;
        dorms[d][1] = i2;
        assigned[i1] = true;
        assigned[i2] = true;

        for (int m = 2; m < roomSize; m++) {
            double bestScore = -1e9;
            int bestIdx = -1;

            for (int i = 0; i < n; i++) {
                if (assigned[i]) continue;

                double avgSim = 0.0;
                for (int k = 0; k < m; k++) {
                    avgSim += simMatrix[i][dorms[d][k]];
                }
                avgSim /= m;

                if (avgSim > bestScore) {
                    bestScore = avgSim;
                    bestIdx = i;
                }
            }

            if (bestIdx != -1) {
                dorms[d][m] = bestIdx;
                assigned[bestIdx] = true;
            } else {
                // �Ҳ������ʵ�ѧ������ǰ����
                break;
            }
        }
    }

    // ����ʣ��ѧ�����������������
    if (remaining > 0) {
        vector<int> lastDorm;
        for (int i = 0; i < n; i++) {
            if (!assigned[i]) {
                lastDorm.push_back(i);
            }
        }
        dorms[dormCount] = lastDorm;
    }

    return dorms;
}

// ��鲢���δ����ѧ��
void checkUnassignedStudents(const vector<Student>& students, 
                             const vector<vector<int> >& dorms) {
    vector<bool> assigned(students.size(), false);
    vector<int> unassigned;
    
    for (size_t i = 0; i < dorms.size(); i++) {
        for (size_t j = 0; j < dorms[i].size(); j++) {
            assigned[dorms[i][j]] = true;
        }
    }
    
    for (size_t i = 0; i < students.size(); i++) {
        if (!assigned[i]) {
            unassigned.push_back(i);
        }
    }
    
    if (!unassigned.empty()) {
        cout << "\n����: " << unassigned.size() 
             << "��ѧ��δ����������:\n";
        for (size_t i = 0; i < unassigned.size(); i++) {
            cout << "  - " << students[unassigned[i]].name << endl;
        }
    }
}

// ������
int main() {
    cout << "=== ����ƥ��ϵͳ���������ƶ� + ̰�ľ����㷨�� ===\n";

    vector<Student> students = readStudentsFromCSV("../data.csv");
    if (students.empty()) {
        cerr << "����û��ѧ�����ݣ�\n";
        return 1;
    }

    cout << "�Ѽ��� " << students.size() << " ��ѧ�����ݡ�\n";

    vector<vector<double> > features = buildFeatureVectors(students);
    vector<vector<double> > simMatrix = computeCosineSimilarity(features);

    int roomSize;
    cout << "������ÿ�������������4~12����";
    cin >> roomSize;
    if (roomSize < 4 || roomSize > 12) {
        cout << "��Ч���룬Ĭ������Ϊ 4 �ˡ�\n";
        roomSize = 4;
    }

    // ��ʼƥ��
    vector<vector<int> > dorms = matchDormsGreedyClustering(students, simMatrix, roomSize);

    // ���δ����ѧ��
    checkUnassignedStudents(students, dorms);

    // ������ս��
    cout << "\n=== ��������������հ棩 ===\n";
    int dormCount = 0;
    for (size_t i = 0; i < dorms.size(); ++i) {
        const vector<int>& dorm = dorms[i];
        if (dorm.empty()) continue; // ����������
        
        cout << "���� " << ++dormCount << "��" << dorm.size() << "�ˣ�:\n";

        // ��������г�Ա
        for (size_t j = 0; j < dorm.size(); ++j) {
            int idx = dorm[j];
            Student s = students[idx];

            string wakeTime = formatTime(s.wakeUpMin);
            string sleepTime = formatTime(s.sleepMin);

            cout << "  ��Ա: " << setw(5) << left << s.name
                 << " | ��: " << setw(7) << wakeTime
                 << " | ˯��: " << setw(7) << sleepTime
                 << " | ���ɾ�: " << s.tidy << "/5"
                 << " | �罻: " << s.social << "/5\n";
        }

        // ����ƽ�����ƶȣ������ἶ��ֻ����һ�Σ�
        double totalSim = 0.0;
        int count = 0;
        for (size_t m = 0; m < dorm.size(); ++m) {
            for (size_t n = m + 1; n < dorm.size(); ++n) {
                totalSim += simMatrix[dorm[m]][dorm[n]];
                ++count;
            }
        }
        
        double avgSimilarity = (count > 0) ? totalSim / count * 100 : 0.0;
        cout << "  ƽ�����ƶ�: " << fixed << setprecision(2) << avgSimilarity << "%\n\n";
    }
	cout<<"���һ�����ҿ�����Ϣ���ƶȼ��ͣ����Կ��ǽ���󼸸������ͬѧ���ֵ��������ٵ�����"<<endl;
    return 0;
}
