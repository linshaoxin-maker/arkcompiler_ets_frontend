import argparse
import os

import yaml
import requests
from lxml import etree
from datetime import datetime, timedelta, time

from result import get_result


configs = {}


def parse_config():
    config_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'config.yaml')
    with open(config_file_path, 'r', encoding='utf-8') as config_file:
        global configs
        configs = yaml.safe_load(config_file)


def get_url(name, page):
    url_prefix = 'https://gitee.com/openharmony/'
    url_suffix = f'/pulls?assignee_id=&author_id=&label_ids=&label_text=&milestone_id=&page={page}&priority=&project_type=&scope=&search=&single_label_id=&single_label_text=&sort=closed_at+desc&status=merged&target_project=&tester_id='
    url = url_prefix + name + url_suffix

    return url


def get_html(url):
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3'
    }
    try:
        response = requests.get(url, headers=headers, verify=False)
        if response.status_code == 200:
            return response.text
    except Exception as e:
        print(e)
        return None


def crawl_committer(repo_list, start_time, end_time):
    crawl_max_page = configs.get('crawl_max_page')
    data_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data.txt')
    if os.path.exists(data_file):
        os.remove(data_file)
    for repo_name in repo_list:
        for i in range(1, crawl_max_page + 1):
            url = get_url(repo_name, str(i))
            print(url)
            html = get_html(url)
            tree = etree.HTML(html)
            commit_list = tree.xpath('/html/body/div[2]/div[2]/div[2]/div[2]/div')
            for commit_task in commit_list:
                title = commit_task.xpath('.//div[1]/a/text()')[0]
                committer = commit_task.xpath('.//div[3]/span[2]/a/span/text()')[0]
                commit_time_str = commit_task.xpath('.//div[3]/span[4]/span/text()')[0].strip()
                pr_link = commit_task.xpath('.//div[1]/a/@href')[0]

                time = datetime.strptime(commit_time_str, '%Y-%m-%d %H:%M')
                if start_time <= time <= end_time:
                    print("在预期的时间段内")
                    print(title)
                    print(committer)
                    print(commit_time_str)
                    print(pr_link)
                    print('---------------------------------')
                    with open(data_file, 'a', encoding='utf-8') as file:
                        file.write(f"{repo_name}, {title}, {committer}, {commit_time_str}, {pr_link}\n")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--startTime', type=str, dest='start_time', default=None,
                        help='specify crawl start time')
    parser.add_argument('--repoName', type=str, dest='repo_name', default=None,
                        nargs='+',
                        help='specify which repo you want to crawl')
    return parser.parse_args()


if __name__ == '__main__':
    parse_config()
    end_time = datetime.now()
    yesterday = end_time - timedelta(days=1)
    start_time = datetime(yesterday.year, yesterday.month, yesterday.day, 0, 0, 0)
    repo_list = configs.get('repo_list')

    arguments = parse_args()
    if arguments.start_time is not None:
        time_str = datetime.strptime(arguments.start_time, '%Y-%m-%d')
        start_time = datetime.combine(time_str, time.min)
        end_time = start_time + timedelta(days=1)
        print('爬取开始时间', start_time)
        print('爬取结束时间', end_time)
    if arguments.repo_name is not None:
        repo_list = arguments.repo_name
        print(repo_list)

    crawl_committer(repo_list, start_time, end_time)
    get_result()



