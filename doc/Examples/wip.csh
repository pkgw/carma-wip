#! /bin/csh -f
#  Script to generate WIP hardcopies.
#  It assumes WIPBIN points to the directory where the WIP binary resides.
#  This script may be useful if the printer is not a PostScript device;
#  simply change the device name in the command below to the driver desired.
#  If the printing device is PostScript, why not try setting `doplot' to
#  true in wip.tex (see the file $WIPSRC/00README)?
#
#
set wipdevice = "/vps"
set binpath = "../.."
unset fileList
#
while ($#argv)
  if ("$argv[1]" == "-p") then
    shift
    set binpath = "$argv[1]"
  else if ("$argv[1]" == "-d") then
    shift
    set wipdevice = "$argv[1]"
  else if ($?fileList == 0) then
    set fileList = ( $argv[*] )
    break
  else
    echo "Illegal option -- $argv[1] -- provided."
    break
  endif
  shift
end
#
if ($?fileList == 0) then
  echo "Usage: $0 [-p binpath] [-d device] file [files...]"
  echo 'where binpath is the path to the WIP binary;'
  echo 'device is a legal WIP plotting device; and'
  echo 'file.wip is the name of the WIP plot file.'
  exit 1
endif
#
foreach i ($fileList)
    if ($i:e == 'wip') set i = $i:r
    echo $binpath/wip -x -d ${i}.ps${wipdevice} $i.wip
    $binpath/wip -x -d ${i}.ps${wipdevice} $i.wip
end
exit 0
