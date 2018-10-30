#!/bin/bash
export PATH="/home/avinesh/anaconda2/bin:$PATH"
which python
python summarizer/cascade.py "$@"
