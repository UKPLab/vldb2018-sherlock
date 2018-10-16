#!/usr/bin/env bash
. .venv/bin/activate
python summarizer/cascade.py "$@"
deactivate
