#!/bin/env ruby

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

require 'json'
require 'stringio'
require 'uri'
require 'optparse'

$options = options = {}
$errorCode = 0
OptionParser.new do |opts|
    opts.on '-o=FILE', '--output=FILE', 'Output file (or directory for --separate)'
    opts.on '-s', '--separate', 'Dump each file to a single package'
    opts.on '--prologue', 'Emit common prologue in every file'
    opts.on '--min-visibility=VISIBILITY', 'minimum visibility to emit: private<internal<protected<public'
    opts.on '--error-on-comment-absence', 'emit error if error is absent'
    opts.on '--error-behaviour=VALUE', 'what to do with errors: either `omit` or `fail`'
    opts.on '--tags-filter=VALUE', 'expression to test tags, that are passed as an array named tags, example: "tags.include?(\'common\') || tags.empty?"'
    opts.on '--help' do
        puts opts
    end
end.parse!(into: options)

options[:"min-visibility"] ||= "private"

if options[:separate]
    if options[:output] == nil
        raise "separate can't be used without output"
    end
end

packages = {}

$knownErrs = []

ARGV.each { |f|
    if f == '-'
        contents = STDIN.read
    else
        contents = File.read f
    end
    obj = JSON.parse contents
    obj["packages"].each { |name, v|
        raise "duplicate package #{name} in #{f} and #{packages[name]["__file"]}" if packages.has_key? name
        v["__file"] = f
        packages[name] = v
        if obj["errors"]
            $knownErrs.concat(obj["errors"])
        end
    }
}

class Array
    def intersperse(elem)
        self.flat_map { |e| [e, elem] }[0...-1]
    end
    def intercalate(with)
        self.flat_map { |e| e + with }[0...-with.size]
    end
end

class PrimitiveDumper
    def initialize()
        @titleDepth = 0
        @trace = []
        @logs = []
        @errs = []
        @lines = []
    end

    def curLine()
        return @lines[-1]
    end

    def makeLine(kind)
        lin = [kind]
        @lines << lin
        return lin
    end

    def mRaise(ex)
        raise "#{ex}\ntrace: #{@trace}\nlog: #{@logs}"
    end

    private def renderLineTextArray(arr, buf: "")
        arr.each { |a|
            assertSize = Proc.new { |s|
                mRaise("wrong #{a} ; expected size #{s}") if a.size != s
            }
            if a.kind_of? String
                buf << escape(a)
            elsif a.kind_of? Array
                case a[0]
                when :mono
                    assertSize.(2)
                    buf << "``#{a[1].rstrip}`` "
                when :nest
                    assertSize.(2)
                    renderLineTextArray a[1], buf: buf
                when :bold
                    buf << "**"
                    renderLineTextArray(a[1..-1], buf: buf)
                    buf << "**"
                when :link
                    assertSize.(3)
                    buf << "`"
                    renderLineTextArray(a[1], buf: buf)
                    buf << " <#{a[2]}>`_ "
                when :url
                    assertSize.(2)
                    buf << "`#{a[1]} <#{a[1]}>`_"
                when :ref
                    assertSize.(3)
                    buf << ":ref:`"
                    renderLineTextArray(a[1], buf: buf)
                    buf << "<#{a[2]}>`"
                when :space
                    assertSize.(1)
                    buf << "|nbsp| "
                when :kw
                    assertSize.(2)
                    buf << ":kw:`" << escape(a[1]) << "`"
                else
                    mRaise "wrong line #{a} in #{arr}"
                end
            else
                mRaise "unknown #{a} in #{arr}"
            end
        }
        mRaise("wrong array #{arr}") if /\n/ =~ buf
        buf
    end

    def str()
        buf = ""
        prev = OpenStruct.new
        @lines.each_with_index { |l, i|
            prevDifferent = prev.kind != l[0]
            if prevDifferent
                buf << "\n"
            end
            case l[0]
            when :title
                sym = case l[1]
                    when 0..1
                        "*"
                    when 2
                        "="
                    when 3
                        "-"
                    when 4
                        "~"
                    else 5
                        "^"
                end
                dat = renderLineTextArray(l[2..-1])
                if dat == ""
                    dat = renderLineTextArray([keywordaize("anonymous")])
                end
                if l[1] == 0
                    buf << (sym * dat.size) << "\n"
                end
                buf << dat << "\n"
                buf << (sym * dat.size) << "\n\n"
            when :internal
                buf << l[1]
            when :line
                buf << "| " << renderLineTextArray(l[1..-1])
            when :sig
                if prevDifferent
                    buf << ".. container:: doccodeblock\n\n"
                end
                buf << "  " << renderLineTextArray(l[1..-1])
            when :ol
                buf << " - " << renderLineTextArray(l[1..-1])
            when :ul
                buf << " - " << renderLineTextArray(l[1..-1])
            when :anchor
                mRaise("wrong #{l}") if l.size != 2
                buf << ".. _#{l[1]}:"
            when :code
                mRaise("wrong code #{l}") if l.size != 2
                if prevDifferent
                    buf << ".. code-block::\n\n"
                end
                buf << "  " << l[1]
            when :hline
                mRaise("wrong #{l}") if l.size != 1
                if i + 1 != @lines.size
                    buf << "------\n"
                end
            else
                mRaise "wrong kind #{l[0]}"
            end
            buf << "\n"
            prev.kind = l[0]
        }
        buf
    end

    def title(name, allowEmpty: true)
        makeLine(:title) << @titleDepth
        if name.kind_of? Array
            curLine.concat(name)
        else
            curLine << name
        end
        old = curLine
        begin
            @titleDepth += 1
            trace("#{name}") {
                yield
            }
        ensure
            @titleDepth -= 1
        end
        if not allowEmpty and curLine == old
            @lines.pop
        end
    end

    private def escape(s)
        s.gsub(/[\*`":\\<>\[\]_]/) { |c| "\\#{c}" }
    end

    def keywordaize(x)
        [:kw, x]
    end
end

class Dumper < PrimitiveDumper
    def skip(o)
        tst = Proc.new { |x|
            case x
            when 'public'
                5
            when 'protected'
                4
            when nil, 'internal'
                3
            when 'private'
                2
            else
                mRaise "unknown visibility #{x} #{o}"
            end
        }
        myVis = [o["exported"] ? "public" : "internal", o["visibility"]].map { |x| tst.(x) }.max
        return true if myVis < tst.($options[:"min-visibility"])
        tagsFilter = $options[:"tags-filter"]
        if tagsFilter
            tags = (o["comment"] || {"tags": nil})["tags"] || []
            return !eval(tagsFilter, binding)
        end
        return false
    end
    def dumpVisibility(o)
        if o["exported"]
            curLine << keywordaize("export") << " "
        end
        if o["visibility"]
            curLine << keywordaize(o["visibility"]) << " "
        end
    end

    def dumpCommentBlock(blk, title: nil, inline: false)
        if blk.size == 0
            return
        end
        newLine = method(:makeLine)
        if inline
            bad = [false]
            newLine = Proc.new { |*args|
                if args[0] == :line and not bad[0]
                    curLine << " "
                    next curLine
                end
                bad[0] = true
                next makeLine(*args)
            }
        end
        newLine.(:line)
        if title
            curLine.concat(title)
        end
        blk.each { |b|
            if b.kind_of? String
                curLines = b.split("\n").map{ |i| i.gsub(/\s+/, ' ') }
                if not curLines.empty?
                    curLine << curLines[0]
                    curLines[1..-1].each { |l|
                        newLine.(:line) << l
                    }
                end
            elsif b["kind"] == "link"
                if not b["text"]
                    if b["link"] =~ URI::DEFAULT_PARSER.regexp[:ABS_URI]
                        curLine << [:url, b["link"]]
                    else
                        curLine << [:mono, b["link"]]
                    end
                else
                    curLine << [:link, [b["text"]], b["link"]]
                end
            elsif b["kind"] == "block-code"
                b["data"].split(/\n/).each { |l|
                    newLine.(:code) << l
                }
            else
                error("unknown #{b}")
                curLine << b.to_s
            end
        }
        if not inline
            makeLine :line
        end
    end

    def dumpComment(comment, mode, dat: nil)
        if comment == nil
            if $options[:"error-on-comment-absence"]
                error("comment is absent")
            end
            return
        end
        if comment["brief"]
            dumpCommentBlock comment["brief"]
        end
        if comment["deprecated"]
            dumpCommentBlock comment["deprecated"], title: [[:bold, 'DEPRECATED:'], " "]
        end
        if comment["description"]
            dumpCommentBlock comment["description"], title: [[:bold, 'Description:'], " "]
        end
        if comment["targs"]
            makeLine(:line) << [:bold, "Type Arguments:"] << " "
            comment["targs"].each { |a|
                makeLine(:ul) << a["name"] << " --- "
                dumpCommentBlock a["data"], inline: true
            }
        end
        if mode == :method
            if comment["returns"]
                dumpCommentBlock comment["returns"], title: [[:bold, 'Returns:'], ' ']
            end
            if comment["params"] and comment["params"].any? { |a| a["data"] }
                args = Hash[dat["obj"]["args"].map { |a| [a["name"], {"type" => a["type"]}] }]
                comment["params"].each { |a|
                    if not args.has_key?(a["name"])
                        error "doesn't have param #{a["name"]}"
                        next
                    end
                    args[a["name"]]["comment"] = a["data"]
                }
                makeLine(:line) << [:bold, 'Arguments:']
                dat["obj"]["args"].each { |a|
                    makeLine(:ol) << a["name"] << ": " << [:nest, reprType(a["type"])]
                    if args[a["name"]]["comment"]
                        curLine << " --- "
                        dumpCommentBlock args[a["name"]]["comment"], inline: true
                    end
                }
            end
            if dat and dat["obj"]["throws"] or comment["throws"] and comment["throws"].size > 0
                makeLine(:line) << [:bold, "Throws:"]
            end
            if comment["throws"]
                comment["throws"].each { |t|
                    makeLine(:ul) << [:nest, reprType(t["type"])] << " "
                    dumpCommentBlock t["data"], inline: true
                }
            end
        end
        if comment["other"]
            comment["other"].each { |o|
                mRaise("wrong other #{o["kind"]}") unless o["kind"] == "other"
                tit = o["title"]
                tit[0] = tit[0].upcase
                dumpCommentBlock o["data"], title: [[:bold, tit + ":"], " "]
            }
        end
    end

    def dumpMethod(m, ignoreStatic: false)
        return if skip m
        trace(m["name"], log: m) {
            makeLine :sig
            dumpVisibility(m)
            if m["final"]
                curLine << keywordaize("final") << " "
            end
            if m["override"]
                curLine << keywordaize("override") << " "
            end
            if m["static"] && !ignoreStatic
                curLine << keywordaize("static") << " "
            end
            if m["property"]
                curLine << keywordaize(m["property"]) << " "
            end

            makeLine(:sig) << m["name"] << [:nest, reprTArgs(m, isDecl: true)] << "("
            needsNewLine = false
            if m["args"].size > 2 or m["args"].any? { |a| a["type"]["kind"] == "fn" }
                needsNewLine = true
                makeLine :sig
            end
            m["args"].each_with_index{ |a, i|
                if needsNewLine
                    curLine << [:space]
                end
                if a["rest"]
                    curLine << "..."
                end
                curLine << a["name"] << ": " << [:nest, reprType(a["type"])]
                if i + 1 != m["args"].size
                    if needsNewLine
                        curLine << ","
                        makeLine :sig
                    else
                        curLine << ", "
                    end
                end
            }
            if needsNewLine
                makeLine :sig
            end
            curLine << "): " << [:nest, reprType(m["ret"])]
            if m["throws"]
                curLine << " " << keywordaize("throws")
            end

            dumpComment m["comment"], :method, dat: { "obj" => m }

            makeLine :hline
        }
    end

    def dumpProp(p, ignoreStatic: false)
        return if skip p
        trace(p["name"]) {
            makeLine :sig
            dumpVisibility p
            if p["static"] && !ignoreStatic
                curLine << keywordaize("static") << " "
            end
            if p["readonly"]
                curLine << keywordaize("readonly") << " "
            end
            if p["const"]
                curLine << keywordaize("const") << " "
            end
            makeLine(:sig) << p["name"] << ": " << [:nest, reprType(p["type"])]

            dumpComment p["comment"], :prop
        }

        makeLine(:hline)
    end

    def normalize(d, n, ks: ["static", "name", Proc.new { |x| x["args"] ? x["args"].size : 0 }])
        d[n] ||= []
        d[n].sort_by!{ |a|
            ks.map { |k|
                if k.respond_to?(:call)
                    next k.(a)
                end
                vl = a[k]
                if vl === true
                    1
                elsif vl === false
                    0
                else
                    vl
                end
            }
        }
    end

    def reprTArgs(t, isDecl: false)
        dumper = method(:reprType)
        if isDecl
            dumper = Proc.new { |x| [x]}
        end
        res = []
        return res unless t["targs"]
        res << "<" << [:nest, t["targs"].map { |a| dumper.(a) }.intercalate([", "])] << ">"
        res
    end

    def reprType(t)
        if t.kind_of? String
            return [keywordaize(t)]
        end
        case t["kind"]
        when "var"
            [t["name"]]
        when "union"
            t["alts"].map { |x| reprType(x) }.intercalate([' | '])
        when "arr"
            reprType(t["elem"]) + ["[]"]
        when "named"
            p = t["pack"] || ""
            if p == @curPack || p == ""
                p = ""
            else
                p += "."
            end
            p += t["name"]
            [[:ref, [p], namedTypeLabelStr("#{t["pack"]}.#{t["name"]}")]] + reprTArgs(t)
        when "fn"
            reprTArgs(t) + ["("] + t["args"].map { |a| [a["name"], ": "] + reprType(a["type"]) }.intercalate([", "]) + [") => "] + reprType(t["ret"])
        else
            mRaise "unknown kind #{t["kind"]} in #{t}"
        end
    rescue
        STDERR.puts(JSON.dump({ trace: @trace, type: t, log: @logs }))
        raise
    end

    def namedTypeLabelStr(name)
        name = "#{name.gsub('/', '.')}"
        name = name + '.' + name.chars.map { |c| c == c.upcase ? 'u' : 'l' }.join
        name
    end

    def dumpClassAnchor(pname, cname)
        makeLine(:anchor) << "#{namedTypeLabelStr "#{pname}.#{cname}"}";
    end

    def prelude
        return if not $options[:prologue]
        makeLine(:internal) << ".. role:: kw"
        makeLine(:internal) << "  :class: dockeyword"
        makeLine(:internal) << "\n"
        makeLine(:internal) << ".. |nbsp| unicode:: U+00A0 .. NO-BREAK SPACE\n\n"
    end

    def fillInClass(cname, cls)
        dumpComment cls["comment"], :class

        normalize cls, "props"
        normalize cls, "methods", ks: ["static", "name"]
        title("Properties", allowEmpty: false) {
            cls["props"].each { |p|
                dumpProp p
            }
        }
        title("Methods", allowEmpty: false) {
            (cls["methods"] || []).each { |m|
                dumpMethod m
            }
        }
    end

    def trace(s, log: nil)
        @trace.push s
        if log != nil
            @logs.push log
        end
        yield
    ensure
        if log != nil
            @logs.pop
        end
        @trace.pop
    end

    def error(e)
        @errs.push({ "trace" => @trace.dup, "text" => e })
    end

    def classTitleRepr(name, targs)
        res = [name]
        if targs and targs.size > 0
            res << "<" << [:nest, targs.intersperse(", ")] << ">"
        end
        res
    end

    def fillInPackage(pname, pack)
        normalize pack, "props"
        normalize pack, "methods"
        title("Variables", allowEmpty: false) {
            pack["props"].each { |p|
                dumpProp p, ignoreStatic: true
            }
        }
        title("Enums", allowEmpty: false) {
            (pack["enums"] || []).each { |cname, cls|
                next if skip cls
                dumpClassAnchor(pname, cname)
                title(classTitleRepr(cname, cls["targs"])) {
                    makeLine(:sig)
                    dumpVisibility(cls)
                    dumpComment cls["comment"], :enum
                    cls["members"].each { |mem|
                        val = mem["value"]
                        if val.kind_of?(Hash)
                            case val["kind"]
                            when "num"
                                val = val["value"]
                            else
                                mRaise("unknown kind #{val["kind"]}")
                            end
                        elsif val.kind_of? String
                            val = "\"#{val}\""
                        else
                            val = "#{val}"
                        end
                        makeLine(:sig) << mem["name"] << " = " << val
                        dumpComment mem["comment"], :enumItem
                        makeLine(:hline)
                    }
                }
            }
        }
        title("Functions", allowEmpty: false) {
            pack["methods"].each { |m|
                dumpMethod m, ignoreStatic: true
            }
        }
        title("Aliases", allowEmpty: false) {
            (pack["aliases"] || []).each { |cname, cls|
                next if skip cls
                dumpClassAnchor(pname, cname)
                title(classTitleRepr(cname, cls["targs"])) {
                    makeLine(:sig) << "= " << [:nest, reprType(cls["type"])]
                }
            }
        }
        title("Interfaces", allowEmpty: false) {
            (pack["interfaces"] || []).each { |cname, cls|
                next if skip cls
                dumpClassAnchor(pname, cname)
                title(classTitleRepr(cname, cls["targs"])) {
                    fillInClass cname, cls
                }
            }
        }
        title("Classes", allowEmpty: false) {
            (pack["classes"] || []).each { |cname, cls|
                next if skip cls
                dumpClassAnchor(pname, cname)
                title(classTitleRepr(cname, cls["targs"])) {
                    makeLine(:sig)
                    dumpVisibility(cls)
                    if cls["final"]
                        curLine << keywordaize("final") << " "
                    end
                    if cls["extends"]
                        makeLine(:sig) << keywordaize("extends") << " " << [:nest, reprType(cls["extends"])]
                    end
                    if cls["implements"] and (cls["implements"].size > 0)
                        makeLine(:sig) << keywordaize("implements")
                        cls["implements"].each { |i|
                            makeLine(:sig) << [:space] << [:nest, reprType(i)] << ","
                        }
                    end
                    fillInClass cname, cls
                }
            }
        }
    end

    def run(packages)
        prelude
        packages.each { |pname, pack|
            @curPack = pname.gsub('/', '.')
            trace(pname) {
                title(pname) {
                    fillInPackage pname, pack
                }
            }
            if $options[:separate]
                save pname
            end
        }
        @errs.concat($knownErrs)
        if @errs.size > 0 && $options[:"error-behaviour"] == "fail"
            $errorCode = 1
        end
        if @errs.size > 0 && $options[:"error-behaviour"] != "omit"
            title("Errors", allowEmpty: false) {
                @errs.each { |e|
                    makeLine(:line) << e["text"]
                    if e["trace"]
                        curLine << ", trace:"
                        e["trace"].each { |t|
                            makeLine(:ul) << t
                        }
                    end
                }
            }

            save "_errors"
        end
        if not $options[:separate]
            save "<main>"
        end
    end

    def save(pack)
        if pack == ""
            pack = "anonymous"
        end
        dump = str
        @lines = []
        prelude
        if $options[:output] == nil
            puts dump
        else
            path = "#{$options[:output]}"
            if $options[:separate]
                path += "/#{pack.gsub(/\//, '.')}.rst"
            end
            puts "saved `#{pack}` -> #{path}"
            File.write path, dump
        end
    end
end

dumper = Dumper.new
dumper.run(packages)

exit $errorCode
