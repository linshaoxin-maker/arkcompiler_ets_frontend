# Ets documentation generator

## Parts

### ets_docgen
C++ file that uses es2panda library to generate documentation. It operates in few modes: file, project, stdlib; it builds documentation for corresponding es2panda target. As a result it produces `.json` file, that contains all packages that passed a filter and their entities. This file can be distributed with closed-source code.

### ets_doc2rst.rb
Ruby program that transforms one or more `.json` files from `ets_docgen` into `.rst` files. Types are linked to their declaration.

## Usage
Generate standard library documentation to stdout in the following way:
```sh
./ets_docgen --mode=stdlib "" / | ./ets_doc2rst.rb -
```

Or save to a few files:
```sh
ets_docgen --mode=stdlib "" / \
        | ets_doc2rst.rb \
            --output packages \
            --separate \
            --error-on-comment-absence \
            --error-behaviour=omit \
            --min-visibility=protected \
            -
```
For more options, see `--help` of these executables. They support working with different file modes and provide few filtering options, including filtering by package name, visibility, and tags.
