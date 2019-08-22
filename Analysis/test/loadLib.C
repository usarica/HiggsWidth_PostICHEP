{
  gSystem->Load("$CMSSW_BASE/src/CMSDataTools/AnalysisTree/test/loadLib.C");

  gSystem->AddIncludePath("-I$CMSSW_BASE/src/CMS_HiggsWidth_TemplateMaker/Analysis/interface/");
  gSystem->AddIncludePath("-I$CMSSW_BASE/src/CMS_HiggsWidth_TemplateMaker/Analysis/test/");
  gSystem->Load("libCMS_HiggsWidth_TemplateMakerAnalysis.so");
}
