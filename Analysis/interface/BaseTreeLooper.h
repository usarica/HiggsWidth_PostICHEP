#ifndef BASETREELOOPER_H
#define BASETREELOOPER_H

#include "IvyBase.h"
#include "CJLSTSet.h"
#include "ReweightingBuilder.h"
#include "ZXFakeRateHandler.h"
#include "Discriminant.h"
#include "SystematicsHelpers.h"


class BaseTreeLooper : IvyBase{
public:
  enum SampleIdStorageType{
    kNoStorage,
    kStoreByRunAndEventNumber,
    kStoreByMH,
    kStoreByHashVal
  };

protected:
  SampleIdStorageType sampleIdOpt; // When not kNoStorage, stores a sample identifier and original tree entry index in each output event

  // List of trees to loop over
  std::vector<CJLSTTree*> treeList;

  // Max. events to process
  int maxNEvents;

  // External dependencies
  std::unordered_map<TString, std::pair<Discriminant*, std::vector<TString>>> KDbuilders;
  std::unordered_map<TString, ReweightingBuilder*> Rewgtbuilders;
  std::unordered_map<TString, ZXFakeRateHandler*> ZXFakeRateHandlers;
  std::unordered_map<TString, void(*)(BaseTreeLooper*, SimpleEntry&)> externalFunctions;
  std::unordered_map<TString, SystematicsHelpers::SystematicsClass*> SystVariations;

  // List of products
  std::vector<SimpleEntry> productList;
  std::vector<SimpleEntry>* productListRef;
  BaseTree* productTree;
  void addProduct(SimpleEntry& product, unsigned int* ev_rec=nullptr);

  // Flush product list into tree
  void recordProductsToTree();

  // Abstract function to loop over a single event
  virtual bool runEvent(CJLSTTree* tree, float const& externalWgt, SimpleEntry& product)=0;

public:
  // Constructors
  BaseTreeLooper();
  BaseTreeLooper(CJLSTTree* inTree);
  BaseTreeLooper(std::vector<CJLSTTree*> const& inTreeList);
  BaseTreeLooper(CJLSTSet const* inTreeSet);
  void addTree(CJLSTTree* tree);

  // Destructors
  virtual ~BaseTreeLooper();

  // Add the necessary objects
  void addDiscriminantBuilder(TString KDname, Discriminant* KDbuilder, std::vector<TString> const& KDvars);
  void addReweightingBuilder(TString rewgtname, ReweightingBuilder* Rewgtbuilder);
  void addZXFakeRateHandler(TString frname, ZXFakeRateHandler* ZXFakeRateHandler);
  void addExternalFunction(TString fcnname, void(*fcn)(BaseTreeLooper*, SimpleEntry&));
  void addSystematic(TString systname, SystematicsHelpers::SystematicsClass* systVar);
  void setExternalProductList(std::vector<SimpleEntry>* extProductListRef=nullptr);
  void setExternalProductTree(BaseTree* extTree=nullptr);

  // Max. events
  void setMaximumEvents(int n);

  // Sample id storage option
  // POWHEG can be stored by mH, but might be better to use hash in others
  void setSampleIdStorageOption(SampleIdStorageType opt);

  // Function to loop over the tree list
  virtual void loop(bool loopSelected, bool loopFailed, bool keepProducts);

  // Get the products
  std::vector<SimpleEntry> const& getProducts() const;
  // Move the products
  void moveProducts(std::vector<SimpleEntry>& targetColl);
  // Clear the products
  void clearProducts();

};


#endif
