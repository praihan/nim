#include <nim/Application.h>
#include <nim/nim_Assert.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <bitset>
#include "tinycon.h"
#include "rlutil.h"
#include "parse.hpp"

using std::vector;
using std::map;
using std::string;
using std::basic_stringstream;
using std::stringstream;
using std::ostringstream;
using std::getline;
using std::cout;
using std::cin;
using std::endl;
using std::ostream;
using std::move;
using std::transform;
using std::setw;
using std::left;
using std::right;
using std::streamsize;
using std::bitset;
using nim::int32;

#define PILE_MAX 20
#define PILE_MIN 10

#define CONSOLE_WIDTH 80
#define DESCRIPTION_WIDTH 50

#define ERR_GENERIC "Error"
#define ERR_SYNTAX "SyntaxError"
#define ERR_ARGUMENT "ArgumentError"
#define ERR_RANGE "RangeError"

namespace nim
{
    namespace detail
    {
        struct Pile
        {
            Pile& operator =(int32 new_count);
            Pile& operator =(const Pile& other);
            Pile& operator --();
            Pile& operator ++();
            Pile operator --(int32);
            Pile operator ++(int32);
            Pile& operator -=(int32 diff);
            Pile& operator -=(const Pile& other);
            Pile& operator +=(int32 diff);
            Pile& operator +=(const Pile& other);

            bool operator ==(int32 other_count) const;
            bool operator ==(const Pile& other) const;

            void Rnd();

            int32 Count() const;
            operator int32() const;

            Pile();
            Pile(int32 new_count);

        private:
            int32 count;
        };


        // Utils

        template <typename T>
        static vector<T>& split(const T& s, vector<T>& elems);
        template <typename T>
        static vector<T> split(const T& s);

        static void lowercase(string& s);

        static vector<string> word_wrap_fill(const string& text, size_t line_length);

        static string print_err(const string& err_type);

        struct NimImpl
        {
            vector<string> Cmd;
            Pile Piles[3];
            bool Player1Turn;
            bool CPU;
            tinyConsole* Console;

            string Player1Name;
            string Player2Name;
            string CPUName;

            bool Quit;

            void DecideTurn()
            {
                Player1Turn = (::rand() % 2) ? true : false;
            }

            void SwitchTurn()
            {
                Player1Turn ^= true;
            }

            void SetCurrentPlayerName(const string& prompt)
            {
                (Player1Turn ? Player1Name : Player2Name) = prompt;
            }

            const string& GetCurrentPlayerName() const
            {
                return ((Player1Turn) ? Player1Name : Player2Name);
            }

            void Rnd()
            {
                Piles[0].Rnd();
                Piles[1].Rnd();
                Piles[2].Rnd();
            }

            void Restart()
            {
                Rnd();
                DecideTurn();
                StartTurn();
            }

            void StartTurn()
            {
                UpdatePrompt();
                ShowPiles();
                if (CPU && !Player1Turn)
                {
                    CPUTurn();
                }
            }

            void CPUTurn()
            {
                // int(ceil(log2(PILE_MAX)))
#define BITSET_MAX 5
                bitset<BITSET_MAX> state(0);

                for (auto i = (BITSET_MAX - 1); i >= 0; --i)
                {
                    // odd check for each bit
                    state.set(i, !!((((Piles[0] >> i) & 1) + ((Piles[1] >> i) & 1) + ((Piles[2] >> i) & 1)) & 1));
                }

                auto move_made = false;
                for (auto i = (BITSET_MAX - 1); i >= 0; --i)
                {
                    // highest set bit
                    if (state.test(i))
                    {
                        for (auto j = 0; j < (BITSET_MAX - 1); ++j)
                        {
                            // pile with highest set bit
                            if ((Piles[j] >> i) & 1)
                            {
                                // xor for state 00000
                                auto target = (state ^ bitset<BITSET_MAX>(Piles[j])).to_ulong();
                                CPUTake(Piles[j] - int32(target), j);
                                move_made = true;
                                break;
                            }
                        }
                        break;
                    }
                }

                // no good moves, take 1 from biggest pile
                if (!move_made)
                {
                    auto max_index = 0;
                    for (auto j = 1; j < (BITSET_MAX - 1); ++j)
                    {
                        if (Piles[j] > Piles[max_index]) { max_index = j; }
                    }
                    CPUTake(1, max_index);
                }

                NextTurn();
            }

            void CPUTake(int32 num, int32 pile)
            {
                cout << CPUName << "> " << "take " << num << " from " << (pile + 1) << "\n";
                Piles[pile] -= num;
            }

            void UpdatePrompt()
            {
                Console->setPrompt(GetCurrentPlayerName() + "> ");
            }

            void NextTurn()
            {
                if (GameOver())
                {
                    if (Player1Turn || !CPU)
                    {
                        cout << "  Congratulations, " << GetCurrentPlayerName() << "! You have won!";
                    }
                    else
                    {
                        cout << "  The CPU has won the game.";
                    }
                    cout << "\n\n";
                    Console->quit();
                }
                else
                {
                    SwitchTurn();
                    StartTurn();
                }
            }

            void ShowPiles()
            {
                cout << *this << "\n";
            }

            friend ostream& operator <<(ostream& os, const NimImpl& i)
            {
                os << "  " << i.Piles[0] << "  " << i.Piles[1] << "  " << i.Piles[2];
                return os;
            }

            bool GameOver() const
            {
                return (Piles[0] == 0) && (Piles[1] == 0) && (Piles[2] == 0);
            }
        };
    }

    // Definition of impl

    struct Application::impl : public detail::NimImpl
    {
    };

    namespace detail
    {

        struct ConsoleCmd
        {
            string Name;
            void(*Callback)(NimImpl* nimpl, const vector<string>& parts);
        };

        struct ConsoleCmdDesc
        {
            string Syntax; // syntax for invoking command
            vector<string> Description; // lines of description for using command
        };

        static void CmdHelp(NimImpl*, const vector<string>&);
        static void CmdShow(NimImpl*, const vector<string>&);
        static void CmdTake(NimImpl*, const vector<string>&);
        static void CmdName(NimImpl*, const vector<string>&);
        static void CmdHow2Play(NimImpl*, const vector<string>&);
        static void CmdRestart(NimImpl*, const vector<string>&);
        static void CmdExit(NimImpl*, const vector<string>&);
        static void CmdRq(NimImpl*, const vector<string>&);
        static void CmdColor(NimImpl*, const vector<string>&);

        // Set up word wrapping for help
        static void WordWrapSetUp();

        static map<string, ConsoleCmdDesc> ConsoleCmdDescs = {
            { "help", { "help [command_name]...", { "Display the help screen (or the help for specified commands only)." } } },
            { "show", { "show [pile]...", { "Show the piles (or the specified piles in the order of [pile], and valid pile is one of {1,2,3} corresponding to the pile number)" } } },
            { "take", { "[take] <number> [from] <pile>", { "Take <number> of chips (in range [1, pile length]) from <pile>-th pile (in range [1, 3])." } } },
            { "name", { "name <name>", { "Set your name to <name>. Special characters and spaces are allowed (case-sensitive)." } } },
            { "how2play", { "how2play", { "Print rules of the game and how to play NIM with this program." } } },
            { "restart", { "restart [cpu|human]", { "Restart game with either CPU or human opponent." } } },
            { "exit", { "exit", { "Exit the entire program." } } },
            { "rq", { "rq", { "Ragequit." } } },
            { "color", { "color <color>", { "Sets the font color to <color> (one of {blue, green, cyan, red, magenta, brown, grey, darkgrey, lightblue, lightgreen, lightcyan, lightred, lightmagenta, yellow, white} (case-insensitive))." } } }
        };

        static const ConsoleCmd Commands[] = {
            { "help", &CmdHelp },
            { "show", &CmdShow },
            { "take", &CmdTake },
            { "name", &CmdName },
            { "how2play", &CmdHow2Play },
            { "restart", &CmdRestart },
            { "exit", &CmdExit },
            { "rq", &CmdRq },
            { "color", &CmdColor },
            { "", nullptr }
        };

        static const map<string, int> ColorsMap = {
            { "black", rlutil::BLACK },
            { "blue", rlutil::BLUE },
            { "green", rlutil::GREEN },
            { "cyan", rlutil::CYAN },
            { "red", rlutil::RED },
            { "magenta", rlutil::MAGENTA },
            { "brown", rlutil::BROWN },
            { "grey", rlutil::GREY },
            { "darkgrey", rlutil::DARKGREY },
            { "lightblue", rlutil::LIGHTBLUE },
            { "lightgreen", rlutil::LIGHTGREEN },
            { "lightcyan", rlutil::LIGHTCYAN },
            { "lightred", rlutil::LIGHTRED },
            { "lightmagenta", rlutil::LIGHTMAGENTA },
            { "yellow", rlutil::YELLOW },
            { "white", rlutil::WHITE }
        };

        struct NimConsole : public tinyConsole
        {
            NimConsole(NimImpl* g) : tinyConsole("player>"), game(g) {}

            virtual int trigger(string s) override
            {
                vector<string> parts;
                split(s, parts);
                if (parts.size() == static_cast<size_t>(0)) { return 0; }
                string& cmd_name = parts[0];
                lowercase(cmd_name);
                for (auto i = 0;; ++i)
                {
                    auto& cmd = Commands[i];
                    if (cmd.Name == "")
                    {
                        using namespace numerics;
                        int32 test_int;
                        if (parse_integral<int32>(cmd_name.c_str(), &test_int))
                        {
                            parts.insert(parts.begin(), "take");
                            CmdTake(game, parts);
                            return 0;
                        }
                        cout << print_err(ERR_SYNTAX) << "Command '" << cmd_name << "' not found. Type 'help' for list of available commands.\n";
                        break;
                    }
                    else if (cmd.Name == cmd_name)
                    {
                        cmd.Callback(game, parts);
                        break;
                    }
                }
                return 0;
            }

        private:
            NimImpl* game;
        };
    }

    int Application::Run()
    {
        auto& game = *m_impl;

        game.DecideTurn();
        game.Player1Name = "player1";
        game.Player2Name = "player2";
        game.CPUName = "cpu";
        game.Quit = false;

        rlutil::CursorHider cursor_hider;
#ifndef NIM_USE_DEFAULT_FONT_COLOR
        rlutil::setColor(rlutil::WHITE);
#endif

        detail::WordWrapSetUp();

        cout << "  Welcome to the interactive NIM. Type 'how2play' for instructions and rules.\n"
                "  Type 'help' for detailed help.\n";

        do
        {
            detail::NimConsole console(m_impl);
            game.Console = &console;
            cout << "  Would you like to play against a CPU or a human? {cpu|human}" << "\n";
            do
            {
                cout << "> ";
                string in;
                getline(cin, in);
                stringstream ss(in);
                if (ss >> in)
                {
                    detail::lowercase(in);
                    if (in == "exit" || in == "rq")
                    {
                        game.Quit = true;
                        console.quit();
                        return 0;
                    }
                    string dummy;
                    if (ss >> dummy)
                    {
                        cout << detail::print_err(ERR_ARGUMENT) << "Expected only 1 argument, one of {cpu,human}.\n";
                        continue;
                    }
                    if (in == "human")
                    {
                        game.CPU = false;
                    }
                    else if (in == "cpu")
                    {
                        game.CPU = true;
                    }
                    else
                    {
                        cout << detail::print_err(ERR_ARGUMENT) << "Expected one of {cpu,human}. Got '" << in << "'.\n";
                        continue;
                    }
                    break;
                }
            } while (true);

            cout << "----\n";

            game.StartTurn();
            console.run();

            game.Rnd();

        } while (!game.Quit);

        return 0;
    }

    Application::Application(int argc, const char* argv[]) :
        m_impl(new Application::impl())
    {
        m_impl->Cmd.reserve(argc - 1);
        for (auto i = 1; i < argc; ++i)
        {
            m_impl->Cmd.push_back(argv[i]);
        }
    }

    Application::~Application()
    {
        delete m_impl;
    }



    
    // Commands

    namespace detail
    {

        static void PrintHelpForCmd(const ConsoleCmdDesc& desc_item)
        {
            cout << "  " << desc_item.Syntax;
            auto syntax_len = (desc_item.Syntax.length() + 2) % CONSOLE_WIDTH;
            if (syntax_len >= (CONSOLE_WIDTH - DESCRIPTION_WIDTH - 1))
            {
                syntax_len = 0;
                cout << "\n";
            }
            const auto& desc_lines = desc_item.Description;
            for (const auto& line : desc_lines)
            {
                cout << setw((CONSOLE_WIDTH - 1 - syntax_len)) << line << "\n";
                syntax_len = 0;
            }
            cout << "\n";
        }

        static void CmdHelp(NimImpl* nimpl, const vector<string>& parts)
        {
            cout << right;
            auto arg_count = parts.size();
            if (arg_count == 1)
            {
                for (const auto& elem : ConsoleCmdDescs)
                {
                    PrintHelpForCmd(elem.second);
                }
            }
            else
            {
                if (arg_count == 2)
                {
                    auto arg = parts[1];
                    lowercase(arg);
                    if (arg == "me")
                    {
                        cout << "  You're on your own buddy.\n";
                        return;
                    }
                }
                for (auto i = unsigned(1); i < arg_count; ++i)
                {
                    auto arg = parts[i];
                    lowercase(arg);
                    auto search = ConsoleCmdDescs.find(arg);
                    if (search == ConsoleCmdDescs.end())
                    {
                        cout << print_err(ERR_SYNTAX) << "  Command '" << arg << "' not found\n";
                    }
                    else
                    {
                        PrintHelpForCmd(search->second);
                    }
                }
            }
        }

        static void CmdShow(NimImpl* nimpl, const vector<string>& parts)
        {
            auto arg_count = parts.size();
            if (arg_count == 1)
            {
                nimpl->ShowPiles();
                return;
            }
            ostringstream output_stream;
            for (auto i = unsigned(1); i < arg_count; ++i)
            {
                const auto& arg = parts[i];
                int32 val;
                using namespace numerics;
                if (!parse_integral<int32>(arg.c_str(), &val))
                {
                    cout << print_err(ERR_ARGUMENT) << "Could not parse '" << arg << "' as an integer.\n";
                    return;
                }
                if (val < 1 || val > 3)
                {
                    cout << print_err(ERR_RANGE) << "Expected <pile> in range [1, 3], got '" << val << "'.\n";
                    return;
                }
                output_stream << nimpl->Piles[val - 1] << "  ";
            }
            auto output = output_stream.str();
            cout << "  " << output.substr(0, output.length() - 2) << "\n";
        }

        static void CmdTake(NimImpl* nimpl, const vector<string>& parts)
        {
            auto arg_count = parts.size();
            if (arg_count == 1)
            {
                cout << print_err(ERR_ARGUMENT) << "Arguments <number> AND <pile> not found. Type 'help take' for usage details.\n";
                return;
            }
            else if (arg_count == 2)
            {
                cout << print_err(ERR_ARGUMENT) << "Argument <pile> not found. Type 'help take' for usage details.\n";
                return;
            }
            auto parts2 = parts[2];
            lowercase(parts2);
            auto has_from = (parts2 == "from");
            if (arg_count >= uint32(4 + (has_from ? 1 : 0)))
            {
                cout << print_err(ERR_ARGUMENT) << "Too many arguments. Type 'help take' for usage details.\n";
                return;
            }
            if (has_from && (arg_count == 3))
            {
                cout << print_err(ERR_ARGUMENT) << "Argument <pile> not found. Type 'help take' for usage details.\n";
                return;
            }

            int32 number;
            int32 pile_index;
            using namespace numerics;

            if (!parse_integral<int32>(parts[1].c_str(), &number))
            {
                cout << print_err(ERR_ARGUMENT) << "Could not parse '" << parts[1] << "' as an integer.\n";
                return;
            }
            const auto& pile_string = (has_from ? parts[3] : parts[2]);
            if (!parse_integral<int32>(pile_string.c_str(), &pile_index))
            {
                cout << print_err(ERR_ARGUMENT) << "Could not parse '" << pile_string << "' as an integer.\n";
                return;
            }

            if (pile_index < 1 || pile_index > 3)
            {
                cout << print_err(ERR_RANGE) << "Expected <pile> in range [1, 3], got '" << pile_index << "'.\n";
                return;
            }

            auto& pile = nimpl->Piles[pile_index - 1];

            if (pile == 0)
            {
                cout << print_err(ERR_RANGE) << "Pile " << pile_index << " is empty.\n";
                return;
            }
            else if (number < 1 || number > pile)
            {
                cout << print_err(ERR_RANGE) << "Expected <number> in range [1, pile length (" << int32(pile) << ")], got '" << number << "'.\n";
                return;
            }

            pile -= number;

            nimpl->NextTurn();
        }

        static void CmdName(NimImpl* nimpl, const vector<string>& parts)
        {
            auto arg_count = parts.size();
            if (arg_count == 1)
            {
                cout << print_err(ERR_ARGUMENT) << "Argument <name> not found. Type 'help name' for usage details.\n";
                return;
            }
            string name;
            for (auto i = unsigned(1); i < arg_count; ++i)
            {
                name += parts[i] + " ";
            }
            nimpl->SetCurrentPlayerName(name.substr(0, name.length() - 1));
            nimpl->UpdatePrompt();
        }

        static void CmdHow2Play(NimImpl* nimpl, const vector<string>& parts)
        {
            static const string HOW2PLAY = 
                #include "how2play.txt"
            ;
            cout << HOW2PLAY << "\n";
        }

        static void CmdRestart(NimImpl* nimpl, const vector<string>& parts)
        {
            auto arg_count = parts.size();
            if (arg_count == 1) { cout << "\n"; nimpl->Console->quit(); return; }
            if (arg_count > 2)
            {
                cout << print_err(ERR_ARGUMENT) << "Expected only 1 argument, one of {cpu,human}.\n";
                return;
            }
            auto opponent_type = parts[1];
            lowercase(opponent_type);
            if (opponent_type == "human")
            {
                nimpl->CPU = false;
            }
            else if (opponent_type == "cpu")
            {
                nimpl->CPU = true;
            }
            else
            {
                cout << detail::print_err(ERR_ARGUMENT) << "Expected one of {cpu,human}. Got '" << opponent_type << "'.\n";
                return;
            }
            cout << "----\n";
            nimpl->Restart();
        }

        static void CmdExit(NimImpl* nimpl, const vector<string>& parts)
        {
            nimpl->Console->quit();
            nimpl->Quit = true;
        }

        static void CmdRq(NimImpl* nimpl, const vector<string>& parts)
        {
            CmdExit(nimpl, parts);
        }

        static void CmdColor(NimImpl* nimpl, const vector<string>& parts)
        {
            auto arg_count = parts.size();
            if (arg_count == 1)
            {
                cout << print_err(ERR_ARGUMENT) << "Argument <color> not found. Type 'help color' for usage details.\n";
                return;
            }
            else if (arg_count > 2)
            {
                cout << print_err(ERR_ARGUMENT) << "Too many arguments. Type 'help take' for usage details.\n";
                return;
            }
            auto color_name = parts[1];
            lowercase(color_name);
            auto search = ColorsMap.find(color_name);
            if (search == ColorsMap.end())
            {
                cout << print_err(ERR_ARGUMENT) << "Could not find color named '" << color_name << "'. Type 'help color' for usage details.\n";
                return;
            }
            rlutil::setColor(search->second);
        }




        // Utils implementation

        template <typename T>
        static vector<T>& split(const T& s, vector<T>& elems)
        {
            basic_stringstream<typename T::value_type> ss(s);
            T item;
            while (ss >> item) {
                elems.push_back(item);
            }
            return elems;
        }

        template <typename T>
        static vector<T> split(const T& s)
        {
            vector<T> elems;
            split<T>(s, elems);
            return move(elems);
        }

        static void lowercase(string& s)
        {
            transform(s.begin(), s.end(), s.begin(), ::tolower);
        }

        static vector<string> word_wrap_fill(const string& text, size_t line_length)
        {
            stringstream words(text);
            vector<string> lines;
            string word;
            ostringstream line;

            if (words >> word)
            {
                line << word;
                auto space_left = line_length - word.length();
                while (words >> word)
                {
                    if (space_left < word.length() + 1)
                    {
                        auto str = line.str();
                        auto len = str.length();
                        if (len < line_length)
                        {
                            str.insert(len, line_length - len, ' ');
                        }
                        lines.push_back(str);
                        line.str("");
                        line << word;
                        space_left = line_length - word.length();
                    }
                    else
                    {
                        line << " " << word;
                        space_left -= word.length() + 1;
                    }
                }
                auto str = line.str();
                auto len = str.length();
                if (len < line_length)
                {
                    str.insert(len, line_length - len, ' ');
                }
                lines.push_back(str);
            }
            return move(lines);
        }

        static string print_err(const string& err_type)
        {
            return "> " + err_type + ": ";
        }

        static void WordWrapSetUp()
        {
            for (auto& cmd_desc : ConsoleCmdDescs)
            {
                auto& desc = cmd_desc.second;
                desc.Description = move(word_wrap_fill(desc.Description[0], DESCRIPTION_WIDTH));
            }
        }




        // Pile implementation

        Pile& Pile::operator =(int32 new_count)
        {
            NIM_ASSERT(new_count >= 0);
            count = new_count;
            return *this;
        }

        Pile& Pile::operator =(const Pile& other)
        {
            count = other.count;
            return *this;
        }

        Pile& Pile::operator --()
        {
            --count;
            NIM_ASSERT(count >= 0);
            return *this;
        }

        Pile& Pile::operator ++()
        {
            ++count;
            NIM_ASSERT(count <= PILE_MAX);
            return *this;
        }

        Pile Pile::operator --(int32)
        {
            Pile p{ count };
            --count;
            NIM_ASSERT(count >= 0);
            return move(p);
        }

        Pile Pile::operator ++(int32)
        {
            Pile p{ count };
            ++count;
            NIM_ASSERT(count <= PILE_MAX);
            return move(p);
        }

        Pile& Pile::operator -=(int32 diff)
        {
            count -= diff;
            NIM_ASSERT(count >= 0 && count <= PILE_MAX);
            return *this;
        }

        Pile& Pile::operator -=(const Pile& other)
        {
            return *this -= other.count;
        }

        Pile& Pile::operator +=(int32 diff)
        {
            count += diff;
            NIM_ASSERT(count >= 0 && count <= PILE_MAX);
            return *this;
        }

        Pile& Pile::operator +=(const Pile& other)
        {
            return *this += other.count;
        }

        bool Pile::operator ==(int32 other_count) const
        {
            return count == other_count;
        }

        bool Pile::operator ==(const Pile& other) const
        {
            return count == other.count;
        }

        int32 Pile::Count() const
        {
            return count;
        }

        Pile::operator int32() const
        {
            return count;
        }

        void Pile::Rnd()
        {
            count = PILE_MIN + static_cast<int>((::rand() % (PILE_MAX - PILE_MIN)));
        }

        Pile::Pile()
        {
            Rnd();
        }

        Pile::Pile(int32 new_count)
        {
            NIM_ASSERT(new_count >= 0 && new_count <= PILE_MAX);
            count = new_count;
        }

    }

}