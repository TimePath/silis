python

import glob
import os

cwd = os.getcwd()  # os.path.dirname(__file__)
for it in glob.glob(f"{os.path.join(cwd, 'gdb')}/*.py"):
    gdb.execute(f"source {it}")
