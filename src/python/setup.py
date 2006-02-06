#
#  (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

from distutils.core import setup, Extension
setup(name="Lintel",
      version="0.0",
      ext_modules=[Extension("Lintel", ["Lintel.C"],
                             include_dirs=[".."],
                             library_dirs=[".."],
                             libraries=["stdc++", "Lintel-debug"])
                   ])


