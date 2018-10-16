@echo off



echo all args:
echo %*

set PTYHONPATH=%PYTHONPATH%;C:\Users\hatieke\Projects\ukp-thesis\casum_summarizer
set VIRTUAL_ENV=C:\Users\hatieke\Projects\ukp-thesis\casum_summarizer\.venv_windows
set PATH=%VIRTUAL_ENV%\Scripts;%PATH%
python summarizer/single_iteration_pipes.py %*
