import os

from jinja2 import Environment, FileSystemLoader


def get_result():
    data_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data.txt')
    with open(data_file, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    data = {}
    for line in lines:
        line = line.strip()
        values = line.split(',')

        project = values[0]
        item = {
            'description': values[1],
            'author': values[2],
            'time': values[3],
            'link': f'https://gitee.com/{values[4]}'.replace(' ', '')
        }

        if project in data:
            data[project].append(item)
        else:
            data[project] = [item]

    template_dir = os.path.dirname(__file__)
    env = Environment(loader=FileSystemLoader(template_dir))
    template = env.get_template('template.html')
    output = template.render(data=data)

    commit_log_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'commit_log.html')
    with open(commit_log_file, 'w', encoding='utf-8') as file:
        file.write(output)


if __name__ == '__main__':
    get_result()
