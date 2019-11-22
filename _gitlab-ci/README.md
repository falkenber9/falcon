FALCON CI
=========

This file contains some useful hints with respect to Continuous Integration (CI).

## Overview
This directory contains Docker files (in directory ``docker`` ) to build FALCON and its dependencies from scratch and in different variants (e.g. w/wo radio).

In addition, this folder holds additional scripts for full tests, to ensure that a template iq sample file is decoded as expected. The template files are included into a git submodule in directory ``testscripts``

Building containers, launching them, and starting the tests is managed by Gitlab as scripted by the file ``../.gitlab-ci.yml``

## Running tests locally
Launch the script ``run-local.sh`` to run a subset of testcases covered by ``../.gitlab-ci.yml``.

This includes:

- Build docker container for ``base_ubuntu_srsgui``
- Build docker container for ``variant_cmnalib_uhd`` (with cmnalib and with UHD)
- Build FALCON from source in **current working tree**.
- Run testscript ``testscripts/run-all.sh``
- Stay in bash inside the container for further investigation


## Updating the Templates
Occasionally, newer versions might increase accuracy of FALCON or change the output format. Consequently, the template files must be updates accordingly.

The script ``run-local.sh`` maps the host's ``/tmp`` directory into the container's ``/tmp/tmp-host`` directory. The testscripts copy their templates and test output files into that directory.

Currently this includes the following files:
```
stripped-template-falcon-dci.csv
stripped-test-falcon-dci.csv
template-falcon-stats.csv
template-owl-stats.csv
test-falcon-stats.csv
test-owl-stats.csv
stripped-test-falcon-dci2.csv
template-falcon-dci.csv
template-owl-dci.csv
test-falcon-dci.csv
test-owl-dci.csv
```

To update the templates according to the current output of the units under test, copy the following files

```
cp /tmp/test-falcon-dci.csv testdata/template-falcon-dci.csv
cp /tmp/test-falcon-stats.csv testdata/template-falcon-stats.csv

cp /tmp/test-owl-dci.csv testdata/template-owl-dci.csv
cp /tmp/test-owl-stats.csv testdata/template-owl-stats.csv
```

- Commit changes inside submodule ``testdata``.
- Commit changes to this (FALCON) repository.


## Docker Disk Space Maintenance
To reclaim disk space used by the docker images, occasinally purge the docker image registry on the gitlab ci server (and on your local machine as well, if running tests locally)

```sh
  docker image prune --all --filter "until=24h"
  docker container prune --filter "until=24h"
  docker volume prune

  #hardcore remove everything
  #docker system prune -af

```