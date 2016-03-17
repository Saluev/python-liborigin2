import sys
from distutils.core import setup
from distutils.extension import Extension

# we'd better have Cython installed, or it's a no-go
try:
    from Cython.Distutils import build_ext
    from Cython.Build import cythonize
except:
    print("You don't seem to have Cython installed. Please get a")
    print("copy from http://cython.org and install it")
    sys.exit(1)


src = ["liborigin.pyx", "OriginObj.cpp",
       "opj2dat.cpp", "Origin750Parser.cpp", "OriginFile.cpp",
       "Origin600Parser.cpp", "Origin800Parser.cpp", "OriginParser.cpp",
       "Origin610Parser.cpp", "Origin810Parser.cpp",
       "Origin700Parser.cpp", "OriginDefaultParser.cpp"]
surf3d_ext = Extension("liborigin", src, language='c++')
# surf3d_ext = cythonize("liborigin.pyx", sources=src, language="c++")

# finally, we can pass all this to distutils
extensions = [surf3d_ext]
setup(
    name="liborigin",
    ext_modules=extensions,
    cmdclass={'build_ext': build_ext},
)
