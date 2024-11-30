/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dep_analyzer.h"

void DepAnalyzer::AddImports(ark::es2panda::parser::ETSParser *parser)
{
    ark::es2panda::util::StringView firstSourceFilePath = parser->GetGlobalProgram()->AbsoluteName();
    sourcesPath_.push_back(std::string(firstSourceFilePath));

    ark::es2panda::util::ImportPathManager *manager = parser->GetImportPathManager();
    auto &parseList = manager->ParseList();

    for (auto pl : parseList) {
        sourcesPath_.push_back(std::string(pl.sourcePath));
    }
}

void DepAnalyzer::AnalyzeDeps(int argc, const char **argv)
{
    const auto *impl = es2panda_GetImpl(ES2PANDA_LIB_VERSION);
    auto *cfg = impl->CreateConfig(argc, argv);
    if (cfg == nullptr) {
        return;
    }
    auto *cfgImpl = reinterpret_cast<ark::es2panda::public_lib::ConfigImpl *>(cfg);

    es2panda_Context *ctx = impl->CreateContextFromString(cfg, cfgImpl->options->ParserInput().c_str(),
                                                          cfgImpl->options->SourceFile().c_str());
    impl->ProceedToState(ctx, ES2PANDA_STATE__IMPORT_PARSED);

    auto *ctxImpl = reinterpret_cast<ark::es2panda::public_lib::Context *>(ctx);
    auto *parser = reinterpret_cast<ark::es2panda::parser::ETSParser *>(ctxImpl->parser);

    AddImports(parser);

    impl->DestroyContext(ctx);
    impl->DestroyConfig(cfg);
}