/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_PARSER_INCLUDE_PROGRAM_H
#define ES2PANDA_PARSER_INCLUDE_PROGRAM_H

#include "macros.h"
#include "mem/pool_manager.h"
#include "os/filesystem.h"
#include "util/ustring.h"
#include "util/path.h"
#include "varbinder/varbinder.h"

#include "es2panda.h"

namespace ark::es2panda::ir {
class BlockStatement;
}  // namespace ark::es2panda::ir

namespace ark::es2panda::varbinder {
class VarBinder;
}  // namespace ark::es2panda::varbinder

namespace ark::es2panda::parser {
enum class ScriptKind { SCRIPT, MODULE, STDLIB };

class Program {
public:
    using ExternalSource = ArenaUnorderedMap<util::StringView, ArenaVector<Program *>>;
    template <typename T>
    static Program NewProgram(ArenaAllocator *allocator)
    {
        auto *varbinder = allocator->New<T>(allocator);
        return Program(allocator, varbinder);
    }

    Program(ArenaAllocator *allocator, varbinder::VarBinder *varbinder)
        : allocator_(allocator),
          varbinder_(varbinder),
          externalSources_(allocator_->Adapter()),
          extension_(varbinder->Extension())
    {
    }

    void SetKind(ScriptKind kind)
    {
        kind_ = kind;
    }

    NO_COPY_SEMANTIC(Program);
    DEFAULT_MOVE_SEMANTIC(Program);

    ~Program() = default;

    ArenaAllocator *Allocator() const
    {
        return allocator_;
    }

    const varbinder::VarBinder *VarBinder() const
    {
        return varbinder_;
    }

    varbinder::VarBinder *VarBinder()
    {
        return varbinder_;
    }

    ScriptExtension Extension() const
    {
        return extension_;
    }

    ScriptKind Kind() const
    {
        return kind_;
    }

    util::StringView SourceCode() const
    {
        return sourceCode_;
    }

    const util::StringView &SourceFilePath() const
    {
        return sourceFile_.GetPath();
    }

    const util::Path &SourceFile() const
    {
        return sourceFile_;
    }

    util::StringView SourceFileFolder() const
    {
        return sourceFileFolder_;
    }

    util::StringView FileName() const
    {
        return sourceFile_.GetFileName();
    }

    util::StringView AbsoluteName() const
    {
        return sourceFile_.GetAbsolutePath();
    }

    util::StringView ResolvedFilePath() const
    {
        return resolvedFilePath_;
    }

    ir::BlockStatement *Ast()
    {
        return ast_;
    }

    const ir::BlockStatement *Ast() const
    {
        return ast_;
    }

    void SetAst(ir::BlockStatement *ast)
    {
        ast_ = ast;
    }

    ir::ClassDefinition *GlobalClass()
    {
        return globalClass_;
    }

    const ir::ClassDefinition *GlobalClass() const
    {
        return globalClass_;
    }

    void SetGlobalClass(ir::ClassDefinition *globalClass)
    {
        globalClass_ = globalClass;
    }

    ExternalSource &ExternalSources()
    {
        return externalSources_;
    }

    const ExternalSource &ExternalSources() const
    {
        return externalSources_;
    }

    void SetSource(const util::StringView &sourceCode, const util::StringView &sourceFilePath,
                   const util::StringView &sourceFileFolder)
    {
        sourceCode_ = sourceCode;
        sourceFile_ = util::Path(sourceFilePath, Allocator());
        sourceFileFolder_ = sourceFileFolder;
    }

    void SetSource(const ark::es2panda::SourceFile &sourceFile)
    {
        sourceCode_ = util::UString(sourceFile.source, Allocator()).View();
        sourceFile_ = util::Path(sourceFile.filePath, Allocator());
        sourceFileFolder_ = util::UString(sourceFile.fileFolder, Allocator()).View();
        resolvedFilePath_ = util::UString(sourceFile.resolvedPath, Allocator()).View();
    }

    const util::StringView &GetPackageName() const
    {
        return packageName_;
    }

    void SetPackageName(util::StringView packageName)
    {
        packageName_ = packageName;
    }

    const bool &IsEntryPoint() const
    {
        return entryPoint_;
    }

    void MarkEntry()
    {
        entryPoint_ = true;
    }

    varbinder::ClassScope *GlobalClassScope();
    const varbinder::ClassScope *GlobalClassScope() const;

    varbinder::GlobalScope *GlobalScope();
    const varbinder::GlobalScope *GlobalScope() const;

    util::StringView PackageClassName(util::StringView className);

    std::string Dump() const;

    void DumpSilent() const;

private:
    ArenaAllocator *allocator_ {};
    varbinder::VarBinder *varbinder_ {};
    ir::BlockStatement *ast_ {};
    ir::ClassDefinition *globalClass_ {};
    util::StringView sourceCode_ {};
    util::Path sourceFile_ {};
    util::StringView sourceFileFolder_ {};
    util::StringView packageName_ {};
    util::StringView resolvedFilePath_ {};
    ExternalSource externalSources_;
    ScriptKind kind_ {};
    ScriptExtension extension_ {};
    bool entryPoint_ {};
};
}  // namespace ark::es2panda::parser

#endif
