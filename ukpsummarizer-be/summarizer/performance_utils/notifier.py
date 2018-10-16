from abc import ABCMeta, abstractmethod
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import getpass
import shutil
from subprocess import call


class AbstractNotifier(ABCMeta):
    @abstractmethod
    def __init__(self, time, folder_name):
        pass

    @abstractmethod
    def send_payload(self):
        pass


class DropboxNotifier(AbstractNotifier):
    def __init__(self, time, folder_name):
        self.start_time = time
        self.folder_name = folder_name
        self.dropbox_path = "~/.ukpsummarizer/measurements/"

    def send_payload(self):
        call(["cp", "-R", self.folder_name, self.dropbox_path])
        pass


class EmailNotifier(AbstractNotifier):
    """docstring for ResultNotifier"""

    def __init__(self, time, folder_name):
        self.smtpObj = smtplib.SMTP_SSL('smtp.gmail.com')  # smtp connection
        self.smtpObj.ehlo()  # identify yourself to smtp server (EHLO)
        # self.smtpObj.starttls()

        # Next, log in to the server
        self.pswd = getpass.getpass('Password:')
        self.login()

        self.folder_name = folder_name
        self.start_time = time
        self.attachment_name = time + ".zip"

    def login(self):
        try:
            self.smtpObj.login("orkanpython", self.pswd)
        except smtplib.SMTPException as e:
            raise e

    def zip_folder(self, folder_name):
        return shutil.make_archive(folder_name, 'zip', folder_name)

    def send_payload(self):
        sender = "orkanpython@gmail.com"
        receiver = "orkan.oezyurt@gmail.com"
        subject = "IMDS run done"

        msg = MIMEMultipart()
        msg['From'] = sender
        msg['To'] = receiver
        msg['Subject'] = subject

        body = "Run started at {} is done.".format(self.start_time)
        msg.attach(MIMEText(body, 'plain'))

        filename = self.zip_folder(self.folder_name)
        with open(filename, 'rb') as attachment:
            part = MIMEBase('application', 'octet-stream')
            part.set_payload(attachment.read())
        encoders.encode_base64(part)
        part.add_header('Content-Disposition', "attachment; filename= %s" % self.attachment_name)
        msg.attach(part)

        # self.smtpObj.set_debuglevel(1)
        self.login()
        self.smtpObj.sendmail(sender, receiver, msg.as_string())

        pass
