#ifndef SAMPLES_H
#define SAMPLES_H

#include "HostHelpers.h"
#include <string>

// LHC sqrts and data period
constexpr unsigned int theSqrts = 13;
const TString theDataPeriod = "2017";

// MH and GH reference values
constexpr float MHRefVal=125;
constexpr float GHRefVal=4.07e-3;

// CJLST samples directory
constexpr unsigned int CJLSTversion = 180416;
const TString CJLSTdate = std::to_string(CJLSTversion);
const TString CJLSTrootdir = HostHelpers::GetCJLSTSamplesDirectory(CJLSTdate);
const TString CJLSTsamplesdir = CJLSTrootdir + "/" + CJLSTdate;

const TString TREE_NAME = "ZZTree/candTree";
const TString TREE_FAILED_NAME = "ZZTree/candTree_failed";
const TString TREE_CRZLL_NAME = "CRZLLTree/candTree";
const TString TREE_CRZL_NAME = "CRZLTree/candTree";
const TString COUNTERS_NAME = "ZZTree/Counters";
const TString COUNTERS_CRZLL_NAME = "CRZLLTree/Counters";
const TString COUNTERS_CRZL_NAME = "CRZLTree/Counters";

constexpr float xsecScale = 1e3;

#endif
