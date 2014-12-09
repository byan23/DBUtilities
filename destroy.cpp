#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;
  
  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
    return BADCATPARM;
  //remove all relevant schema info from both the relcat and attrcat
  status = removeInfo(relation);
  if(status != OK) return status;
  status = attrCat->dropRelation(relation);
  //destroy the heapfile corresponding to the relation
  status = destroyHeapFile(relation);
  if(status != OK) return status;

  return OK;
}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
  Status status;
  AttrDesc *attrs;
  int attrCnt, i;

  if (relation.empty()) return BADCATPARM;
  status = getRelInfo(relation, attrCnt, attrs);
  if(status != OK) return status;
  for(i = 0; i < attrCnt; i++){
   const string atname(attrs[i].attrName);
   status = removeInfo(relation, atname);
   if(status != OK) return status;
  }
  delete [] attrs;
  attrs = NULL;
  return OK;

}


