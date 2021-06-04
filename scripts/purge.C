#include <TFile.h>
#include <TClass.h>
#include <TKey.h>
#include <TROOT.h>

void purge(const char* filename){
  TFile* f = new TFile(filename, "UPDATE");

  TIter next(f->GetListOfKeys());
  TKey *key;
  while ((key = (TKey*)next())) {
    TClass *cl = gROOT->GetClass(key->GetClassName());
    if(!cl->InheritsFrom("TDirectory")){
      key->Delete();
    }
  }

  f->Close();
}
