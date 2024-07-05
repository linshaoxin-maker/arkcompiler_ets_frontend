import os
import json
import pytz
import logging
import smtplib
from pathlib import Path
from datetime import datetime
from email import encoders
from email.header import Header
from email.mime.base import MIMEBase
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart


test_logger = logging.getLogger("test_logger")


def add_manifest_to_html(html_body: str) -> str:
    tz = pytz.timezone("Asia/Shanghai")
    date_time = datetime.now(tz).strftime("%Y-%m-%d %H:%M:%S:%f %Z")
    html_body = html_body.replace("{{ datetime }}", date_time)
    return html_body


def result_with_style(result, counts):
    result = counts > 0 and result + f" ({counts})" or result
    return f'<span class="{ "passed" if result == "Passed" else "failed" }">{result}</span>'


def construct_rt_html_body(summary_path: Path, html_body: str) -> str:
    with open(summary_path, "r", encoding="utf-8") as f:
        content = json.load(f)

    test_main = content["test_main"]
    main_failures = test_main["failures"]
    main_failures_html = "".join([f"<li>{failure}</li>" for failure in main_failures])
    test_rules = content["test_rules"]
    rules_failures = test_rules["failures"]
    rules_failures_html = "".join([f"<li>{failure}</li>" for failure in rules_failures])
    test_regression = content["test_regression"]
    regression_failures = test_regression["failures"]
    regression_failures_html = "".join(
        [f"<li>{failure}</li>" for failure in regression_failures]
    )

    html_body = (
        html_body.replace(
            "{{ rt_main_result }}",
            result_with_style(test_main["overall_result"], len(main_failures)),
        )
        .replace("{{ rt_main_failures }}", main_failures_html)
        .replace(
            "{{ rt_rules_result }}",
            result_with_style(test_rules["overall_result"], len(rules_failures)),
        )
        .replace("{{ rt_rules_failures }}", rules_failures_html)
        .replace(
            "{{ rt_regression_result }}",
            result_with_style(
                test_regression["overall_result"], len(regression_failures)
            ),
        )
        .replace("{{ rt_regression_failures }}", regression_failures_html)
    )

    return html_body


def construct_tsc_html_body(summary_path: Path, html_body: str) -> str:
    with open(summary_path, "r", encoding="utf-8") as f:
        content = json.load(f)

    tsc_test = content["test"]
    res_flag = tsc_test["overall_result"]
    failures = tsc_test["failures"]
    failures_html = "".join(
        [
            f"<li>{failure['case']}<ul><li>task: {failure['task']}</li><li>{failure['error']}</li></ul></li>"
            for failure in failures
        ]
    )

    counts = len(failures)

    html_body = html_body.replace(
        "{{ tsc_test_result }}", result_with_style(res_flag, counts)
    ).replace("{{ tsc_test_failures }}", failures_html)

    return html_body


def construct_arktstest_html_body(summary_path: Path, html_body: str) -> str:
    with open(summary_path, "r", encoding="utf-8") as f:
        content = json.load(f)

    arkts_1_0_test = content["arktstest_v1.0"]
    arkts_1_0_flag = arkts_1_0_test["overall_result"]
    arkts_1_0_failures = arkts_1_0_test["failures"]
    arkts_1_0_failures_html = "".join(
        [f"<li>{failure}</li>" for failure in arkts_1_0_failures]
    )

    arkts_1_1_test = content["arktstest_v1.1"]
    arkts_1_1_flag = arkts_1_1_test["overall_result"]
    arkts_1_1_failures = arkts_1_1_test["failures"]
    arkts_1_1_failures_html = "".join(
        [f"<li>{failure}</li>" for failure in arkts_1_1_failures]
    )

    html_body = (
        html_body.replace(
            "{{ arktstest_v1.0_result }}",
            result_with_style(arkts_1_0_flag, len(arkts_1_0_failures)),
        )
        .replace("{{ arktstest_v1.0_failures }}", arkts_1_0_failures_html)
        .replace(
            "{{ arktstest_v1.1_result }}",
            result_with_style(arkts_1_1_flag, len(arkts_1_1_failures)),
        )
        .replace("{{ arktstest_v1.1_failures }}", arkts_1_1_failures_html)
    )

    return html_body


def send_email(html_content: str, attachment_path: Path = None):
    test_logger.info("Starting to send email...")
    sender = ""
    receivers = []
    cc_receivers = []
    password = os.environ.get("EMAIL_PASSWORD")

    message = MIMEMultipart()
    message["From"] = sender
    message["To"] = ", ".join(receivers)
    message["CC"] = ", ".join(cc_receivers)
    message["Subject"] = Header("ArkTS daily test report", "utf-8")
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
        test_logger.warning("Attachment path is invalid or does not exist.")

    try:
        if password is None:
            raise ValueError("Email password is not configured.")
        smtp_obj = smtplib.SMTP_SSL("smtp.isoftstone.com", 465)
        smtp_obj.login(sender, password)
        smtp_obj.sendmail(sender, receivers + cc_receivers, message.as_string())
        test_logger.info("Email sent successfully")
    except (smtplib.SMTPException, ValueError) as e:
        test_logger.error("Error: Unable to send email. {}".format(e))
    finally:
        if "smtp_obj" in locals():
            smtp_obj.quit()
