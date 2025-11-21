#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <variant>
#include <cctype>
#include <stdexcept>
#include <algorithm>

using namespace std;

// ======================= variant 타입 정의 =======================

using CSVValue = std::variant<int, bool, std::string>;
using CSVRow = std::vector<CSVValue>;

// ======================= CSVResult 구조체 =======================

struct CSVResult {
    std::vector<CSVRow> cells;
    size_t rows{ 0 };
    size_t cols{ 0 };

    // (r, c) 위치의 값 얻기 (const 참조)
    const CSVValue& get(size_t r, size_t c) const {
        if (r >= rows || c >= cols) {
            throw std::out_of_range(
                "CSVResult::get - index out of range: (" +
                std::to_string(r) + ", " + std::to_string(c) + ")"
            );
        }
        return cells[r][c];
    }
};

// ======================= 유틸 함수들 =======================

// 양쪽 공백 제거
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// 소문자로 변환
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char ch) { return std::tolower(ch); });
    return s;
}

// 정수 문자열인지 검사 (선행 +, - 허용)
bool isIntegerString(const std::string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s.size() == 1) return false;
        i = 1;
    }
    for (; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }
    return true;
}

// 하나의 토큰을 CSVValue로 파싱
CSVValue parseCSVValue(const std::string& token) {
    std::string s = trim(token);
    if (s.empty()) {
        return std::string{};
    }

    // true / false → bool
    std::string lower = toLower(s);
    if (lower == "true")  return true;
    if (lower == "false") return false;

    // 정수라면 int
    if (isIntegerString(s)) {
        try {
            int v = std::stoi(s);
            return v;
        }
        catch (...) {
            // overflow 등 발생 시 그냥 문자열로 둔다
        }
    }

    // 나머지는 string
    return s;
}

// CSV 파일 읽기
CSVResult readCSV(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("Failed to open CSV file: " + path);
    }

    CSVResult result;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        CSVRow row;

        while (std::getline(ss, token, ',')) {
            row.push_back(parseCSVValue(token));
        }

        if (!row.empty()) {
            result.cols = std::max(result.cols, row.size());
            result.cells.push_back(std::move(row));
        }
    }

    result.rows = result.cells.size();
    if (result.rows == 0 || result.cols == 0) {
        throw std::runtime_error("CSV is empty or invalid.");
    }

    return result;
}

// ======================= CSVValue → int / bool 변환 헬퍼 =======================

int toInt(const CSVValue& v) {
    if (auto p = std::get_if<int>(&v)) {
        return *p;
    }
    if (auto p = std::get_if<bool>(&v)) {
        return *p ? 1 : 0;
    }
    if (auto p = std::get_if<std::string>(&v)) {
        std::string s = trim(*p);
        if (isIntegerString(s)) {
            return std::stoi(s);
        }
    }
    throw std::runtime_error("Cannot convert CSVValue to int");
}

bool toBool(const CSVValue& v) {
    if (auto p = std::get_if<bool>(&v)) {
        return *p;
    }
    if (auto p = std::get_if<int>(&v)) {
        return (*p != 0);
    }
    if (auto p = std::get_if<std::string>(&v)) {
        std::string lower = toLower(trim(*p));
        if (lower == "true" || lower == "1") return true;
        if (lower == "false" || lower == "0") return false;
    }
    throw std::runtime_error("Cannot convert CSVValue to bool");
}

// 특정 행을 vector<int>로
std::vector<int> toIntRow(const CSVResult& csv, size_t rowIndex) {
    if (rowIndex >= csv.rows) {
        throw std::out_of_range("toIntRow: row index out of range");
    }
    const CSVRow& row = csv.cells[rowIndex];
    std::vector<int> result;
    result.reserve(row.size());
    for (const auto& v : row) {
        result.push_back(toInt(v));
    }
    return result;
}

// 특정 행을 vector<bool>로
std::vector<bool> toBoolRow(const CSVResult& csv, size_t rowIndex) {
    if (rowIndex >= csv.rows) {
        throw std::out_of_range("toBoolRow: row index out of range");
    }
    const CSVRow& row = csv.cells[rowIndex];
    std::vector<bool> result;
    result.reserve(row.size());
    for (const auto& v : row) {
        result.push_back(toBool(v));
    }
    return result;
}

// ======================= 문제: 빈 배열에 추가, 삭제하기 =======================

// arr, flag를 받아 최종 X를 반환
std::vector<int> solution(std::vector<int> arr,
     std::vector<bool> flag)
{
    std::vector<int> X;

    size_t n = std::min(arr.size(), flag.size());
    for (size_t i = 0; i < n; ++i) {
        if (flag[i]) {
            // X 뒤에 arr[i]를 arr[i] * 2번 추가
            if (arr[i] < 0) {
                throw std::runtime_error("arr[i] is negative.");
            }
            X.insert(X.end(), static_cast<size_t>(arr[i]) * 2, arr[i]);
        }
        else {
            // X에서 마지막 arr[i]개의 원소 제거
            if (arr[i] < 0 || static_cast<size_t>(arr[i]) > X.size()) {
                throw std::runtime_error("Too many elements to remove from X.");
            }
            for (int k = 0; k < arr[i]; ++k) {
                X.pop_back();
            }
        }
    }
    return X;
}

// 결과 출력용
void printVector(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

// ======================= main =======================

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <csv-file-path>\n";
            return 1;
        }

        std::string csvPath = argv[1];
        CSVResult csv = readCSV(csvPath);

        // 1행 → arr, 2행 → flag 라고 가정
        if (csv.rows < 2) {
            throw std::runtime_error("CSV must have at least 2 rows (arr, flag).");
        }

        std::vector<int>  arr = toIntRow(csv, 0);
        std::vector<bool> flag = toBoolRow(csv, 1);

        if (arr.size() != flag.size()) {
            std::cerr << "Warning: arr and flag length differ. "
                "Using min length.\n";
        }

        std::vector<int> result = solution(arr, flag);

        std::cout << "Result X = ";
        printVector(result);

    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
