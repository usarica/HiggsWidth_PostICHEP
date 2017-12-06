#include "ACHypothesisHelpers.h"


using namespace std;


TString ACHypothesisHelpers::getACHypothesisName(ACHypothesisHelpers::ACHypothesis hypo){
  switch (hypo){
  case kL1:
    return "L1";
  case kA2:
    return "a2";
  case kA3:
    return "a3";
  default:
    return "";
  };
}

std::vector<DiscriminantClasses::Type> ACHypothesisHelpers::getACHypothesisKDSet(ACHypothesisHelpers::ACHypothesis hypo, CategorizationHelpers::Category category){
  std::vector<DiscriminantClasses::Type> res;
  if (category==CategorizationHelpers::Inclusive || category==CategorizationHelpers::Untagged){
    switch (hypo){
    case kSM:
      res.push_back(DiscriminantClasses::kDbkgkin);
      //res.push_back(DiscriminantClasses::kDggint);
      break;
    case kL1:
      res.push_back(DiscriminantClasses::kDbkgkin);
      res.push_back(DiscriminantClasses::kDL1dec);
      break;
    case kA2:
      res.push_back(DiscriminantClasses::kDbkgkin);
      res.push_back(DiscriminantClasses::kDa2dec);
      break;
    case kA3:
      res.push_back(DiscriminantClasses::kDbkgkin);
      res.push_back(DiscriminantClasses::kDa3dec);
      break;
    default:
      break;
    };
  }
  return res;
}

std::vector<TString> ACHypothesisHelpers::getACHypothesisKDNameSet(ACHypothesisHelpers::ACHypothesis hypo, CategorizationHelpers::Category category){
  std::vector<DiscriminantClasses::Type> KDset = ACHypothesisHelpers::getACHypothesisKDSet(hypo, category);
  vector<TString> res;
  for (auto& type:KDset) res.push_back(DiscriminantClasses::getKDName(type));
  return res;
}
