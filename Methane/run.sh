#!/bin/bash

#convienience function to change xml option
changeoption(){
    sed -i "s&<${1}.*>.*</${1}>&<${1}>${2}</${1}>&" $3
}
#convienience function to delete xml option
deleteoption(){
 sed -i "s&<${1}.*>.*</${1}>&&" $2
}

echo $VOTCASHARE

#runs the mapping from MD coordinates to segments and creates .sql file

xtp_map -t MD_FILES/topol.tpr -c MD_FILES/conf.gro -s system.xml -f state.sql

# you can explore the created .sql file with e.g. sqlitebrowser

# output MD and QM mappings into extract.trajectory_md.pdb and extract.trajectory_qm.pdb files

xtp_dump -e trajectory2pdb -f state.sql

#make OPTIONFILE folder, you can put all options into a single options.xml file but experience has shown, that this is better.

mkdir OPTIONFILES

#copy neighborlistfile from votca share folder to here

cp $VOTCASHARE/xtp/xml/neighborlist.xml OPTIONFILES/

changeoption constant 0.8 OPTIONFILES/neighborlist.xml

#run neighborlist calculator

xtp_run -e neighborlist -o OPTIONFILES/neighborlist.xml -f state.sql


# read in reorganisation energies stored in system.xml to state.sql

cp $VOTCASHARE/xtp/xml/einternal.xml OPTIONFILES/

xtp_run -e einternal -o OPTIONFILES/einternal.xml -f state.sql


#site energies

#setup jobfile xqmultipole has no own jobfile thus wehave to use the jobwriter

cp $VOTCASHARE/xtp/xml/jobwriter.xml OPTIONFILES/

changeoption keys "mps.chrg mps.background" OPTIONFILES/jobwriter.xml 

xtp_run -e jobwriter -o OPTIONFILES/jobwriter.xml -f state.sql -s 0
mv jobwriter.mps.background.tab mps.tab
mv jobwriter.mps.chrg.xml xqmultipole.jobs
#Only run the first 3 jobs and set the rest to complete
sed -i "s/AVAILABLE/COMPLETE/g" xqmultipole.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' xqmultipole.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' xqmultipole.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' xqmultipole.jobs


#running xqmultipole

cp $VOTCASHARE/ctp/xml/xqmultipole.xml OPTIONFILES/

changeoption job_file xqmultipole.jobs OPTIONFILES/xqmultipole.xml
changeoption emp_file mps.tab OPTIONFILES/xqmultipole.xml
#switch polarisation off for tutorial
changeoption induce 0 OPTIONFILES/xqmultipole.xml

changeoption pdb_check 0 OPTIONFILES/xqmultipole.xml
deleteoption write_chk OPTIONFILES/xqmultipole.xml
echo "Running xqmultipole, rerouting output to xqmultipole.log"
xtp_parallel -e xqmultipole -f state.sql -o OPTIONFILES/xqmultipole.xml -s 0 -t 1 -c 1000 -j "run" > xqmultipole.log

# xqmultipole has no parser to read the siteenergies into the sql file, write a python script or look at https://github.com/JensWehner/votca-scripts/blob/master/xtp/xtp_parseewald.py


#running eqm

cp $VOTCASHARE/xtp/xml/eqm.xml OPTIONFILES/
cp $VOTCASHARE/xtp/packages/mbgft.xml OPTIONFILES/
cp $VOTCASHARE/xtp/packages/xtpdft.xml OPTIONFILES/
cp $VOTCASHARE/xtp/packages/esp2multipole.xml OPTIONFILES/

changeoption dftpackage OPTIONFILES/xtpdft.xml OPTIONFILES/eqm.xml
changeoption gwbse_options OPTIONFILES/mbgft.xml OPTIONFILES/eqm.xml

changeoption threads 0 OPTIONFILES/xtpdft.xml
changeoption openmp 0 OPTIONFILES/mbgft.xml
changeoption openmp 0 OPTIONFILES/esp2multipole.xml

xtp_parallel -e eqm -o OPTIONFILES/eqm.xml -f state.sql -s 0 -j "write"
sed -i "s/AVAILABLE/COMPLETE/g" eqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' eqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' eqm.jobs

xtp_parallel -e eqm -o OPTIONFILES/eqm.xml -f state.sql -s 0 -j run -c 1 -t 1

#running iqm

cp $VOTCASHARE/xtp/xml/iqm.xml OPTIONFILES/
cp $VOTCASHARE/xtp/packages/mbgft.xml OPTIONFILES/mbgft_pair.xml
cp $VOTCASHARE/xtp/packages/xtpdft.xml OPTIONFILES/xtpdft_pair.xml
cp $VOTCASHARE/xtp/packages/bsecoupling.xml OPTIONFILES/

changeoption bsecoupling_options OPTIONFILES/bsecoupling.xml OPTIONFILES/iqm.xml
changeoption dftpackage OPTIONFILES/xtpdft_pair.xml OPTIONFILES/iqm.xml
changeoption gwbse_options OPTIONFILES/mbgft_pair.xml OPTIONFILES/iqm.xml
changeoption read_guess 1 OPTIONFILES/xtpdft_pair.xml
changeoption energy 1e-2 OPTIONFILES/xtpdft_pair.xml

changeoption threads 0 OPTIONFILES/xtpdft_pair.xml
changeoption openmp 0 OPTIONFILES/mbgft_pair.xml
changeoption openmp 0 OPTIONFILES/bsecoupling.xml
changeoption openmp 0 OPTIONFILES/bsecoupling.xml

changeoption tasks "singlets,triplets,iqm" OPTIONFILES/mbgft_pair.xml



xtp_parallel -e iqm -o OPTIONFILES/iqm.xml -f state.sql -s 0 -j "write"
sed -i "s/AVAILABLE/COMPLETE/g" iqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' iqm.jobs

xtp_parallel -e iqm -o OPTIONFILES/iqm.xml -f state.sql -s 0 -j run -c 1 -t 1













