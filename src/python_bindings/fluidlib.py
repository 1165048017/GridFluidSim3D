import ctypes
import glob
import os

class LibraryLoadError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def __load_library(name):
        libdir = os.path.dirname(__file__)
        files = glob.glob(libdir + "/*" + name + "*")

        library = None
        for f in files:
            try:
                library = ctypes.cdll.LoadLibrary(f)
                break
            except:
                continue

        if not library:
            raise LibraryLoadError("Unable to load library: " + name)

        return library

def __init_library(lib):
    None

lib = __load_library("fluidcore")
__init_library(lib)