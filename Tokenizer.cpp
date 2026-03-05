// Tokenizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include <chrono>
#include <functional>

#include "tokentype.hpp"

struct SourcePos {
    std::string filename;
    size_t line;

    // Default: unspecified position.
    SourcePos() : filename(""), line(0) {}

    // Construct a position for a given file and (1-based) line number.
    SourcePos(const std::string& f, size_t l) : filename(f), line(l) {}

    // Equality considers both filename and line.
    bool operator==(const SourcePos& other) const { return filename == other.filename && line == other.line; }

    // Ordering used for sorting and containers (maps/sets).
    // Primary key: filename (lexicographic)
    // Secondary key: line (ascending)
    bool operator<(const SourcePos& other) const
    {
        return (filename < other.filename) ||
            (filename == other.filename && line < other.line);
    }

    // Convenience greater-than; follows the same tuple ordering semantics.
    bool operator>(const SourcePos& other) const
    {
        return (filename > other.filename) ||
            (filename == other.filename && line > other.line);
    }

    // Print a compact representation showing only the base filename and line.
    // Example output: "[main.cpp 42]"
    void print() const
    {
        auto path = filename;
        std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
        std::cout << "[" << base_filename << " " << line << "]\n";
    }
};

#include "parserdict.hpp"

struct Token {
    TOKEN_TYPE type;
    std::string value;
    SourcePos pos;
    size_t line_pos;
    bool start;

    Token()
    {
        type = TOKEN_TYPE::INVALID;
        value = "";
        pos = SourcePos();
        line_pos = 0;
        start = false;
    }

    Token(TOKEN_TYPE type, std::string_view value, SourcePos pos, size_t line_pos, bool start) :
        type(type), value(value), pos(pos), line_pos(line_pos), start(start)
    {
    }

    bool operator==(const Token& other) const
    {
        return
            type == other.type &&
            value == other.value &&
            pos == other.pos &&
            line_pos == other.line_pos;
    }
};

struct ParseNode {
    char ch;
    TOKEN_TYPE type;
    std::vector<std::shared_ptr<ParseNode>> child;

    ParseNode(char ch) : ch(ch), type(INVALID) {}
    ParseNode(char ch, TOKEN_TYPE type) : ch(ch), type(type) {}
    ParseNode(char ch, TOKEN_TYPE type, std::vector<std::shared_ptr<ParseNode>> child) : ch(ch), type(type), child(child) {}
};

extern std::shared_ptr<ParseNode> ops_root;

#include "ops_tree.hpp"

/// <summary>
/// Prints a single token with its index, type, value, and source position.
/// Used for debugging the lexer output and parser state.
/// </summary>
/// <param name="index">The index of the token to print.</param>
void printTokens(std::vector<Token> tokens)
{
    auto index = 0;
    for (auto& tok : tokens) {
        std::cout <<
            "[" <<
            std::right <<
            std::setw(4) <<
            index++ <<
            std::setw(0) <<
            "] " <<
            std::left <<
            std::setw(15) <<
            parserDict[tok.type] <<         // Token type name from dictionary
            std::right <<
            std::setw(0) <<
            " " <<
            std::left <<
            std::setw(20) <<
            (tok.type == EOL ? "\\n" : tok.value) << " ";  // Show \n for EOL tokens
        tok.pos.print();  // Print source file and line number
    }
}

// does not include EOL
const auto is_white = [](int n) ->bool
    {
        return n == ' ' || n == '\t';
    };

// assumes upper case
const auto is_alpha = [](int n) ->bool
    {
        return n >= 'A' && n <= 'Z';
    };

// assumes upper case
const auto is_numeric = [](int n) ->bool
    {
        return n >= '0' && n <= '9';
    };

const auto is_hex = [](int n) ->bool
    {
        return (n >= '0' && n <= '9') || (n >= 'A' && n <= 'F');
    };

const auto is_decimal = [](int num) ->bool
    {
        return (num >= '0' && num <= '9');
    };

const auto is_octal = [](int num) ->bool
    {
        return (num >= '0' && num <= '7');
    };

const auto is_binary = [](int n) ->bool
    {
        return (n == '0' || n == '1');
    };

std::vector<Token> tokenize(const std::vector<std::pair<SourcePos, std::string_view>>& input)
{
    std::vector<Token> tokens;

    // Helper function for number tokenization (HEX, etc.)
    auto tokenizeNumber = [](const std::string_view& str, size_t& strPos, size_t sz, size_t toksz, bool& start, TOKEN_TYPE type, const auto& predicate, std::vector<Token>& tokens, const SourcePos& pos) ->bool
    {
        auto lastnumpos = strPos;
        auto done = false;
        auto ch2 = toupper(str[strPos + toksz]);
        do {
            while (strPos + toksz < sz && predicate(ch2)) {
                lastnumpos = strPos + toksz++;
                ch2 = toupper(str[lastnumpos + 1]);
            }

            if ((strPos + toksz >= sz) || (!is_white(ch2))) {
                done = true;
                continue;
            }

            while (strPos + toksz < sz && is_white(ch2)) {
                ch2 = toupper(str[strPos + ++toksz]);
            }
        } while (!done);

        toksz = lastnumpos - strPos + 1;
        tokens.push_back({ type, str.substr(strPos, toksz), pos, strPos, start });
        strPos += toksz;
        start = false;
        return toksz > 1 || type == TOKEN_TYPE::DECNUM;
    };

    const auto handle_single = [](std::string_view str, size_t& strPos, size_t sz, TOKEN_TYPE s, std::vector<Token>& tokens, const SourcePos& pos, bool& start) ->bool
        {
            auto toktype = s;
            size_t toksz = 1;
            tokens.push_back({ toktype, str.substr(strPos, toksz), pos, strPos, start });
            strPos += toksz;
            start = false;
            return true;
        };

    const auto handle_pair = [](std::string_view str, size_t& strPos, size_t sz, char ch, TOKEN_TYPE s, TOKEN_TYPE d, std::vector<Token>& tokens, const SourcePos& pos, bool& start) ->bool
        {
            auto toktype = s;
            size_t toksz = 1;
            if (strPos + 1 < sz) {
                auto& ch2 = str[strPos + toksz];
                if (ch2 == ch) {
                    toksz = 2;
                    toktype = d;
                }
            }
            tokens.push_back({ toktype, str.substr(strPos, toksz), pos, strPos, start });
            strPos += toksz;
            start = false;
            return true;
        };

    for (auto& [pos, str] : input) {
        size_t strPos = 0;
        auto start = true;
        auto sz = str.size();
        size_t toksz = 0;
        while (strPos < sz) {
            auto& ch = str[strPos];

            auto upper = toupper(ch);
            if (is_alpha(upper))
            {
                auto curnode = ops_root;
                std::shared_ptr<ParseNode> found = nullptr;
                auto done = false;
                toksz = 0;
                size_t fountoksz = 0;
                do {
                    if (strPos + toksz >= sz)
                        break;

                    auto it = std::lower_bound(curnode->child.begin(), curnode->child.end(), upper,
                        [](const std::shared_ptr<ParseNode>& node, char c)
                        {
                            return node->ch < c;
                        });

                    if (it == curnode->child.end() || (*it)->ch != upper)
                        break;
                    curnode = (*it);
                    ++toksz;
                    if (curnode->type != TOKEN_TYPE::INVALID) {
                        if (strPos + toksz < sz) {
                            auto ch2 = str[strPos + toksz];
                            if (is_white(ch2) || ch2 == '\n') {
                                found = curnode;
                                fountoksz = toksz;
                            }
                        }
                    }
                    upper = toupper(str[strPos + toksz]);
                } while (!done);
                    
                if (found != nullptr) {
                    tokens.push_back({ found->type, str.substr(strPos, fountoksz), pos, strPos, start });
                    strPos += toksz;
                    start = false;
                    continue;
                }
                else {
                    std::cout << "Unknown token in " << str << "\n\n";
                    return tokens;
                }
            }

            switch (ch) {
                case ' ':
                case '\t':
                {
                    toksz = 1;
                    while (strPos + toksz < sz && (is_white(str[strPos + toksz])))
                        toksz++;
                    strPos += toksz;
                    break;
                }

                case '\n':
                {
                    toksz = 1;
                    if (strPos + toksz < sz && str[strPos + toksz] == '\r') {
                        toksz++;
                    }
                    tokens.push_back({ TOKEN_TYPE::EOL, str.substr(strPos, toksz), pos, strPos, start });
                    strPos += toksz;
                    start = false;
                    break;
                }

                case '\r':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::EOL, tokens, pos, start);
                    break;
                }

                case '(':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::LPAREN, tokens, pos, start);
                    break;
                }

                case ')':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::RPAREN, tokens, pos, start);
                    break;
                }

                case ';':
                {
                    toksz = 1;
                    while (strPos + toksz < sz && str[strPos + toksz] != '\n')
                        toksz++;
                    tokens.push_back({ TOKEN_TYPE::COMMENT, str.substr(strPos, toksz), pos, strPos, start });
                    strPos += toksz;
                    start = false;
                    break;
                }

                case ':':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::COLAN, tokens, pos, start);
                    break;
                }

                case '+':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::PLUS, tokens, pos, start);
                    break;
                }

                case '-':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::MINUS, tokens, pos, start);
                    break;
                }

                case '*':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::MUL, tokens, pos, start);
                    break;
                }

                case '/':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::DIV, tokens, pos, start);
                    break;
                }

                case ',':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::COMMA, tokens, pos, start);
                    break;
                }

                case '#':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::POUND, tokens, pos, start);
                    break;
                }

                case '~':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::ONESCOMP, tokens, pos, start);
                    break;
                }

                case '@':
                {
                    handle_single(str, strPos, sz, TOKEN_TYPE::AT, tokens, pos, start);
                    break;
                }

                case '$':
                {
                    tokenizeNumber(str, strPos, sz, 1, start, TOKEN_TYPE::HEXNUM, is_hex, tokens, pos);
                    break;
                }

                case '%':
                {
                    tokenizeNumber(str, strPos, sz, 1, start, TOKEN_TYPE::BINNUM, is_binary, tokens, pos);
                    break;
                }

                case '<':
                {
                    handle_pair(str, strPos, sz, ch, TOKEN_TYPE::LT, TOKEN_TYPE::SLEFT, tokens, pos, start);
                    break;
                }

                case '>':
                {
                    handle_pair(str, strPos, sz, ch, TOKEN_TYPE::GT, TOKEN_TYPE::SRIGHT, tokens, pos, start);
                    break;
                }

                case '|':
                {
                    handle_pair(str, strPos, sz, ch, TOKEN_TYPE::BIT_OR, TOKEN_TYPE::LOGICAL_OR, tokens, pos, start);
                    break;
                }

                case '=':
                {
                    handle_pair(str, strPos, sz, ch, TOKEN_TYPE::EQUAL, TOKEN_TYPE::DEQUAL, tokens, pos, start);
                    break;
                }

                case '&':
                {
                    if (strPos + 1 < sz) {
                        toksz = 1;
                        auto& ch2 = str[strPos + toksz];
                        if (is_octal(ch2)) {
                            tokenizeNumber(str, strPos, sz, toksz, start, TOKEN_TYPE::OCTNUM, is_octal, tokens, pos);
                            break;
                        }
                    }
                    handle_pair(str, strPos, sz, ch, TOKEN_TYPE::BIT_AND, TOKEN_TYPE::LOGICAL_AND, tokens, pos, start);
                    break;
                }

                case '0':
                {
                    toksz = 1;
                    if (strPos + toksz < sz && toupper(str[strPos + toksz]) == 'O') {
                        toksz++;
                    }
                    tokenizeNumber(str, strPos, sz, toksz, start, TOKEN_TYPE::OCTNUM, is_octal, tokens, pos);
                    break;
                }

                case '1':
                case '2':
                case '3':
                case '4':
                case '5': 
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    tokenizeNumber(str, strPos, sz, 1, start, TOKEN_TYPE::DECNUM, is_decimal, tokens, pos);
                    break;
                }
                 
                case '\'':
                case '"':
                {
                    toksz = 1;
                    while (strPos + toksz < sz && str[strPos + toksz] != ch) {
                        if (str[strPos + toksz] == '\\')
                            toksz++;
                        toksz++;
                    }
                    if (str[strPos + toksz] != ch) {
                        std::cout << "Unknown token in " << str;
                        return tokens;
                    }

                    toksz++;
                    auto toktype = (toksz > 4 || (toksz == 3 && str[strPos + toksz - 2] != '\\'))
                        ? TOKEN_TYPE::TEXT
                        : TOKEN_TYPE::CHAR;
                    tokens.push_back({ toktype, str.substr(strPos, toksz), pos, strPos, start });
                    strPos += toksz;
                    start = false;
                    break;
                }
                default:
                    std::cout << "Unknown token in " << str;
                    return tokens;
            }
        }
    }
    return tokens;
}

int main()
{
    const std::vector<std::pair<SourcePos, std::string_view>> input =
    {
        { { "testfile.asm", 1 }, "; this is a comment test\n"},
        { { "testfile.asm", 2 }, "  'GRAMPA' ; this is another comment test\n" },
        { { "testfile.asm", 3 }, "'G' '\\n'\n"},
        { { "testfile.asm", 4 }, " 510101 ; decimal number\n"},
        { { "testfile.asm", 5 }, " %1010 1010  %10101\n"},
        { { "testfile.asm", 6 }, " $A6a5 D45F $DEAD\n"},
        { { "testfile.asm", 7 }, " JMP $A6a5\n"},
        { { "testfile.asm", 8 }, " ANC $A6a5\n"},
        { { "testfile.asm", 9 }, " ANC2 $D6a5\n"},
        { { "testfile.asm", 10 }, " jsr\n"},
        { { "testfile.asm", 11 }, " &66 && 7\n"},
        { { "testfile.asm", 10 }, " 1 & 1\n"},
        { { "testfile.asm", 11 }, " %101 | 1\n"},
        { { "testfile.asm", 12 }, " 55 || 2\n"},
    };
    
    auto start_time = std::chrono::high_resolution_clock::now();
    auto tokens = tokenize(input);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    auto seconds = duration.count() / 1000000.0;
    std::cout << "tokenize took " << seconds << " seconds\n";

    printTokens(tokens);
}
