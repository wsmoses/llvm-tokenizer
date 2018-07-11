import os
import re
import sys
import platform
import subprocess
import urllib.request
import tarfile



from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' # if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j2']

        env = os.environ.copy()
        print(env)
        if 'LLVM_PREFIX' not in env:
            #env['LLVM_PREFIX'] = str(subprocess.check_output(["/home/wmoses/git/Parallel-IR/build/bin/llvm-config", "--prefix"], universal_newlines=True)).rstrip('\n')
            env['LLVM_PREFIX'] = str(subprocess.check_output(["llvm-config-5.0", "--prefix"], universal_newlines=True)).rstrip('\n')
        cmake_args.append("-DLLVM_PREFIX="+env["LLVM_PREFIX"])
        print(cmake_args)
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        env['VERBOSE'] = '1'
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', '-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)


        ##CLgen
        #subprocess.check_call("%s configure --with-cuda" % (sys.executable,), cwd="./third-party/clgen", env=env, shell=True)
        #subprocess.check_call("make PYTHON=%s" % (sys.executable,), cwd="./third-party/clgen", env=env, shell=True)

        ##CSmith
        subprocess.check_call("./configure".split(" "), cwd="./third-party/csmith", env=env)
        subprocess.check_call("make", cwd="./third-party/csmith", shell=True)
        subprocess.check_call("make install", cwd="./third-party/csmith", shell=True)



class CSmithExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

setup(
    name='pyllvm',
    version='0.0.1',
    author='William S. Moses',
    author_email='wmoses@mit.edu',
    description='Some python LLVM bindings',
    long_description='',
    install_requires=['deap','boto3', 'fabric'],
    ext_modules=[CMakeExtension('pyllvm')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)
