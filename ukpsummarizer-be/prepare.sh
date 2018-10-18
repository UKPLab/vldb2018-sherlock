#!/usr/bin/ bash
# pip and virtualenv have to be provided by the OS.
#pip install virtualenv
#conda create -n .venv python=2.7
. activate .venv
pip install -r requirements.txt
. deactivate
