# Documentation for utils package
## Script Purpose
Download the latest test images for the test suite and flash them onto dayu200.
Retrieve the commit information of the repositories within the time range from 0:00 of the previous day to the time the script is executed.
Automatically send test results via email.

## Dependency Installation

```
python3 -m pip install pywinauto lxml requests

```

## download.py

Download and extract the image.

### Parameter Description

sdkUrl\dayuUrl: The URL of the image to be downloaded.
sdkDate\dayuDate: The date of the image to be obtained (in the format "2024-01-01").
sdkPath\dayuPath: The location where the image will be replaced (multiple paths can be entered).
continue: If this parameter is provided, the script will restart the test after downloading the image.

## get_commit_log.py

### Parameter Description

Retrieve the commit log of the repository.

startTime: The day for which you need to retrieve the commit log.
repoName: The repositories for which you need to retrieve the commit information (multiple repository names can be entered).

## update.py

Replace the image at the desired location.

### Parameter Description

sdkFilePat\dayuFilePath: Specify the file to be copied.
sdkOutputPath\dayuOutputPath: Specify the destination paths where the file will be copied (multiple paths can be entered).

## send_email.py

Automatically send test result emails.

## autoburn.py

Flash the image onto the rk board.

## Notes

The sdkUrl\dayuUrl parameter must end with api_version, otherwise the replacement of the image will fail.
If no value is provided for sdkUrl\dayuUrl, the script will automatically retrieve the image URL from the official website.
If no parameters are provided, the tool will run according to the default configuration.