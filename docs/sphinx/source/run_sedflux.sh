#! /bin/sh
#PBS -l nodes=1:ppn=1
 
RUN_DIR=$HOME/my_simulation_dir
INIT_FILE=my_init_file.kvf
 
cd $RUN_DIR && \
sedflux -3 --init-file=$INIT_FILE --msg="Brief description of simulation"

