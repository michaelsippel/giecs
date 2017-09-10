
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <boost/program_options.hpp>
#include "linenoise/linenoise.h"

#include <giecs/bits.hpp>
#include <giecs/types.hpp>
#include <giecs/core.hpp>
#include <giecs/memory/context.hpp>

#include "language.hpp"
#include "forth.hpp"
#include "lisp.hpp"
#include "brainfuck.hpp"

using namespace giecs;
namespace po = boost::program_options;

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t>
std::shared_ptr<repl::Language> selectLanguage(std::string l, memory::Context<page_size, align_t> const& context, Core<page_size, align_t, addr_t>& core)
{
    if(l == "lisp")
        return std::make_shared<repl::lang::Lisp<page_size, align_t, addr_t>>(context, core, 0x10000);
    if(l == "asm")
        return std::make_shared<repl::lang::LispASM<page_size, align_t, addr_t>>(context, core, 0x10000);
    if(l == "forth")
        return std::make_shared<repl::lang::Forth<page_size, align_t, addr_t>>(context, core, 0x10000);
    if(l == "brainfuck")
        return std::make_shared<repl::lang::Brainfuck<page_size, align_t, addr_t>>(context, 0x10000);

    std::cout << "Unreconized language " << l << std::endl;
    return nullptr;
}

int main(int argc, char** argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "print this help")
    ("repl", "start Read-Eval-Print-Loop")
    ("language,L", po::value<std::string>(), "select language")
    ("input-file", po::value<std::vector<std::string>>(), "input files")
    ;
    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }

    // set up vm
    std::size_t const page_size = 4096;
    std::size_t const word_width = 32;

    typedef Bits<8> byte;
    typedef Bits<word_width> word;
    typedef Int<word_width> iword;
    auto const context = memory::Context<page_size, byte>();
    auto core = Core<page_size, byte, iword>();

    std::shared_ptr<repl::Language> lang = nullptr;
    std::shared_ptr<repl::Language> store = nullptr;

    if(vm.count("language"))
    {
        std::string l = vm["language"].as<std::string>();
        lang = selectLanguage<page_size, byte, iword, word>(l, context, core);
    }

    // parse file
    if(vm.count("input-file"))
    {
        std::vector<std::string> paths = vm["input-file"].as<std::vector<std::string>>();
        for(auto path : paths)
        {
            std::ifstream file(path);
            if(file.good())
                lang->parse(file);
            else
                std::cout << "error opening file " << path << std::endl;
        }
    }

    // Read-Eval-Print-Loop
    std::cout << "Starting Read-Eval-Print-Loop. Ctrl+C to exit." << std::endl;
    if(vm.count("repl") || !vm.count("input-file"))
    {
        linenoiseHistoryLoad("history.txt");
        linenoiseHistorySetMaxLen(20);
        char* line;
        char name[64];

        bool running = true;
        while(running)
        {
            while(lang == nullptr)
            {
                std::cout << "Please select language: ";
                std::string l;
                std::cin >> l;
                lang = selectLanguage<page_size, byte, iword, word>(l, context, core);
            }

            lang->name(name);
            strcat(name, " >>> ");

            if((line = linenoise(name)) != NULL)
            {
                if(lang->parse(line) != 0)
                {
                    store = lang;
                    lang = nullptr;
                }

                if(line[0] != '\0')
                {
                    linenoiseHistoryAdd(line);
                    linenoiseHistorySave("history.txt");
                }
                linenoiseFree(line);
            }

        }
    }

    return 0;
}

