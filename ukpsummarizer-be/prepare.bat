pip install virtualenv
virtualenv --system-site-packages --unzip-setuptools  .venv
set VIRTUAL_ENV=%~dp0\.venv
set PATH=%VIRTUAL_ENV%\Scripts;%PATH%
pip install -r requirements.txt
