# fuzzilli

## Project Description
The main purpose of this project is to perform fuzzilli adaptation testing on ES2aBC.
This project relies on Python 3, Git, Swift, ets_runtime component ark_js_vm executable files and ets_frond component es2abc executable file.
All outputs of this project are located in the fuzzilli/output directory.

## Usage notes
Execute Python script run_ fuzzy.py, automatically executes fuzzilli source code downloads, swift tool downloads, patch updates, compilation operations, and runs fuzzilli.

## Run an example
$:cd arkcompiler/ets_frontend/
$:python fuzzilli/run_fuzzy.py

### Precautions
This project relies on ets_runtime's component ark_js_vm executable files and ets_frontend's component es2abc executable file.  