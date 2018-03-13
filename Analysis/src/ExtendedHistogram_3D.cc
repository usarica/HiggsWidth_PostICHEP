#include "HelperFunctions.h"
#include "ExtendedHistogram_3D.h"
#include "MELAStreamHelpers.hh"


using namespace HelperFunctions;
using namespace MELAStreamHelpers;


ExtendedHistogram_3D::ExtendedHistogram_3D() : ExtendedHistogram(), histo(nullptr), prof_x(nullptr), prof_y(nullptr), prof_z(nullptr){}
ExtendedHistogram_3D::ExtendedHistogram_3D(const TString name_, const TString title_) : ExtendedHistogram(name_, title_), histo(nullptr), prof_x(nullptr), prof_y(nullptr), prof_z(nullptr){}
ExtendedHistogram_3D::ExtendedHistogram_3D(const TString name_, const TString title_, const ExtendedBinning& xbinning_, const ExtendedBinning& ybinning_, const ExtendedBinning& zbinning_) :
  ExtendedHistogram(name_, title_),
  xbinning(xbinning_),
  ybinning(ybinning_),
  zbinning(zbinning_),
  histo(nullptr), prof_x(nullptr), prof_y(nullptr), prof_z(nullptr)
{
  build();
}
ExtendedHistogram_3D::ExtendedHistogram_3D(ExtendedHistogram_3D const& other) : ExtendedHistogram(other), xbinning(other.xbinning), ybinning(other.ybinning), histo(nullptr), prof_x(nullptr), prof_y(nullptr), prof_z(nullptr){
  if (other.histo) histo = new TH3F(*(other.histo));
  if (other.prof_x) prof_x = new TProfile(*(other.prof_x));
  if (other.prof_y) prof_y = new TProfile(*(other.prof_y));
  if (other.prof_z) prof_z = new TProfile(*(other.prof_z));
}
ExtendedHistogram_3D::~ExtendedHistogram_3D(){
  delete histo;
  delete prof_x;
  delete prof_y;
  delete prof_z;
}

void ExtendedHistogram_3D::swap(ExtendedHistogram_3D& other){
  std::swap(name, other.name);
  std::swap(title, other.title);
  std::swap(xbinning, other.xbinning);
  std::swap(ybinning, other.ybinning);
  std::swap(zbinning, other.zbinning);
  std::swap(histo, other.histo);
  std::swap(prof_x, other.prof_x);
  std::swap(prof_y, other.prof_y);
  std::swap(prof_z, other.prof_z);
}
ExtendedHistogram_3D& ExtendedHistogram_3D::operator=(const ExtendedHistogram_3D& other){
  ExtendedHistogram_3D tmp(other);
  this->swap(tmp);
  return *this;
}

void ExtendedHistogram_3D::setNameTitle(const TString name_, const TString title_){
  ExtendedHistogram::setNameTitle(name_, title_);
  if (histo){ histo->SetName(name); histo->SetTitle(title); }
  if (prof_x){ prof_x->SetName(Form("%s_prof_%s", name.Data(), xbinning.getLabel().Data())); prof_x->SetTitle(title); }
  if (prof_y){ prof_y->SetName(Form("%s_prof_%s", name.Data(), ybinning.getLabel().Data())); prof_y->SetTitle(title); }
  if (prof_z){ prof_z->SetName(Form("%s_prof_%s", name.Data(), zbinning.getLabel().Data())); prof_z->SetTitle(title); }
}
void ExtendedHistogram_3D::setBinning(const ExtendedBinning& binning, const int xyz, const TString label){
  if (xyz==0){
    xbinning = binning;
    if (label!="") xbinning.setLabel(label);
  }
  else if (xyz==1){
    ybinning = binning;
    if (label!="") ybinning.setLabel(label);
  }
  else{
    zbinning = binning;
    if (label!="") zbinning.setLabel(label);
  }
}
void ExtendedHistogram_3D::build(){
  reset();
  if (xbinning.isValid() && ybinning.isValid() && zbinning.isValid()){
    const double* xbins = xbinning.getBinning();
    const double* ybins = ybinning.getBinning();
    const double* zbins = zbinning.getBinning();
    const int nbinsx = xbinning.getNbins();
    const int nbinsy = ybinning.getNbins();
    const int nbinsz = zbinning.getNbins();
    histo = new TH3F(name, title, nbinsx, xbins, nbinsy, ybins, nbinsz, zbins); histo->GetXaxis()->SetTitle(xbinning.getLabel()); histo->GetYaxis()->SetTitle(ybinning.getLabel()); histo->GetZaxis()->SetTitle(zbinning.getLabel()); histo->Sumw2();
    prof_x = new TProfile(Form("%s_prof_%s", name.Data(), xbinning.getLabel().Data()), title, nbinsx, xbins); prof_x->GetXaxis()->SetTitle(xbinning.getLabel()); prof_x->Sumw2();
    prof_y = new TProfile(Form("%s_prof_%s", name.Data(), ybinning.getLabel().Data()), title, nbinsy, ybins); prof_y->GetXaxis()->SetTitle(ybinning.getLabel()); prof_y->Sumw2();
    prof_z = new TProfile(Form("%s_prof_%s", name.Data(), zbinning.getLabel().Data()), title, nbinsz, zbins); prof_z->GetXaxis()->SetTitle(zbinning.getLabel()); prof_z->Sumw2();
  }
}
void ExtendedHistogram_3D::reset(){
  delete histo;
  delete prof_x;
  delete prof_y;
  delete prof_z;
}

void ExtendedHistogram_3D::fill(double x, double y, double z, double wgt){
  if (histo) histo->Fill(x, y, z, wgt);
  if (prof_x) prof_x->Fill(x, x, wgt);
  if (prof_y) prof_y->Fill(y, y, wgt);
  if (prof_z) prof_z->Fill(z, z, wgt);
}

void ExtendedHistogram_3D::rebin(ExtendedBinning const* binningX, ExtendedBinning const* binningY, ExtendedBinning const* binningZ){
  bool condX=(!binningX);
  bool condY=(!binningY);
  bool condZ=(!binningZ);
  if ((condX || binningX->isValid()) && (condY || binningY->isValid()) && (condZ || binningZ->isValid()) && !(condX && condY && condZ)){
    std::vector<std::pair<TProfile const*, unsigned int>> condProfs;
    if (condX && prof_x) condProfs.emplace_back(prof_x, 0);
    if (condY && prof_y) condProfs.emplace_back(prof_y, 1);
    if (condZ && prof_z) condProfs.emplace_back(prof_z, 2);
    ExtendedBinning bX, bY, bZ;
    if (!condX) bX=*binningX;
    else bX=xbinning;
    if (!condY) bY=*binningY;
    else bY=ybinning;
    if (!condZ) bZ=*binningZ;
    else bZ=zbinning;
    if (histo) rebinHistogram(histo, bX, bY, bZ, (condX || condY || condZ ? &condProfs : nullptr));
    if (prof_x && !condX) rebinProfile(prof_x, bX);
    if (prof_y && !condY) rebinProfile(prof_y, bY);
    if (prof_z && !condZ) rebinProfile(prof_z, bZ);
  }
}

TH3F* ExtendedHistogram_3D::getCumulantHistogram(TString newname) const{
  if (!histo) return nullptr;
  if (newname=="") newname=Form("Cumulant_%s", histo->GetName());
  TH3F* res = nullptr;
  HelperFunctions::getCumulantHistogram(histo, res, newname);
  return res;
}

ExtendedHistogram_3D ExtendedHistogram_3D::divideHistograms(ExtendedHistogram_3D const& h1, ExtendedHistogram_3D const& h2, bool useEffErr, TString newname){
  if (newname=="") newname=Form("h_%s_over_%s", h1.name.Data(), h2.name.Data());
  ExtendedHistogram_3D res(h2); res.setNameTitle(newname); res.histo->Reset("ICES");
  if (!h1.histo || !h2.histo) return res;
  HelperFunctions::divideHistograms(h1.histo, h2.histo, res.histo, useEffErr);
  if (!useEffErr){
    if (h1.prof_x && h2.prof_x){
      for (unsigned int bin=0; bin<=h2.xbinning.getNbins()+1; bin++){
        double val[2]={ h1.prof_x->GetBinContent(bin), h2.prof_x->GetBinContent(bin) };
        double errsq[2]={ pow(h1.prof_x->GetBinError(bin), 2), pow(h2.prof_x->GetBinError(bin), 2) };
        double valnew=0;
        double wnew=0;
        for (unsigned int ii=0; ii<2; ii++){ if (errsq[ii]!=0.){ valnew += val[ii]/errsq[ii]; wnew += 1./errsq[ii]; } }
        if (wnew==0.) valnew = 0;
        else valnew /= wnew;
        res.prof_x->SetBinContent(bin, valnew);
        res.prof_x->SetBinError(bin, sqrt(1./wnew));
      }
    }
    if (h1.prof_y && h2.prof_y){
      for (unsigned int bin=0; bin<=h2.ybinning.getNbins()+1; bin++){
        double val[2]={ h1.prof_y->GetBinContent(bin), h2.prof_y->GetBinContent(bin) };
        double errsq[2]={ pow(h1.prof_y->GetBinError(bin), 2), pow(h2.prof_y->GetBinError(bin), 2) };
        double valnew=0;
        double wnew=0;
        for (unsigned int ii=0; ii<2; ii++){ if (errsq[ii]!=0.){ valnew += val[ii]/errsq[ii]; wnew += 1./errsq[ii]; } }
        if (wnew==0.) valnew = 0;
        else valnew /= wnew;
        res.prof_y->SetBinContent(bin, valnew);
        res.prof_y->SetBinError(bin, sqrt(1./wnew));
      }
    }
    if (h1.prof_z && h2.prof_z){
      for (unsigned int bin=0; bin<=h2.zbinning.getNbins()+1; bin++){
        double val[2]={ h1.prof_z->GetBinContent(bin), h2.prof_z->GetBinContent(bin) };
        double errsq[2]={ pow(h1.prof_z->GetBinError(bin), 2), pow(h2.prof_z->GetBinError(bin), 2) };
        double valnew=0;
        double wnew=0;
        for (unsigned int ii=0; ii<2; ii++){ if (errsq[ii]!=0.){ valnew += val[ii]/errsq[ii]; wnew += 1./errsq[ii]; } }
        if (wnew==0.) valnew = 0;
        else valnew /= wnew;
        res.prof_z->SetBinContent(bin, valnew);
        res.prof_z->SetBinError(bin, sqrt(1./wnew));
      }
    }
  }
  return res;
}

void ExtendedHistogram_3D::constructFromTree(TTree* tree, float& xvar, float& yvar, float& zvar, float& weight, ExtendedBinning const* binningX, ExtendedBinning const* binningY, ExtendedBinning const* binningZ){
  if (!tree) return;
  if (binningX) setBinning(*binningX, 0, binningX->getLabel());
  if (binningY) setBinning(*binningY, 1, binningY->getLabel());
  if (binningZ) setBinning(*binningZ, 2, binningZ->getLabel());
  build();
  for (int ev=0; ev<tree->GetEntries(); ev++){
    tree->GetEntry(ev);
    fill(xvar, yvar, zvar, weight);
  }
}
