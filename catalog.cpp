///////////////////////////////////////////////////////////////////////////////
////                   ALL STUDENTS COMPLETE THESE SECTIONS
//// Title:            front end catalogs
//// Files:            catalog.cpp, heapfile.cpp heapfile.h, db.h, db.cpp, buf.cpp, 
////                   buf.h, error.cpp, error.h, page.cpp, page.h, bufhash.cpp,
////                   destroy.cpp, help.cpp, create.cpp, load.cpp etc.
//// Semester:         CS564 Fall 2014
////
//// Author:           Cong Sun
//// Email:            csun27@wisc.edu
//// CS Login:         cong
////////////////////// PAIR PROGRAMMERS COMPLETE THIS SECTION ////////////////////
//// Pair Partner:     Boqun Yan
//// Email:            byan23@wisc.edu
//// CS Login:         boqun
////////////////////////////// 80 columns wide //////////////////////////////////

#include "catalog.h"

///**
// * It generates and implements a relation catalog and attribute catalog 
// * for a DBMS
// * <p>Bugs: No bug
// * @author Cong Sun
// *         Boqun Yan
// */
//

RelCatalog::RelCatalog(Status &status) :
	 HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}

/* Get relation descriptor for a relation
 * 
 * @param string relation the relation name of the relation we want to create
 * 	  RelDesc record the variable containing the description info of a relation
 * @return OK if successfully got
 *              otherwise on failures
 */

const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
  if (relation.empty())
    return BADCATPARM;

  Status status;
  Record rec;
  RID rid;

	//cout<<"Getting info of relation: "<<relation<<endl;

  //invoke the startScan() to open a scan on the relcat relation
  HeapFileScan* scan = new HeapFileScan(RELCATNAME, status);
  if(status != OK) return status;
  status = scan->startScan(0, relation.length(), STRING, relation.c_str(), EQ);
  if(status != OK){ cout <<"error in cat.cpp, getInfo1\n"; return status;}
  //call scanNext() and getRecord() to get the desired tuple.
  status = scan->scanNext(rid);
  //no desired relation tuple found, return RELNOTFOUND
  if(status == FILEEOF) return RELNOTFOUND;
  if(status != OK) return status;
  status = scan->getRecord(rec);
  if(status != OK){ cout<<"error in cat.cpp, getInfo2\n"; return status;}
  //memcpy() the tuple out of the bufPool into the return para record
  memcpy(&record, rec.data, rec.length);
  
  //just for debugging, compare the memcpy result
  if(strncmp((char*)rec.data, record.relName, MAXNAME))
  {
	cout<<"data: "<<(char*)rec.data<<" not matching relName: "<<record.relName<<endl;
  }
  scan->endScan();
  delete scan;
  scan = NULL;
  return OK;
}

/* Add information to catalog
 *
 *  @param RelDesc record the RelDEsc instance containing the information of the relation
 *  @return OK on successful addition
 *  	otherwise on failures
 */

const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
  //check to see if relation already exists
  const string tmpRel(record.relName);
  RelDesc tmpRec;
  status = getInfo(tmpRel, tmpRec);
  if(status != RELNOTFOUND)
  {
  	if(status != OK)
	{
		cout<<"err in cat.cpp, addInfo\n";
		return status;
	}
	return RELEXISTS;
  }
  //creat an InsertFileScan object on the relCatalog table
  ifs = new InsertFileScan(RELCATNAME, status);
  if(status != OK) return status;
  //create a record
  Record newRec;
  newRec.data = &record;
  newRec.length = sizeof(RelDesc);
  //insert it into the relCat table using the insertRecord of InsertFileScan
  status = ifs->insertRecord(newRec, rid);
  if(status != OK) return status;
  delete ifs;
  ifs = NULL;
  return OK;

}

/* Remove tuple from catalog
 * 
 * @param string relation the relation name it trirs to remove
 *
 * @return OK on successful removement
 * 	otherwise on failures
 *
 */

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;

  //start a filter scan on relcat to locate the rid of desired tuple
  hfs = new HeapFileScan(RELCATNAME, status);
  if(status != OK) return status;
  status = hfs->startScan(0, relation.length(), STRING, relation.c_str(), EQ);
  if(status != OK) { cout<<"error in cat.cpp, removInfo\n"; return status;}
  status = hfs->scanNext(rid);
  if((status != OK) && (status != FILEEOF)){ cout <<"err in cat.cpp, rmInfo\n"; return status;} 
  //call deleteRecord() to remove it
  if(status == OK) status = hfs->deleteRecord();
  if((status != OK) && (status != NORECORDS) && (status != FILEEOF)) return status;
  hfs->endScan();
  delete hfs;
  hfs = NULL;
  if(status != OK) return RELNOTFOUND;
  return OK;

}

/*
 * get rid of catalog
 */
RelCatalog::~RelCatalog()
{
// nothing should be needed here
}


/*
* constructor
* open the heapfile contains attribute catalog
*/
AttrCatalog::AttrCatalog(Status &status) :
	 HeapFile(ATTRCATNAME, status)
{
// nothing should be needed here
}

/* Get the attribute descriptor record for attribute attrName in relation relName
 * 
 * @param string relation the relation name it tries to get the info of
 *        string attrName the attribute name it tries to get info of the relation
 *        AttrDesc record  return the attrDesc through this param
 * @return OK on successful search
 *         otherwise on failures
 */
const Status AttrCatalog::getInfo(const string & relation, 
				  const string & attrName,
				  AttrDesc &record)
{  
  Status status;
  RID rid = NULLRID;
  Record rec;
  HeapFileScan*  hfs = NULL;
  AttrDesc *tmp;
  const char *crelation = relation.c_str();
  const char *cattriname = attrName.c_str();
  
  if (relation.empty() || attrName.empty()) return BADCATPARM;
  hfs = new HeapFileScan(ATTRCATNAME, status); //new heapfile scan obj
  if (status != OK) return status;
  //initialize a file scanner, using relation name as filter first
  status = hfs->startScan(0, strlen(crelation), STRING, crelation, EQ);
  if (status != OK) return status;
  //scan the heap file
  while((status = hfs->scanNext(rid)) != FILEEOF){
    status = hfs->getRecord(rec);
    if(status != OK) return status;
    tmp = (AttrDesc*)rec.data;
    //use c string to comapre
    if(strncmp(tmp->attrName, cattriname, strlen(cattriname)) == 0){
      //matching relation and matching attrName
      record = *tmp;
      hfs->endScan();
      delete hfs;
      hfs = NULL;
      return OK;
    }
  }
  hfs->endScan();
  delete hfs;
  hfs = NULL;
  //if out of the while loop; 
  //2 cases: 1. we have found the right relation but no right attrname
  //         2. we have not found the right relation
  if(rid.pageNo == -1 || rid.slotNo == -1) return RELNOTFOUND;
  
  return ATTRNOTFOUND;
}

/* add the attribute descriptor record
 * 
 * @param  AttrDesc record attrDesc user you want to add to the attri catalog
 * @return OK on successful search
 *         otherwise on failures
 */
const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
  AttrDesc tmp;
  const string relation(record.relName);
  const string attrName(record.attrName);
   //check for duplicates
  status = getInfo(relation, attrName, tmp);
  if(status == OK) return DUPLATTR;
  Record toAdd;
  toAdd.data = &record;
  toAdd.length = sizeof(record);
  ifs = new InsertFileScan(ATTRCATNAME, status);
  if(status != OK) return status;
  status = ifs->insertRecord(toAdd, rid);
  delete ifs;
  ifs = NULL;
  return status;
}

/* Removes the tuple from attrcat that corresponds to attribute attrName of relation.
 * 
 * @param string relation the relation name the to be removed tuple has
 *        string attrName the attribute name the to-be-removed tuple has
 * @return OK on successful search
 *         otherwise on failures
 */
const Status AttrCatalog::removeInfo(const string & relation, 
			       const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  HeapFileScan*  hfs;
  AttrDesc *tmp;
  //convert the relation name and attribute name into c-string
  const char *crelation = relation.c_str();
  const char *cattriname = attrName.c_str();
  if (relation.empty() || attrName.empty()) return BADCATPARM;
  hfs = new HeapFileScan(ATTRCATNAME, status); //new heapfile scan obj
  if (status != OK) return status;
  status = hfs->startScan(0, strlen(crelation), STRING, crelation, EQ);
  if (status != OK) return status;
  // scan the heapfile using the filter -> find matching relation name
  while((status = hfs->scanNext(rid)) != FILEEOF){
    status = hfs->getRecord(rec);
    if(status != OK) return status;
    tmp = (AttrDesc*)rec.data;
    //use c string to compare
    if(strncmp(tmp->attrName, cattriname, strlen(cattriname)) == 0){
      //matching relation and matching attrName
      hfs->deleteRecord();
      hfs->endScan();
      delete hfs;
      hfs = NULL;
      return OK;
    }
  }
  hfs->endScan();
  delete hfs;
  hfs = NULL;
  return ATTRNOTFOUND;
}


/*
 * Get (by reference) descriptors for all attributes of the relation via an array 
 * of AttrDesc structures, and return the number of attributes in attrCnt
 * @param string relation the relation name of the relation from which we want to get attrs
 *        int attrCnt the number of attributes that belongs to this relation
 *        AttrDesc* attrs the array to be returned that contains the attrs that belong to
 *                  the relation
 * @return OK on successful search
 *         otherwise on failures
 */
const Status AttrCatalog::getRelInfo(const string & relation, 
				     int &attrCnt,
				     AttrDesc *&attrs)
{
  Status status;
  RID rid = NULLRID; 
  Record rec;
  HeapFileScan*  hfs;
  int i = 0;
  int maxCnt = -1;
  const char *crelation = relation.c_str();
  if (relation.empty()) return BADCATPARM;
  hfs = new HeapFileScan(ATTRCATNAME, status); //new heapfile scan obj
  if(status != OK) return status;
  //get the maximum length of the array
  maxCnt = hfs->getRecCnt();
  attrs = new AttrDesc[maxCnt];
  status = hfs->startScan(0, strlen(crelation), STRING, crelation, EQ);
  if (status != OK) return status;
  while((status = hfs->scanNext(rid)) != FILEEOF){
    status = hfs->getRecord(rec);
    if(status != OK) return status;
    attrs[i] = *((AttrDesc*)(rec.data));
    i++;
  }
  if(rid.pageNo == -1 || rid.slotNo == -1) return RELNOTFOUND;
  //get the attribute number
  attrCnt = i;
  hfs->endScan();
  delete hfs;
  hfs = NULL;
  return OK;
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}
