/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <ios>
#include <optional>
#include "es2panda.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include "util/arktsconfig.h"
#include "utils/pandargs.h"

namespace panda::es2panda::docgenapp {
using mem::MemConfig;

class MemManager {
public:
    explicit MemManager()
    {
        constexpr auto COMPILER_SIZE = 256_MB;

        MemConfig::Initialize(0, 0, COMPILER_SIZE, 0, 0, 0);
        PoolManager::Initialize(PoolType::MMAP);
    }

    NO_COPY_SEMANTIC(MemManager);
    NO_MOVE_SEMANTIC(MemManager);

    ~MemManager()
    {
        PoolManager::Finalize();
        MemConfig::Finalize();
    }
};

class Options {
public:
    Options() = default;

    bool ParseModule() const
    {
        return false;
    }

    std::optional<std::string> Parse(int argc, const char **argv)
    {
        panda::PandArg<bool> help("help", false, "print help message");

        panda::PandArg<std::string> std_lib("stdlib", "", "Path to standard library");
        panda::PandArg<bool> op_ets_module("ets-module", false, "Compile the input as ets-module");
        panda::PandArg<std::string> arkts_config("arktsconfig", DEFAULT_ARKTSCONFIG,
                                                 "Path to arkts configuration file");
        panda::PandArg<bool> op_make_doc_secure("doc-secure", true, "Omit file paths from error traces");
        panda::PandArg<std::string> op_make_doc_filter("doc-filter", ".*",
                                                       "Make documentation package name filter (regex)");
        panda::PandArg<std::string> mode("mode", "file", "compilation mode: file|project|stdlib = file");

        panda::PandArg<std::string> output_file("output", "", "output json");
        panda::PandArg<std::string> input_file("input", "", "input path");

        panda::PandArgParser arg_parser;

        arg_parser.Add(&help);
        arg_parser.Add(&std_lib);
        arg_parser.Add(&op_ets_module);
        arg_parser.Add(&arkts_config);
        arg_parser.Add(&op_make_doc_secure);
        arg_parser.Add(&op_make_doc_filter);
        arg_parser.Add(&mode);

        arg_parser.EnableTail();
        arg_parser.PushBackTail(&output_file);
        arg_parser.PushBackTail(&input_file);

        if (!arg_parser.Parse(argc, argv)) {
            return arg_parser.GetErrorString() + arg_parser.GetHelpString();
        }

        CheckHelp(help.GetValue(), arg_parser);

        compiler_options_.std_lib = std_lib.GetValue();
        compiler_options_.arkts_config = std::make_shared<ArkTsConfig>(arkts_config.GetValue());

        if (!compiler_options_.arkts_config->Parse()) {
            return "can't parse arkts config " + arkts_config.GetValue();
        }

        if (auto err = ParseCompilationMode(compiler_options_.compilation_mode, mode.GetValue()); err) {
            return err;
        }

        compiler_options_.is_ets_module = op_ets_module.GetValue();
        compiler_options_.make_doc.emplace();
        compiler_options_.make_doc->secure = op_make_doc_secure.GetValue();
        compiler_options_.make_doc->filter = op_make_doc_filter.GetValue();
        compiler_options_.make_doc->output_path = output_file.GetValue();
        compiler_output_ = output_file.GetValue();
        source_file_ = input_file.GetValue();

        return std::nullopt;
    }

    const es2panda::CompilerOptions &CompilerOptions() const
    {
        return compiler_options_;
    }

    const std::string &CompilerOutput() const
    {
        return compiler_output_;
    }

    void SetCompilerOutput(const std::string &compiler_output)
    {
        compiler_output_ = compiler_output;
    }

    const std::string &SourceFile() const
    {
        return source_file_;
    }

private:
    es2panda::CompilerOptions compiler_options_ {};
    std::string compiler_output_;
    std::string source_file_;

    static std::optional<std::string> ParseCompilationMode(CompilationMode &mode, std::string_view str_mode)
    {
        if (str_mode == "stdlib") {
            mode = CompilationMode::GEN_STD_LIB;
        } else if (str_mode == "project") {
            mode = CompilationMode::PROJECT;
        } else if (str_mode == "file") {
            mode = CompilationMode::SINGLE_FILE;
        } else {
            return "unknown mode " + std::string {str_mode};
        }
        return std::nullopt;
    }

    static void CheckHelp(bool print_help, PandArgParser &arg_parser)
    {
        if (print_help) {
            std::cout << arg_parser.GetHelpString() << std::endl;
            exit(0);
        }
    }
};

static int CompileFromSource(es2panda::Compiler &compiler, es2panda::SourceFile &input, Options *options)
{
    [[maybe_unused]] auto program = compiler.Compile(input, options->CompilerOptions());

    ASSERT(program == nullptr);

    const auto &err = compiler.GetError();

    // Intentional exit or --parse-only option usage.
    if (err.Type() == ErrorType::INVALID) {
        return 0;
    }

    std::cout << err.TypeString() << ": " << err.Message();
    std::cout << " [" << (err.File().empty() ? "<unknown>" : err.File()) << ":" << err.Line() << ":" << err.Col() << "]"
              << std::endl;

    return err.ErrorCode();
}

static int CompileFromConfig(es2panda::Compiler &compiler, Options *options)
{
    auto compilation_list = FindProjectSources(options->CompilerOptions().arkts_config);
    if (compilation_list.empty()) {
        std::cerr << "Error: No files to compile" << std::endl;
        return 1;
    }

    unsigned overall_res = 0;
    for (auto &[src, dst] : compilation_list) {
        std::ifstream input_stream(src);
        if (input_stream.fail()) {
            std::cerr << "Error: Failed to open file: " << src << std::endl;
            return 1;
        }

        std::stringstream ss;
        ss << input_stream.rdbuf();
        std::string parser_input = ss.str();
        input_stream.close();
        es2panda::SourceFile input(src, parser_input, options->ParseModule());
        options->SetCompilerOutput(dst);

        auto res = CompileFromSource(compiler, input, options);
        if (res != 0) {
            std::cout << "> ets_docgen: failed to compile from " << src << " to " << dst << std::endl;
            overall_res |= static_cast<unsigned>(res);
        }
    }

    return overall_res;
}

static int Run(int argc, const char **argv)
{
    auto options = std::make_unique<Options>();

    if (auto err = options->Parse(argc, argv); err) {
        std::cerr << err.value() << std::endl;
        return 1;
    }

    Logger::ComponentMask mask {};
    mask.set(Logger::Component::ES2PANDA);
    Logger::InitializeStdLogging(Logger::Level::WARNING, mask);

    es2panda::Compiler compiler {ScriptExtension::ETS};

    if (options->CompilerOptions().compilation_mode == CompilationMode::PROJECT) {
        return CompileFromConfig(compiler, options.get());
    }

    std::string_view source_file;
    std::string parser_input;
    if (options->CompilerOptions().compilation_mode == CompilationMode::GEN_STD_LIB) {
        source_file = "etsstdlib.ets";
        parser_input = "";
    } else {
        source_file = options->SourceFile();
        std::ifstream fle {std::string {source_file}};
        if (!fle.is_open() || fle.bad()) {
            std::cerr << "can't read " << source_file << std::endl;
            return 1;
        }
        parser_input.assign(std::istreambuf_iterator<char>(fle), std::istreambuf_iterator<char>());
    }
    es2panda::SourceFile input(source_file, parser_input, options->ParseModule());
    return CompileFromSource(compiler, input, options.get());
}

}  // namespace panda::es2panda::docgenapp

int main(int argc, const char **argv)
{
    panda::es2panda::docgenapp::MemManager mm;
    return panda::es2panda::docgenapp::Run(argc, argv);
}
