import csv, codecs
import re
from os import path


def read_csv(filename):
    with open(filename, 'rb') as csvfile:
        reader = csv.reader(csvfile, delimiter=",", quotechar= '"')
        rows = [row for row in reader]
        return rows

def read_file(filename):
    with codecs.open(filename, 'r', 'utf-8', errors='ignore') as fp:
        return fp.read()


def resolve_against_iobase(ds, iobasedir):
    if ds.startswith("/"):
        relative_path = re.search('^(/)(.*)$', ds).group(2)
    else:
        relative_path = ds
    return path.normpath(path.join(iobasedir, relative_path))