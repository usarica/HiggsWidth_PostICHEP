#include "common_includes.h"


#ifndef doDebugKD
#define doDebugKD true
#endif
#ifndef doDebugKDExt
#define doDebugKDExt false
#endif

class EventAnalyzer : public BaseTreeLooper{
protected:
  Channel channel;
  Category category;

  bool runEvent(CJLSTTree* tree, float const& externalWgt, SimpleEntry& product);

public:
  float infTrackingVal;
  float supTrackingVal;

  EventAnalyzer(Channel channel_, Category category_) : BaseTreeLooper(), channel(channel_), category(category_){}
  EventAnalyzer(CJLSTTree* inTree, Channel channel_, Category category_) : BaseTreeLooper(inTree), channel(channel_), category(category_){}
  EventAnalyzer(std::vector<CJLSTTree*> const& inTreeList, Channel channel_, Category category_) : BaseTreeLooper(inTreeList), channel(channel_), category(category_){}
  EventAnalyzer(CJLSTSet const* inTreeSet, Channel channel_, Category category_) : BaseTreeLooper(inTreeSet), channel(channel_), category(category_){}

};
bool EventAnalyzer::runEvent(CJLSTTree* tree, float const& externalWgt, SimpleEntry& product){
  bool validProducts=(tree!=nullptr);
  if (validProducts){
    // Get main observables
    float& ZZMass = *(valfloats["ZZMass"]);
    //float& GenHMass = *(valfloats["GenHMass"]);
    product.trackingval = ZZMass; product.setNamedVal("ZZMass", ZZMass);

    // Check if trackingval is between the range requested
    validProducts &= (product.trackingval>=this->infTrackingVal && product.trackingval<this->supTrackingVal);

    // Construct the weights
    float wgt = externalWgt;
    wgt *= (*(valfloats["dataMCWeight"]))*(*(valfloats["trigEffWeight"]))*(*(valfloats["PUWeight"]))*(*(valfloats["genHEPMCweight"]));
    for (auto rewgt_it=Rewgtbuilders.cbegin(); rewgt_it!=Rewgtbuilders.cend(); rewgt_it++){
      auto& rewgtBuilder = rewgt_it->second;
      if (rewgt_it->first=="MELARewgt"){
        float mela_wgt_sum = rewgtBuilder->getSumPostThresholdWeights(tree);
        float mela_wgt = (mela_wgt_sum!=0. ? rewgtBuilder->getPostThresholdWeight(tree)/mela_wgt_sum : 0.); // Normalized to unit
        float mela_samplewgt, mela_sumsamplewgts;
        if (ReweightingBuilder::useNeffInNormComponent){
          mela_samplewgt = rewgtBuilder->getSumPostThresholdNeffs(tree);
          mela_sumsamplewgts = rewgtBuilder->getSumAllPostThresholdNeffs(tree);
        }
        else{
          mela_samplewgt = rewgtBuilder->getSumPostThresholdSqWeightInvs(tree);
          mela_sumsamplewgts = rewgtBuilder->getSumAllPostThresholdSqWeightInvs(tree);
        }
        if (mela_sumsamplewgts!=0.) mela_wgt *= mela_samplewgt / mela_sumsamplewgts;
        //unsigned int mela_nevts = rewgtBuilder->getSumNonZeroWgtEvents(tree);
        //unsigned int mela_sumnevts = rewgtBuilder->getSumAllNonZeroWgtEvents(tree);
        //if (mela_sumnevts!=0) mela_wgt *= static_cast<float>(mela_nevts) / static_cast<float>(mela_sumnevts);
        mela_wgt *= rewgtBuilder->getNormComponent(tree);
        wgt *= mela_wgt;
      }
      else wgt *= rewgtBuilder->getPostThresholdWeight(tree);
    }

    const unsigned int nCheckWeights=3;
    const TString strCheckWeights[nCheckWeights]={
      "KFactor_QCD_ggZZ_Nominal","KFactor_EW_qqZZ","KFactor_QCD_qqZZ_M"
    };
    for (unsigned int icw=0; icw<nCheckWeights; icw++){
      auto cwit=valfloats.find(strCheckWeights[icw]);
      if (cwit!=valfloats.cend()) wgt *= *(cwit->second);
    }

    product.weight = wgt; product.setNamedVal("weight", wgt);
    if (std::isnan(wgt) || std::isinf(wgt) || wgt==0.){
      if (wgt!=0.){
        MELAerr << "EventAnalyzer::runEvent: Invalid weight " << wgt << " is being discarded at mass " << ZZMass << " for tree " << tree->sampleIdentifier << "." << endl;
        exit(1);
      }
      validProducts=false;
    }

    // Compute the KDs
    // Reserve the special DjjVBF, DjjZH and DjjWH discriminants
    float DjjVBF=-1;
    float DjjWH=-1;
    float DjjZH=-1;
    float KDreq=-999;
    for (auto it=KDbuilders.cbegin(); it!=KDbuilders.cend(); it++){
      auto& KDbuilderpair = it->second;
      auto& KDbuilder = KDbuilderpair.first;
      auto& strKDVarsList = KDbuilderpair.second;
      vector<float> KDBuildVals; KDBuildVals.reserve(strKDVarsList.size());
      for (auto const& s : strKDVarsList) KDBuildVals.push_back(*(valfloats[s]));
      float KD = KDbuilder->update(KDBuildVals, ZZMass);
      validProducts &= !(std::isnan(KD) || std::isinf(KD));

      if (it->first=="DjjVBF") DjjVBF=KD;
      else if (it->first=="DjjZH") DjjZH=KD;
      else if (it->first=="DjjWH") DjjWH=KD;
      else if (it->first=="KD"){
        product.setNamedVal(it->first, KD);
        KDreq=KD;
      }
    }
    validProducts &= (KDreq>=0.);
    //if (KDreq<0.) cout << "KDreq invalid -> " << KDreq << endl;

    // Category check
    SimpleEntry catvars;
    catvars.setNamedVal("DjjVBF", DjjVBF);
    catvars.setNamedVal("DjjZH", DjjZH);
    catvars.setNamedVal("DjjWH", DjjWH);
    Category catFound = CategorizationHelpers::getCategory(catvars, false);
    validProducts &= (category==Inclusive || category==catFound);

    // Channel check
    validProducts &= SampleHelpers::testChannel(channel, *(valshorts["Z1Flav"]), *(valshorts["Z2Flav"]));

    /*
    if (validProducts) cout << "Everyhting is ok." << endl;
    else{
      cout << "Some stuff went wrong" << endl;
      if (category==catFound && SampleHelpers::testChannel(channel, *(valshorts["Z1Flav"]), *(valshorts["Z2Flav"]))){
        cout << "Correct cat and channel but ";
        if (KDreq<0.) cout << "KD<0 ";
        if (std::isnan(KDreq) || std::isinf(KDreq)){
          cout << "KD=" << KDreq << " ";
          for (auto const& s : KDbuilders["KD"].second) cout << s << ": " << *(valfloats[s]) << " ";
        }
        if (wgt==0.) cout << "wgt failed ";
        if (!(product.trackingval>=this->infTrackingVal && product.trackingval<this->supTrackingVal)) cout << "Mass window (" << product.trackingval << ": " << this->infTrackingVal << ", " << this->supTrackingVal << ") failed ";
        cout << endl;
      }
    }
    cout << endl;
    */
  }

  return validProducts;
}


void constructSamples(TString sampleType, float sqrts, const std::vector<TString>& KDvars, CJLSTSet*& theSampleSet){
  vector<TString> samples;
  if (sampleType=="ggHPowheg" || sampleType=="Sig") samples.push_back("gg_Sig_POWHEG");
  if (sampleType=="VBFPowheg" || sampleType=="Sig") samples.push_back("VBF_Sig_POWHEG");
  if (sampleType=="ZHPowheg" || sampleType=="Sig") samples.push_back("ZH_Sig_POWHEG");
  if (sampleType=="WHPowheg" || sampleType=="Sig") samples.push_back("WH_Sig_POWHEG");
  if (sampleType=="WplusHPowheg" || sampleType=="Sig") samples.push_back("WplusH_Sig_POWHEG");
  if (sampleType=="WminusHPowheg" || sampleType=="Sig") samples.push_back("WminusH_Sig_POWHEG");
  if (sampleType=="ggHMCFM" || sampleType=="Sig") samples.push_back("gg_Sig_SM_MCFM");
  if (sampleType=="qqBkg" || sampleType=="Bkg") samples.push_back("qq_Bkg_Combined");
  if (sampleType=="ggBkg" || sampleType=="Bkg") samples.push_back("gg_Bkg_MCFM");
  if (samples.empty()){
    string rawSampleSet = sampleType.Data();
    vector<string> sampleSetProcessed;
    HelperFunctions::splitOptionRecursive(rawSampleSet, sampleSetProcessed, '+');
    for (string& str:sampleSetProcessed) samples.push_back(str.c_str()); // This means there is a compound request
  }
  vector<TString> samplesList;
  SampleHelpers::getSamplesList(sqrts, samples, samplesList);
  theSampleSet = new CJLSTSet(samplesList);

  theSampleSet->bookXS(); // "xsec"
  theSampleSet->bookOverallEventWgt(); // Gen weigts "PUWeight", "genHEPMCweight" and reco weights "dataMCWeight", "trigEffWeight"
  for (auto& tree:theSampleSet->getCJLSTTreeList()){
    // Book common variables needed for analysis
    tree->bookBranch<float>("GenHMass", 0);
    tree->bookBranch<float>("ZZMass", -1);
    tree->bookBranch<short>("Z1Flav", 0);
    tree->bookBranch<short>("Z2Flav", 0);
    // Variables for KDs
    for (auto& v:KDvars) tree->bookBranch<float>(v, 0);
    // Variables for reweighting on a case-by-case basis
    tree->bookBranch<float>("KFactor_QCD_ggZZ_Nominal", 1);
    tree->bookBranch<float>("KFactor_EW_qqZZ", 1);
    tree->bookBranch<float>("KFactor_QCD_qqZZ_M", 1);
  }
}

class KDConstantByMass{
protected:
  float sqrts;
  TString strKD;
  unsigned int nstepsiter;
  int maxNEventsPerBin;
  float infTrackingVal;
  float supTrackingVal;
  std::vector<CJLSTSet::NormScheme> NormSchemeA;
  std::vector<CJLSTSet::NormScheme> NormSchemeB;
  std::vector<std::pair<float, float>> ConstantFractionRanges[2];
  std::vector<std::pair<int, int>> ConstantFractionBinRanges[2];

  void LoopForConstant(
    vector<SimpleEntry>(&index)[2],
    vector<unsigned int>(&indexboundaries)[2],
    TProfile*& px,
    TH1F*& hrec
  );

public:
  KDConstantByMass(float sqrts_, TString strKD_);

  void setNStepsIter(unsigned int nsteps){ nstepsiter=nsteps; }
  void setMaxNEventsPerBin(unsigned int nevents){ maxNEventsPerBin=nevents; }
  void setNormSchemeA(std::vector<CJLSTSet::NormScheme> scheme){ NormSchemeA=scheme; }
  void setNormSchemeB(std::vector<CJLSTSet::NormScheme> scheme){ NormSchemeB=scheme; }
  void setMinMaxTrackingVal(float const minVal, float const maxVal){ infTrackingVal=minVal; supTrackingVal=maxVal; }
  void ensureConstantSampleFraction(unsigned int whichSet, std::vector<std::pair<float, float>> const& setranges){ if (whichSet<2) ConstantFractionRanges[whichSet]=setranges; }

  void run(
    vector<TString> strSamples[2], vector<vector<TString>> strMelaWgts[2],
    SampleHelpers::Channel channel, CategorizationHelpers::Category category,
    unsigned int divisor, const bool writeFinalTree, vector<pair<vector<float>, pair<float, float>>>* manualboundary_validity_pairs=0
  );

  static void fixWeights(std::vector<SimpleEntry>& index);

};

KDConstantByMass::KDConstantByMass(float sqrts_, TString strKD_) :
  sqrts(sqrts_), strKD(strKD_),
  nstepsiter(100),
  maxNEventsPerBin(-1),
  infTrackingVal(-1), supTrackingVal(-2)
{}


///////////////////
// Event helpers //
///////////////////
void KDConstantByMass::LoopForConstant(
  vector<SimpleEntry>(&index)[2],
  vector<unsigned int>(&indexboundaries)[2],
  TProfile*& px,
  TH1F*& hrec
){
  cout << "Begin KDConstantByMass::LoopForConstant" << endl;

  int nbins = indexboundaries[0].size()-1;
  vector<pair<float, float>> finalFractionPerBin; finalFractionPerBin.assign(nbins, pair<float, float>(2, 2));

  for (int bin=0; bin<nbins; bin++){
    cout << "Bin " << bin << " / " << nbins << " is now being scrutinized..." << endl;
    // Determine whether fraction of a set needs to keep fixed
    bool keepFractionFixed[2]={ 0 };
    for (unsigned int ih=0; ih<2; ih++){
      for (pair<int, int>& valrange:ConstantFractionBinRanges[ih]){
        if (bin>=valrange.first && bin<=valrange.second){ keepFractionFixed[ih]=true; break; }
      }
    }
    if (bin==0 || (keepFractionFixed[0] && keepFractionFixed[1])){ for (unsigned int ih=0; ih<2; ih++) keepFractionFixed[ih]=false; }
    for (unsigned int ih=0; ih<2; ih++){ if (keepFractionFixed[ih]) cout << " - Fraction of events for set " << ih << " will be kept the same as previous bin." << endl; }

    // First find the average KD in each sample
    float sumKD[2]={ 0 }; float sumWgt[2]={ 0 }; float avgKD[2]={ 0 };
    std::vector<SimpleEntry>::iterator it_begin[2], it_end[2];
    for (unsigned int ih=0; ih<2; ih++){
      unsigned int const& evlow = indexboundaries[ih].at(bin);
      unsigned int const& evhigh = indexboundaries[ih].at(bin+1);
      unsigned int offsetlow=0;
      unsigned int offsethigh=0;
      if (maxNEventsPerBin>=0){
        unsigned int evdiff=evhigh-evlow;
        if ((int)evdiff>maxNEventsPerBin){
          evdiff -= maxNEventsPerBin;
          offsetlow = evdiff/2;
          offsethigh = evdiff-offsetlow;
        }
      }
      it_begin[ih] = index[ih].begin()+evlow+offsetlow;
      it_end[ih] = index[ih].begin()+evhigh-offsethigh;
      cout << " - Scanning events [ " << evlow+offsetlow << " , " << evhigh-offsethigh << " ) for sample set " << ih << endl;
    }
    for (unsigned int ih=0; ih<2; ih++){
      for (std::vector<SimpleEntry>::iterator it_inst=it_begin[ih]; it_inst!=it_end[ih]; it_inst++){
        float const& KD = it_inst->namedfloats["KD"];
        float const& varTrack = it_inst->trackingval;
        float const& weight = it_inst->weight;
        sumKD[ih] += KD*weight;
        sumWgt[ih] += weight;

        if (px->GetXaxis()->GetBinLowEdge(bin+1)>varTrack || px->GetXaxis()->GetBinLowEdge(bin+2)<=varTrack) cerr
          << "Something terrible happened! " << varTrack << " is outside ["
          << px->GetXaxis()->GetBinLowEdge(bin+1) << " , " << px->GetXaxis()->GetBinLowEdge(bin+2)
          << "]" << endl;

        px->Fill(varTrack, varTrack, weight);
      }
      cout << " - Sum of weights for sample set " << ih << " = " << sumWgt[ih] << endl;
      avgKD[ih]=sumKD[ih]/sumWgt[ih];
      cout << " - Average KD for sample set " << ih << " = " << avgKD[ih] << endl;
    }

    float marginlow=1;
    float marginhigh=20;
    float Cfound=0;
    float centralConstant = (avgKD[0]+avgKD[1])*0.5; centralConstant = 1./(1./centralConstant-1.);
    unsigned int it=0;
    while (true){
      float mindiff=2;
      short sgnMinDiff=0;
      float finalFraction[2]={ 2, 2 };
      unsigned int nsteps = nstepsiter;
      if (it==0) nsteps*=1000;
      else if (it==1) nsteps*=100;
      else if (it==2) nsteps*=10;

      MELAout << " - Iteration " << it << " with margins = " << marginlow << ", " << marginhigh << endl;
      MELAout << "   - Checking c = [ " << centralConstant*(1.-marginlow) << " , " << centralConstant*(1.+marginhigh) << " ] in " << nsteps << " steps." << endl;
      //if (it>0) break;

      for (unsigned int step=0; step<=nsteps; step++){
        HelperFunctions::progressbar(step, nsteps);
        float testC = centralConstant*((1.-marginlow) + (marginhigh+marginlow)*(float(step))/((float) nsteps));
        //MELAout << testC << endl;
        if (testC<=0.){
          //MELAerr << "Cannot test " << testC << " (iteration " << it << ", step " << step << " / " << nsteps << endl;
          continue;
        }

        float sumWgtAll[2]={ 0 };
        float sumWgtHalf[2]={ 0 };
        for (unsigned int ih=0; ih<2; ih++){
          for (std::vector<SimpleEntry>::iterator it_inst=it_begin[ih]; it_inst!=it_end[ih]; it_inst++){
            float const& KDold = it_inst->namedfloats["KD"];
            float const& weight = it_inst->weight;
            if (KDold==-999.) continue;
            float KD = KDold/(KDold+(1.-KDold)*testC);
            if (std::isnan(KD) || std::isinf(KD)){
              cerr << "Something went terribly wrong! KD is " << KD << endl;
              continue;
            }
            else if (KD<0.){
              cerr << "KD is invalid (" << KD << ")" << endl;
              continue;
            }
            sumWgtAll[ih] += weight;
            if (
              (KD>=0.5 && ih==0)
              ||
              (KD<0.5 && ih==1)
              ) sumWgtHalf[ih] += weight;
          }
          sumWgtHalf[ih]=sumWgtHalf[ih]/sumWgtAll[ih];
        }

        float sumWgtHalfDiff;
        if (keepFractionFixed[0]) sumWgtHalfDiff = sumWgtHalf[0] - finalFractionPerBin.at(bin-1).first;
        else if (keepFractionFixed[1]) sumWgtHalfDiff = sumWgtHalf[1] - finalFractionPerBin.at(bin-1).second;
        else sumWgtHalfDiff=sumWgtHalf[0]-sumWgtHalf[1];
        float absSumWgtHalfDiff=fabs(sumWgtHalfDiff);
        short sgnSumWgtHalfDiff = TMath::Sign(float(1), sumWgtHalfDiff);
        if (mindiff>absSumWgtHalfDiff){
          finalFraction[0] = sumWgtHalf[0];
          finalFraction[1] = sumWgtHalf[1];
          mindiff=absSumWgtHalfDiff;
          if (sgnMinDiff==0 && sgnSumWgtHalfDiff!=0) sgnMinDiff=sgnSumWgtHalfDiff;
          Cfound=testC;
        }
        if (sgnMinDiff!=sgnSumWgtHalfDiff) break;
      }
      cout << "  - New c found = " << Cfound << " (old was " << centralConstant << ")" << endl;
      cout << "  - Final fractions were = " << finalFraction[0] << " , " << finalFraction[1] << " (sign of difference: " << sgnMinDiff << ")" << endl;
      finalFractionPerBin.at(bin).first=finalFraction[0];
      finalFractionPerBin.at(bin).second=finalFraction[1];
      if (fabs(Cfound/centralConstant-1)<1e-4 && it>0) break;
      centralConstant=Cfound;
      if (it>2){
        marginhigh /= float(nsteps)/10.;
        marginlow /= float(nsteps)/10.;
      }
      else{
        marginhigh /= 5.;
        marginlow /= 2.;
      }
      Cfound=0;
      it++;
    }

    hrec->SetBinContent(bin+1, centralConstant);
  }

  cout << "End KDConstantByMass::LoopForConstant" << endl;
}
void KDConstantByMass::run(
  vector<TString> strSamples[2], vector<vector<TString>> strMelaWgts[2],
  SampleHelpers::Channel channel, CategorizationHelpers::Category category,
  unsigned int divisor, const bool writeFinalTree, vector<pair<vector<float>, pair<float, float>>>* manualboundary_validity_pairs
){
  if (strKD=="") return;

  // Set categorization scheme
  CategorizationHelpers::setGlobalCategorizationScheme(CategorizationHelpers::UntaggedOrJJVBFOrHadVH);

  cout << "Begin KDConstantByMass::run" << endl;
  for (unsigned int ih=0; ih<2; ih++){ assert(strSamples[ih].size()==strMelaWgts[ih].size()); }

  DiscriminantClasses::Type KDtype = DiscriminantClasses::getKDType(strKD);
  vector<TString> KDvars = DiscriminantClasses::getKDVars(KDtype);
  Discriminant* KDbuilder = constructKDFromType(KDtype);
  if (!KDbuilder) return;

  // Register the categorization discriminants
  vector<KDspecs> KDlist;
  if (category!=Inclusive) getCategorizationDiscriminants(sNominal, KDlist);

  vector<SimpleEntry> index[2];

  gSystem->Exec("mkdir -p ./output/KDConstants");
  TString coutput = Form("KDConstant_m4l_%s", strKD.Data());
  if (channel!=SampleHelpers::NChannels) coutput += Form("_%s", SampleHelpers::getChannelName(channel).Data());
  if (category!=CategorizationHelpers::Inclusive) coutput += Form("_%s", CategorizationHelpers::getCategoryName(category).Data());
  if (sqrts>0.) coutput += Form("_%.0fTeV", sqrts);
  TFile* foutput = TFile::Open(Form("./output/KDConstants/%s%s", coutput.Data(), ".root"), "recreate");

  int nEntries[2]={ 0 };
  float infimum=0;
  float supremum=sqrts*1000.;
  if (infTrackingVal<supTrackingVal){
    infimum=infTrackingVal;
    supremum=supTrackingVal;
    cout
      << "KDConstantByMass::run: WARNING! Initial values of infimum and supremum are changed to ("
      << infimum << ", " << supremum
      << ") by hand! Please make sure this is really what you would like to do."
      << endl;
  }

  for (unsigned int ih=0; ih<2; ih++){
    vector<CJLSTSet*> theSets;
    theSets.assign(strSamples[ih].size(), nullptr);
    for (unsigned int ihs=0; ihs<strSamples[ih].size(); ihs++) constructSamples(strSamples[ih].at(ihs), 13, KDvars, theSets.at(ihs));
    auto melawgtcollit=strMelaWgts[ih].begin();
    std::vector<CJLSTSet::NormScheme> const& schemeSet = (ih==0 ? NormSchemeA : NormSchemeB);
    unsigned int iset=0;
    for (auto& theSet:theSets){
      for (auto& tree:theSet->getCJLSTTreeList()){
        for (auto& strWgt:(*melawgtcollit)) tree->bookBranch<float>(strWgt, 0);
        for (auto& KD:KDlist){ for (auto& v:KD.KDvars) tree->bookBranch<float>(v, 0); }
      }
      melawgtcollit++;

      theSet->setPermanentWeights((schemeSet.empty() ? CJLSTSet::NormScheme_NgenOverNgenWPU : schemeSet.at(iset)), true, true);

      for (auto& tree:theSet->getCJLSTTreeList()) tree->silenceUnused(); // Will no longer book another branch
      iset++;
    }

    // Setup GenHMass inclusive binning
    ExtendedBinning GenHMassInclusiveBinning("GenHMass");
    melawgtcollit=strMelaWgts[ih].begin();
    for (auto& theSet:theSets){
      ReweightingBuilder* melarewgtBuilder = nullptr;
      // Construct MELARewgt for each set
      if (!melawgtcollit->empty()){
        // Binning for MELARewgt
        ExtendedBinning GenHMassBinning("GenHMass");
        float MHValfirst=theSet->getCJLSTTreeList().at(0)->MHVal; bool MHsame=false;
        for (unsigned int is=1; is<theSet->getCJLSTTreeList().size(); is++){
          if (theSet->getCJLSTTreeList().at(is)->MHVal==MHValfirst){
            MHsame=true;
            break;
          }
          else MHValfirst=theSet->getCJLSTTreeList().at(is)->MHVal;
        }
        if (MHsame && MHValfirst>0.){
          GenHMassBinning.addBinBoundary(100);
          GenHMassBinning.addBinBoundary(MHValfirst-5);
          GenHMassBinning.addBinBoundary(MHValfirst+5);
          GenHMassBinning.addBinBoundary(160);
          GenHMassBinning.addBinBoundary(220);
          GenHMassBinning.addBinBoundary(450);
          GenHMassBinning.addBinBoundary(1300);
        }
        else{
          for (unsigned int is=0; is<theSet->getCJLSTTreeList().size()-1; is++){
            if (theSet->getCJLSTTreeList().at(is)->MHVal>0. && theSet->getCJLSTTreeList().at(is+1)->MHVal>0.){
              float boundary = (theSet->getCJLSTTreeList().at(is)->MHVal + theSet->getCJLSTTreeList().at(is+1)->MHVal)/2.;
              GenHMassBinning.addBinBoundary(boundary);
            }
          }
        }
        if (GenHMassBinning.isValid()){
          GenHMassBinning.addBinBoundary(0);
          GenHMassBinning.addBinBoundary(sqrts*1000.);
        }
        else{
          GenHMassBinning.addBinBoundary(0);
          GenHMassBinning.addBinBoundary(100);
          GenHMassBinning.addBinBoundary(160);
          GenHMassBinning.addBinBoundary(220);
          GenHMassBinning.addBinBoundary(450);
          GenHMassBinning.addBinBoundary(1300);
          GenHMassBinning.addBinBoundary(sqrts*1000.);
        }
        melarewgtBuilder = new ReweightingBuilder(*melawgtcollit, ReweightingFunctions::getSimpleWeight);
        melarewgtBuilder->rejectNegativeWeights(true);
        melarewgtBuilder->setDivideByNSample(true);
        melarewgtBuilder->setWeightBinning(GenHMassBinning);
        for (auto& tree:theSet->getCJLSTTreeList()) melarewgtBuilder->setupWeightVariables(tree, 0.999, 50);
      }

      EventAnalyzer theAnalyzer(theSet, channel, category);
      theAnalyzer.infTrackingVal=infimum;
      theAnalyzer.supTrackingVal=supremum;
      // Book common variables needed for analysis
      theAnalyzer.addConsumed<float>("PUWeight");
      theAnalyzer.addConsumed<float>("genHEPMCweight");
      theAnalyzer.addConsumed<float>("dataMCWeight");
      theAnalyzer.addConsumed<float>("trigEffWeight");
      theAnalyzer.addConsumed<float>("GenHMass");
      theAnalyzer.addConsumed<float>("ZZMass");
      theAnalyzer.addConsumed<short>("Z1Flav");
      theAnalyzer.addConsumed<short>("Z2Flav");
      // Add K factors
      theAnalyzer.addConsumed<float>("KFactor_QCD_ggZZ_Nominal");
      theAnalyzer.addConsumed<float>("KFactor_EW_qqZZ");
      theAnalyzer.addConsumed<float>("KFactor_QCD_qqZZ_M");
      // Add discriminant builders
      theAnalyzer.addDiscriminantBuilder("KD", KDbuilder, KDvars);
      for (auto& KD:KDlist){ theAnalyzer.addDiscriminantBuilder(KD.KDname, KD.KD, KD.KDvars); }
      // Add reweighting builders
      theAnalyzer.addReweightingBuilder("MELARewgt", melarewgtBuilder);

      // Loop
      theAnalyzer.setExternalProductList(&(index[ih]));
      theAnalyzer.loop(true, false, true);

      delete melarewgtBuilder;
      melawgtcollit++;
    }
    for (auto& theSet:theSets) delete theSet;

    float firstVal=1000.*sqrts;
    float lastVal=0;
    cout << "Determining min/max for set " << ih << " (size=" << index[ih].size() << ")" << endl;
    for (auto const& ev:index[ih]){
      firstVal=std::min(firstVal, ev.trackingval);
      lastVal=std::max(lastVal, ev.trackingval);
    }
    firstVal = (float) ((int) firstVal); firstVal -= (float) (((int) firstVal)%10);
    lastVal = (float) ((int) (lastVal+0.5)); lastVal += (float) (10-((int) lastVal)%10);
    infimum = std::max(firstVal, infimum);
    supremum = std::min(lastVal, supremum);
    cout << "Unmodified Nproducts: " << index[ih].size() << ", firstVal: " << firstVal << ", lastVal: " << lastVal << endl;
  }
  for (unsigned int ih=0; ih<2; ih++){
    cout << "Cropping events for set " << ih << " based on min/max = " << infimum << " / " << supremum << endl;
    SimpleEntry::cropByTrueVal(index[ih], infimum, supremum);
    cout << "Sorting..." << endl;
    std::sort(index[ih].begin(), index[ih].end());
    nEntries[ih]=index[ih].size();
    cout << "Nentries remaining = " << nEntries[ih] << " | var = [ " << infimum << " , " << supremum << " ]" << endl;
  }

  vector<unsigned int> indexboundaries[2];
  for (unsigned int ih=0; ih<2; ih++) indexboundaries[ih].push_back(0);
  {
    unsigned int iit[2]={ divisor, divisor };
    while (iit[0]<index[0].size() && iit[1]<index[1].size()){
      if (
        (index[0].at(iit[0]).trackingval+index[0].at(iit[0]-1).trackingval)*0.5
        <
        (index[1].at(iit[1]).trackingval+index[1].at(iit[1]-1).trackingval)*0.5
        ){
        while (
          (iit[0]+1)<index[0].size() &&
          (
          (index[0].at(iit[0]).trackingval+index[0].at(iit[0]-1).trackingval)*0.5
          <
          (index[1].at(iit[1]).trackingval+index[1].at(iit[1]-1).trackingval)*0.5
          )
          &&
          (
          (index[0].at(iit[0]+1).trackingval+index[0].at(iit[0]).trackingval)*0.5
          <
          (index[1].at(iit[1]).trackingval+index[1].at(iit[1]-1).trackingval)*0.5
          )
          ) iit[0]++;
      }
      else if (
        (index[0].at(iit[0]).trackingval+index[0].at(iit[0]-1).trackingval)*0.5
          >
        (index[1].at(iit[1]).trackingval+index[1].at(iit[1]-1).trackingval)*0.5
        ){
        while (
          (iit[1]+1)<index[1].size() &&
          (
          (index[0].at(iit[0]).trackingval+index[0].at(iit[0]-1).trackingval)*0.5
          >
          (index[1].at(iit[1]).trackingval+index[1].at(iit[1]-1).trackingval)*0.5
          )
          &&
          (
          (index[0].at(iit[0]).trackingval+index[0].at(iit[0]-1).trackingval)*0.5
          >
          (index[1].at(iit[1]+1).trackingval+index[1].at(iit[1]).trackingval)*0.5
          )
          ) iit[1]++;
      }

      if (
        (index[0].size()-iit[0])<divisor
        ||
        (index[1].size()-iit[1])<divisor
        ) break;

      for (unsigned int ih=0; ih<2; ih++){ indexboundaries[ih].push_back(iit[ih]); iit[ih] += divisor; }
    }
  }
  for (unsigned int ih=0; ih<2; ih++) indexboundaries[ih].push_back(index[ih].size());
  for (unsigned int ih=0; ih<2; ih++) cout << "Final size of indexboundaries[" << ih << "] = " << indexboundaries[ih].size() << endl;

  unsigned int nbins=indexboundaries[0].size()-1;
  vector<float> binboundarylist;
  binboundarylist.push_back(infimum);
  for (unsigned int ix=1; ix<nbins; ix++){
    float binboundary = max(
      (index[0].at(indexboundaries[0].at(ix)-1).trackingval+index[0].at(indexboundaries[0].at(ix)).trackingval)*0.5
      ,
      (index[1].at(indexboundaries[1].at(ix)-1).trackingval+index[1].at(indexboundaries[1].at(ix)).trackingval)*0.5
    );

    cout << "Initial bin boundary for bin " << ix << ": " << binboundary << endl;

    bool skip=false;
    if (manualboundary_validity_pairs){
      for (auto const& mbvpair : (*manualboundary_validity_pairs)){
        const pair<float, float>& valrange = mbvpair.second;
        skip = (
          (valrange.first<0 || valrange.first<=binboundary)
          &&
          (valrange.second<0 || valrange.second>binboundary)
          );
        if (skip) break;
      }
    }
    if (!skip) binboundarylist.push_back(binboundary);
  }
  binboundarylist.push_back(supremum);

  if (manualboundary_validity_pairs){
    for (auto& mbvpair : (*manualboundary_validity_pairs)){
      vector<float>& bvals = mbvpair.first;
      for (unsigned int ib=0; ib<bvals.size(); ib++){
        float& bval = bvals.at(ib);
        cout << "Adding manual boundary " << bval << endl;
        addByLowest<float>(binboundarylist, bval, true);
      }
    }
  }

  nbins = binboundarylist.size()-1;
  float* binning = new float[nbins+1];
  for (unsigned int ix=0; ix<=nbins; ix++){
    binning[ix] = binboundarylist[ix];
    cout << "Boundary (" << ix << ")= " << binning[ix] << endl;
  }

  // Recalibrate index boundaries
  if (manualboundary_validity_pairs){
    for (unsigned int ih=0; ih<2; ih++){
      indexboundaries[ih].clear();
      indexboundaries[ih].push_back(0);
      unsigned int ix=1;
      for (unsigned int ev=1; ev<index[ih].size(); ev++){
        if (ix==nbins) break;
        if (index[ih].at(ev).trackingval>=binning[ix]){
          indexboundaries[ih].push_back(ev);
          ix++;
        }
      }
      indexboundaries[ih].push_back(index[ih].size());
    }
  }

  // Find for which bins the fraction of a set should be constant
  for (unsigned int ih=0; ih<2; ih++){
    for (pair<float, float>& valrange:ConstantFractionRanges[ih]){
      int ilow=-1, ihigh=-1;
      float difflow=1000.*sqrts;
      float diffhigh=1000.*sqrts;
      for (unsigned int ix=0; ix<nbins; ix++){
        if (fabs(valrange.first-binning[ix])<difflow){ ilow=ix; difflow=fabs(valrange.first-binning[ix]); }
        if (fabs(valrange.first-binning[ix+1])<diffhigh){ ihigh=ix; diffhigh=fabs(valrange.first-binning[ix+1]); }
      }
      if (ilow>=0 && ihigh>=0 && ilow<=ihigh) ConstantFractionBinRanges[ih].push_back(pair<int, int>(ilow, ihigh));
    }
  }

  foutput->cd();

  TH1F* h_varTrack_Constant = new TH1F("varReco_Constant", "", nbins, binning); h_varTrack_Constant->Sumw2();
  TProfile* p_varTrack = new TProfile("avg_varReco", "", nbins, binning); p_varTrack->Sumw2();
  delete[] binning;

  for (unsigned int ih=0; ih<2; ih++){
    bool hasMELARewgt=false;
    for (vector<TString> const& v:strMelaWgts[ih]){ if (!v.empty()){ hasMELARewgt=true; break; } }
    if (hasMELARewgt) KDConstantByMass::fixWeights(index[ih]);
  }
  if (writeFinalTree){
    for (unsigned int bin=0; bin<nbins; bin++){
      for (unsigned int ih=0; ih<2; ih++){
        TTree* theFinalTree = new TTree(Form("Sample%i_Bin%i", ih, bin), "");
        unsigned int const& evlow = indexboundaries[ih].at(bin);
        unsigned int const& evhigh = indexboundaries[ih].at(bin+1);
        unsigned int offsetlow=0;
        unsigned int offsethigh=0;
        if (maxNEventsPerBin>=0){
          unsigned int evdiff=evhigh-evlow;
          if ((int)evdiff>maxNEventsPerBin){
            evdiff -= maxNEventsPerBin;
            offsetlow = evdiff/2;
            offsethigh = evdiff-offsetlow;
          }
        }
        SimpleEntry::writeToTree(index[ih].cbegin()+evlow+offsetlow, index[ih].cbegin()+evhigh-offsethigh, theFinalTree);
        foutput->WriteTObject(theFinalTree);
        delete theFinalTree;
      }
    }
  }

  LoopForConstant(
    index, indexboundaries,
    p_varTrack,
    h_varTrack_Constant
  );

  TGraphErrors* gr = makeGraphFromTH1(p_varTrack, h_varTrack_Constant, "gr_varReco_Constant");
  foutput->WriteTObject(p_varTrack);
  foutput->WriteTObject(h_varTrack_Constant);
  foutput->WriteTObject(gr);
  delete gr;
  delete h_varTrack_Constant;
  delete p_varTrack;
  foutput->Close();

  delete KDbuilder;

  NormSchemeA.clear();
  NormSchemeB.clear();
  cout << "End KDConstantByMass::run" << endl;
}
void KDConstantByMass::fixWeights(std::vector<SimpleEntry>& index){
  const unsigned int nMarginalMax = 100;
  const unsigned int nMarginalMaxMult = 1000;
  const float nMarginalMaxFrac = 1./static_cast<float const>(nMarginalMaxMult);
  const unsigned int countThreshold=nMarginalMaxMult*nMarginalMax;

  int const nbinsraw = (1000*theSqrts-70)/5;
  TH1F* hmass = new TH1F("hmass", "", nbinsraw, 70, 13000);
  // Initial loop over the tree to count the events in each bin
  for (SimpleEntry const& product:index) hmass->Fill(product.trackingval); // Do not use weight; just count

  // Determine the final binning to set the weight thresholds
  MELAout
    << "KDConstantByMass::fixWeights: "
    << "Determining the final binning to set the weight thresholds"
    << endl;
  ExtendedBinning binning;
  binning.addBinBoundary(hmass->GetXaxis()->GetBinLowEdge(hmass->GetNbinsX()+1));
  vector<unsigned int> counts;
  unsigned int count=0;
  for (int bin=hmass->GetNbinsX(); bin>=0; bin--){
    count += hmass->GetBinContent(bin);
    if (count>countThreshold || bin==0){
      counts.push_back(count);
      binning.addBinBoundary(hmass->GetXaxis()->GetBinLowEdge(bin));
      count=0;
    }
  }
  delete hmass;
  std::reverse(counts.begin(), counts.end());
  MELAout
    << "KDConstantByMass::fixWeights: "
    << "counts.size()=" << counts.size() << "=?" << "nbins=" << binning.getNbins()
    << endl;
  // These lines guarantee count>countThreshold in every bin
  if (counts.at(0)<countThreshold){
    counts.at(1) += counts.at(0);
    counts.erase(counts.begin());
    binning.removeBinLowEdge(1);
  }
  MELAout
    << "KDConstantByMass::fixWeights: "
    << "counts.size()=" << counts.size() << "=?" << "nbins=" << binning.getNbins()
    << endl;

  // Collect the count*nMarginalMaxFrac events with highest weights
  MELAout
    << "KDConstantByMass::fixWeights: "
    << "Collecting the count*" << nMarginalMaxFrac << " events with highest weights in " << binning.getNbins() << " bins"
    << endl;
  vector<vector<float>> wgtcollList;
  wgtcollList.assign(binning.getNbins(), vector<float>());
  for (SimpleEntry const& product:index){
    float const& trackingval = product.trackingval;
    float const& weight = product.weight;
    int bin = binning.getBin(trackingval);
    if (bin>=0 && bin<(int) binning.getNbins()){
      vector<float>& wgtcoll=wgtcollList.at(bin);
      const unsigned int maxPrunedSize = std::ceil(float(counts.at(bin))*nMarginalMaxFrac);
      if (wgtcoll.size()<maxPrunedSize) addByHighest(wgtcoll, fabs(weight), false);
      else if (wgtcoll.back()<fabs(weight)){
        addByHighest(wgtcoll, fabs(weight), false);
        wgtcoll.pop_back();
      }
    }
  }
  MELAout
    << "KDConstantByMass::fixWeights: "
    << "Determining the weight thresholds"
    << endl;
  vector<float> wgtThresholds; wgtThresholds.reserve(binning.getNbins());
  for (auto const& wgtcoll:wgtcollList){
    unsigned int ns=wgtcoll.size();
    float threshold=0.5*(wgtcoll.at(ns-1)+wgtcoll.at(ns-2));
    if (wgtcoll.front()*5.<threshold) threshold=wgtcoll.front();
    else MELAout
      << "KDConstantByMass::fixWeights: "
      << "Threshold " << threshold << " is different from max. weight " << wgtcoll.front()
      << endl;
    wgtThresholds.push_back(threshold);
  }

  // Fix the weights
  for (SimpleEntry& product:index){
    float const& trackingval = product.trackingval;
    float& weight = product.weight;
    int bin = binning.getBin(trackingval);
    if (bin>=0 && bin<(int) binning.getNbins() && fabs(weight)>wgtThresholds.at(bin)) weight = pow(wgtThresholds.at(bin), 2)/weight;
  }
}


/*
SPECIFIC COMMENT:
- Multiplies by Pmjj
*/
void getKDConstant_DjjZH(float sqrts=13, const bool writeFinalTree=false){
  float divisor=20000;
  TString strKD="DjjZH";

  vector<TString> strSamples[2];
  strSamples[0].push_back("ZHPowheg");
  strSamples[1].push_back("ggHPowheg");
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  {
    pair<float, float> valrange(70, 145);
    vector<float> manualboundaries;
    manualboundaries.push_back(105);
    manualboundaries.push_back(119);
    manualboundaries.push_back(124.7);
    manualboundaries.push_back(131.4);
    manualboundaries.push_back(144.15);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }
  {
    pair<float, float> valrange(570, 680);
    vector<float> manualboundaries;
    manualboundaries.push_back(579.738);
    manualboundaries.push_back(672.24);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }
  {
    pair<float, float> valrange(750, 910);
    vector<float> manualboundaries;
    manualboundaries.push_back(775.266);
    manualboundaries.push_back(903.041);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }
  {
    pair<float, float> valrange(1150, sqrts*1000.);
    vector<float> manualboundaries;
    manualboundaries.push_back(1196);
    manualboundaries.push_back(1535);
    manualboundaries.push_back(2132.47);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }

  KDConstantByMass constProducer(sqrts, strKD);
  constProducer.run(
    strSamples, strMelaWgts,
    SampleHelpers::NChannels, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}
void getKDConstant_DjjWH(float sqrts=13, const bool writeFinalTree=false){
  float divisor=20000;
  TString strKD="DjjWH";

  vector<TString> strSamples[2];
  strSamples[0].push_back("WHPowheg");
  strSamples[1].push_back("ggHPowheg");
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  {
    pair<float, float> valrange(70, 125);
    vector<float> manualboundaries;
    manualboundaries.push_back(105);
    manualboundaries.push_back(119);
    manualboundaries.push_back(124.37);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
      ));
  }
  {
    pair<float, float> valrange(129, 135);
    vector<float> manualboundaries;
    manualboundaries.push_back(132.3);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
      ));
  }
  {
    pair<float, float> valrange(2600, sqrts*1000.);
    vector<float> manualboundaries;
    manualboundaries.push_back(2750);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
    ));
  }

  KDConstantByMass constProducer(sqrts, strKD);
  {
    std::vector<CJLSTSet::NormScheme> NormSchemeA;
    NormSchemeA.assign(strSamples[0].size(), CJLSTSet::NormScheme_None);
    NormSchemeA.at(0)=CJLSTSet::NormScheme_XsecOverNgen_RelRenormToSumNgen;
    constProducer.setNormSchemeA(NormSchemeA);
  }
  constProducer.run(
    strSamples, strMelaWgts,
    SampleHelpers::NChannels, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}

/* SPECIFIC COMMENT: NONE */
void getKDConstant_DjjVBF(float sqrts=13, const bool writeFinalTree=false){
  float divisor=40000;
  TString strKD="DjjVBF";

  vector<TString> strSamples[2];
  strSamples[0].push_back("VBFPowheg");
  strSamples[1].push_back("ggHPowheg");
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  {
    pair<float, float> valrange(70, 120);
    vector<float> manualboundaries;
    manualboundaries.push_back(105);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
    ));
  }
  {
    pair<float, float> valrange(750, 3500);
    vector<float> manualboundaries;
    manualboundaries.push_back(770);
    manualboundaries.push_back(850);
    manualboundaries.push_back(1100);
    manualboundaries.push_back(1400);
    manualboundaries.push_back(2100);
    manualboundaries.push_back(2650);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
    ));
  }

  KDConstantByMass constProducer(sqrts, strKD);
  constProducer.run(
    strSamples, strMelaWgts,
    SampleHelpers::NChannels, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}

/* SPECIFIC COMMENT: NONE */
void getKDConstant_DjVBF(float sqrts=13, const bool writeFinalTree=false){
  float divisor=40000;
  TString strKD="DjVBF";

  vector<TString> strSamples[2];
  strSamples[0].push_back("VBFPowheg");
  strSamples[1].push_back("ggHPowheg");
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  {
    pair<float, float> valrange(70, 120);
    vector<float> manualboundaries;
    manualboundaries.push_back(105);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
    ));
  }
  {
    pair<float, float> valrange(1000, 2000);
    vector<float> manualboundaries;
    manualboundaries.push_back(1400);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(
      manualboundaries, valrange
    ));
  }

  KDConstantByMass constProducer(sqrts, strKD);
  constProducer.run(
    strSamples, strMelaWgts,
    SampleHelpers::NChannels, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}

/* SPECIFIC COMMENT: NONE */
void getKDConstant_Dbkgkin(const Channel channel, float sqrts=13, const bool writeFinalTree=false){
  if (channel!=k2e2mu && channel!=k4e && channel!=k4mu) return;
  const TString strChannel = getChannelName(channel);

  float divisor=21000;
  if (channel==k2e2mu) divisor = 50000;
  TString strKD="Dbkgkin";

  vector<TString> strSamples[2];
  strSamples[0].push_back("ggHPowheg");
  //strSamples[0].push_back("ggHMCFM");
  strSamples[0].push_back(
    Form("gg_Bkg_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0PH_MCFM_%s", strChannel.Data())
  );
  strSamples[0].push_back("VBFPowheg");
  strSamples[1].push_back("qqBkg");
  strSamples[1].push_back("ggHPowheg");
  strSamples[1].push_back(
    Form("gg_Bkg_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0PH_MCFM_%s", strChannel.Data())
  );
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());
  // Reweight ggBkg decay kinematics to qqBkg
  SampleHelpers::addXsecBranchNames(strMelaWgts[0].at(1));
  strMelaWgts[0].at(1).push_back("p_Gen_GG_SIG_kappaTopBot_1_ghz1_1_MCFM");
  SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(1));
  strMelaWgts[1].at(1).push_back("p_Gen_QQB_BKG_MCFM");
  strMelaWgts[1].at(1).push_back("p_Gen_CPStoBWPropRewgt");
  SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(2));
  strMelaWgts[1].at(2).push_back("p_Gen_QQB_BKG_MCFM");

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  if (channel==k4e){
    {
      pair<float, float> valrange(70, 147);
      vector<float> manualboundaries;
      manualboundaries.push_back(75); manualboundaries.push_back(85);
      manualboundaries.push_back(89); manualboundaries.push_back(93); manualboundaries.push_back(96);
      manualboundaries.push_back(100); manualboundaries.push_back(105);
      manualboundaries.push_back(113); manualboundaries.push_back(117.5); manualboundaries.push_back(120);
      manualboundaries.push_back(122); manualboundaries.push_back(124); manualboundaries.push_back(126);
      manualboundaries.push_back(129); manualboundaries.push_back(135); manualboundaries.push_back(139);
      manualboundaries.push_back(143); manualboundaries.push_back(146);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(180, 188);
      vector<float> manualboundaries;
      manualboundaries.push_back(182); manualboundaries.push_back(184); manualboundaries.push_back(187);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(189, 362);
      vector<float> manualboundaries;
      for (unsigned int it=0; it<=8; it++) manualboundaries.push_back(195.+it*10.);
      for (unsigned int it=1; it<=5; it++) manualboundaries.push_back(275.+it*15.);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(2000, 13000);
      vector<float> manualboundaries;
      manualboundaries.push_back(3300);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }
  else if (channel==k4mu){
    {
      pair<float, float> valrange(70, 146);
      vector<float> manualboundaries;
      manualboundaries.push_back(75); manualboundaries.push_back(85);
      manualboundaries.push_back(89); manualboundaries.push_back(93); manualboundaries.push_back(96);
      manualboundaries.push_back(100); manualboundaries.push_back(105);
      manualboundaries.push_back(113); manualboundaries.push_back(117.5); manualboundaries.push_back(120);
      manualboundaries.push_back(122); manualboundaries.push_back(124); manualboundaries.push_back(126);
      manualboundaries.push_back(129); manualboundaries.push_back(135); manualboundaries.push_back(139);
      manualboundaries.push_back(143);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(147, 401);
      vector<float> manualboundaries;
      manualboundaries.push_back(153); manualboundaries.push_back(160); manualboundaries.push_back(170);
      manualboundaries.push_back(180); manualboundaries.push_back(185); manualboundaries.push_back(190);
      for (unsigned int it=0; it<=9; it++) manualboundaries.push_back(200.+it*10.);
      for (unsigned int it=0; it<=5; it++) manualboundaries.push_back(300.+it*20.);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(490, 560);
      vector<float> manualboundaries;
      manualboundaries.push_back(520);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(2500, 13000);
      vector<float> manualboundaries;
      manualboundaries.push_back(3300);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }
  else if (channel==k2e2mu){
    {
      pair<float, float> valrange(70, 146);
      vector<float> manualboundaries;
      manualboundaries.push_back(75); manualboundaries.push_back(85);
      manualboundaries.push_back(89); manualboundaries.push_back(93); manualboundaries.push_back(96);
      manualboundaries.push_back(100); manualboundaries.push_back(105);
      manualboundaries.push_back(113); manualboundaries.push_back(117.5); manualboundaries.push_back(120);
      manualboundaries.push_back(122); manualboundaries.push_back(124); manualboundaries.push_back(126);
      manualboundaries.push_back(129); manualboundaries.push_back(135); manualboundaries.push_back(139);
      manualboundaries.push_back(143);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(147, 401);
      vector<float> manualboundaries;
      manualboundaries.push_back(153); manualboundaries.push_back(160); manualboundaries.push_back(170);
      manualboundaries.push_back(180); manualboundaries.push_back(185); manualboundaries.push_back(190);
      for (unsigned int it=0; it<=9; it++) manualboundaries.push_back(200.+it*10.);
      for (unsigned int it=0; it<=5; it++) manualboundaries.push_back(300.+it*20.);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(2500, 13000);
      vector<float> manualboundaries;
      manualboundaries.push_back(3300);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }

  KDConstantByMass constProducer(sqrts, strKD);
  constProducer.setMaxNEventsPerBin(80000);
  {
    std::vector<CJLSTSet::NormScheme> NormSchemeB;
    NormSchemeB.assign(strSamples[1].size(), CJLSTSet::NormScheme_NgenOverNgenWPU);
    NormSchemeB.at(0)=CJLSTSet::NormScheme_XsecOverNgen;
    constProducer.setNormSchemeB(NormSchemeB);
  }
  constProducer.run(
    strSamples, strMelaWgts,
    channel, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}

void getKDConstant_Dbkgdec(const Channel channel, float sqrts=13, const bool writeFinalTree=false){
  if (channel!=k2e2mu && channel!=k4e && channel!=k4mu) return;
  const TString strChannel = getChannelName(channel);

  float divisor=21000;
  if (channel==k2l2l || channel==k2e2mu) divisor = 50000;
  TString strKD="Dbkgdec";

  vector<TString> strSamples[2];
  strSamples[0].push_back("ggHPowheg");
  strSamples[0].push_back(
    Form("gg_Bkg_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0PH_MCFM_%s", strChannel.Data())
  );
  strSamples[0].push_back("VBFPowheg");
  strSamples[1].push_back("ggHPowheg");
  strSamples[1].push_back(
    Form("gg_Bkg_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0PH_MCFM_%s", strChannel.Data())
  );
  strSamples[1].push_back("ggHPowheg");
  strSamples[1].push_back(
    Form("gg_Bkg_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0PH_MCFM_%s", strChannel.Data())
  );
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());
  // Reweight ggBkg decay kinematics to qqBkg
  SampleHelpers::addXsecBranchNames(strMelaWgts[0].at(1));
  strMelaWgts[0].at(1).push_back("p_Gen_GG_SIG_kappaTopBot_1_ghz1_1_MCFM");
  SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(0));
  strMelaWgts[1].at(0).push_back("p_Gen_QQB_BKG_MCFM");
  strMelaWgts[1].at(0).push_back("p_Gen_CPStoBWPropRewgt");
  SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(1));
  strMelaWgts[1].at(1).push_back("p_Gen_QQB_BKG_MCFM");
  SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(2));
  strMelaWgts[1].at(2).push_back("p_Gen_GG_BKG_MCFM");
  strMelaWgts[1].at(2).push_back("p_Gen_CPStoBWPropRewgt");
  SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(3));
  strMelaWgts[1].at(3).push_back("p_Gen_GG_BKG_MCFM");

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  /*
  {
    pair<float, float> valrange(70, 142);
    vector<float> manualboundaries;
    manualboundaries.push_back(75); manualboundaries.push_back(85);
    manualboundaries.push_back(89); manualboundaries.push_back(93); manualboundaries.push_back(96);
    manualboundaries.push_back(100); manualboundaries.push_back(105); manualboundaries.push_back(110); manualboundaries.push_back(115);
    manualboundaries.push_back(120); manualboundaries.push_back(123); manualboundaries.push_back(135);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }
  {
    pair<float, float> valrange(600, 2500);
    vector<float> manualboundaries;
    manualboundaries.push_back(700); manualboundaries.push_back(900); manualboundaries.push_back(1100); manualboundaries.push_back(1400); manualboundaries.push_back(1900);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }
  */
  KDConstantByMass constProducer(sqrts, strKD);
  constProducer.run(
    strSamples, strMelaWgts,
    channel, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}

/* SPECIFIC COMMENT: NONE */
void getKDConstant_Dggbkgkin(const Channel channel, float sqrts=13, const bool writeFinalTree=false){
  if (channel!=k2e2mu && channel!=k4e && channel!=k4mu) return;
  const TString strChannel = getChannelName(channel);

  float divisor=50000;
  if (channel==k2l2l || channel==k2e2mu) divisor = 50000;
  TString strKD="Dggbkgkin";

  vector<TString> strSamples[2];
  //strSamples[0].push_back("ggHPowheg");
  //strSamples[0].push_back("ggHMCFM");
  //strSamples[1].push_back("ggBkg");
  strSamples[0].push_back(
    Form("gg_Bkg_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_SM_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0M_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_Sig_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI_0PH_MCFM_%s", strChannel.Data())
    + TString("+")
    + Form("gg_BSI10_0PH_MCFM_%s", strChannel.Data())
  );
  strSamples[1]=strSamples[0];
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());
  for (unsigned int is=0; is<strSamples[0].size(); is++){
    SampleHelpers::addXsecBranchNames(strMelaWgts[0].at(is));
    strMelaWgts[0].at(is).push_back(TemplateHelpers::OffshellGGProcessHandle.getMELAHypothesisWeight(GGProcessHandler::GGSig, ACHypothesisHelpers::kSM));
  }
  for (unsigned int is=0; is<strSamples[1].size(); is++){
    SampleHelpers::addXsecBranchNames(strMelaWgts[1].at(is));
    strMelaWgts[1].at(is).push_back(TemplateHelpers::OffshellGGProcessHandle.getMELAHypothesisWeight(GGProcessHandler::GGBkg, ACHypothesisHelpers::kSM));
  }

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  {
    pair<float, float> valrange(70, 170);
    vector<float> manualboundaries;
    manualboundaries.push_back(100);
    manualboundaries.push_back(110);
    manualboundaries.push_back(115);
    manualboundaries.push_back(119);
    manualboundaries.push_back(120.9);
    manualboundaries.push_back(121.8);
    manualboundaries.push_back(122.8);
    manualboundaries.push_back(123.6);
    manualboundaries.push_back(124.3);
    manualboundaries.push_back(125.06);
    manualboundaries.push_back(125.9);
    manualboundaries.push_back(127.3);
    manualboundaries.push_back(136.1);
    manualboundaries.push_back(146);
    manualboundaries.push_back(157);
    manualboundaries.push_back(165);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }
  {
    pair<float, float> valrange(200.2, 3100);
    vector<float> manualboundaries;
    manualboundaries.push_back(203);
    manualboundaries.push_back(207);
    manualboundaries.push_back(211);
    manualboundaries.push_back(216);
    manualboundaries.push_back(223);
    manualboundaries.push_back(232);
    manualboundaries.push_back(236);
    manualboundaries.push_back(250);
    manualboundaries.push_back(270);
    manualboundaries.push_back(290);
    manualboundaries.push_back(310);
    manualboundaries.push_back(400);
    manualboundaries.push_back(500);
    manualboundaries.push_back(600);
    manualboundaries.push_back(700);
    manualboundaries.push_back(900);
    manualboundaries.push_back(1300);
    manualboundaries.push_back(1800);
    manualboundaries.push_back(2400);
    manualboundaries.push_back(3000);
    manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
  }

  KDConstantByMass constProducer(sqrts, strKD);
  constProducer.setMaxNEventsPerBin(divisor);
  {
    std::vector<CJLSTSet::NormScheme> NormSchemeA;
    NormSchemeA.assign(strSamples[0].size(), CJLSTSet::NormScheme_NgenOverNgenWPU);
    constProducer.setNormSchemeA(NormSchemeA);

    std::vector<CJLSTSet::NormScheme> NormSchemeB;
    NormSchemeB.assign(strSamples[1].size(), CJLSTSet::NormScheme_NgenOverNgenWPU);
    constProducer.setNormSchemeB(NormSchemeB);
  }
  constProducer.run(
    strSamples, strMelaWgts,
    channel, CategorizationHelpers::Inclusive,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}

/* SPECIFIC COMMENT: NONE */
void getKDConstant_DbkgjjEWQCD(const Channel channel, const Category category, float sqrts=13, const bool writeFinalTree=false){
  if (channel!=k2l2l && channel!=k4l) return;
  if (category!=JJVBFTagged && category!=HadVHTagged) return;

  float divisor=10000;
  if (channel==k2l2l || channel==k2e2mu) divisor = 20000;
  TString strKD="DbkgjjEWQCD";

  vector<TString> strSamples[2];
  if (category==JJVBFTagged){
    strSamples[0].push_back("VBFPowheg");
    strSamples[1].push_back("VBFPowheg");
  }
  else if (category==HadVHTagged){
    strSamples[0].push_back("WplusH_Sig_POWHEG+WminusH_Sig_POWHEG+ZH_Sig_POWHEG");
    strSamples[1].push_back("WplusH_Sig_POWHEG"); strSamples[1].push_back("WminusH_Sig_POWHEG"); strSamples[1].push_back("ZH_Sig_POWHEG");
  }
  strSamples[1].push_back("qqBkg");
  vector<vector<TString>> strMelaWgts[2]; for (unsigned int ih=0; ih<2; ih++) strMelaWgts[ih].assign(strSamples[ih].size(), vector<TString>());
  for (unsigned int ih=1; ih<2; ih++){
    unsigned int iw=0;
    for (auto& wlist:strMelaWgts[ih]){
      SampleHelpers::addXsecBranchNames(wlist);
      if (!(ih==1 && iw==strMelaWgts[ih].size()-1)){
        wlist.push_back(OffshellVVProcessHandle.getMELAHypothesisWeight((ih==0 ? VVProcessHandler::VVSig : VVProcessHandler::VVBkg), kSM));
        wlist.push_back("p_Gen_CPStoBWPropRewgt");
      }
      iw++;
    }
  }

  vector<pair<vector<float>, pair<float, float>>> manualboundary_validity_pairs;
  if (category==JJVBFTagged && channel==k2l2l){
    {
      pair<float, float> valrange(70, 121);
      vector<float> manualboundaries;
      manualboundaries.push_back(95);
      manualboundaries.push_back(105);
      manualboundaries.push_back(115);
      manualboundaries.push_back(120);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(130, 158);
      vector<float> manualboundaries;
      manualboundaries.push_back(133);
      //manualboundaries.push_back(138);
      manualboundaries.push_back(149.5);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(161, 205);
      vector<float> manualboundaries;
      manualboundaries.push_back(168);
      manualboundaries.push_back(178);
      manualboundaries.push_back(187);
      manualboundaries.push_back(198);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(210, 250);
      vector<float> manualboundaries;
      manualboundaries.push_back(222);
      manualboundaries.push_back(240);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(260, 450);
      vector<float> manualboundaries;
      manualboundaries.push_back(287);
      manualboundaries.push_back(327);
      manualboundaries.push_back(380);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(800, 1600);
      vector<float> manualboundaries;
      manualboundaries.push_back(1152);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(2500, sqrts*1000.);
      vector<float> manualboundaries;
      manualboundaries.push_back(3000);
      manualboundaries.push_back(3400);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }
  else if (category==JJVBFTagged && channel==k4l){
    {
      pair<float, float> valrange(70, 123.5);
      vector<float> manualboundaries;
      manualboundaries.push_back(95);
      manualboundaries.push_back(105);
      manualboundaries.push_back(113);
      manualboundaries.push_back(115);
      manualboundaries.push_back(119.1);
      manualboundaries.push_back(121);
      manualboundaries.push_back(123.1);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(124, 129);
      vector<float> manualboundaries;
      manualboundaries.push_back(128);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(136, 178);
      vector<float> manualboundaries;
      manualboundaries.push_back(137.8);
      manualboundaries.push_back(149.96);
      manualboundaries.push_back(160.5);
      manualboundaries.push_back(166);
      manualboundaries.push_back(175);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(180, 199);
      vector<float> manualboundaries;
      manualboundaries.push_back(180.6);
      manualboundaries.push_back(184);
      manualboundaries.push_back(191);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(665, 810);
      vector<float> manualboundaries;
      manualboundaries.push_back(665.4);
      manualboundaries.push_back(809);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(900, 1300);
      vector<float> manualboundaries;
      manualboundaries.push_back(903.77);
      manualboundaries.push_back(1295.78);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(1300, 1750);
      vector<float> manualboundaries;
      manualboundaries.push_back(1739.01);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(2800, sqrts*1000.);
      vector<float> manualboundaries;
      manualboundaries.push_back(3350);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }
  else if (category==HadVHTagged && channel==k2l2l){
    {
      pair<float, float> valrange(70, 132);
      vector<float> manualboundaries;
      manualboundaries.push_back(101);
      manualboundaries.push_back(114.5);
      manualboundaries.push_back(121);
      manualboundaries.push_back(124.8);
      //manualboundaries.push_back(126.5);
      manualboundaries.push_back(129);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(135, 147);
      vector<float> manualboundaries;
      manualboundaries.push_back(141.5);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(191, 207);
      vector<float> manualboundaries;
      manualboundaries.push_back(200);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(450, 770);
      vector<float> manualboundaries;
      manualboundaries.push_back(457.778);
      manualboundaries.push_back(583.63);
      manualboundaries.push_back(765.544);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(450, 770);
      vector<float> manualboundaries;
      manualboundaries.push_back(457.778);
      manualboundaries.push_back(583.63);
      manualboundaries.push_back(765.544);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(1860, sqrts*1000.);
      vector<float> manualboundaries;
      manualboundaries.push_back(1970);
      manualboundaries.push_back(2500);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }
  else if (category==HadVHTagged && channel==k4l){
    {
      pair<float, float> valrange(70, 135);
      vector<float> manualboundaries;
      manualboundaries.push_back(100);
      manualboundaries.push_back(114);
      manualboundaries.push_back(121);
      manualboundaries.push_back(124);
      manualboundaries.push_back(126.5);
      //manualboundaries.push_back(131);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(300, 1350);
      vector<float> manualboundaries;
      //manualboundaries.push_back(301);
      manualboundaries.push_back(347.302);
      //manualboundaries.push_back(370.658);
      manualboundaries.push_back(427.896);
      manualboundaries.push_back(481.677);
      manualboundaries.push_back(542.162);
      manualboundaries.push_back(613.381);
      manualboundaries.push_back(702.238);
      manualboundaries.push_back(802.943);
      manualboundaries.push_back(937.149);
      //manualboundaries.push_back(1360.01);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
    {
      pair<float, float> valrange(1400, sqrts*1000.);
      vector<float> manualboundaries;
      manualboundaries.push_back(2000);
      manualboundaries.push_back(2600);
      manualboundary_validity_pairs.push_back(pair<vector<float>, pair<float, float>>(manualboundaries, valrange));
    }
  }

  KDConstantByMass constProducer(sqrts, strKD);
  {
    std::vector<CJLSTSet::NormScheme> NormSchemeA;
    NormSchemeA.assign(strSamples[0].size(), CJLSTSet::NormScheme_None);
    NormSchemeA.at(0)=CJLSTSet::NormScheme_XsecOverNgen_RelRenormToSumNgen;
    constProducer.setNormSchemeA(NormSchemeA);
  }
  {
    vector<pair<float, float>> setranges; setranges.push_back(pair<float, float>(350, sqrts*1000.));
    constProducer.ensureConstantSampleFraction(0, setranges);
  }
  constProducer.run(
    strSamples, strMelaWgts,
    channel, category,
    divisor, writeFinalTree, &manualboundary_validity_pairs
  );
}


void generic_SmoothKDConstantProducer(
  float sqrts, TString strname, TString strcustomselection,
  TF1* (*lowf)(TSpline3*, double, double, bool),
  TF1* (*highf)(TSpline3*, double, double, bool),
  bool useFaithfulSlopeFirst, bool useFaithfulSlopeSecond,
  vector<pair<pair<double, double>, unsigned int>>* addpoints=nullptr
){
  const double xmin=0;
  const double xmax=(sqrts>0 ? (double) sqrts*1000. : 15000.);

  gSystem->Exec("mkdir -p ./output/KDConstants");
  TString cinput = Form("KDConstant_m4l_%s", strname.Data());
  if (strcustomselection!="") cinput += Form("_%s", strcustomselection.Data());
  if (sqrts>0.) cinput += Form("_%.0fTeV", sqrts);

  TFile* finput = TFile::Open(Form("./output/KDConstants/%s%s", cinput.Data(), ".root"), "read");
  TFile* foutput = TFile::Open(Form("./output/KDConstants/Smooth%s%s", cinput.Data(), ".root"), "recreate");
  foutput->cd();

  TGraphErrors* tg = (TGraphErrors*) finput->Get("gr_varReco_Constant");
  foutput->WriteTObject(tg);

  if (addpoints!=0){ for (auto& prange : *addpoints) addPointsBetween(tg, prange.first.first, prange.first.second, prange.second); }

  int n = tg->GetN();
  double* xx = tg->GetX();
  double* ex = tg->GetEX();
  double* yy = tg->GetY();
  double* ey = tg->GetEY();

  TSpline3* sp;
  sp = convertGraphToSpline3(tg, useFaithfulSlopeFirst, useFaithfulSlopeSecond);
  double tglow = xx[0];
  double tghigh = xx[tg->GetN()-1];
  TF1* lowFcn = lowf(sp, xmin, tglow, true);
  TF1* highFcn = highf(sp, tghigh, xmax, false);
  lowFcn->SetNpx((int) (tglow-xmin)*5);
  highFcn->SetNpx((int) (xmax-tghigh)*5);

  vector<pair<double, double>> points;
  for (double xval=xmin; xval<tglow; xval+=1){
    double yval = lowFcn->Eval(xval);
    addByLowest<double, double>(points, xval, yval);
  }
  for (int ix=0; ix<n; ix++){
    addByLowest<double, double>(points, xx[ix], yy[ix]);
  }
  int tghigh_int = ((int) ((tghigh+1.)/100.+0.5))*100;
  if (tghigh>=(double) tghigh_int) tghigh_int+=100;
  for (double xval=tghigh_int; xval<=xmax; xval+=100){
    double yval = highFcn->Eval(xval);
    addByLowest<double, double>(points, xval, yval);
  }

  int nn_new = points.size();
  cout << "Number of new points: " << nn_new-n << endl;
  double* xy_new[2];
  for (unsigned int i=0; i<2; i++) xy_new[i] = new double[nn_new];
  for (int ix=0; ix<nn_new; ix++){
    xy_new[0][ix] = points.at(ix).first;
    xy_new[1][ix] = points.at(ix).second;
  }

  delete highFcn;
  delete lowFcn;
  delete sp;

  TGraph* tg_updated = new TGraph(nn_new, xy_new[0], xy_new[1]);
  tg_updated->SetName(Form("%s_Smooth", tg->GetName()));
  foutput->WriteTObject(tg_updated);

  sp = convertGraphToSpline3(tg_updated);
  sp->SetNpx(tg_updated->GetN()*100);
  foutput->WriteTObject(sp);
  delete sp;
  delete tg_updated;
  for (unsigned int i=0; i<2; i++) delete[] xy_new[i];

  foutput->Close();
  finput->Close();
}


void SmoothKDConstantProducer_DjjZH(){
  vector<pair<pair<double, double>, unsigned int>> addpoints;
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(103, 114), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(150, 155), 5);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(155.1, 158), 4);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(140, 146), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(138.5, 139.9), 3);
    addpoints.push_back(points);
  }
  /*
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(100, 117.8), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(118.1, 125), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(117, 118), 3);
    addpoints.push_back(points);
  }
  */
  generic_SmoothKDConstantProducer(
    13, "DjjZH", "",
    &getFcn_a0timesexpa1X,
    &getFcn_a0plusa1timesXN<3>,
    true, true,
    &addpoints
  );
}

void SmoothKDConstantProducer_DjjWH(){
  vector<pair<pair<double, double>, unsigned int>> addpoints;
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(103, 114), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(113.5, 114.5), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(115, 119), 10);
    addpoints.push_back(points);
  }
  {
    pair<pair<double, double>, unsigned int> points(pair<double, double>(119.2, 122.2), 3);
    addpoints.push_back(points);
  }
  generic_SmoothKDConstantProducer(
    13, "DjjWH", "",
    &getFcn_a0timesexpa1X,
    &getFcn_a0plusa1timesXN<3>,
    true, true,
    &addpoints
  );
}

void SmoothKDConstantProducer_DjjVBF(){
  TString strprod="VBF";

  vector<pair<pair<double, double>, unsigned int>> addpoints;
  {
    pair<double, double> xminmax(1000, 3000); unsigned int nadd=5;
    pair<pair<double, double>, unsigned int> addsingle(xminmax, nadd); addpoints.push_back(addsingle);
  }

  generic_SmoothKDConstantProducer(
    13, Form("Djj%s", strprod.Data()), "",
    &getFcn_a0plusa1timesXN<1>,
    &getFcn_a0timesexpa1X,
    //&getFcn_a0plusa1overX,
    true, true,
    &addpoints
  );
}

void SmoothKDConstantProducer_DjVBF(){
  TString strprod="VBF";
  generic_SmoothKDConstantProducer(
    13, Form("Dj%s", strprod.Data()), "",
    &getFcn_a0plusa1timesXN<1>,
    &getFcn_a0timesexpa1X,
    true, true
  );
}

void SmoothKDConstantProducer_Dbkgkin(const Channel channel){
  if (channel!=k2e2mu && channel!=k4e && channel!=k4mu) return;
  const TString strChannel = getChannelName(channel);
  generic_SmoothKDConstantProducer(
    13, Form("Dbkgkin_%s", strChannel.Data()), "",
    &getFcn_a0plusa1timesXN<1>,
    &getFcn_a0timesexpa1X,
    //&getFcn_a0plusa1overXN<6>,
    true, true
  );
}

void SmoothKDConstantProducer_Dggbkgkin(const Channel channel){
  if (channel!=k2e2mu && channel!=k4e && channel!=k4mu) return;
  const TString strChannel = getChannelName(channel);
  generic_SmoothKDConstantProducer(
    13, Form("Dggbkgkin_%s", strChannel.Data()), "",
    &getFcn_a0plusa1timesXN<1>,
    &getFcn_a0timesexpa1X,
    //&getFcn_a0plusa1overXN<6>,
    true, false
  );
}

void SmoothKDConstantProducer_DbkgjjEWQCD(const Channel channel, const Category category){
  if (channel!=k2l2l && channel!=k4l) return;
  if (category!=JJVBFTagged && category!=HadVHTagged) return;
  const TString strChannel = getChannelName(channel);

  typedef TF1* (*PatchFunction)(TSpline3*, double, double, bool);
  PatchFunction lowf=nullptr;
  PatchFunction highf=nullptr;
  lowf=&getFcn_a0timesexpa1X;
  highf=&getFcn_a0plusa1timesatana2timesXminusa3;
  vector<pair<pair<double, double>, unsigned int>> addpoints;
  if (category==JJVBFTagged && channel==k2l2l){
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(195, 255), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(182, 204), 15);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(160, 172), 3);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(123.7, 152), 20);
      addpoints.push_back(points);
    }
  }
  else if (category==JJVBFTagged && channel==k4l){
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(865, 1065), 20);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(380, 420), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(253, 340), 20);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(195, 250), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(189, 199), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(180.5, 187), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(156, 165), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(145, 154), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(135, 143), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(117.5, 120), 5);
      addpoints.push_back(points);
    }
  }
  else if (category==HadVHTagged && channel==k2l2l){
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(1750, 2150), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(1200, 1745), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(210, 250), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(194, 208), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(179, 191), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(172, 176), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(152, 165), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(138, 150), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(123.5, 126.5), 5);
      addpoints.push_back(points);
    }
  }
  else if (category==HadVHTagged && channel==k4l){
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(1200, 2300), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(470, 570), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(290, 360), 10);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(185.5, 192.5), 5);
      addpoints.push_back(points);
    }
    {
      pair<pair<double, double>, unsigned int> points(pair<double, double>(125.5, 130), 5);
      addpoints.push_back(points);
    }
  }

  generic_SmoothKDConstantProducer(
    13, Form("DbkgjjEWQCD_%s_%s", strChannel.Data(), CategorizationHelpers::getCategoryName(category).Data()), "",
    lowf,
    highf,
    true, false,
    &addpoints
  );
}


void testConstant(TString fname, TString cfname, float massmin, float massmax, float KDcutval){
  TFile* cFile = TFile::Open(cfname, "read");
  TSpline3* spC = (TSpline3*) cFile->Get("sp_gr_varReco_Constant");

  TFile* treeFile = TFile::Open(fname, "read");
  TH1F* htmp = (TH1F*)treeFile->Get("varReco_Constant");
  const unsigned int nbins = htmp->GetNbinsX();

  TFile testFile("test.root", "recreate");
  TH1F hA("hA", "", 50, 0, 1); hA.Sumw2();
  TH1F hB("hB", "", 50, 0, 1); hB.Sumw2();
  float KD, ZZMass, weight;
  for (unsigned int ih=0; ih<2; ih++){
    treeFile->cd();
    float sumWgt=0;
    float sumWgtWithCut=0;
    vector<TTree*> treeList;
    for (unsigned int bin=0; bin<nbins; bin++){ treeList.push_back((TTree*)treeFile->Get(Form("Sample%i_Bin%i", ih, bin))); }
    for (auto& tree:treeList){
      tree->SetBranchAddress("KD", &KD);
      tree->SetBranchAddress("ZZMass", &ZZMass);
      tree->SetBranchAddress("weight", &weight);
      for (int ev=0; ev<tree->GetEntries(); ev++){
        tree->GetEntry(ev);
        if (ZZMass>=massmin && ZZMass<massmax){
          sumWgt += weight;
          if (ih==0){
            hA.Fill(KD, weight);
          }
          else{
            hB.Fill(KD, weight);
          }
          if (KD>=KDcutval) sumWgtWithCut += weight;
        }
      }
    }
    cout << "Sample " << ih << " cut efficiency: " << sumWgtWithCut << " / " << sumWgt << " = " << sumWgtWithCut/sumWgt << endl;
  }
  hA.Scale(1./hA.Integral());
  hB.Scale(1./hB.Integral());
  testFile.WriteTObject(&hA);
  testFile.WriteTObject(&hB);

  // Plot ROC curve
  TGraph* tgROC=HelperFunctions::createROCFromDistributions(&hA, &hB, "ROC");
  tgROC->GetYaxis()->SetTitle("Sample A efficiency");
  tgROC->GetXaxis()->SetTitle("Sample B efficiency");
  testFile.WriteTObject(tgROC);
  delete tgROC;

  testFile.Close();
  treeFile->Close();
  cFile->Close();
}


/*
g-constants for AC
*/

TGraph* getSingleTGraph(TString fname){
  TDirectory* curdir = gDirectory;

  cout << "Opening file " << fname << endl;

  TFile* finput = TFile::Open(Form("../data/JHUGenXsec/%s%s", fname.Data(), ".root"), "read");
  if (!finput) return nullptr;
  TGraph* tgold = (TGraph*) finput->Get("Graph");
  double* xx = tgold->GetX();
  double* yy = tgold->GetY();

  vector<pair<double, double>> xyinterm;
  for (int ip=0; ip<tgold->GetN(); ip++){
    if (std::isnan(xx[ip]) || std::isinf(xx[ip])) continue;
    if (std::isnan(yy[ip]) || std::isinf(yy[ip])) continue;
    xyinterm.push_back(pair<double, double>(xx[ip], yy[ip]));
  }
  TGraph* tginterm = makeGraphFromPair(xyinterm, "tginterm");
  TSpline3* spinterm = convertGraphToSpline3(tginterm);
  delete tginterm;

  vector<pair<double, double>> xyfinal;
  for (int ip=0; ip<tgold->GetN(); ip++){
    if (std::isnan(xx[ip]) || std::isinf(xx[ip])) continue;
    if (std::isnan(yy[ip]) || std::isinf(yy[ip])) yy[ip] = spinterm->Eval(xx[ip]);
    xyfinal.push_back(pair<double, double>(xx[ip], yy[ip]));
  }
  delete spinterm;

  curdir->cd();
  TGraph* result = makeGraphFromPair(xyfinal, Form("tgfinal_%s", fname.Data()));

  finput->Close();
  return result;
}

void generic_gConstantProducer(TString strprod, TString strhypo, bool useproddec=false){
  TString strSM = "SM";
  TString strSMbare = strSM;
  if (strhypo=="L1Zgs") strSM += "_photoncut";

  TFile* foutput = TFile::Open(Form("gConstant_%s%s_%s%s", strprod.Data(), (useproddec ? "_ProdDec" : ""), strhypo.Data(), ".root"), "recreate");
  TGraph* tgSMhypo=nullptr;
  TGraph* tgBSMhypo=nullptr;
  if (strprod!="VH"){
    tgSMhypo = getSingleTGraph(Form("%s_%s", strprod.Data(), strSM.Data()));
    tgBSMhypo = getSingleTGraph(Form("%s_%s", strprod.Data(), strhypo.Data()));
  }
  else{
    TGraph* tgSMhypoZH = getSingleTGraph(Form("%s_%s", "ZH", strSM.Data()));
    TGraph* tgBSMhypoZH = getSingleTGraph(Form("%s_%s", "ZH", strhypo.Data()));
    TGraph* tgSMhypoWH = getSingleTGraph(Form("%s_%s", "WH", strSMbare.Data()));
    TGraph* tgBSMhypoWH = getSingleTGraph(Form("%s_%s", "WH", strhypo.Data()));
    constexpr float xsec_ZH_SM = 	0.883888217433748;
    constexpr float xsec_WH_SM = 	0.532731376975169 + 0.839913391993366;
    if (tgSMhypoZH){
      const float scale = xsec_ZH_SM/tgSMhypoZH->Eval(125);
      MELAout << "ZH xsec (125) is scaled by " << scale << endl;
      multiplyTGraph(tgSMhypoZH, scale);
      if (tgBSMhypoZH) multiplyTGraph(tgBSMhypoZH, scale);
    }
    if (tgSMhypoWH){
      const float scale = xsec_WH_SM/tgSMhypoWH->Eval(125);
      MELAout << "WH xsec (125) is scaled by " << scale << endl;
      multiplyTGraph(tgSMhypoWH, scale);
      if (tgBSMhypoWH) multiplyTGraph(tgBSMhypoWH, scale);
    }
    if (tgSMhypoZH && tgSMhypoWH) tgSMhypo = addTGraphs(tgSMhypoZH, tgSMhypoWH);
    else if (tgSMhypoZH) tgSMhypo = new TGraph(*tgSMhypoZH);
    else if (tgSMhypoWH) tgSMhypo = new TGraph(*tgSMhypoWH);
    if (tgBSMhypoZH && tgBSMhypoWH) tgBSMhypo = addTGraphs(tgBSMhypoZH, tgBSMhypoWH);
    else if (tgBSMhypoZH) tgBSMhypo = new TGraph(*tgBSMhypoZH);
    else if (tgBSMhypoWH) tgBSMhypo = new TGraph(*tgBSMhypoWH);
    delete tgSMhypoZH;
    delete tgSMhypoWH;
    delete tgBSMhypoZH;
    delete tgBSMhypoWH;
  }
  if (useproddec){
    TGraph* tgSMhypodec = getSingleTGraph(Form("HZZ2e2mu_%s", strSM.Data()));
    TGraph* tgBSMhypodec = getSingleTGraph(Form("HZZ2e2mu_%s", strhypo.Data()));

    TGraph* tgSMhyponew = multiplyTGraphs(tgSMhypo, tgSMhypodec);
    TGraph* tgBSMhyponew = multiplyTGraphs(tgBSMhypo, tgBSMhypodec);

    delete tgSMhypo; tgSMhypo=tgSMhyponew;
    delete tgBSMhypo; tgBSMhypo=tgBSMhyponew;
  }

  TGraph* result = divideTGraphs(tgSMhypo, tgBSMhypo, 0.5, 0.5);
  delete tgSMhypo;
  delete tgBSMhypo;
  foutput->WriteTObject(result);
  TSpline3* spresult = convertGraphToSpline3(result);
  foutput->WriteTObject(spresult);
  cout << "Result of spline at 125 GeV = " << spresult->Eval(125) << endl;
  delete spresult;
  delete result;
  foutput->Close();
}

void gConstantProducer(){
  generic_gConstantProducer("WH", "g2");
  generic_gConstantProducer("WH", "g4");
  generic_gConstantProducer("WH", "L1");
  generic_gConstantProducer("ZH", "g2");
  generic_gConstantProducer("ZH", "g4");
  generic_gConstantProducer("ZH", "L1");
  generic_gConstantProducer("ZH", "L1Zgs");
  generic_gConstantProducer("VH", "g2");
  generic_gConstantProducer("VH", "g4");
  generic_gConstantProducer("VH", "L1");
  generic_gConstantProducer("VH", "L1Zgs");
  generic_gConstantProducer("VBF", "g2");
  generic_gConstantProducer("VBF", "g4");
  generic_gConstantProducer("VBF", "L1");
  generic_gConstantProducer("VBF", "L1Zgs");
  generic_gConstantProducer("HZZ2e2mu", "g2");
  generic_gConstantProducer("HZZ2e2mu", "g4");
  generic_gConstantProducer("HZZ2e2mu", "L1");
  generic_gConstantProducer("HZZ2e2mu", "L1Zgs");
}

