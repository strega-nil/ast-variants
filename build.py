"""
In order to build, call this script.

You might have to replace the path I use with your cl path

... hax
"""

from subprocess import call

PATH_TO_VS = "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/"
PATH_TO_TOOLS = PATH_TO_VS + "Tools/MSVC/14.11.25503/"
PATH_TO_KIT = "C:/Program Files (x86)/Windows Kits/10/"
KIT_VERSION = "10.0.15063.0/"
PATH_TO_MSVC = PATH_TO_TOOLS + "bin/HostX64/x64/"
STDCXX_INCLUDES = PATH_TO_TOOLS + "include"
STDC_INCLUDES = PATH_TO_KIT + "Include/" + KIT_VERSION + "ucrt"
COMPILER_FLAGS = [
    "/I" + STDCXX_INCLUDES,
    "/I" + STDC_INCLUDES,
    "/Iinclude",
    "/c",
    "/std:c++latest",
    "/EHsc",
    "/nologo",
    "/W4",
    "/wd4100",
    "/permissive-",
]
LINKER_FLAGS = [
    "/LIBPATH:" + PATH_TO_TOOLS + "lib/x64/",
    "/LIBPATH:" + PATH_TO_TOOLS + "atlmfc/lib/x64/",
    "/LIBPATH:" + PATH_TO_VS + "Auxiliary/VS/lib/x64/",
    "/LIBPATH:" + PATH_TO_KIT + "Lib/" + KIT_VERSION + "ucrt/x64/",
    "/LIBPATH:" + PATH_TO_KIT + "Lib/" + KIT_VERSION + "um/x64/",
    "/NOLOGO",
]


def main():
    """
    main function, to get pylint off my back about naming ;)
    """
    files = ["source/main.cpp", "source/ustd/utility.cpp"]
    if True:
        compiler = PATH_TO_MSVC + "cl.exe"
        linker = PATH_TO_MSVC + "link.exe"
        for file in files:
            res = call([compiler, *COMPILER_FLAGS, file])
            if res != 0:
                exit(res)
        obj_files = ["main.obj", "utility.obj"]
        res = call([linker, *LINKER_FLAGS, *obj_files])
        if res != 0:
            exit(res)
    else:
        compat_flags = ["-Xclang", "-flto-visibility-public-std"]
        lang_flags = ["-Iinclude", "-std=c++17", "-c"]
        warning_flags = ["-Wall", "-Wextra", "-pedantic"]
        for file in files:
            res = call(
                ["clang++", *compat_flags, *lang_flags, *warning_flags, file])
            if res != 0:
                exit(res)
        obj_files = ["main.o", "utility.o"]
        res = call(["clang++", *obj_files, "-o", "main.exe"])
        if res != 0:
            exit(res)


if __name__ == "__main__":
    main()
