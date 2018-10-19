#!/bin/ bash

export PATH="/home/local/UKP/avinesh/anaconda/bin:$PATH"
which python
python summarizer/cascade.py "$@"
