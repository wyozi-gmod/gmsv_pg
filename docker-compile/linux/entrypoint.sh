#!/bin/bash

set -e

MY_WORKSPACE=$PWD

# copy project files from workingdir to project root
mkdir /project
cp -r . /project

# premake
cd /project/pg
premake5 gmake

cd /project/pg/project
make

cp /project/pg/bin/gmsv_pg_linux.dll $MY_WORKSPACE/gmsv_pg_linux.dll