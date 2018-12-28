#!/bin/bash
#Macro to submit calibration jobs one after the other with an automatic file decompression,
#changing the file name to a format read by calibration
#running the calibration
#removing defined files for intermediate step
#Input: 
# $1=slot from which we start
# $2=slot from which we end
# $3=layer which we calibrate
# $4= name of file with a list of directories (format as in report from run2)
# $5=path in which we have all the directories with calibration data
# $6=first file which we calibrate per a given position
# $7=last file which we calibrate per a given position
# $8=file with TOT correction
# $9=Number of running period, depending on it we use different json file with detector configuration
#  and different run number
#example:
#source run_calibAll.sh 3 5 3 FileListRun2.txt /media/silarski/backup/JPETRefII_data/L3/ 1 1 TOT_run1_2.root 2
ScinStart=$1
ScinStop=$2
Lay=$3
FileName=$4
Filepath=$5
NumFilStart=$6
NumFilStop=$7
TOTFile=$8
RunPeriod=$9
#
#Here specify which kind of files You want to analyse, root or hld
#
Tfile="hld"
#
if [[ $RunPeriod -eq 1 ]];
then
json="large_barrel.json"
RunJson=44
UserParams="userParams.json"
fi
if [[ $RunPeriod -gt 1 ]]; 
then
json="detectorSetupRun2345.json"
RunJson=2
UserParams="userParams.json"
fi
echo "##############################################################"
echo "--------------------------------------------------------------"
echo "Using the following settings: -l $json -i $RunJson -c $TOTFile"
echo "--------------------------------------------------------------"
echo "Bash version ${BASH_VERSION}..."
while read linia
    do 
  i=`echo $linia | awk -F":" '{print $1}' | awk -F"_" '{print $2}'`
  dir=`echo $linia | awk -F":" '{print $2}'| awk '{print $1}'`
  j=`echo $linia | awk -F":" '{print $1}' | awk -F"_" '{print $1}'`
  if [[ ($i -ge $ScinStart) ]] && [[ ($i -le $ScinStop) ]] && [[ ($j -eq $Lay) ]];
  then
      s=$i
      PackedId=$(($((100*Lay)) + i))
      zonk=`grep TimeWindowCreator_MainStrip $UserParams | awk -F"\"" '{print $4}'`
      echo $zonk
      echo $UserParams
      sed -i -e "s/$zonk/$PackedId/g" "$UserParams"    
      echo "Packed slot ID= $PackedId"
      echo "--------------------------------------------------------------"
      echo "Submitting job for Slot $i Layer $j"
      echo "--------------------------------------------------------------"
      ile=`ls -ltr $Filepath$dir/*.xz | wc -l`
      echo "Number of files per position=$ile"
      for ((ii=$NumFilStart+1; ii<=$NumFilStop+1; ii++)); do
	if [[ ($ii -gt $ile+1) ]];
	then
	 break   
	fi    
      list=`echo ls -1 $5$dir/*.xz | awk -F"dabc_" '{print $'"${ii}"'}' | awk '{print $1}'`
      FilN=`echo ls -1 $5$dir/*.xz | awk -F"dabc_" '{print $'"${ii}"'}' | awk '{print $1}' |awk -F"." '{print $1}'`
       OldFileName="$Filepath$dir/dabc_$FilN"
       if [[ ($i -gt 9) ]];
	then   
        NewFileName="$Filepath$dir/File${ii}_Layer${Lay}_slot${s}"
       else
	NewFileName="$Filepath$dir/File${ii}_Layer${Lay}_slot0${s}"
       fi 	   
       Hdir=`pwd`
      if [[ "$Tfile" == "hld" ]];
      then 
       if [[ -f "$NewFileName.$Tfile" ]];
	then
	 echo "$NewFileName.$Tfile exists we do not decompress the file"  
	else
	 unxz -k $Filepath$dir/dabc_$FilN.$Tfile.xz
	 mv $OldFileName.$Tfile $NewFileName.$Tfile
	fi 
         echo "##############################################################"
	 echo "Start of analysis of File $NewFileName.$Tfile ($Filepath$dir/dabc_$FilN.$Tfile)"
	 echo "##############################################################"
         echo "./TimeCalibration.x -t hld -f $NewFileName.$Tfile -i $RunJson -c $TOTFile -u $UserParams"
	 ./TimeCalibration_dev.x -t hld -f $NewFileName.$Tfile -i $RunJson -c $TOTFile -u $UserParams
	 rm $NewFileName.hld.root
	 rm $NewFileName.tslot.raw.root
	 rm $NewFileName.raw.sig.root
	 rm $NewFileName.phys.sig.root
	 rm $NewFileName.calib.root
      else
	./TimeCalibration_dev.x -t $Tfile -f $NewFileName.$Tfile -i $RunJson -c $TOTFile -u $UserParams
	 rm $NewFileName.hld
         rm $NewFileName.hld.root
         rm $NewFileName.tslot.raw.root
         rm $NewFileName.raw.sig.root
         rm $NewFileName.phys.sig.root
         rm $NewFileName.hits.root
         rm $NewFileName.calib.root
      fi  
     done
      echo "DONE WITH SLOT $i LAYER $j"
  fi
done < $FileName

