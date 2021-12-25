#!/bin/bash

sudo docker run -v $PWD:/sandbox:Z abouteiller/mpi-ft-ulfm mpirun --with-ft ulfm --map-by :OVERSUBSCRIBE --mca btl tcp,self "$@"
