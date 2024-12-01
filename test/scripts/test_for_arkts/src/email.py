import json
import logging
import smtplib
from datetime import datetime
from email import encoders
from email.header import Header
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from pathlib import Path

import pytz

from src import utils

logger = logging.getLogger("test_logger")


def result_with_style(result: str, counts: int) -> str:
    result = counts > 0 and result + f" ({counts})" or result
    return (
        f'<span class="{"passed" if result == "Passed" else "failed"}">{result}</span>'
    )


def add_manifest(html_body: str) -> str:
    logger.info("Adding manifest to html")
    tz = pytz.timezone("Asia/Shanghai")
    date_time = datetime.now(tz).strftime("%Y-%m-%d %H:%M:%S:%f %Z")
    html = f"""
    <h3>Branch: master</h3>
    <h3>DateTime: {date_time}</h3>
    """
    return html_body.replace("{{ manifest }}", html)


def add_sdk_generated_time(sdk_completion_time: datetime | None, html_body: str) -> str:
    logger.info("Adding sdk completion time to html")
    if sdk_completion_time:
        html = f"""
        <h3>SDKGeneratedTime: {sdk_completion_time.strftime("%Y-%m-%d %H:%M:%S")}</h3>
        """
        html_body = html_body.replace("{{ sdk_completion_time }}", html)
    else:
        html_body = html_body.replace("{{ sdk_completion_time }}", "")
    return html_body


def add_rt_result(summary_file: Path | None, html_body: str) -> str:
    logger.info("Adding RT test result to html")
    if summary_file:
        with open(summary_file, encoding="utf-8") as f:
            content = json.load(f)

        test_main = content["test_main"]
        main_failures = test_main["failures"]
        main_failures_html = "".join([
            f"<li>{failure}</li>" for failure in main_failures
        ])
        test_rules = content["test_rules"]
        rules_failures = test_rules["failures"]
        rules_failures_html = "".join([
            f"<li>{failure}</li>" for failure in rules_failures
        ])
        test_regression = content["test_regression"]
        regression_failures = test_regression["failures"]
        regression_failures_html = "".join([
            f"<li>{failure}</li>" for failure in regression_failures
        ])

        html = f"""
        <h3>RT Test Result</h3>
        <blockquote>
        <p>test_main: Language Feature Test</p>
        <p>test_rules: Rules Test</p>
        <p>test_regression: Issue Test</p>
        </blockquote>
        <p><strong>test_main:</strong> {result_with_style(test_main["overall_result"], len(main_failures))}</p>
        <ul>
        {main_failures_html}
        </ul>
        <p><strong>test_rules:</strong> {result_with_style(test_rules["overall_result"], len(rules_failures))}</p>
        <ul>
        {rules_failures_html}
        </ul>
        <p><strong>test_regression:</strong> {result_with_style(test_regression["overall_result"], len(regression_failures))}</p>
        <ul>
        {regression_failures_html}
        </ul>
        <hr>
        """
        html_body = html_body.replace("{{ rt_test_results }}", html)
    else:
        html = """
        <h3>RT Test Result</h3>
        <p class="failed"><strong>RT test failed</strong></p>
        <hr>
        """
        html_body = html_body.replace("{{ rt_test_results }}", html)

    return html_body


def add_tsc_result(summary_file: Path | None, html_body: str) -> str:
    logger.info("Adding TSC test result to html")
    if summary_file:
        with open(summary_file, encoding="utf-8") as f:
            content = json.load(f)

        tsc_test = content["test"]
        res_flag = tsc_test["overall_result"]
        failures = tsc_test["failures"]
        failures_html = "".join([
            f"<li>{failure['case']}<ul><li>task: {failure['task']}</li><li>{failure['error']}</li></ul></li>"
            for failure in failures
        ])

        html = f"""
        <h3>TSC Test Result</h3>
        <p><strong>tsc_result:</strong> {result_with_style(res_flag, len(failures))}</p>
        <ul>
        {failures_html}
        </ul>
        <hr>
        """
        html_body = html_body.replace("{{ tsc_test_results }}", html)
    else:
        html = """
        <h3>TSC Test Result</h3>
        <p class="failed"><strong>TSC test failed</strong></p>
        <hr>
        """
        html_body = html_body.replace("{{ tsc_test_results }}", html)

    return html_body


def add_cookbook_result(summary_file: Path | None, html_body: str) -> str:
    logger.info("Adding cookbook test result to html")
    if summary_file:
        with open(summary_file, encoding="utf-8") as f:
            content = json.load(f)

        cookbook_res = content["cookbook"]
        res_flag = cookbook_res["overall_result"]
        failures = cookbook_res["failures"]
        failures_html = "".join([f"<li>{failure}</li>" for failure in failures])

        html = f"""
        <h3>cookbook Test Result</h3>
        <p><strong>cookbook_result:</strong> {result_with_style(res_flag, len(failures))}</p>
        <ul>
            {failures_html}
        </ul>
        <hr>
        """
        html_body = html_body.replace("{{ issue_cookbook }}", html)
    else:
        html = """
        <h3>cookbook Test Result</h3>
        <p class="failed"><strong>cookbook test failed</strong></p>
        <hr>
        """
        html_body = html_body.replace("{{ issue_cookbook }}", html)

    return html_body


def add_arktstest_result(summary_file: Path | None, html_body: str) -> str:
    logger.info("Adding arkTSTest test result to html")
    if summary_file:
        with open(summary_file, encoding="utf-8") as f:
            content = json.load(f)

        arkts_1_0_test = content["arktstest_v1.0"]
        arkts_1_0_flag = arkts_1_0_test["overall_result"]
        arkts_1_0_failures = arkts_1_0_test["failures"]
        arkts_1_0_failures_html = "".join([
            f"<li>{failure}</li>" for failure in arkts_1_0_failures
        ])

        arkts_1_1_test = content["arktstest_v1.1"]
        arkts_1_1_flag = arkts_1_1_test["overall_result"]
        arkts_1_1_failures = arkts_1_1_test["failures"]
        arkts_1_1_failures_html = "".join([
            f"<li>{failure}</li>" for failure in arkts_1_1_failures
        ])

        html = f"""
        <h3>arkTSTest Test Result</h3>
        <p><strong>arktstest_v1.0_result:</strong> {result_with_style(arkts_1_0_flag, len(arkts_1_0_failures))}</p>
        <ul>
        {arkts_1_0_failures_html}
        </ul>
        <p><strong>arktstest_v1.1_result:</strong> {result_with_style(arkts_1_1_flag, len(arkts_1_1_failures))}</p>
        <ul>
        {arkts_1_1_failures_html}
        </ul>
        <hr>
        """
        html_body = html_body.replace("{{ arktstest_test_results }}", html)
    else:
        html = """
        <h3>arkTSTest Test Result</h3>
        <p class="failed"><strong>arkTSTest test failed</strong></p>
        <hr>
        """
        html_body = html_body.replace("{{ arktstest_test_results }}", html)

    return html_body


def add_libs_verification_result(summary_file: Path | None, html_body: str) -> str:
    logger.info("Adding 3rd libs verification result to html")
    if summary_file:
        with open(summary_file, encoding="utf-8") as f:
            content = json.load(f)

        verification_res = content["libs_verification"]
        res_flag = verification_res["overall_result"]
        failures = verification_res["failures"]
        failures_html = "".join([f"<li>{failure}</li>" for failure in failures])
        skiped_projects = verification_res["skiped_projects"]
        skiped_html = "".join([f"<li>{project}</li>" for project in skiped_projects])

        html = f"""
        <h3>3rd libs verification Result</h3>
        <p><strong>verify_result:</strong> {result_with_style(res_flag, len(failures))}</p>
        <ul>
            {failures_html}
        </ul>
        <p><strong>skipped_projects:</strong></p>
        <ul>
            {skiped_html}
        </ul>
        <hr>
        """
        html_body = html_body.replace("{{ verify_3rd_libs }}", html)
    else:
        html = """
        <h3>3rd libs verification Result</h3>
        <p class="failed"><strong>3rd libs verification failed</strong></p>
        <hr>
        """
        html_body = html_body.replace("{{ verify_3rd_libs }}", html)

    return html_body


def send_email(html_content: str, attachment_path: Path | None) -> None:
    logger.info("Starting to send email...")
    email_config: dict = utils.config.get("email")
    sender = email_config["sender"]
    receivers = email_config["receivers"]
    cc_receivers = email_config["cc_receivers"]
    username = email_config["username"]
    password = email_config["password"]

    message = MIMEMultipart()
    message["From"] = sender
    message["To"] = ", ".join(receivers)
    message["CC"] = ", ".join(cc_receivers)
    message["Subject"] = str(Header("ArkTS daily test report", "utf-8"))
    message.attach(MIMEText(html_content, "html"))

    if attachment_path and attachment_path.exists():
        part = MIMEBase("application", "octet-stream")
        with attachment_path.open("rb") as attachment_file:
            part.set_payload(attachment_file.read())
        encoders.encode_base64(part)
        part.add_header(
            "Content-Disposition",
            f"attachment; filename={attachment_path.name}",
        )
        message.attach(part)
    else:
        logger.warning("Attachment path is invalid or does not exist.")

    smtp_obj = None
    try:
        if password is None:
            msg = "Email password is not configured."
            raise ValueError(msg)
        smtp_obj = smtplib.SMTP_SSL(
            email_config["smtp_server"],
            email_config["smtp_port"],
        )
        smtp_obj.login(username, password)
        smtp_obj.sendmail(sender, receivers + cc_receivers, message.as_string())
        logger.info("Email sent successfully")
    except (smtplib.SMTPException, ValueError) as e:
        logger.error(f"Error: Unable to send email. {e}")
    finally:
        if smtp_obj is not None:
            smtp_obj.quit()
