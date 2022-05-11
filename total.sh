#!/bin/bash

#APEX evaluation
# ./run.sh /home/zzy/dataset/generate_random_ycsb.dat binary uint64 100000000 200000000 insert 1 apex 1 0 > apex-ycsb-insert.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longtitudes.dat binary uint64 10000000 210000000 insert 1 apex 1 0 > apex-longitudes-insert.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 10000000 410000000 insert 1 apex 1 0 > apex-longlat-insert.data
#./run.sh /home/zzy/dataset/lognormal.dat binary uint64 10000000 160000000 insert 1 apex 1 0 > apex-lognormal-insert.data

./run.sh /home/zzy/dataset/generate_random_ycsb.dat binary uint64 10000000 410000000 search 1 apex 1 0 > apex-ycsb-search.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longtitudes.dat binary uint64 10000000 210000000 search 0 apex 1 0 > apex-longitudes-search.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 100000000 400000000 search 1 apex 1 0 > apex-longlat-search.data
# ./run.sh /home/zzy/dataset/lognormal.dat binary uint64 10000000 160000000 search 1 apex 1 0 > apex-lognormal-search.data

# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 10000000 410000000 mixed 1 apex 1 0.2 > apex-ycsb-mixed20.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 10000000 410000000 mixed 1 apex 1 0.4 > apex-ycsb-mixed40.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 10000000 410000000 mixed 1 apex 1 0.6 > apex-ycsb-mixed60.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 10000000 410000000 mixed 1 apex 1 0.8 > apex-ycsb-mixed80.data
# ./run.sh /home/zzy/dataset/generate_random_osm_longlat.dat binary uint64 10000000 410000000 mixed 1 apex 1 1.0 > apex-ycsb-mixed100.data