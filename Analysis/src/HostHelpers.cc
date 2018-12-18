#include "HostHelpers.h"
#include <iostream>


TString HostHelpers::GetCJLSTSamplesDirectory(const TString proddir){
  const Hosts host = GetHostLocation();

  TString theDir;
  if (host==kLXPLUS) theDir = "/eos/user/u/usarica/CJLST/4l";
  else if (host==kMARCC) theDir = "/work-zfs/lhc/CJLSTtrees";

  if (theDir!=""){
    TString testdir = theDir + "/" + proddir;
    if (DirectoryExists(testdir.Data())) return theDir;
    else if (host==kLXPLUS){
      theDir = "root://lxcms03//data3/Higgs";
      return theDir; // There is no easy way to check if proddir exists over xrootd
    }
  }

  std::cout << "CJLST samples directory could not be found!" << std::endl;
  assert(0);
  return "";
}
