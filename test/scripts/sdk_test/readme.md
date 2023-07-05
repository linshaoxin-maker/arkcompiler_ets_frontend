
# SDK Test Suite Overview
This test suite can perform end-to-end SDK test verification. There are two ways to perform verification:
1) Verify if abc and sourcemap are generated in the compiled package.
2) Verify if the application of the compiled package can run normally (this feature is under development).

## SDK Test Suite Usage
### Operating Environment
The SDK test automation script runs on Windows, Python 3.7 and above. The MAC version has not been verified yet.

### Test Preparation
1. Ensure that Deveco is installed in the environment.
2. Install dependencies of the test suite:
`python3 -m pip install pyyaml validators requests httpx tqdm json5 pandas`
3. Modify the configuration file `config.yaml`, configure relevant parameters of Deveco and the test application. Detailed configuration instructions can be found in the file.

### Running Tests
The test suite supports daily and manual runs.

#### Daily Run
The daily run will download the SDK built on the current day from the trunk branch and use it to perform a full test verification.
Run the command: `python entry.py`

#### Manual Run
Run the command: `python run.py`
By default, it will run all the test items. Optional parameters can be viewed through `--help`.