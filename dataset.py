"""Script to select dataset for training """

# File Types
# 1. Raw Files
# 2. CSmith
# 3. Clgen:
#   Github link

# Makefile

import subprocess
import os
from enum import Enum
import shutil

class InputType(Enum):
    RAW_FILE = 1
    CSMITH = 2
    CLGEN = 3
    GITHUB = 4

# Can initialize multiple objects of different type for training
class Data():
    """ Dataset object
    :param input_type: Input type. Listed in InputType
    :param path: Path

    """
    def __init__(self, path, *args, **kwargs ):
        self.path = path
        self.size = 0

    def generate(self):
        pass

    def save_obj(self, fn):
        import pickle
        obj = os.path.join(self.path, str(fn))
        f = open(obj, 'wb')
        pickle.dump(self, f)
        f.close()

    def load_obj(self, fn, type):
        import pickle
        path = os.path.join(self.path, str(fn))

        if os.path.getsize(path) > 0:
            with open(path, 'rb') as f:
                obj = pickle.load(f)
                if not isinstance(obj, type):
                    raise TypeError('Unpickled object is not of type {}'.format(self))
                else:
                    return obj
        return None

class Rawfile(Data):
    """ Existing files
    :param fn: Filename
    :param src_path: Folder containing the source code with name fn and auxiliary files

    """

    def __init__(self, path, fn, src_path, *args, **kwargs):
        super(Rawfile, self).__init__(path, *args, **kwargs)
        self.src_path = src_path

    def generate(self):
        if os.path.normpath(self.src_path) == os.path.normpath(self.path):
            print("Source folder and destination folder are the same!\n")
            return

        output = super(Rawfile, self).generate()

        src_files = os.listdir(self.src_path)

        for file in src_files:
            full_file = os.path.join(self.src_path, file)
            if (os.path.isfile(full_file)):
                shutil.copy(full_file, self.path)
        dst_files = os.listdir(self.path)
        if self.fn not in dst_files:
            raise IOError(self.fn +" not in " + self.src_path + "!")

        print("Generated File: "+ output + '\n')

class Csmith(Data):
    """ Csmith generated data
    :param csmith_path: Csmith executable
    :param options: Csmith options

    :param template_path: Folder containing auxiliary files for compilation
    """

    def __init__(self, path, csmith_path, options, template_path, \
                 max_size = 10000, min_size = 0, timeout = 10, fn_prefix = "", *args, **kwargs):
        self.obj_name = "csmith.obj"
        self.size = 0

        super(Csmith, self).__init__(path, *args, **kwargs)

        # See if there is Csmith object stored in the path
        dump_obj = os.path.join(self.path, self.obj_name)
        if (os.path.isfile(dump_obj)):
            csmith_obj = super(Csmith, self).load_obj(self.obj_name, Csmith)
            if csmith_obj:
                self.size = csmith_obj.size

        self.csmith_path = csmith_path
        self.options = options
        self.template_path = template_path
        self.max_size = max_size
        self.min_size = min_size
        self.timeout = timeout

        self.fn_prefix = fn_prefix

    def generate(self, size):
        print(self.size)
        if self.size >= size:
            print("Number of existing programs: %d\t\t\t\t"
                  "Number of requested: %d\n"
                  "No new programs are generated!\n"%(self.size, size))
            return

        os.makedirs(self.path, exist_ok=True)
        csmith = os.path.join(self.csmith_path, "csmith")

        # Copy the skeleton code over
        if self.size < size:
            skeleton_files = os.listdir(self.template_path)

            for file in skeleton_files:
                full_file = os.path.join(self.template_path, file)
                if (os.path.isfile(full_file)):
                    shutil.copy(full_file, self.path)

        while self.size < size:
            output_fn = self.fn_prefix + str(self.size) + ".c"
            output = os.path.join(self.path, output_fn)

            while True:
                # Run csmith
                cmd = csmith + " " + self.options + " -o " + output

                # Generate the  program and get the line count
                p = subprocess.Popen(cmd + "; wc -l " + output, shell=True, stdout=subprocess.PIPE,\
                                                            stderr=subprocess.PIPE)
                out, err = p.communicate()
                if p.returncode != 0:
                    super(Csmith, self).save_obj(self.obj_name)
                    raise IOError(err)

                lines = int(out.strip().split()[0])
                if lines > self.max_size:
                    continue

                # Run compilation
                CC = "gcc"
                cmd = CC + " "+ output + " -o " + output.replace(".c", "")
                p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,\
                        stderr=subprocess.PIPE)
                out, err = p.communicate()

                if p.returncode != 0:
                    super(Csmith, self).save_obj(self.obj_name)
                    raise IOError(err)

                exe = os.path.join(self.path, output_fn.replace(".c", ""))
                cmd = exe
                try:
                    out = subprocess.check_output(exe, shell=True, stderr= subprocess.STDOUT, timeout = self.timeout)
                # except subprocess.CalledProcessError as e:
                except Exception as e:
                    print("Program not terminates in %d s.\n"%(self.timeout))
                    continue
                else:
                    os.remove(exe)
                    break
                # run = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                # timer = Timer(self.timeout, kill, [run])
                # try:
            print("Generated File: "+ output + '\n')
            self.size += 1
        super(Csmith, self).save_obj(self.obj_name)

    def list(self):
        paths = [ os.path.abspath(os.path.join(self.path, self.fn_prefix + str(i))) for i in range(self.size)]
        return paths

class CLgen(Data):
    """
    Generate OpenCL benchmarks using clgen
    1. Github Corpus: source code from Github
    2. Generated OpenCL benchmakr from clgen
    Now only supports Github

    """
    def __init__(self, path, github = True, *args, **kwargs):
        super(CLgen, self).__init__(path, *args, **kwargs)

        # type: 0 for clgen, 1 for github
        self.github = github

        # fetch_github will fetch all existing cl repo on github
        # timeout is used to limited the time to collect github data
        # TODO we might need to write our own fetch github
        # if we need to generate exact number of examples

        if self.github:
            try:
                self.github_username = kwargs["github_username"]
                self.github_passwd = kwargs["github_passwd"]
                self.github_token = kwargs["github_token"]
                self.github_timeout = kwargs["github_timeout"]
            except Exception as e:
                print("Please specify args for: ")
                print(e)
                raise
        else:
            try:
                self.model_file = kwargs["model_file"]
                self.sampler_file = kwargs["sampler_file"]
            except Exception as e:
                print("Using the example files.\n")
                self.model_file = os.path.abspath("./clgen/model.json")
                self.sampler_file = os.path.abspath("./clgen/sampler.json")

    def generate(self):
        # Try import clgen if installed
        try:
            import clgen
        except ImportError:
            raise ImportError

        print (clgen.__version__)
        os.makedirs(self.path, exist_ok=True)

        db = "clgen.db"

        output_db = os.path.join(self.path, db)
        if not os.path.exists(output_db):
            clgen.dbutil.create_db(output_db, self.github)

        # Github
        if (self.github):
            import multiprocessing
            import time

            def fetch_github(output_db):
                clgen._fetch.fetch_github(output_db, self.github_username, \
                    self.github_passwd, self.github_token)

            p = multiprocessing.Process(target=fetch_github, args = (output_db,))
            p.start()
            p.join(self.github_timeout)
            if p.is_alive():
                print ("Running fetch_github for %d\n", self.github_timeout)
                p.terminate()
                p.join()
        else:
            from labm8 import jsonutil
            model_json = jsonutil.loads(open(self.model_file, "r").read())
            print(model_json)
            model = clgen.Model.from_json(model_json)

            sampler_json = jsonutil.loads(open(self.sampler_file).read())
            sampler = clgen.Sampler.from_json(sampler_json)

            model.train()
            sampler.sample(model)
            output_db = sampler.cache(model)["kernels.db"]

        # Dump the programs to files
        clgen.dbutil.dump_db(output_db, self.path+"/db", dir=True, input_samples=True)

# class Github(Data):
#     def __init__(self, link, *args, **kwargs):
#         self.link = ""
#         super(Github, self).__init__( *args, **kwargs)

def test():

    # Csmith test
    c = Csmith(path="./test/", csmith_path="/Users/qijing.huang/Documents/LegUp+RNN/csmith/src", options="--no-structs --no-pointers --no-math64 --max-funcs 4 --no-unions", template_path="/Users/qijing.huang/Documents/LegUp+RNN/HLSRNNRL/Dataset/csmith-stuff/scripts/skeleton")
    c.generate(10)
    print(c.list())

    # Rawfile test
    c = Rawfile(path="./test1/", fn="test.c", src_path="./test1")
    c.generate()

    #clgen test
    c = CLgen(path="./test2", github= True, github_username="", github_passwd="", github_token="", github_timeout = 30)
    c.generate()

    c = CLgen(path= "./test3", github = False)
    c.generate()

#TODO we can create object and add it to clgen db

test()
