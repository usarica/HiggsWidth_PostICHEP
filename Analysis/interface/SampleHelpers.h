#ifndef SAMPLEHELPERS_H
#define SAMPLEHELPERS_H

#include "SampleHelpersCore.h"
#include "SystematicVariations.h"


namespace SampleHelpers{
  enum Channel{
    k4mu,
    k4e,
    k2e2mu,
    k4l,
    k2l2l,
    NChannels
  };

  TString getChannelName(const SampleHelpers::Channel chan);
  TString getChannelLabel(const SampleHelpers::Channel chan);
  SampleHelpers::Channel getChannelFromName(const TString channame);
  bool testChannel(SampleHelpers::Channel const& targetChannel, short const& Z1Flav, short const& Z2Flav, bool checkSS=false);

  std::vector<TString> constructSamplesList(TString strsample, float sqrts, SystematicsHelpers::SystematicVariationTypes syst=SystematicsHelpers::sNominal);
  void getSamplesList(float sqrts, std::vector<TString> const& s, std::vector<TString>& vs, SystematicsHelpers::SystematicVariationTypes syst=SystematicsHelpers::sNominal, std::vector<unsigned int>* ns=nullptr);
  void getSamplePairs(float sqrts, std::vector<TString> const& s1, std::vector<TString> const& s2, std::vector<TString>& vs1, std::vector<TString>& vs2, SystematicsHelpers::SystematicVariationTypes syst=SystematicsHelpers::sNominal);

  std::vector<TString> getXsecBranchNames();
  void addXsecBranchNames(std::vector<TString>& vars);

  template <typename T> std::vector<std::pair<T*, T*>> getZXFR_SS();
}

// Pairs are (e-EB, e-EE), (mu-EB, mu-EE)
template <typename T> std::vector<std::pair<T*, T*>> SampleHelpers::getZXFR_SS(){
  TString hname[2][2]={
    { "FR_SS_electron_EB", "FR_SS_electron_EE" },
    { "FR_SS_muon_EB", "FR_SS_muon_EE" }
  };
  TFile* finput;
  finput = TFile::Open("../data/FakeRate_SS_Moriond368.root", "read");
  std::vector<std::pair<T*, T*>> result;
  for (int f=0; f<2; f++){
    if (!(finput!=0 && finput->IsOpen())){ MELAStreamHelpers::MELAerr << "getZXFR_SS: File is not open!" << std::endl; return result; }
    else MELAStreamHelpers::MELAout << "getZXFR_SS: File opened" << std::endl;
    T* htmp[2];
    gROOT->cd();
    for (unsigned int t=0; t<2; t++) htmp[t] = (T*) finput->Get(hname[f][t]);
    result.push_back(std::pair<T*, T*>((T*) htmp[0]->Clone(Form("%s_copy", hname[f][0].Data())), (T*) htmp[1]->Clone(Form("%s_copy", hname[f][1].Data()))));
  }
  if (finput!=0 && finput->IsOpen()) finput->Close();
  return result;
}

template std::vector<std::pair<TH1F*, TH1F*>> SampleHelpers::getZXFR_SS<TH1F>();
template std::vector<std::pair<TGraphErrors*, TGraphErrors*>> SampleHelpers::getZXFR_SS<TGraphErrors>();


#endif
