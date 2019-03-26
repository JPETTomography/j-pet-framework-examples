#!/bin/bash
#=======================================================================================
# README
#=======================================================================================       
#Macro to submit calibration jobs one after the other with an automatic file decompression,
#changing the file name to a format read by calibration
#running the calibration
#removing defined files for intermediate step
#Input: 
# $1=slot from which we start
# $2=slot from which we end
# $3=layer which we calibrate
# $4= name of file with a list of directories (format as in report from run2)
# $5=path in which we have all the directories with calibration data or all the files already analysed
# before in case of root files analysis
# NumFilStart=first file which we calibrate per a given position
# $6=last file which we calibrate per a given position
# $7=Number of running period, depending on it we use different json file with detector configuration
#  and different run number (for run 6 use 61 for A part, 62 for B part, 63 for C and 64 for D
# $8=type of the file to be analyzed (hld or root,if xz the macro will extrct the hld file with hld flag)
# $9=Otput directory for the final histograms
# ./TimeCalibration_iter.x is a version of the exec which starts analysis from hld files (used with Tfile="hld")
# ./TimeCalibration_iterroot.x is a version of the exec which starts analysis from hits level (used with Tfile="root")
# it sshould be compiled and renamed by user
# The userParams and DetectorSetup are defined as specific files for each run, e.g. for run 4 we have to use
# the userParamsRun4.json and detectorSetupRun4.json. Here we assume that the detectorSetup, TOT, TDC nonlinearity are
# in CalibrationFiles/X_RUN/ as given from the Petwiki
# IT IS THE USER WHO SHOULD PROVIDE PATH TO THESE FILES (if conditions below the variables definition) 
# The Unzipper specifies the program (together with options) used  to unzip the files (xz and bz2 require different programs)
# example:
# source run_calibAll_iter.sh 3 5 3 FileListRun2.txt /media/silarski/backup/JPETRefII_data/L3/ 1 2 hld /media/silarski/backup/
#
ScinStart=$1
ScinStop=$2
Lay=$3
FileName=$4
Filepath=$5
NumFilStart=1
NumFilStop=$6
RunJson=$7
Tfile=$8
OutDir=$9
#FileExt="hld.xz"
#FileExt2="hits.root"
#
DetectorSetup="CalibrationFiles/${RunJson}_RUN/detectorSetupRun$RunJson.json"
UserParams="userParamsRun${RunJson}.json"
#
if [[ $RunJson -eq 61 ]];
then
DetectorSetup="CalibrationFiles/6_RUN/detectorSetupRun6A.json"
UserParams="userParamsRun6A.json"
fi
if [[ $RunJson -eq 62 ]];
then
DetectorSetup="CalibrationFiles/6_RUN/detectorSetupRun6B.json"
UserParams="userParamsRun6B.json"
fi
if [[ $RunJson -eq 63 ]];
then
DetectorSetup="CalibrationFiles/6_RUN/detectorSetupRun6C.json"
UserParams="userParamsRun6C.json"
fi
if [[ $RunJson -eq 64 ]];
then
DetectorSetup="CalibrationFiles/6_RUN/detectorSetupRun6D.json"
UserParams="userParamsRun6D.json"
fi
#
echo "##############################################################"
echo "--------------------------------------------------------------"
echo "Using the following userPams.json file: $UserParams "
echo "Using the following DetectorSetup file: $DetectorSetup "
echo "Using the following extension: $Tfile"
echo "--------------------------------------------------------------"
echo "Bash version ${BASH_VERSION}..."
pacan=`pwd`
echo $pacan
#
while read linia
    do 
  i=`echo $linia | awk -F":" '{print $1}' | awk -F"_" '{print $2}'`
  dir=`echo $linia | awk -F":" '{print $2}'| awk '{print $1}'`
  j=`echo $linia | awk -F":" '{print $1}' | awk -F"_" '{print $1}'`
  if [[ ($i -ge $ScinStart) ]] && [[ ($i -le $ScinStop) ]] && [[ ($j -eq $Lay) ]];
  then
      s=$i
      PackedId=$(($((100*Lay)) + i))
      zonk=`grep TimeWindowCreator_MainStrip_int $UserParams | awk -F"\"" '{print $4}'`
      zonk1=`grep TimeWindowCreator_MainStrip_int $UserParams| awk -F"[ ]" '{print $2}'`
      echo $zonk
      echo $UserParams
      tekst="\"TimeWindowCreator_MainStrip_int\"":"\"${PackedId}\","
      echo $zonk1
      echo $tekst
#
      sed -i -e "s/$zonk1/$tekst/g" "$UserParams"
      echo "Packed slot ID= $PackedId"
      echo "--------------------------------------------------------------"
      echo "Submitting job for Slot $i Layer $j"
      echo "--------------------------------------------------------------"
     if [[ "$Tfile" == "hld" ]];
      then 
	if [[ `ls -ltr $Filepath$dir/*.xz` ]];
        then
	    FileExt="hld.xz"
	    Unzipper=$(echo "xz -kd")
	else
	 if [[ `ls -ltr $Filepath$dir/*.bz2` ]];
         then
	  FileExt="hld.bz2"
	  Unzipper=$(echo "lbzip2 -k -d -c")
	 fi
	fi    
       ile=`ls -ltr $Filepath$dir/*.$FileExt | wc -l`
       echo "Number of files per position=$ile"
       for ((ii=$NumFilStart; ii<=$NumFilStop; ii++)); do
#	   echo $ii
	if [[ ($ii > $ile) ]];
	then
	 break   
	fi    
	agh=$(($ii+1))
#      list=`echo ls -1 $Filepath$dir/*.${FileExt}| awk -F"dabc_" '{print $'"${ii}"'}' | awk '{print $1}'`
      FilN=`echo ls -1 $Filepath$dir/*.${FileExt}| awk -F"dabc_" '{print $'"${agh}"'}' | awk '{print $1}' |awk -F"." '{print $1}'`
       OldFileName="$Filepath$dir/dabc_$FilN.$FileExt"
#       echo $FilN
#       echo $OldFileName
#       echo $agh
       if [[ ($i -gt 9) ]];
	then   
	 NewFileName="Layer${Lay}_slot${s}_${ii}"
       else
	 NewFileName="Layer${Lay}_slot0${s}_${ii}"
       fi
####
       if [[ -f " $NewFileName.$Tfile" ]];
	then
	 echo "$NewFileName.$Tfile exists we do not decompress the file"  
       else
	  if [[ -f "$OldFileName" ]];
	  then
	   echo "Copy and unzip $OldFileName.$Tfile.xz to $NewFileName.$Tfile"   
	   $Unzipper $OldFileName --stdout >> $NewFileName.$Tfile
	  fi
#	    
	fi 
         echo "##############################################################"
	 echo "Start of analysis of File $NewFileName.$Tfile ($Filepath$dir/dabc_$FilN.$Tfile)"
	 echo "##############################################################"
         echo "./TimeCalibration_iter.x -t hld -f $NewFileName.$Tfile -i $RunJson -u $UserParams -b -o $OutDir -p conf_trb3.xml -l $DetectorSetup"
	 ./TimeCalibration_iter.x -t hld -f $NewFileName.$Tfile -i $RunJson -u $UserParams -b -o $OutDir p conf_trb3.xml -l $DetectorSetup
	 rm $NewFileName.hld
	 rm $NewFileName.hld.root
	 rm $OutDir$NewFileName.tslot.raw.root
	 rm $OutDir$NewFileName.raw.sig.root
	 rm $OutDir$NewFileName.phys.sig.root
#	 rm $OutDir$NewFileName.hits.root
#	 rm $OutDir$NewFileName.calib.root
       done
     else
#
#If we work with files already preselected
#  
        ile=`ls -ltr $Filepath/Layer${Lay}_slot0${s}*${Tfile} | wc -l`
         echo "##############################################################"
         echo "I have found $ile files for Layer ${Lay} and slot ${s}"
         echo "##############################################################"
        for ((k=$NumFilStart; k<=$NumFilStop; k++)); do
          if [[ ($k -gt $ile) ]];
          then
           break
          fi
	 NewFileName="${Filepath}/Layer${Lay}_slot0${s}_${k}.${Tfile}"
         echo "##############################################################"
	 echo "Start of analysis of File $NewFileName"
	 echo "##############################################################"
         echo "./TimeCalibration_iterroot.x -t root -f $NewFileName -i $RunJson -u $UserParams -b -o $OutDir -p conf_trb3.xml -l $DetectorSetup"
	 ./TimeCalibration_iterroot.x -t root -f $NewFileName -i $RunJson -u $UserParams -b -o $OutDir -p conf_trb3.xml -l $DetectorSetup
	done
      fi
###
      echo "DONE WITH SLOT $i LAYER $j"
  fi
done < $FileName

