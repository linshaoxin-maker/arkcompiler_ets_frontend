# utils包使用说明
## 脚本目的
* 为测试套下载好每日最新的测试镜像，然后烧录dayu200；
* 获取前一天0点到脚本运行时这个时间段内仓库的提交信息；
* 邮件自动发送测试结果

## 依赖安装
```
python3 -m pip install pywinauto lxml requests

```

## download.py
下载并解压镜像
### 参数说明
1.sdkUrl\dayuUrl 输入需要下载的镜像的地址
2.sdkDate\dayuDate 你想要获取哪一天的镜像(时间格式为2024-01-01)
3.sdkPah\dayuPath 你想将镜像替换到什么位置（可输入多个）
4.continue 输入改参数后下载完镜像后重新开始测试
## get_commit_log.py
### 参数说明
获取仓库的提交记录
1.startTime 你需要获取哪一天的提交记录
2.repoName 你需要获取哪些仓库的提交信息（可输入多个）
## update.py
将镜像替换到需要的位置
### 参数说明
1.sdkFilePat\dayuFilePath 需填写你需要复制的文件是哪一个
2.sdkOutputPath\dayuOutputPath 填写你需要复制到哪里（可输入多个）
## send_email.py
自动发送测试邮件
## autoburn.py
将镜像烧录到rk板上
## 注意事项
sdk的地址目前最后必须是api_version没有版本号会导致替换失败
sdkUrl\dayuUrl 这个参数如果后面不输入内容就会自动去官网获取镜像地址
不输入参数将按照配置去执行
