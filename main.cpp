#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <cctype> // 包含 tolower 函数

using namespace std;

// 学生结构体
struct Student {
    string name;
    int wakeUpMin;   // 起床时间（分钟）
    int sleepMin;    // 睡觉时间（分钟）
    int tidy;        // 爱干净程度（1~5）
    int social;      // 社交倾向（1~5）
};

// 时间字符串转分钟数（如 "6:00" -> 360）
int timeToMinutes(const string& timeStr) {
    int hour = 0, minute = 0;
    char colon;
    stringstream ss(timeStr);
    if (ss >> hour >> colon >> minute) {
        return hour * 60 + minute;
    }
    return -1; // 格式错误返回 -1
}

// 时间格式化函数：将 0:00 显示为 24:00
string formatTime(int minutes) {
    if (minutes < 0) return "未知";

    int hour = minutes / 60;
    int min = minutes % 60;

    // 将 0:00 显示为 24:00
    if (hour == 0 && min == 0) {
        return "24:00";
    }

    return string(1, '0' + (hour / 10 % 10)) + string(1, '0' + (hour % 10)) + ":" +
           string(1, '0' + (min / 10 % 10)) + string(1, '0' + (min % 10));
}

// 读取 CSV 数据
vector<Student> readStudentsFromCSV(const string& filename) {
    vector<Student> students;
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cerr << "错误：无法打开文件 " << filename << endl;
        return students;
    }

    string line;
    getline(file, line);  // 跳过表头

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

// 向量化 + 加权归一化
vector<vector<double> > buildFeatureVectors(const vector<Student>& students) {
    const double w_wake = 2.0;
    const double w_sleep = 2.0;
    const double w_tidy = 1.0;
    const double w_social = 1.0;

    int n = students.size();

    vector<vector<double> > features(n, vector<double>(4));

    // 找最大最小值（用于归一化）
    int minWake = 9999, maxWake = 0;
    int minSleep = 9999, maxSleep = 0;
    for (int i = 0; i < n; ++i) {
        if (students[i].wakeUpMin >= 0 && students[i].wakeUpMin < minWake) minWake = students[i].wakeUpMin;
        if (students[i].wakeUpMin >= 0 && students[i].wakeUpMin > maxWake) maxWake = students[i].wakeUpMin;
        if (students[i].sleepMin >= 0 && students[i].sleepMin < minSleep) minSleep = students[i].sleepMin;
        if (students[i].sleepMin >= 0 && students[i].sleepMin > maxSleep) maxSleep = students[i].sleepMin;
    }

    // 防止除零错误
    double wakeRange = maxWake - minWake;
    double sleepRange = maxSleep - minSleep;
    if (wakeRange < 1e-6) wakeRange = 1;
    if (sleepRange < 1e-6) sleepRange = 1;

    // 构建特征向量并归一化
    for (int i = 0; i < n; ++i) {
        double wakeNorm = (double)(students[i].wakeUpMin >= 0 ? students[i].wakeUpMin - minWake : 0) / wakeRange;
        double sleepNorm = (double)(students[i].sleepMin >= 0 ? students[i].sleepMin - minSleep : 0) / sleepRange;
        double tidyNorm = (double)(students[i].tidy - 1) / 4;
        double socialNorm = (double)(students[i].social - 1) / 4;

        features[i][0] = wakeNorm * w_wake;
        features[i][1] = sleepNorm * w_sleep;
        features[i][2] = tidyNorm * w_tidy;
        features[i][3] = socialNorm * w_social;

        // 再次单位化
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

// 计算余弦相似度矩阵
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

// 返回宿舍分配结果
vector<vector<int> > matchDormsGreedyClustering(
    const vector<Student>& students,
    const vector<vector<double> >& simMatrix,
    int roomSize) {

    int n = students.size();
    vector<bool> assigned(n, false); // 是否已被分配
    vector<vector<int> > dorms;

    // 计算宿舍数量（考虑不能整除的情况）
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
                // 找不到合适的学生，提前结束
                break;
            }
        }
    }

    // 处理剩余学生（如果不能整除）
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

// 检查并输出未分配学生
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
        cout << "\n警告: " << unassigned.size() 
             << "名学生未被分配宿舍:\n";
        for (size_t i = 0; i < unassigned.size(); i++) {
            cout << "  - " << students[unassigned[i]].name << endl;
        }
    }
}

// 主函数
int main() {
    cout << "=== 宿舍匹配系统（余弦相似度 + 贪心聚类算法） ===\n";

    vector<Student> students = readStudentsFromCSV("../data.csv");
    if (students.empty()) {
        cerr << "错误：没有学生数据！\n";
        return 1;
    }

    cout << "已加载 " << students.size() << " 名学生数据。\n";

    vector<vector<double> > features = buildFeatureVectors(students);
    vector<vector<double> > simMatrix = computeCosineSimilarity(features);

    int roomSize;
    cout << "请输入每间宿舍的人数（4~12）：";
    cin >> roomSize;
    if (roomSize < 4 || roomSize > 12) {
        cout << "无效输入，默认设置为 4 人。\n";
        roomSize = 4;
    }

    // 初始匹配
    vector<vector<int> > dorms = matchDormsGreedyClustering(students, simMatrix, roomSize);

    // 检查未分配学生
    checkUnassignedStudents(students, dorms);

    // 输出最终结果
    cout << "\n=== 宿舍分配结果（最终版） ===\n";
    int dormCount = 0;
    for (size_t i = 0; i < dorms.size(); ++i) {
        const vector<int>& dorm = dorms[i];
        if (dorm.empty()) continue; // 跳过空宿舍
        
        cout << "宿舍 " << ++dormCount << "（" << dorm.size() << "人）:\n";

        // 先输出所有成员
        for (size_t j = 0; j < dorm.size(); ++j) {
            int idx = dorm[j];
            Student s = students[idx];

            string wakeTime = formatTime(s.wakeUpMin);
            string sleepTime = formatTime(s.sleepMin);

            cout << "  成员: " << setw(5) << left << s.name
                 << " | 起床: " << setw(7) << wakeTime
                 << " | 睡觉: " << setw(7) << sleepTime
                 << " | 爱干净: " << s.tidy << "/5"
                 << " | 社交: " << s.social << "/5\n";
        }

        // 计算平均相似度（在宿舍级别只计算一次）
        double totalSim = 0.0;
        int count = 0;
        for (size_t m = 0; m < dorm.size(); ++m) {
            for (size_t n = m + 1; n < dorm.size(); ++n) {
                totalSim += simMatrix[dorm[m]][dorm[n]];
                ++count;
            }
        }
        
        double avgSimilarity = (count > 0) ? totalSim / count * 100 : 0.0;
        cout << "  平均相似度: " << fixed << setprecision(2) << avgSimilarity << "%\n\n";
    }
	cout<<"最后一个寝室可能作息相似度极低，可以考虑将最后几个宿舍的同学划分到人数更少的宿舍"<<endl;
    return 0;
}
