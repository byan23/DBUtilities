#include "catalog.h"

/* Create the catalog and heapfile of a relation named 'relation' 
 * 
 * @param string relation the relation name of the relation we want to create
 * 	  int attCnt the number of attributes that belongs to the relation
 * 	  attrInfo attrList[] the attribute information of all the attributes 
 * 	  	of the relation
 * @return OK if successfully created
 * 		otherwise on failures
 */
const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status status;
  Status tmpStatus;
  RelDesc rd;
  AttrDesc ad;
  bool attrTooLong;
  bool tupleTooLong;
  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;


	//cout<<"Creating new relation: "<<relation<<endl;
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
	tupleTooLong = false;
	//check if offset more than PAGESIZE
	if(curOffset >= PAGESIZE)
		tupleTooLong = true;
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
	//terminate the creation if something's wrong
	if(status != OK || attrTooLong || tupleTooLong)
	{	
		//clear the attrCat as if the relation has never been added
		tmpStatus = attrCat->dropRelation(relation);
		if(tmpStatus != OK) {cout<<"err in create.cpp, deleting failur\n"; return tmpStatus;}

		tmpStatus = removeInfo(relation);
		if(tmpStatus != OK){ cout<<"err in create.cpp, deleting failure3\n"; return tmpStatus;}
		if(tupleTooLong)
			return NOSPACE;
		if(attrTooLong)
			return ATTRTOOLONG;
		return status;
	}
	curOffset += attrList[i].attrLen;
  }

  //create a HeapFile instance to hold tuples of the relation(like last prject?)
  status = createHeapFile(relation);
  if(status != OK) return status;
  return OK;
}

