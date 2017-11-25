"""
In order to build, call this script.

You might have to replace the path I use with your cl path

... hax
"""

import os
from subprocess import call

KIT_VERSION = "10.0.15063.0/"
MSVC_VERSION = "14.11.25503/"

PATH_TO_VS = "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/"
PATH_TO_TOOLS = PATH_TO_VS + "Tools/MSVC/" + MSVC_VERSION
PATH_TO_KIT = "C:/Program Files (x86)/Windows Kits/10/"
PATH_TO_MSVC = PATH_TO_TOOLS + "bin/HostX64/x64/"
STDCXX_INCLUDES = PATH_TO_TOOLS + "include"
STDC_INCLUDES = PATH_TO_KIT + "Include/" + KIT_VERSION + "ucrt"

MSVC_COMPILER_FLAGS = [
    "/I" + STDCXX_INCLUDES,
    "/I" + STDC_INCLUDES,
    "/Iinclude",
    "/c",
    "/std:c++17",
    "/EHsc",
    "/nologo",
    "/W4",
    "/wd4100",  # because msvc's template implementation is annoying
    "/wd4456",
    "/permissive-",
    "/DUSTD_MSVC_COMPATIBLE",
    "/GR-",
]
MSVC_LINKER_FLAGS = [
    "/LIBPATH:" + PATH_TO_TOOLS + "lib/x64/",
    "/LIBPATH:" + PATH_TO_TOOLS + "atlmfc/lib/x64/",
    "/LIBPATH:" + PATH_TO_VS + "Auxiliary/VS/lib/x64/",
    "/LIBPATH:" + PATH_TO_KIT + "Lib/" + KIT_VERSION + "ucrt/x64/",
    "/LIBPATH:" + PATH_TO_KIT + "Lib/" + KIT_VERSION + "um/x64/",
    "/NOLOGO",
]

CLANG_COMPILER_FLAGS = [
    "-Xclang",
    "-flto-visibility-public-std",
    "-Iinclude",
    "-std=c++17",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-fno-ms-compatibility",
    "-fno-delayed-template-parsing",
    "-fno-rtti",
    "-D", "_HAS_STATIC_RTTI=0",
    "-D", "USTD_STD_COMPATIBLE",
]

FILES = ["source/main.cpp"]


def base_file_name(file):
    """
    take a file name like "boop/foo.bar" and give you "foo"
    returns the thing before the first dot in the thing after the last slash
    """
    base_file = str.split(file, "/")[-1]
    name = str.split(base_file, ".")[0]
    return name


def msvc():
    compiler = PATH_TO_MSVC + "cl.exe"
    linker = PATH_TO_MSVC + "link.exe"
    obj_files = []
    for file in FILES:
        output_file = "build/" + base_file_name(file) + ".obj"
        output = "/Fo" + output_file
        list.append(obj_files, output_file)
        res = call([compiler, *MSVC_COMPILER_FLAGS, file, output])
        if res != 0:
            exit(res)
    res = call([linker, "/OUT:ast-msvc.exe", *MSVC_LINKER_FLAGS,
                *obj_files])
    if res != 0:
        exit(res)


def clang():
    obj_files = []
    for file in FILES:
        output_file = "build/" + base_file_name(file) + ".o"
        output = "-o" + output_file
        list.append(obj_files, output_file)
        res = call(
            ["clang++", "-c", *CLANG_COMPILER_FLAGS, file, output])
        if res != 0:
            exit(res)
    res = call(["clang++", *obj_files, "-o", "ast-clang.exe"])
    if res != 0:
        exit(res)


def main():
    """
    main function, to get pylint off my back about naming ;)
    """
    if not os.path.exists("build"):
        os.makedirs("build")
    clang()
    msvc()


if __name__ == "__main__":
    main()
