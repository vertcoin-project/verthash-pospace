from setuptools import setup, Extension

verthashsources = [
	'h2.c',
       'tiny_sha3/sha3.c'
]

verthashincludes = [
	'.', 
	'./tiny_sha3'
]

verthash_module = Extension('verthash',
                            sources=verthashsources+['verthashmodule.c'],
                            extra_compile_args=['-std=c99'],
                            include_dirs=verthashincludes)

setup(name = 'verthash',
      version = '0.0.1',
      author_email = 'jameslovejoy1@gmail.com',
      author = 'James Lovejoy',
      url = 'https://github.com/metalicjames/verthash-pospace',
      description = 'Python bindings for Verthash proof of work function',
      ext_modules = [verthash_module])
