### Environment Setup

> Currently, this project can only be run on WSL.

To run this project, ensure the following software is installed:

- Python
- Git
- Node.js

### Run

1. Configure the `config.json` file:
    - `ohos_sdk`: The path to the SDK
    - `cookbook_path`: The path to the cookbook project

2. Set up the project:

```Bash
python -m venv .venv
source .venv/bin/activate
python -m pip install pytz requests
```

3. Run the test:

```Bash
python run_test.py
```

If you need to send an email. Please check the `config.json` and `src/email.py:send_email()`