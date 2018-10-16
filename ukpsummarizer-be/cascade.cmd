@echo off

set PTYHONPATH=%PYTHONPATH%;%~dp0
set VIRTUAL_ENV=%~dp0\.venv
set PATH=%VIRTUAL_ENV%\Scripts;%PATH%
REM ECHO python summarizer/cascade.py %*
python summarizer/cascade.py %*
