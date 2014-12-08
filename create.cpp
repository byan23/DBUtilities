#include "catalog.h"


const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status status;
  Status tmpStatus;
  RelDesc rd;
  AttrDesc ad;
  bool attrTooLong;

  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;


	cout<<"Creating new relation: "<<relation<<endl;
  //make sure that a relation with the same name doesn't already exist(using getInfo())
  status = getInfo(relation, rd);
  if(status != RELNOTFOUND)
  {
	if(status != OK)
	{ 
		cout<<"err in create.cpp, getInfo\n";
		return status;
	}
 	return RELEXISTS;
  }
  

  //add a tuple to the relcat relation
  //filling in an instance of RelDesc and invoke relCatalog::addInfo()
  strncpy(rd.relName, relation.c_str(), MAXNAME);
  rd.attrCnt = attrCnt;
  status = addInfo(rd);
  if(status != OK) return status;

  //for each of attrCnt attributes, invoke AttrCatalog::addInfo() (refers to global attrCat)
  //passing appropriate attribute information from the attrList[] array
  int curOffset = 0;
  for(int i = 0; i < attrCnt; i++)
  {
	attrTooLong = false;
	//passing appropriate attribute info
	strncpy(ad.relName, attrList[i].relName, MAXNAME);
 	strncpy(ad.attrName, attrList[i].attrName, MAXNAME);
	ad.attrOffset = curOffset;
	ad.attrType = attrList[i].attrType;
	//check STRING attr length
	if(ad.attrType == 0)
	{
		if(attrList[i].attrLen >= MAXSTRINGLEN)
			attrTooLong = true;
	}	
  	ad.attrLen = attrList[i].attrLen;
  	status = attrCat->addInfo(ad);
	if(status != OK || attrTooLong)
	{	
		//delete all attribute before i from attrCat 
		//(clear the attrCat as if the relation has never been added)
		for(int j = 0; j < i; j++)
		{
			const string attribute(attrList[j].attrName);
			tmpStatus = attrCat->removeInfo(relation, attribute);
			if(tmpStatus != OK){ cout<<"err in create.cpp, deleting failure\n"; return tmpStatus;}
		}
		//delete the current ith attribute in attrCat, since it's already added.
		if(attrTooLong)
		{
			const string attribute(attrList[i].attrName);
			tmpStatus = attrCat->removeInfo(relation, attribute);
			if(tmpStatus != OK){ cout<<"err in create.cpp, deleting failure2\n"; return tmpStatus;}
			removeInfo(relation);
			return ATTRTOOLONG;
		}	
		removeInfo(relation);
		return status;
	}
	curOffset += attrList[i].attrLen;
  }
  /*//add a tuple to the relcat relation
  //filling in an instance of RelDesc and invoke relCatalog::addInfo()
  strncpy(rd.relName, relation.c_str(), MAXNAME);
  rd.attrCnt = attrCnt;
  status = addInfo(rd);
  if(status != OK) return status; 
*/
  //create a HeapFile instance to hold tuples of the relation(like last prject?)
  status = createHeapFile(relation);
  if(status != OK) return status;
  return OK;
}

