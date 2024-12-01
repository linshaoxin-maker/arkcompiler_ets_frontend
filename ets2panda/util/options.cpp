/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "options.h"

#include "os/filesystem.h"
#include "utils/pandargs.h"

#include "arktsconfig.h"

#include <random>
#include <utility>

#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
#include "bytecode_optimizer/bytecodeopt_options.h"
#include "compiler/compiler_options.h"
#endif

namespace ark::es2panda::util {

static std::string Usage(const ark::PandArgParser &argparser)
{
    std::stringstream ss;

    ss << argparser.GetErrorString() << std::endl;
    ss << "Usage: es2panda [OPTIONS] [input file]" << std::endl;
    ss << std::endl;
    ss << "optional arguments:" << std::endl;
    ss << argparser.GetHelpString() << std::endl;
    ss << std::endl;

    return ss.str();
}

static std::string GetVersion()
{
    std::stringstream ss;

    ss << std::endl;
    ss << "  Es2panda Version " << ES2PANDA_VERSION << std::endl;

// add check for PANDA_PRODUCT_BUILD after normal version tracking will be implemented
#ifdef ES2PANDA_DATE
    ss << std::endl;
    ss << "  Build date: ";
    ss << ES2PANDA_DATE;
#endif  // ES2PANDA_DATE
#ifdef ES2PANDA_HASH
    ss << std::endl;
    ss << "  Last commit hash: ";
    ss << ES2PANDA_HASH;
    ss << std::endl;
#endif  // ES2PANDA_HASH

    return ss.str();
}

template <typename T>
bool Options::CallPandArgParser(const std::vector<std::string> &args, T &options)
{
    ark::PandArgParser parser;
    options.AddOptions(&parser);

    if (!parser.Parse(args)) {
        std::cerr << parser.GetErrorString();
        std::cerr << parser.GetHelpString();
        return false;
    }

    if (auto optionsErr = options.Validate(); optionsErr) {
        std::cerr << "Error: " << optionsErr.value().GetMessage() << std::endl;
        return false;
    }

    return true;
}

bool Options::CallPandArgParser(const std::vector<std::string> &args)
{
    ark::PandArgParser parser;
    AddOptions(&parser);
    parser.PushBackTail(&inputFile_);
    parser.EnableTail();
    parser.EnableRemainder();
    if (!parser.Parse(args) || IsHelp()) {
        errorMsg_ = Usage(parser);
        return false;
    }

    if (auto optionsErr = Validate(); optionsErr) {
        std::cerr << "Error: " << optionsErr.value().GetMessage() << std::endl;
        return false;
    }

    return true;
}

static std::tuple<std::string_view, std::string_view, std::string_view> SplitPath(std::string_view path)
{
    std::string_view fileDirectory;
    std::string_view fileBaseName = path;
    auto lastDelimPos = fileBaseName.find_last_of(ark::os::file::File::GetPathDelim());
    if (lastDelimPos != std::string_view::npos) {
        ++lastDelimPos;
        fileDirectory = fileBaseName.substr(0, lastDelimPos);
        fileBaseName = fileBaseName.substr(lastDelimPos);
    }

    // Save all extensions.
    std::string_view fileExtensions;
    auto fileBaseNamePos = fileBaseName.find_first_of('.');
    if (fileBaseNamePos > 0 && fileBaseNamePos != std::string_view::npos) {
        fileExtensions = fileBaseName.substr(fileBaseNamePos);
        fileBaseName = fileBaseName.substr(0, fileBaseNamePos);
    }

    return {fileDirectory, fileBaseName, fileExtensions};
}

/**
 * @brief Generate evaluated expression wrapping code.
 * @param sourceFilePath used for generating a unique package name.
 * @param input expression source code file stream.
 * @param output stream for generating expression wrapper.
 */
static void GenerateEvaluationWrapper(std::string_view sourceFilePath, std::ifstream &input, std::stringstream &output)
{
    static constexpr std::string_view EVAL_PREFIX = "eval_";
    static constexpr std::string_view EVAL_SUFFIX = "_eval";

    auto splittedPath = SplitPath(sourceFilePath);
    auto fileBaseName = std::get<1>(splittedPath);

    std::random_device rd;
    std::stringstream ss;
    ss << EVAL_PREFIX << fileBaseName << '_' << rd() << EVAL_SUFFIX;
    auto methodName = ss.str();

    output << "package " << methodName << "; class " << methodName << " { private static " << methodName << "() { "
           << input.rdbuf() << " } }";
}

bool Options::ParseInputOutput()
{
    auto isDebuggerEvalMode = IsDebuggerEval();
    if (isDebuggerEvalMode && compilationMode_ != CompilationMode::SINGLE_FILE) {
        errorMsg_ = "Error: When compiling with --debugger-eval-mode single input file must be provided";
        return false;
    }

    if (compilationMode_ == CompilationMode::SINGLE_FILE) {
        std::ifstream inputStream(SourceFileName());
        if (inputStream.fail()) {
            errorMsg_ = "Failed to open file: ";
            errorMsg_.append(SourceFileName());
            return false;
        }

        std::stringstream ss;
        if (isDebuggerEvalMode) {
            GenerateEvaluationWrapper(SourceFileName(), inputStream, ss);
        } else {
            ss << inputStream.rdbuf();
        }
        parserInputContents_ = ss.str();
    }

    if (WasSetOutput()) {
        if (compilationMode_ == CompilationMode::PROJECT) {
            errorMsg_ = "Error: When compiling in project mode --output key is not needed";
            return false;
        }
    } else {
        SetOutput(ark::os::RemoveExtension(BaseName(SourceFileName())).append(".abc"));
    }

    return true;
}

bool Options::Parse(Span<const char *const> args)
{
    std::vector<std::string> es2pandaArgs;
    auto argc = args.size();
    for (size_t i = 1; i < argc; i++) {
        es2pandaArgs.emplace_back(args[i]);
    }

    if (!CallPandArgParser(es2pandaArgs)) {
        return false;
    }

    if (IsVersion()) {
        errorMsg_ = GetVersion();
        return false;
    }
#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
    if ((WasSetBcoCompiler() && !CallPandArgParser(GetBcoCompiler(), ark::compiler::g_options)) ||
        (WasSetBcoOptimizer() && !CallPandArgParser(GetBcoOptimizer(), ark::bytecodeopt::g_options))) {
        return false;
    }
#endif

    DetermineCompilationMode();
    if (!ParseInputOutput()) {
        return false;
    }
    if (!DetermineExtension()) {
        return false;
    }
    if (extension_ != ScriptExtension::JS && IsModule()) {
        errorMsg_ = "Error: --module is not supported for this extension.";
        return false;
    }

    if ((WasSetDumpEtsSrcBeforePhases() || WasSetDumpEtsSrcAfterPhases()) && extension_ != ScriptExtension::STS) {
        errorMsg_ = "--dump-ets-src-* option is valid only with ETS extension";
        return false;
    }

    if (WasSetLogLevel()) {
        logLevel_ = Logger::LevelFromString(GetLogLevel());
    }

    InitCompilerOptions();

    return ProcessEtsSpecificOptions();
}

void Options::InitCompilerOptions()
{
    auto vecToSet = [](const std::vector<std::string> &v) { return std::set<std::string>(v.begin(), v.end()); };
    skipPhases_ = vecToSet(gen::Options::GetSkipPhases());
    verifierWarnings_ = vecToSet(gen::Options::GetVerifierInvariantsAsWarnings());
    verifierErrors_ = vecToSet(gen::Options::GetVerifierInvariantsAsErrors());
    dumpBeforePhases_ = vecToSet(gen::Options::GetDumpBeforePhases());
    dumpEtsSrcBeforePhases_ = vecToSet(gen::Options::GetDumpEtsSrcBeforePhases());
    dumpAfterPhases_ = vecToSet(gen::Options::GetDumpAfterPhases());
    dumpEtsSrcAfterPhases_ = vecToSet(gen::Options::GetDumpEtsSrcAfterPhases());

    if (IsEtsWarnings()) {
        InitializeWarnings();
    }
}

void Options::InitializeWarnings()
{
    std::array<bool, ETSWarnings::COUNT> warningSet {};
    ASSERT(ETSWarnings::LAST < ETSWarnings::COUNT);

    const auto processWarningList = [&warningSet](const auto &list, bool v) {
        static const std::map<std::string_view, std::pair<size_t, size_t>> WARNING_GROUPS {
            {"subset_aware", {ETSWarnings::SUBSET_AWARE_FIRST, ETSWarnings::SUBSET_AWARE_LAST}},
            {"subset_unaware", {ETSWarnings::SUBSET_UNAWARE_FIRST, ETSWarnings::SUBSET_UNAWARE_LAST}}};
        const auto setWarningRange = [&warningSet, v](size_t first, size_t last) {
            for (size_t i = first; i <= last; i++) {
                warningSet[i] = v;
            }
        };
        for (const auto &warningOrGroup : list) {
            if (WARNING_GROUPS.find(warningOrGroup) != WARNING_GROUPS.end()) {
                auto [first, last] = WARNING_GROUPS.at(warningOrGroup);
                setWarningRange(first, last);
                continue;
            }
            ASSERT(ets_warnings::FromString(warningOrGroup) != ETSWarnings::INVALID);
            warningSet[ets_warnings::FromString(warningOrGroup)] = v;
        }
    };
    processWarningList(GetEtsWarningsEnable(), true);
    processWarningList(GetEtsWarningsDisable(), false);
    for (size_t i = ETSWarnings::FIRST; i <= ETSWarnings::LAST; i++) {
        if (warningSet[i]) {
            etsWarningCollection_.emplace_back(static_cast<ETSWarnings>(i));
        }
    }
}

bool Options::DetermineExtension()
{
    if (compilationMode_ == CompilationMode::PROJECT) {
        if (WasSetExtension() && gen::Options::GetExtension() != "sts") {
            errorMsg_ = "Error: only '--extension=sts' is supported for project compilation mode.";
            return false;
        }
        extension_ = ScriptExtension::STS;
        return true;
    }
    std::string sourceFileExtension = SourceFileName().substr(SourceFileName().find_last_of('.') + 1);
    if (!SourceFileName().empty() && WasSetExtension() && gen::Options::GetExtension() != sourceFileExtension) {
        std::cerr << "Warning: Not matching extensions! Sourcefile: " << sourceFileExtension
                  << ", Manual(used): " << gen::Options::GetExtension() << std::endl;
    }
    auto tempExtension = WasSetExtension() ? gen::Options::GetExtension() : sourceFileExtension;
    if (gen::extension::FromString(tempExtension) == ScriptExtension::INVALID) {
        errorMsg_ =
            "Unknown extension of sourcefile, set the '--extension' option or change the file extension "
            "(available options: js, ts, as, sts)";
        return false;
    }

    extension_ = gen::extension::FromString(tempExtension);
    switch (extension_) {
#ifndef PANDA_WITH_ECMASCRIPT
        case ScriptExtension::JS: {
            errorMsg_ = "js extension is not supported within current build";
            return false;
        }
#endif
        case ScriptExtension::STS: {
            std::ifstream inputStream(GetArktsconfig());
            if (inputStream.fail()) {
                errorMsg_ = "Failed to open arktsconfig: ";
                errorMsg_.append(GetArktsconfig());
                return false;
            }
            return true;
        }
        default:
            return true;
    }
}

bool Options::ProcessEtsSpecificOptions()
{
    if (GetExtension() != ScriptExtension::STS) {
        return true;
    }

    if (auto config = ParseArktsConfig(); config != std::nullopt) {
        arktsConfig_ = std::make_shared<ArkTsConfig>(*config);
        return true;
    }

    return false;
}

std::optional<ArkTsConfig> Options::ParseArktsConfig()
{
    auto config = ArkTsConfig {GetArktsconfig()};
    if (!config.Parse()) {
        errorMsg_ = "Invalid ArkTsConfig path: ";
        errorMsg_.append(GetArktsconfig());
        return std::nullopt;
    }
    return std::make_optional(config);
}

}  // namespace ark::es2panda::util
