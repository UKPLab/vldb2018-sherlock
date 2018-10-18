#!/usr/bin/env bash
. activate .venv
python summarizer/cascade.py "$@"
. deactivate
