#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>   // C++17
#include <cctype>       // isdigit
#include <stdexcept>    // runtime_error, out_of_range
#include <algorithm>    // std::min

using namespace std;
namespace fs = std::filesystem;

// ======================= CSVResult 구조체 =======================

struct CSVResult {
    vector<vector<int>> board; // CSV 데이터
    size_t rows{ 0 };            // 행 개수
    size_t cols{ 0 };            // 열 개수
};

// ======================= 유틸 함수들 =======================

// 양쪽 공백 제거 함수
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// 정수 문자열인지 검사 (선행 +, - 허용)
bool isInteger(const string& s) {
    if (s.empty()) return false;

    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s.size() == 1) return false; // "+"만 있는 경우 등
        i = 1;
    }

    for (; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }
    return true;
}

// ======================= CSV 읽기 =======================

CSVResult readCSV(const fs::path& filepath) {
    CSVResult result;

    ifstream file(filepath);
    if (!file.is_open()) {
        throw runtime_error("Error: Cannot open file: " + filepath.string());
    }

    string line;
    size_t lineNum = 0;

    while (getline(file, line)) {
        ++lineNum;
        vector<int> row;
        stringstream ss(line);
        string token;

        while (getline(ss, token, ',')) {
            string t = trim(token);

            // 빈 값이면 에러 (이 문제 조건은 모두 숫자)
            if (t.empty()) {
                throw runtime_error(
                    "Error: Empty value at line " + to_string(lineNum)
                );
            }

            // 정수 형식 체크
            if (!isInteger(t)) {
                throw runtime_error(
                    "Error: Non-integer token '" + t +
                    "' at line " + to_string(lineNum)
                );
            }

            row.push_back(stoi(t));
        }

        if (!row.empty()) {
            // 첫 번째 유효한 줄에서 열 개수 결정
            if (result.board.empty()) {
                result.cols = row.size();
            }
            else {
                // 이후 줄들은 열 개수가 동일해야 함
                if (row.size() != result.cols) {
                    throw runtime_error(
                        "Error: Inconsistent column count at line " +
                        to_string(lineNum)
                    );
                }
            }

            result.board.push_back(std::move(row));
        }
    }

    result.rows = result.board.size();

    if (result.rows == 0 || result.cols == 0) {
        throw runtime_error("Error: Empty CSV file or no valid data.");
    }

    return result;
}

// ======================= 도움말 출력 =======================

void print_help() {
    cout << "사용법:\n"
        << "  program <csv 파일이름> \n\n"
        << "예시:\n"
        << "  program  rectange_4x2.csv \n";
}

// ======================= solution 함수 =======================
int solution(vector<vector<int>> dots) {
    if ((dots.size() != 4) || (dots[0].size() != 2)) {
        return 0;
    }

    int minX = dots[0][0];
    int maxX = dots[0][0];
    int minY = dots[0][1];
    int maxY = dots[0][1];

    for (size_t i = 1; i < dots.size(); ++i)
    {
        minX = min(minX, dots[i][0]);
        maxX = max(maxX, dots[i][0]);
        minY = min(minY, dots[i][1]);
        maxY = max(maxY, dots[i][1]);
    }

    int width = std::abs(maxX - minX);
    int height = std::abs(maxY - minY);

    
    return width * height;
}

// ======================= main =======================

int main(int argc, char* argv[]) {
    try {
        string sFileName;
        bool hasFileName = false;

        // -------- 인자 파싱 --------
        if (argc > 1)
        {
            sFileName = argv[1];
            hasFileName = true;
        }

        if (!hasFileName) {
            print_help();
            throw runtime_error("csv 파일 명이 없음");
        }

        fs::path csvPath = sFileName;

        // -------- CSV 읽기 --------
        CSVResult csv = readCSV(csvPath);

        // 문제 제한사항 체크
        if (csv.rows != 4) {
            throw runtime_error("행 크기가 4 가 아님: " + to_string(csv.rows));
        }
        if (csv.cols != 2) {
            throw runtime_error("열 크기가 2 가 아님: " + to_string(csv.cols));
        }

        // -------- solution 호출 --------
        int ans = solution(csv.board);
        cout << "Rectangle area = "<< ans << "\n";
    }
    catch (const exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
