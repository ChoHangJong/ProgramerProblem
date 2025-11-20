
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
        << "  program -fn <csv 파일이름> -k <정수 k>\n\n"
        << "예시:\n"
        << "  program -fn board_100x100.csv -k 5\n";
}

// ======================= solution 함수 =======================
int solution(vector<vector<int>> board, int k) {
    if (board.empty() || board[0].empty()) {
        return 0;
    }

    size_t rows = board.size();
    size_t cols = board[0].size();

    long long sum = 0;   // overflow 여유를 위해 long long 사용

    for (size_t row = 0; row < rows; ++row) {
        for (size_t col = 0; col < cols; ++col) {
            int idx_sum = static_cast<int>(row + col);
            if (idx_sum <= k) {        // 문제 조건: i + j <= k
                sum += board[row][col];
            }
        }
    }

    return static_cast<int>(sum);
}

// ======================= main =======================

int main(int argc, char* argv[]) {
    try {
        string sFileName;
        int k = 0;
        bool hasFileName = false;
        bool hasK = false;

        // -------- 인자 파싱 --------
        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];

            if (arg == "-fn" && i + 1 < argc) {
                sFileName = argv[++i];
                hasFileName = true;
            }
            else if (arg == "-k" && i + 1 < argc) {
                k = atoi(argv[++i]);
                hasK = true;
            }
        }

        if (!hasFileName || !hasK) {
            print_help();
            if (!hasFileName) {
                throw runtime_error("csv 파일 명이 없음");
            }
            else {
                throw runtime_error("정수 k 값이 없음");
            }
        }

        if (k < 0) {
            throw runtime_error("k 는 0 이상이어야 합니다.");
        }

        fs::path csvPath = sFileName;

        // -------- CSV 읽기 --------
        CSVResult csv = readCSV(csvPath);

        cout << "Rows: " << csv.rows << "\n";
        cout << "Cols: " << csv.cols << "\n";

        // 문제 제한사항 체크
        if (csv.rows > 100) {
            throw runtime_error("행 크기가 100 초과: " + to_string(csv.rows));
        }
        if (csv.cols > 100) {
            throw runtime_error("열 크기가 100 초과: " + to_string(csv.cols));
        }

        if (k >= static_cast<int>(csv.rows + csv.cols)) {
            cout << "경고: k 값이 rows + cols 보다 크거나 같습니다. "
                << "어차피 모든 원소가 포함됩니다.\n";
        }

        // -------- solution 호출 --------
        int ans = solution(csv.board, k);
        cout << "sum(i + j <= " << k << ") = " << ans << "\n";
    }
    catch (const exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}




