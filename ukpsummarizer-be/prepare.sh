#!/usr/bin/env bash
# pip and virtualenv have to be provided by the OS.
#pip install virtualenv
#conda create -n .venv python=2.7
#source .venv/bin/activate
source activate .venv
pip install -r requirements.txt
source deactivate
