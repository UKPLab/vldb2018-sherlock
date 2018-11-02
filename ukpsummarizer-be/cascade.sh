#!/bin/bash
export PATH="~/anaconda2/bin:$PATH"
which python
python summarizer/cascade.py "$@"
