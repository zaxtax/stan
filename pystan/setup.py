from distutils.core import setup
from Cython.Build import cythonize
#from distutils.extension import Extension
#from Cython.Distutils import build_ext

setup(name="pystan",
      version="0.0.1",
      ext_modules=cythonize(
          "stan.pyx",
          sources=[],  # additional source file(s)
          include_dirs=[r'.', r'../src'],
          library_dirs=[r'.'],
          libraries=['m', 'stan'],  # shared libs
          extra_objects=["../bin/libstanc.a",
                         "../bin/libstan.a"],  # static libs
          language="c++",
      ),
      )
