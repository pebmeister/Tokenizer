// BuildTokTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include <chrono>
#include <functional>
#include <algorithm>
#include "tokentype.hpp"

// Updated struct to allow for branching (Trie structure)
struct ParseNode {
    char ch;
    TOKEN_TYPE type;
    std::vector<std::shared_ptr<ParseNode>> child;

    ParseNode(char ch) : ch(ch), type(INVALID) {}
    ParseNode(char ch, TOKEN_TYPE type) : ch(ch), type(type) {}
    ParseNode(char ch, TOKEN_TYPE type, std::vector<std::shared_ptr<ParseNode>> child) : ch(ch), type(type), child(child) {}
};

std::shared_ptr<ParseNode> op_root = std::make_shared<ParseNode>(0);
std::vector<std::pair<TOKEN_TYPE, std::string>> op_toklist = 
{
{ ORA,      "ORA" },
{ AND,      "AND" },
{ EOR,      "EOR" },
{ ADC,      "ADC" },
{ SBC,      "SBC" },
{ CMP,      "CMP" },
{ CPX,      "CPX" },
{ CPY,      "CPY" },
{ DEC,      "DEC" },
{ DEX,      "DEX" },
{ DEY,      "DEY" },
{ INC,      "INC" },
{ INX,      "INX" },
{ INY,      "INY" },
{ ASL,      "ASL" },
{ ROL,      "ROL" },
{ LSR,      "LSR" },
{ ROR,      "ROR" },
{ LDA,      "LDA" },
{ STA,      "STA" },
{ LDX,      "LDX" },
{ STX,      "STX" },
{ LDY,      "LDY" },
{ STY,      "STY" },
{ RMB0,     "RMB0" },
{ RMB1,     "RMB1" },
{ RMB2,     "RMB2" },
{ RMB3,     "RMB3" },
{ RMB4,     "RMB4" },
{ RMB5,     "RMB5" },
{ RMB6,     "RMB6" },
{ RMB7,     "RMB7" },
{ SMB0,     "SMB0" },
{ SMB1,     "SMB1" },
{ SMB2,     "SMB2" },
{ SMB3,     "SMB3" },
{ SMB4,     "SMB4" },
{ SMB5,     "SMB5" },
{ SMB6,     "SMB6" },
{ SMB7,     "SMB7" },
{ STZ,      "STZ" },
{ TAX,      "TAX" },
{ TXA,      "TXA" },
{ TAY,      "TAY" },
{ TYA,      "TYA" },
{ TSX,      "TSX" },
{ TXS,      "TXS" },
{ PLA,      "PLA" },
{ PHA,      "PHA" },
{ PLP,      "PLP" },
{ PHP,      "PHP" },
{ PHX,      "PHX" },
{ PHY,      "PHY" },
{ PLX,      "PLX" },
{ PLY,      "PLY" },
{ BRA,      "BRA" },
{ BPL,      "BPL" },
{ BMI,      "BMI" },
{ BVC,      "BVC" },
{ BVS,      "BVS" },
{ BCC,      "BCC" },
{ BCS,      "BCS" },
{ BNE,      "BNE" },
{ BEQ,      "BEQ" },
{ BBR0,     "BBR0" },
{ BBR1,     "BBR1" },
{ BBR2,     "BBR2" },
{ BBR3,     "BBR3" },
{ BBR4,     "BBR4" },
{ BBR5,     "BBR5" },
{ BBR6,     "BBR6" },
{ BBR7,     "BBR7" },
{ BBS0,     "BBS0" },
{ BBS1,     "BBS1" },
{ BBS2,     "BBS2" },
{ BBS3,     "BBS3" },
{ BBS4,     "BBS4" },
{ BBS5,     "BBS5" },
{ BBS6,     "BBS6" },
{ BBS7,     "BBS7" },
{ STP,      "STP" },
{ WAI,      "WAI" },
{ BRK,      "BRK" },
{ RTI,      "RTI" },
{ JSR,      "JSR" },
{ RTS,      "RTS" },
{ JMP,      "JMP" },
{ BIT,      "BIT" },
{ TRB,      "TRB" },
{ TSB,      "TSB" },
{ CLC,      "CLC" },
{ SEC,      "SEC" },
{ CLD,      "CLD" },
{ SED,      "SED" },
{ CLI,      "CLI" },
{ SEI,      "SEI" },
{ CLV,      "CLV" },
{ NOP,      "NOP" },
{ SLO,      "SLO" },
{ RLA,      "RLA" },
{ SRE,      "SRE" },
{ RRA,      "RRA" },
{ SAX,      "SAX" },
{ LAX,      "LAX" },
{ DCP,      "DCP" },
{ ISC,      "ISC" },
{ ANC,      "ANC" },
{ ANC2,     "ANC2" },
{ ALR,      "ALR" },
{ ARR,      "ARR" },
{ XAA,      "XAA" },
{ AXS,      "AXS" },
{ USBC,     "USBC" },
{ AHX,      "AHX" },
{ SHY,      "SHY" },
{ SHX,      "SHX" },
{ TAS,      "TAS" },
{ LAS,      "LAS" },
};

#include "parserdict.hpp"


extern void printParseTree(const std::shared_ptr<ParseNode>& node);
extern void generateInitializerList(std::shared_ptr<ParseNode>& root);
extern void buildtoktree(std::shared_ptr<ParseNode>&root, std::vector<std::pair<TOKEN_TYPE, std::string>>& toks);

static void insertToken(std::shared_ptr<ParseNode> root, TOKEN_TYPE type, const std::string& word)
{
    auto curnode = root;
    for (auto& ch : word)
    {
        auto it = std::lower_bound(curnode->child.begin(), curnode->child.end(), ch,
            [](const std::shared_ptr<ParseNode>& node, char c) {
                return node->ch < c;
            });

        if (it != curnode->child.end() && (*it)->ch == ch) {
            curnode = *it;
        } else {
            auto newNode = std::make_shared<ParseNode>(ch);
            curnode->child.insert(it, newNode);
            curnode = newNode;
        }
    }

    curnode->type = type;
}

void printParseTreeHelper(const std::shared_ptr<ParseNode>& node, int depth)
{
    if (!node) {
        return;
    }

    if (node->ch != 0) {
        std::string indent(depth * 4, ' ');
        std::cout << indent << "ch: '" << (node->ch == 0 ? '0' : node->ch) << "'";

        if (node->type != INVALID) {
            std::cout << " -> TOKEN_TYPE: " << node->type;
        }

        std::cout << std::endl;
    }
    for (const auto& child : node->child) {
        printParseTreeHelper(child, depth + 1);
    }
}

void printParseTree(const std::shared_ptr<ParseNode>& root)
{
    std::cout << "=== Parse Tree ===" << std::endl;
    printParseTreeHelper(root, 0);
}

void generateInitializerListHelper(const std::shared_ptr<ParseNode>& node, int depth, std::ostream& out)
{
    if (!node) {
        return;
    }

    std::string indent(depth * 4, ' ');
    
    if (node->ch == 0) {
        // Root node
        out << "std::make_shared<ParseNode>(\n";
        out << indent << "  0, TOKEN_TYPE::INVALID,\n";
        out << indent << "  std::vector<std::shared_ptr<ParseNode>>{\n";
    } else {
        out << indent << "std::make_shared<ParseNode>(\n";
        out << indent << "  '" << node->ch << "', ";
        
        if (node->type != INVALID) {
            out << "TOKEN_TYPE::" << parserDict[node->type];
        } else {
            out << "TOKEN_TYPE::INVALID";
        }
        out << ",\n";
        
        if (!node->child.empty()) {
            out << indent << "  std::vector<std::shared_ptr<ParseNode>>{\n";
        } else {
            out << indent << "  std::vector<std::shared_ptr<ParseNode>>{}\n";
            out << indent << ")";
            return;
        }
    }

    for (size_t i = 0; i < node->child.size(); ++i) {
        generateInitializerListHelper(node->child[i], depth + 2, out);
        
        if (i < node->child.size() - 1) {
            out << ",\n";
        } else {
            out << "\n";
        }
    }

    out << indent << "  }\n";
    out << indent << ")";
}

void generateInitializerList(std::shared_ptr<ParseNode>& root)
{
    std::cout << "\n// === Generated Initializer List ===" << std::endl;
    std::cout << "std::shared_ptr<ParseNode> root = \n";
    generateInitializerListHelper(root, 0, std::cout);
    std::cout << ";\n";
}

void buildtoktree(std::shared_ptr<ParseNode>& root, std::vector<std::pair<TOKEN_TYPE, std::string>>& toks)
{
    std::sort(toks.begin(), toks.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    for (const auto& [tok, str] : toks) {
        insertToken(root, tok, str);
    }
}

int main()
{
    buildtoktree(op_root, op_toklist);
    generateInitializerList(op_root);
}
