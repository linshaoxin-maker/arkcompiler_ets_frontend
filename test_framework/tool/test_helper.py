import re
import os

def read_declaration(path):
    start_pattern = re.compile(r'^\/\*\*\-*')
    end_pattern = re.compile(r'^\s*\-+\*\/')
    context = ""
    with open(path,'r', encoding='utf-8', errors='ignore') as f:
        declaration_begin = False
        while True:
            line = f.readline()
            if not line:
                break
            if start_pattern.match(line):
                declaration_begin = True
                continue
            if end_pattern.match(line):
                declaration_begin = False
                break    
            if declaration_begin == True:
                context += line
    return context

def get_path_file(dir_path, all_file_path=None):
    if all_file_path is None:
        all_file_path = []
    file_or_dir = os.listdir(dir_path)
    for file_dir in file_or_dir:
        file_or_dir_path = os.path.join(dir_path, file_dir)
        if '\\' in file_or_dir_path:
            file_or_dir_path = file_or_dir_path.replace('\\', '/')

        # 判断该路径是不是路径，如果是，递归调用
        if os.path.isdir(file_or_dir_path):
            # 递归
            get_path_file(file_or_dir_path, all_file_path)
        else:
            all_file_path.append(file_or_dir_path)

    return all_file_path

def get_disable_list(file_path):
    disable_list = []
    with open(file_path,'r', encoding='utf-8', errors='ignore') as f:
        while True:
            line = f.readline()
            if not line:
                break
            disable_list.append(os.path.abspath(line.strip()))
    return disable_list

def is_disable_case(file_path, disable_list):
    if disable_list == None:
        return False
    if file_path in disable_list:
        return True
    for disable_path in disable_list:
        if disable_path in file_path:
            return True





