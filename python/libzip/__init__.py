import ctypes.util
import glob
import os.path
import platform
import re

from .enums import *

ZIP_UINT16_MAX = 0xFFFF
ZIP_EXTRA_FIELD_ALL = ZIP_UINT16_MAX
ZIP_EXTRA_FIELD_NEW = ZIP_UINT16_MAX
