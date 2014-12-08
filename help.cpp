#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
#include <iomanip>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for
// the user. If no relation is given (relation.empty() is true), then
// it lists all the relations in the database, along with the width in
// bytes of the relation, the number of attributes in the relation,
// and the number of attributes that are indexed.  If a relation is
// given, then it lists all of the attributes of the relation, as well
// as its type, length, and offset, whether it's indexed or not, and
// its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;

  if (relation.empty()) return UT_Print(RELCATNAME);

  status = getInfo(relation, rd);
  if(status != OK) return status;
  status = attrCat->getRelInfo(relation, attrCnt, attrs);
  if(status != OK) return status;
  cout<<"Relation name: "<<rd.relName<<" ("<<attrCnt<<" attributes)\n";
  cout<<right<<setfill(' ')
      <<setw(16)<<"Attribute name"
      <<setw(6)<<"Off"
      <<setw(4)<<"T"
      <<setw(7)<<"Len\n"
      <<right<<setfill('-')
      <<setw(17)<<' '
      <<setw(6)<<' '
      <<setw(4)<<' '
      <<"-----"<<endl;
  for(int i = 0; i < attrCnt; i++)
  {
  	cout<<right<<setfill(' ')
	    <<setw(16)<<attrs[i].attrName
	    <<setw(6)<<attrs[i].attrOffset
	    <<setw(4)<<attrs[i].attrType
	    <<setw(6)<<attrs[i].attrLen<<endl;
  }
  delete [] attrs;
  return OK;
}
