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

rm state.hdf5
#runs the mapping from MD coordinates to segments and creates .hdf5 file

xtp_map -v -t MD_FILES/newfile.data -c MD_FILES/traj.dump -s system.xml -f state.hdf5 -i 99

# you can explore the created .hdf5 file with e.g. hdfview or HDFCompass

# output MD and QM mappings into extract.trajectory_md.pdb and extract.trajectory_qm.pdb files
cp $VOTCASHARE/xtp/xml/mapchecker.xml OPTIONFILES/
changeoption map_file system.xml OPTIONFILES/mapchecker.xml
xtp_run -e mapchecker -o OPTIONFILES/mapchecker.xml -f state.hdf5


#copy neighborlistfile from votca share folder to here

cp $VOTCASHARE/xtp/xml/neighborlist.xml OPTIONFILES/

changeoption constant 0.6 OPTIONFILES/neighborlist.xml

#run neighborlist calculator

xtp_run -e neighborlist -o OPTIONFILES/neighborlist.xml -f state.hdf5 -t 4

# read in reorganisation energies stored in system.xml to state.hdf5

cp $VOTCASHARE/xtp/xml/einternal.xml OPTIONFILES/

xtp_run -e einternal -o OPTIONFILES/einternal.xml -f state.hdf5


#site energies
#we just prepared an optionfile for the mm calculation as changing qmmm.xml for mm is too difficult with bash


cp $VOTCASHARE/xtp/packages/polar.xml OPTIONFILES/
xtp_parallel -e qmmm -o OPTIONFILES/qmmm_mm.xml -f state.hdf5 -j "write"
sed -i "s/AVAILABLE/COMPLETE/g" qmmm_mm_jobs.xml
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' qmmm_mm_jobs.xml
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' qmmm_mm_jobs.xml
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' qmmm_mm_jobs.xml
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' qmmm_mm_jobs.xml

xtp_parallel -e qmmm -o OPTIONFILES/qmmm_mm.xml -f state.hdf5 -j "run"

xtp_parallel -e qmmm -o OPTIONFILES/qmmm_mm.xml -f state.hdf5 -j "read"
#running eanalyze

cp $VOTCASHARE/xtp/xml/eanalyze.xml OPTIONFILES/

xtp_run -e eanalyze -o OPTIONFILES/eanalyze.xml -f state.hdf5

#running eqm
#eqm runs qm calculations for each segment in the hdf5 file, it consists of three stages first writing a jobfile, then running the calculations, if necessary 
# on multiple machines and then reading them into the .hdf5 file
echo "Running eQM"

cp $VOTCASHARE/xtp/xml/eqm.xml OPTIONFILES/
changeoption ranges full OPTIONFILES/eqm.xml
changeoption map_file system.xml OPTIONFILES/eqm.xml

xtp_parallel -e eqm -o OPTIONFILES/eqm.xml -f state.hdf5 -s 0 -j "write"
sed -i "s/AVAILABLE/COMPLETE/g" eqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' eqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' eqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' eqm.jobs

xtp_parallel -e eqm -o OPTIONFILES/eqm.xml -f state.hdf5 -s 0 -j run -c 1 -t 1

#running iqm
#iqm runs qm calculations for each pair in the hdf5 file, it consists of three stages first writing a jobfile, then running the calculations, if necessary 
# on multiple machines and then reading them into the .hdf5 file
echo "Running iQM"

cp $VOTCASHARE/xtp/xml/iqm.xml OPTIONFILES/
changeoption map_file system.xml OPTIONFILES/iqm.xml

# Append the states to read to iqm.xml
TAIL=$(tail -n 2 OPTIONFILES/iqm.xml)
head -n -2 OPTIONFILES/iqm.xml > OPTIONFILES/tmp

cat >> OPTIONFILES/tmp <<- EOM
<readjobfile help="which states to read into the jobfile for each segment type">
     <singlet>thiophene:s1</singlet>
     <triplet>thiophene:t1</triplet>
     <electron>thiophene:e1</electron>
     <hole>thiophene:h1</hole>
</readjobfile>
EOM

echo $TAIL >> OPTIONFILES/tmp
mv OPTIONFILES/tmp  OPTIONFILES/iqm.xml

xtp_parallel -e iqm -o OPTIONFILES/iqm.xml -f state.hdf5 -s 0 -j "write"
sed -i "s/AVAILABLE/COMPLETE/g" iqm.jobs
sed -i '0,/COMPLETE/s/COMPLETE/AVAILABLE/' iqm.jobs

xtp_parallel -e iqm -o OPTIONFILES/iqm.xml -f state.hdf5 -s 0 -j run -c 1 -t 1

xtp_parallel -e iqm -o OPTIONFILES/iqm.xml -f state.hdf5 -j "read"

#running ianalyze

cp $VOTCASHARE/xtp/xml/ianalyze.xml OPTIONFILES/

xtp_run -e ianalyze -o OPTIONFILES/ianalyze.xml -f state.hdf5

#running qmmm 

cp $VOTCASHARE/xtp/packages/gwbse.xml OPTIONFILES/gwbse_qmmm.xml
cp $VOTCASHARE/xtp/packages/xtpdft.xml OPTIONFILES/xtpdft_qmmm.xml

xtp_parallel -e qmmm -o OPTIONFILES/qmmm.xml -f state.hdf5 -j run
