[![PyPi version](https://img.shields.io/pypi/v/licenseheaders.svg)](https://pypi.python.org/pypi/licenseheaders/)
[![Python compatibility](https://img.shields.io/pypi/pyversions/licenseheaders.svg)](https://pypi.python.org/pypi/licenseheaders/)

# licenseheaders

A tool to update, change or add license headers to all files of any of 
the supported types (see below) in or below some directory.

## Usage

````
usage: licenseheaders [-h] [-V] [-v] [-d DIR] [-t TMPL] [-y YEARS] [-o OWNER]
                      [-n PROJECTNAME] [-u PROJECTURL] [--enc ENCODING]
                      [--safesubst]

Python license header updater

optional arguments:
  -h, --help            show this help message and exit
  -V, --version         show program's version number and exit
  -v, --verbose         increases log verbosity (can be specified multiple
                        times)
  -d DIR, --dir DIR     The directory to recursively process (default: .).
  -t TMPL, --tmpl TMPL  Template name or file to use.
  -y YEARS, --years YEARS
                        Year or year range to use.
  -o OWNER, --owner OWNER
                        Name of copyright owner to use.
  -n PROJECTNAME, --projname PROJECTNAME
                        Name of project to use.
  -u PROJECTURL, --projurl PROJECTURL
                        Url of project to use.
  --enc ENCODING        Encoding of program files (default: utf-8)
  --safesubst           Do not raise error if template variables cannot be
                        substituted.

Known extensions: ['.java', '.scala', '.groovy', '.jape', '.js', '.sh', '.csh', '.py', '.pl', '.pl', '.py', '.xml', '.sql', '.c', '.cc', '.cpp', 'c++', '.h', '.hpp', '.rb', '.cs', '.vb', '.erl', '.src', '.config', '.schema']

If -t/--tmpl is specified, that header is added to (or existing header replaced for) all source files of known type
If -t/--tmpl is not specified byt -y/--years is specified, all years in existing header files
  are replaced with the years specified

Examples:
  # add a lgpl-v3 header and set the variables for year, owner, project and url to the given values
  # process all files in the current directory and below
  licenseheaders -t lgpl-v3 -y 2012-2014 -o ThisNiceCompany -n ProjectName -u http://the.projectname.com
  # only update the year in all existing headers
  # process all files in the current directory and below
  licenseheaders -y 2012-2015
  # only update the year in all existing headers, process the given directory
  licenseheaders -y 2012-2015 -d /dir/where/to/start/
````

If licenseheaders is installed as a package (from pypi for instance), one can interact with it as a command line tool:

````
python -m licenseheaders -t lgpl3 -c "Eager Hacker"
````

or directly:

````
licenseheaders -t lgpl3 -c "Eager Hacker"
````


# Installation

Download ``licenseheaders.py`` from ``http://github.com/johann-petrak/licenseheaders`` or :

````
pip install licenseheaders
````

## Template names and files

This library comes with a number of predefined templates. If a template name is specified
which when matched against all predefined template names matches exactly one as a substring,
then that template is used. Otherwise the name is expected to be the path of file.

If a template does not contain any variables of the form `${varname}` it is used as is.
Otherwise the program will try to replace the variable from one of the following 
sources:

- an environment variable with the same name but the prefix `LICENSE_HEADERS_` added
- the command line option that can be used to set the variable (see usage)


## Supported file types and how they are processed

java:
- extensions .java, .scala, .groovy, .jape, .js
- also used for Javascript
- only headers that use Java block comments are recognised as existing headers
- the template text will be wrapped in block comments

script:
- extensions .sh, .csh

perl:
- extension .pl

python:
- extension .py

xml:
- extension .xml

sql:
- extension .sql

c:
- extensions .c, .cc, .cpp, .c++, .h, .hpp

ruby:
- extension .rb

csharp:
- extension .cs

visualbasic:
- extension .vb

erlang:
- extensions .erl, .src, .config, .schema

## License

Licensed under the term of `MIT License`. See attached file LICENSE.txt.


