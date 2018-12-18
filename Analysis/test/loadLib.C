{
  gSystem->Load("$CMSSW_BASE/src/CMSDataTools/AnalysisTree/test/loadLib.C");

  gSystem->AddIncludePath("-I$CMSSW_BASE/src/HiggsWidth_PostICHEP/Analysis/interface/");
  gSystem->AddIncludePath("-I$CMSSW_BASE/src/HiggsWidth_PostICHEP/Analysis/test/");
  gSystem->Load("libHiggsWidth_PostICHEPAnalysis.so");
}
