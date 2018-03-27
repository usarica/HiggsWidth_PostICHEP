#ifndef MAKEFINALTEMPLATES_VV_H
#define MAKEFINALTEMPLATES_VV_H

#include "common_includes.h"
#include "CheckSetTemplatesCategoryScheme.h"


// Constants to affect the template code
#ifndef outputdir_def
#define outputdir_def
const TString user_output_dir = "output/";
#endif

typedef VVProcessHandler ProcessHandleType;


bool getFile(
  const Channel channel, const Category category, const ACHypothesis hypo, const SystematicVariationTypes syst,
  const unsigned int istage, const TString fixedDate,
  ProcessHandler const* thePerProcessHandle,
  std::vector<TFile*>& finputList
){
  const TString strChannel = getChannelName(channel);
  const TString strCategory = getCategoryName(category);
  const TString strACHypo = getACHypothesisName(hypo);
  const TString strStage = Form("Stage%i", istage);
  const TString strSystematics = getSystematicsName(syst);
  const TString strSystematicsOutput = getSystematicsCombineName(category, channel, thePerProcessHandle->getProcessType(), syst);

  // Setup the input directories
  TString sqrtsDir = Form("LHC_%iTeV/", theSqrts);
  TString strdate = todaysdate();
  if (fixedDate!="") strdate=fixedDate;
  TString cinput_common = user_output_dir + sqrtsDir + "Templates/" + strdate + "/FinalTemplates/" + strStage + "/" + strACHypo;

  TString cinput = Form(
    "%s/HtoZZ%s_%s_FinalTemplates_%s_%s%s",
    cinput_common.Data(),
    strChannel.Data(), strCategory.Data(),
    thePerProcessHandle->getProcessName().Data(),
    strSystematicsOutput.Data(),
    ".root"
  );

  if (gSystem->AccessPathName(cinput)){
    MELAerr << "getFilesAndTrees::File " << cinput << " is not found! Run " << strStage << " functions first." << endl;
    return false;
  }
  if (cinput!=""){
    TFile* finput = TFile::Open(cinput, "read");
    if (finput){
      if (!finput->IsZombie()){
        MELAout << "getFile: Opening file " << cinput << endl;
        finputList.push_back(finput);
      }
      else if (finput->IsOpen()){
        finput->Close();
        return false;
      }
      else return false;
    }
  }
  return true;
}


void getControl2DXSlices(
  TDirectory* rootdir, TFile* foutput,
  ProcessHandleType const*& thePerProcessHandle, const ACHypothesis hypo,
  std::vector<ExtendedBinning> const& KDbinning,
  std::vector<TH3F*> const& hTemplates
);


void makeFinalTemplates_VV(const Channel channel, const ACHypothesis hypo, const SystematicVariationTypes syst, CategorizationHelpers::MassRegion massregion, const unsigned int istage=1, const TString fixedDate=""){
  const ProcessHandler::ProcessType proctype=ProcessHandler::kVV;
  if (channel==NChannels) return;
  ProcessHandleType const* outputProcessHandle=(ProcessHandleType const*) getProcessHandlerPerMassRegion(proctype, massregion);
  if (!outputProcessHandle) return;

  vector<ProcessHandler::ProcessType> inputproctypes = { ProcessHandler::kVBF, ProcessHandler::kZH, ProcessHandler::kWH };
  const unsigned int ninputproctypes=inputproctypes.size();

  const TString strChannel = getChannelName(channel);
  const TString strACHypo = getACHypothesisName(hypo);
  const TString strStage = Form("Stage%i", istage);
  const TString strSystematics = getSystematicsName(syst);

  // Setup the output directories
  TString sqrtsDir = Form("LHC_%iTeV/", theSqrts);
  TString strdate = todaysdate();
  if (fixedDate!="") strdate=fixedDate;
  TString coutput_common = user_output_dir + sqrtsDir + "Templates/" + strdate + "/FinalTemplates/" + strStage + "/" + strACHypo;
  gSystem->Exec("mkdir -p " + coutput_common);

  TDirectory* rootdir=gDirectory;

  TString OUTPUT_LOG_NAME = Form(
    "%s/HtoZZ%s_%s_FinalTemplates_%s_%s_%s",
    coutput_common.Data(),
    strChannel.Data(), "AllCategories",
    outputProcessHandle->getProcessName().Data(),
    strSystematics.Data(),
    ".log"
  );
  MELAout.open(OUTPUT_LOG_NAME.Data());

  vector<Category> catList = getAllowedCategories(globalCategorizationScheme);
  for (auto& cat:catList){
    rootdir->cd();
    if (!systematicAllowed(cat, channel, proctype, syst, "")) continue;
    const TString strCategory = getCategoryName(cat);
    const TString strSystematicsOutput = getSystematicsCombineName(cat, channel, outputProcessHandle->getProcessType(), syst);

    vector<ProcessHandleType::TemplateType> tplset;
    vector<ProcessHandleType::HypothesisType> hyposet = outputProcessHandle->getHypothesesForACHypothesis(kSM);
    if (hypo!=kSM){
      vector<ProcessHandleType::HypothesisType> hyposet_tmp = outputProcessHandle->getHypothesesForACHypothesis(hypo);
      for (ProcessHandleType::HypothesisType& v:hyposet_tmp) hyposet.push_back(v);
    }
    for(auto& hypotype:hyposet) tplset.push_back(ProcessHandleType::castIntToTemplateType(ProcessHandleType::castHypothesisTypeToInt(hypotype)));
    const unsigned int ntpls=tplset.size();

    vector<TFile*> finputList;
    for (unsigned int ip=0; ip<ninputproctypes; ip++){
      auto& inputproctype=inputproctypes.at(ip);
      ProcessHandleType const* inputProcessHandle=(ProcessHandleType const*) getProcessHandlerPerMassRegion(inputproctype, massregion);
      if (
        !getFile(
          channel, cat, hypo, syst,
          istage, fixedDate,
          inputProcessHandle,
          finputList
        )
        ) continue;
    }

    TString coutput = Form(
      "%s/HtoZZ%s_%s_FinalTemplates_%s_%s%s",
      coutput_common.Data(),
      strChannel.Data(), strCategory.Data(),
      outputProcessHandle->getProcessName().Data(),
      strSystematicsOutput.Data(),
      ".root"
    );
    MELAout << "Opening output file " << coutput << endl;
    TFile* foutput = TFile::Open(coutput, "recreate");

    vector<TH2F*> htpls_2D; htpls_2D.reserve(ntpls);
    vector<TH3F*> htpls_3D; htpls_3D.reserve(ntpls);
    for (unsigned int ip=0; ip<ninputproctypes; ip++){
      auto& inputproctype=inputproctypes.at(ip);
      ProcessHandleType const* inputProcessHandle=(ProcessHandleType const*) getProcessHandlerPerMassRegion(inputproctype, massregion);
      TFile*& finput=finputList.at(ip);
      for (auto& tpltype:tplset){
        TString intplname = inputProcessHandle->getTemplateName(tpltype);
        TH2F* htmp_2D;
        TH3F* htmp_3D;
        finput->GetObject(intplname, htmp_2D);
        finput->GetObject(intplname, htmp_3D);

        foutput->cd();
        TString outtplname = outputProcessHandle->getTemplateName(tpltype);
        if (htmp_2D){
          TH2F* htplfound=nullptr;
          for (auto& htmp:htpls_2D){
            if (htmp->GetName()==outtplname){
              htplfound=htmp;
              break;
            }
          }
          if (htplfound) htplfound->Add(htmp_2D);
          else{ htplfound = new TH2F(*htmp_2D); htplfound->SetName(outtplname); htplfound->SetTitle(outputProcessHandle->getProcessLabel(tpltype, hypo)); htpls_2D.push_back(htplfound); }
        }
        if (htmp_3D){
          TH3F* htplfound=nullptr;
          for (auto& htmp:htpls_3D){
            if (htmp->GetName()==outtplname){
              htplfound=htmp;
              break;
            }
          }
          if (htplfound) htplfound->Add(htmp_3D);
          else{ htplfound = new TH3F(*htmp_3D); htplfound->SetName(outtplname); htplfound->SetTitle(outputProcessHandle->getProcessLabel(tpltype, hypo)); htpls_3D.push_back(htplfound); }
        }
      }
      rootdir->cd();
    }
    for (TFile*& finput:finputList) finput->Close();

    rootdir->cd();

    // Make 2D slices
    {
      vector<TString> KDset; KDset.push_back("ZZMass"); { vector<TString> KDset2=getACHypothesisKDNameSet(hypo, cat, massregion); appendVector(KDset, KDset2); }
      vector<ExtendedBinning> KDbinning;
      for (auto& KDname:KDset) KDbinning.push_back(getDiscriminantFineBinning(channel, cat, KDname, massregion));
      getControl2DXSlices(rootdir, foutput, outputProcessHandle, hypo, KDbinning, htpls_3D);
    }

    for (auto& htpl:htpls_2D){ foutput->WriteTObject(htpl); delete htpl; }
    for (auto& htpl:htpls_3D){ foutput->WriteTObject(htpl); delete htpl; }
    foutput->Close();
  }
  MELAout.close();
}

void getControl2DXSlices(
  TDirectory* rootdir, TFile* foutput,
  ProcessHandleType const*& thePerProcessHandle, const ACHypothesis hypo,
  std::vector<ExtendedBinning> const& KDbinning,
  std::vector<TH3F*> const& hTemplates
){
  foutput->cd();
  vector<TH3F*> hList;
  for (auto& htpl:hTemplates){
    TString tplname=htpl->GetName();
    TH3F* htmp = new TH3F(*htpl); htmp->SetName(tplname+"_tmp");
    multiplyBinWidth(htmp);
    hList.push_back(htmp);
  }
  thePerProcessHandle->recombineRegularTemplatesToTemplatesWithPhase(hList, hypo);
  for (unsigned int t=0; t<hTemplates.size(); t++){
    ProcessHandleType::TemplateType tpltype = ProcessHandleType::castIntToTemplateType(t);
    bool isPhaseHist = (ProcessHandleType::isInterferenceContribution(tpltype));
    auto*& htpl=hList.at(t);
    TString tplname=hTemplates.at(t)->GetName();
    TString tpltitle=hTemplates.at(t)->GetTitle();
    TString tplxtitle=hTemplates.at(t)->GetXaxis()->GetTitle();
    TString tplytitle=hTemplates.at(t)->GetYaxis()->GetTitle();
    TString tplztitle=hTemplates.at(t)->GetZaxis()->GetTitle();
    TDirectory* savedir=foutput->mkdir(tplname+"_control_" + tplxtitle + "_slices");
    savedir->cd();
    for (int ix=1; ix<=htpl->GetNbinsX(); ix++){
      TString slicename=tplname+(isPhaseHist ? "_Phase_" : "_")+Form("%s_Slice%i", KDbinning.at(0).getLabel().Data(), ix);
      TString slicetitle = tplxtitle + Form(": [%.1f, %.1f]", KDbinning.at(0).getBinLowEdge(ix-1), KDbinning.at(0).getBinLowEdge(ix));
      TH2F* hSlice = getHistogramSlice(htpl, 1, 2, ix, ix, slicename);
      if (!isPhaseHist){
        double integral = getHistogramIntegralAndError(hSlice, 1, hSlice->GetNbinsX(), 1, hSlice->GetNbinsY(), false, nullptr);
        if (integral!=0.) hSlice->Scale(1./integral);
        divideBinWidth(hSlice);
      }
      hSlice->SetOption("colz");
      hSlice->SetTitle(slicetitle);
      hSlice->GetXaxis()->SetTitle(tplytitle);
      hSlice->GetYaxis()->SetTitle(tplztitle);
      savedir->WriteTObject(hSlice);
      delete hSlice;
    }
    foutput->cd();
  }
  for (auto& h:hList) delete h;
  rootdir->cd();
}


#endif