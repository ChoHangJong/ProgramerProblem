//#pragma once


#ifndef _database
#define _database

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <tuple>

namespace _csv
{
    auto csv_read_row(std::istream& in, char delimiter)
    {
        std::stringstream ss;
        bool inquotes = false;
        bool packs = false;
        std::vector<std::string> row;

        while (in.good())
        {
            char c = in.get();

            switch (c)
            {
            case '"':
                if (!inquotes)      // begin quote char
                    inquotes = true;
                else
                {
                    if (in.peek() == '"')   // 2 consecutive quotes resolve to 1
                        ss << static_cast<char>(in.get());
                    else                    // end quote char
                        inquotes = false;
                }
                break;

            case '{':
                if (!packs)
                    packs = true;
                else
                    throw std::ios_base::failure{ "csv_read_row : '{' duplicated" };
                break;

            case '}':
                if (!packs)
                    throw std::ios_base::failure{ "csv_read_row : '{' must be followed by '}'" };
                else
                    packs = false;
                break;

            case '\r': case '\n':
                if (!inquotes && !packs)
                {
                    if (in.peek() == '\n')
                        in.get();

                    row.push_back(ss.str());
                    row.shrink_to_fit();
                    return row;
                }
                break;

            default:
                if (c == delimiter && !inquotes)
                {
                    row.push_back(ss.str());
                    ss.str("");
                }
                else
                    ss << c;
                break;

            }
        }
    }

    template <typename Str_t>
    auto csv_read(const Str_t file_path)
    {
        std::ifstream in(file_path);
        std::vector<std::vector<std::string>> parsed;
        parsed.reserve(0x20);

        if (in.fail())
            throw std::ios_base::failure{ "csv_read : cannot open file" };

        while (in.good())
            parsed.push_back(_csv::csv_read_row(in, ','));

        parsed.shrink_to_fit();
        return parsed;
    }

    // ...

}

template <typename... TupleElems>
class database
{
public:
    template <typename Str_t>
    database(const Str_t file_path)
    {
        auto parsed_csv = _csv::csv_read(file_path);

        // ...

    }

private:
    std::vector<std::tuple<TupleElems...>> impl;
};

#endif
