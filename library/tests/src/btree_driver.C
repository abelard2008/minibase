#include <math.h> 
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pwd.h>

#include "buf.h"
#include "db.h"
#include "btfile.h"

#include "component-tests.h"

#define MAX_COMMAND_SIZE 100


Status BTreeTest::runTests(){

	Status status;

	char real_logname[50];		// the dbname in the case
	char real_dbname[55];		// is followed by the login id.

	// create MINIBASE_DB and MINIBASE_BM

	sprintf(real_logname, "/bin/rm -rf /tmp/btlog");
	sprintf(real_dbname, "/bin/rm -rf /tmp/BTREEDRIVER");

	system(real_logname);
	system(real_dbname);
	
	minibase_globals = new SystemDefs(status,"/tmp/BTREEDRIVER", "/tmp/btlog",
						  1000,500,200,"Clock");

	if (status != OK) {
		minibase_errors.show_errors();
		exit(1);
	}


//	MINIBASE_BM = new BufMgr;
	
	int choice;
	while(1) {
		choice = 0;
		while((choice <1) || (choice >5)) {
			menu();
			std::cout << "Enter your choice[1-5]:  ";
			std::cin >> choice;
			std::cin.ignore(100, '\n');
		}

		switch(choice) {
			case 1:
				test1();
				break;
			case 2:
				test2();
				break;
			case 3:
				test3();
				break;
			case 4:
				test4();
				break;
			case 5:
				delete minibase_globals;
				sprintf(real_logname, "/bin/rm -rf /tmp/btlog");
				sprintf(real_dbname, "/bin/rm -rf /tmp/BTREEDRIVER");
				system(real_logname);
				system(real_dbname);
				return OK;

			default:
				return OK;
		}
	}
 return OK;
}

void BTreeTest::menu() {
	std::cout << "\n[1] Test1 integer key, random\n";
	std::cout << "    	test create, insert, delete and scans\n";
	std::cout << "[2] Test2 integer key\n";
	std::cout << "    	test open, strange scans, destroy\n";
	std::cout << "       destroys BTree - can only be run once\n";
	std::cout << "[3] Test3 - full deletion/shrinking of the tree\n";
	std::cout << "       test create, insert, deletion \n";
	std::cout << "[4] Test4 string key, ascendant\n";
	std::cout << "    	test create, insert, delete, scan(including \n";
	std::cout << "		  strange scans) and destroy\n";
	std::cout << "[5] Quit \n";
}

void BTreeTest::test_scan(IndexFileScan* scan){
	if(scan == NULL) {
		std::cout << "Cannot open a scan." << std::endl;
		minibase_errors.show_errors();
		exit(1);
	}   
	std::cout << "\n----------------Start scan--------------------\n"<< std::endl;

	Status status;
	RID rid;
	int count = 0;

	status = OK;
	while (status == OK) {
//BK
		void* temp = new char[scan->keysize()];
		status = scan->get_next(rid, temp);
		if (status == OK)  {
//	    		std::cout << "Scanning record with [pageNo,slotNo] = ";
//	    		std::cout << "[" << rid.pageNo<<","<<rid.slotNo<<"]"<< std::endl;
			count++;
		}
		else  if (status != DONE)
	    		minibase_errors.show_errors();

		delete [] temp;  // BK
	}

        if (status != DONE){
	  std::cout << "Status = " << status << std::endl;
	  minibase_errors.show_errors();
        }



	std::cout << "\nNumber of records scanned = " << count << std::endl;
	std::cout << "----------------End of scan-----------------------\n";
}

void BTreeTest::test1() {

    std::cout << "\n---------test1()  key type is Integer--random------\n";

    Status status;
    BTreeFile *btf;
    IndexFileScan* scan;
    
    int key, lokey, hikey,i;
    RID rid;
    int num;
    
    std::cout << "Page size: " << MINIBASE_PAGESIZE << " MAX_SPACE: " << MAX_SPACE << std::endl;

    // test create()
	// if index exists, open it else create
    btf = new BTreeFile(status, "BTreeIndex", attrInteger, sizeof(int));
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }
    std::cout << "BTreeIndex created successfully." << std::endl << std::endl;

	PrintInfo(btf);

    // test insert()
    std::cout << "How many key / rid pairs do you want to insert?\n";
    std::cout << "I recommend 20 for a small test and the maximum allowed of 4000 for a big test.\n";
    std::cin >> num;
    std::cin.ignore(100, '\n');
    
    std::cout << "\n---------Start to insert " << num << "  records--------" << std::endl;

struct dummy{
RID r;
int key;
};

    dummy kill[410];
    for (i = 0; i < num; i++) {
        rid.pageNo = i; rid.slotNo = i+1;
        key = rand() % num ;
	if (i % 10 == 0) {
	  kill[(i/10)].r = rid;
	  kill[i/10].key = key;
        }
	// std::cout << "Inserting record with key = " << key << "  [pageNo,slotNo] = ";
	// std::cout << "[" << rid.pageNo<<","<<rid.slotNo<<"]"<< std::endl;

        if (btf->insert(&key, rid) != OK) {
            minibase_errors.show_errors();
        }
    }
    std::cout << "\n----------- End of insert--------------------" << std::endl;

    PrintInfo(btf);
   

    // test delete()
    std::cout << "\n----------- Delete 10% of the records-----------" << std::endl;

    int j = 0;
    for (i = 0; i < num; i++) {
        if (i % 10 == 0) {
	    j++;
	    rid.pageNo = i; rid.slotNo = i+1;
	    key = i;
	    std::cout << "Deleting record with key = " << kill[i/10].key << "  [pageNo,slotNo] = ";
	    std::cout << "[" << kill[i/10].r.pageNo<<","<<kill[i/10].r.slotNo<<"]"<< std::endl;
	    if (btf->Delete(&kill[i/10].key, kill[(i/10)].r) != OK) {
	       minibase_errors.show_errors();
	    }
	    /*
            rid.pageNo = i; rid.slotNo = i+1;
            key = i;
	    std::cout << "Deleting record with key = " << key << "  [pageNo,slotNo] = ";
	    std::cout << "[" << rid.pageNo<<","<<rid.slotNo<<"]"<< std::endl;
            if (btf->Delete(&key, rid) != OK) {
                minibase_errors.show_errors();
            }  */
        }
    }
    std::cout << "Deleted " << j << "records " << std::endl;
    std::cout << "\n----------- End of delete----------------------" << std::endl;

    PrintInfo(btf);
/*
#ifdef DEBUG
    btf->PrintHeader();
    btf->PrintRoot();
    btf->PrintPage(3); 	// this is the first leaf page
    btf->PrintPage(5);  // this is the first internal page
#endif DEBUG
*/


    delete btf;

    btf = new BTreeFile(status, "BTreeIndex");
  
    // test scan and delete_current()
    std::cout << "\n----------- Testing scans -------------" << std::endl;
    lokey = 100;
    hikey = 10;
    while (lokey > hikey){
    	std::cout<<"lokey = "; std::cin>>lokey;
    	std::cout<<"hikey = "; std::cin>>hikey; 
    	std::cout<< std::endl;
	if (lokey > hikey)
	    std::cout << " lokey should be less than hikey " << std::endl;
    }

    //AllScan
    scan = btf->new_scan(NULL,NULL);
    std::cout << "\n---------------Start AllScan---------------" << std::endl;

    test_scan(scan);
    delete scan;   
	
   // PrintInfo(btf);
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();


    //MaxRangeScan
    scan = btf->new_scan(NULL, &hikey);
    std::cout << "\n-------Start MaxRangeScan with hikey = "<<hikey<<"------\n";
    test_scan(scan);
    delete scan;

  //  PrintInfo(btf);
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();

    //MinRangeScan;
    scan = btf->new_scan(&lokey, NULL);
    std::cout << "\n-----Start MinRangeScan with lokey = "<<lokey<<"------\n";
    test_scan(scan);
    delete scan;   

 //   PrintInfo(btf);
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();
    
    //ExactMatch
    scan = btf->new_scan(&hikey, &hikey);
    std::cout << "\n--------Start ExactMatch with key = " <<hikey <<"-------\n";
    test_scan(scan);
    delete scan;

    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();
   
    //MinMaxRangeScan with delete_current()
    scan = btf->new_scan(&lokey, &hikey);
    std::cout << "\n------Start MinMaxRangeScan with lokey = "<<lokey   \
	    << " hikey = "<<hikey<<"------\n";
    std::cout << "Will also perform delete_current()\n";
  
    int count = 0;
    status = OK;
    while (status == OK) {
	    void* temp = new char[scan->keysize()];    // BK
	    if ((status = scan->get_next(rid, temp)) == OK)  {
		count++;
		if ((status = scan->delete_current()) == OK) {
		  std::cout << "Record with [pageNo,slotNo] = ";
		  std::cout << "[" << rid.pageNo<<","<<rid.slotNo<<"] deleted"<< std::endl;
		}
		else
		{
		  std::cout << "Failure to delete record...\n";
		  minibase_errors.show_errors();
                }
             delete [] temp;  // BK
	    }
//	    else if (status != DONE)
//	   	 minibase_errors.show_errors();
    }

    if (status != DONE) {
      std::cout << "Something is wrong in test1\n";
      minibase_errors.show_errors();
    }

    std::cout << "Number of records scanned = " << count << std::endl;
    std::cout << "\n---------------End of scan-----------------------\n";
    delete scan;
    
    PrintInfo(btf);

    delete btf;

	/*
    // test destroyFile()

    btf = new BTreeFile(status, "BTreeIndex");

    std::cout << "\n-------Start to destroy the index----------" << std::endl;
    status = btf->destroyFile();
    if (status != OK)
        minibase_errors.show_errors();

    delete btf;
	*/
}

void BTreeTest::test2() {

	std::cout << "\n-------------test2()  key type is Integer------------\n";
//	std::cout << "BK changed the BTree code to make these tests succeed.\n";
	std::cout <<"\nAll abnormal scans return no records. They do not cause errors.\n";

	Status status;
	BTreeFile *btf;
	IndexFileScan* scan;

	int lokey, hikey;
	RID rid;
//   int num;

	// test open()

	btf = new BTreeFile(status, "BTreeIndex");
	if (status != OK) {
		minibase_errors.show_errors();
		std::cout << "You should run test1 first to create the index!\n";
		return;
	}
	std::cout << "BTreeIndex opened successfully." << std::endl;

	//test abnormal scans
	//lokey > hikey
	lokey = 1000;
	hikey = 100;
	scan = btf->new_scan(&lokey,&hikey);
	std::cout << "\n------Start MinMaxRangeScan with lokey = " << lokey \
		 << " hikey = " << hikey << "--------\n";


	void* temp = new char[scan->keysize()];   // BK
	if ((status = scan->get_next(rid, temp)) == OK)  {
		std::cout << "Error: find next??? no way!" << std::endl;
        minibase_errors.show_errors();
		exit(1);
	}

	if(status != DONE)
	   minibase_errors.show_errors();
	delete scan;
    
	std::cout << " Failed as expected " << std::endl;
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();

	//lokey > largest key
	lokey = 10000;
	hikey = 10010;
	scan = btf->new_scan(&lokey,&hikey);
	std::cout << "\n-----Start MinMaxRangeScan with lokey = " << lokey \
			 << " hikey = " << hikey << "-----\n";

	if ((status = scan->get_next(rid, temp)) == OK)  {
		std::cout << "Error: find next??? no way!" << std::endl;
        minibase_errors.show_errors();
		exit(1);
	}

	
	if(status != DONE)
	   minibase_errors.show_errors();
	delete scan;

	std::cout << " Failed as expected " << std::endl;
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();

	//hikey < smallest key
	lokey = -100;
	hikey = -50;
	scan = btf->new_scan(&lokey,&hikey);
		std::cout << "\n-----Start MinMaxRangeScan with lokey = " << lokey \
			 << " hikey = " << hikey << "------\n";


	if ((status = scan->get_next(rid, temp)) == OK)  {
		std::cout << "Error: find next??? no way!" << std::endl;
        minibase_errors.show_errors();
		exit(1);
	}

	if(status != DONE)
	   minibase_errors.show_errors();
	delete scan;

	std::cout << " Failed as expected " << std::endl;
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();

	// test destroyFile()
	std::cout << "\n------Start to destroy the index------" << std::endl;

	status = btf->destroyFile();
	if (status != OK)
			minibase_errors.show_errors();
	std::cout << "\n------Destroyed the index without any errors---" << std::endl;

	delete [] temp;  // BK
	delete btf;
}

void BTreeTest::test4() {

    std::cout << "\n--------test4() key type is String---------\n";

    Status status;
    BTreeFile *btf;
    IndexFileScan* scan;
    
    int keysize = MAX_KEY_SIZE1;
    char*  key = new char[keysize];
    char*  lokey = new char[keysize];
    char*  hikey = new char[keysize];
    int	i = 0;
    RID rid, lorid;
//    ifstream keysamples("keys");
    std::ifstream keysamples;

	keysamples.open("keys",std::ios::in);
    if (!keysamples)
    {
      std::cout << "keys not found.\n";
	  std::cout << " there is a copy in $MINIBASE_HOME/programs/minibase "<< std::endl;
      return;
    }

    std::cout << "Page size: " << MINIBASE_PAGESIZE << " MAX_SPACE: " << MAX_SPACE << std::endl;

    // test create()
    btf = new BTreeFile(status, "BTreeIndex", attrString, keysize);
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }
    std::cout << "BTreeIndex created successfully." << std::endl;

	PrintInfo(btf);

    // test insert()
    std::cout << "\n------Start to insert records--------" << std::endl;

    while(!keysamples.eof()) {
//	std::cout << "Inserting " << i << std::endl;
        keysamples.getline(key, keysize, '\n');
	std::cout << "Inserting " << i << "\t\tkey = " << key << std::endl;
		rid.pageNo = (int)(key[0]+key[1]+key[2]);
		rid.slotNo = rid.pageNo;
        if (btf->insert(key, rid) != OK) {
            minibase_errors.show_errors();
        }
	
		i++;
		if(i==20) strncpy(lokey,key,keysize);
		if(i==100) strncpy(hikey,key,keysize);
    }
    std::cout << "\nNumber of records inserted is " << i << std::endl;
    std::cout << "\n--------------End of insert----------------" << std::endl;

	PrintInfo(btf);

    // test delete()
    std::cout << "\n------Start to delete some records----------" << std::endl;
    std::cout << "Actually only one record will be deleted.\n";

    // delete the lokey
    lorid.pageNo = (int)(lokey[0]+lokey[1]+lokey[2]);
    lorid.slotNo = lorid.pageNo;

    if (btf->Delete(lokey, lorid) != OK)
                minibase_errors.show_errors();
    else
		std::cout << "\nSuccessfully deleted record with key = " << lokey << std::endl;
    std::cout << "\n---------------End of delete----------------" << std::endl;

	PrintInfo(btf);
/*
#ifdef DEBUG
    btf->PrintHeader();
    btf->PrintRoot();
    btf->PrintPage(3);  // this is the first leaf page
    btf->PrintPage(5);  // this is the first internal page
#endif DEBUG
*/

    delete btf;
    btf = new BTreeFile(status, "BTreeIndex");
  
    // test scan and delete_current
    //AllScan     
    scan = btf->new_scan(NULL,NULL);                                            
    std::cout << "\n---------------Start AllScan------------" << std::endl;
    test_scan(scan);                                                            
    delete scan;                                                                
	
	PrintInfo(btf);

    //MaxRangeScan                                                              
    scan = btf->new_scan(NULL, hikey);                                         
    std::cout << "\n------Start MaxRangeScan with hikey = "<<hikey<< "------\n";                   
    test_scan(scan);                                                            
    delete scan;                                                                
    
	PrintInfo(btf);

    //MinRangeScan;                                                        
    scan = btf->new_scan(lokey, NULL);                                         
    std::cout << "\n-----Start MinRangeScan with lokey = "<<lokey<< "------\n";                   
    test_scan(scan);                                                            
    delete scan;                                                                
																			
	PrintInfo(btf);

    //ExactMatch                                                                
    scan = btf->new_scan(hikey, hikey);                                       
    std::cout << "------Start ExactMatch with key = " <<hikey << "----\n";                     
    test_scan(scan);                                                            
    delete scan;
                                             
    //MinMaxRangeScan
    scan = btf->new_scan(lokey, hikey);
    std::cout << "--------Start MinMaxRangeScan-----------" << std::endl;
    if(scan == NULL) {
	std::cout << "Cannot open a scan." << std::endl;
    }

    std::cout << "------Start scan with lokey = "<<lokey << " hikey = "<<hikey \
		<< "-----" << std::endl;

    int count = 0;
    status = OK;
    while (status == OK) {
	void* temp = new char[scan->keysize()];   // BK
        if ((status = scan->get_next(rid, temp)) == OK)  {
	    count++;
            if ((status = scan->delete_current()) == OK) {
	    	std::cout << "Record with [pageNo,slotNo] = ";
                std::cout << "[" << rid.pageNo<<","<<rid.slotNo<<"] deleted"<< std::endl;
	    }
            else
                minibase_errors.show_errors();
        }
//        else
//            minibase_errors.show_errors();

        delete [] temp;  // BK
    }

    if (status != DONE)
    {
     std::cout << "Problem...\n";
     minibase_errors.show_errors();
    }
    std::cout << "Number of records scanned = " << count << std::endl;
    std::cout << "-------------End of scan---------------\n";
    delete scan;

	PrintInfo(btf);

	std::cout << "\n---------Testing abnormal scans--------\n";

	// test abnormal scans
	// lokey > hikey
	strcpy(lokey, "zabcd");
	strcpy(hikey, "abcde");
	scan = btf->new_scan(lokey, hikey);
	std::cout << "\n----Start MinMaxRangeScan with lokey = " << lokey ;
	std::cout << " hikey = " << hikey << "-------\n" ;
	std::cout << "-------- lokey > hikey ---" << std::endl;

	char *temp1 = new char[MAX_KEY_SIZE1];
	if ((status = scan->get_next(rid, temp1)) == OK) {
		std::cout << " Error: find next??? nothing to find!!" << std::endl;
		minibase_errors.show_errors();
		exit(1);
	}

	if (status != DONE)
		minibase_errors.show_errors();
	delete scan;

	std::cout << " Failed as expected: no records scanned " << std::endl;
	std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
	getchar();

	// lokey > the largest key 
	strcpy(lokey, "zabcd");
	strcpy(hikey, "zcdef");
	scan = btf->new_scan(lokey, hikey);
	std::cout << "\n----Start MinMaxRangeScan with lokey = " << lokey ;
	std::cout << " hikey = " << hikey << "-------\n" ;
	std::cout << " -------lokey is greater the highest key" << std::endl;

	if ((status = scan->get_next(rid, temp1)) == OK) {
		std::cout << " Error: find next??? nothing to find!!" << std::endl;
		minibase_errors.show_errors();
		exit(1);
	}

	if (status != DONE)
		minibase_errors.show_errors();
	delete scan;

	std::cout << " Failed as expected: no records scanned " << std::endl;
	std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
	getchar();

	// hikey < smallest key
	strcpy(lokey, "aaa");
	strcpy(hikey, "aaaaa");
	scan = btf->new_scan(lokey, hikey);
	std::cout << "\n----Start MinMaxRangeScan with lokey = " << lokey ;
	std::cout << " hikey = " << hikey << "-------\n" ;
	std::cout << " -------hikey is smaller the smallest key" << std::endl;

	if ((status = scan->get_next(rid, temp1)) == OK) {
		std::cout << " Error: find next??? nothing to find!!" << std::endl;
		minibase_errors.show_errors();
		exit(1);
	}

	if (status != DONE)
		minibase_errors.show_errors();
	delete scan;

	std::cout << " Failed as expected: no records scanned " << std::endl;
	std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);

    delete temp1; 
    delete btf;
    
    // test destroyFile()

    btf = new BTreeFile(status, "BTreeIndex");

    std::cout << "-------Start to destroy the index-----------" << std::endl;
    status = btf->destroyFile();
    if (status != OK)
        minibase_errors.show_errors();

    delete btf;
    delete key;
    delete lokey;
    delete hikey;
}

void BTreeTest::test3() {


    Status status;
    BTreeFile *btf;
    IndexFileScan* scan;
    
    int key, lokey, hikey;
    RID rid;
    int num = 2000;
	int num_deletes = 1950;
    
	std::cout << "\n---------test3()  key type is Integer--------------\n";
	std::cout << "  ------testing the full deletion/shrinking -------\n";
	std::cout << "\n -----inserting "<<num<<" keys and deleting "<<num_deletes<<" ------\n";
    
    std::cout << "Page size: " << MINIBASE_PAGESIZE << " MAX_SPACE: " << MAX_SPACE << std::endl;

    // test create()
	// if index exists, open it else create
    btf = new BTreeFile(status, "BTreeIndex", attrInteger, sizeof(int));
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }
    std::cout << "BTreeIndex created successfully." << std::endl << std::endl;

	PrintInfo(btf);

    // test insert()
    
    std::cout << "\n---------Start to insert " << num << "  records--------" << std::endl;

struct dummy{
RID r;
int key;
};

	for (int i=0; i < num; i++){
		rid.pageNo = i;
		rid.slotNo = i+1;
		key = i;
        if (btf->insert(&key, rid) != OK) {
            minibase_errors.show_errors();
        }
    }
    std::cout << "\n---------- End of insert--------------------" << std::endl;

    PrintInfo(btf);
   

    // test delete()
    std::cout << "\n---------- Delete the first 500 of the records-----" << std::endl;

    int j = 0;
    for (int i = 0; i < num_deletes; i++) {
	    j++;
	    rid.pageNo = i; rid.slotNo = i+1;
	    key = i;
	    std::cout << "Deleting record with key = " << key << "  [pageNo,slotNo] = ";
	    std::cout << "[" << rid.pageNo<<","<< rid.slotNo << "]" << std::endl;
	    if (btf->Delete(&key, rid) != OK) {
	       minibase_errors.show_errors();
	    }
    }
    std::cout << "Deleted " << j << "records " << std::endl;
    std::cout << "\n----------- End of delete----------------------" << std::endl;

    PrintInfo(btf);

    delete btf;

    btf = new BTreeFile(status, "BTreeIndex");
  
    // test scan and delete_current()
    std::cout << "\n----------- Testing scans -------------" << std::endl;
    lokey = 100;
    hikey = 10;
    while (lokey > hikey){
    	std::cout<<"lokey = "; std::cin>>lokey;
    	std::cout<<"hikey = "; std::cin>>hikey; 
    	std::cout<< std::endl;
	if (lokey > hikey)
	    std::cout << " lokey should be less than hikey " << std::endl;
    }

    //AllScan
    scan = btf->new_scan(NULL,NULL);
    std::cout << "\n---------------Start AllScan---------------" << std::endl;

    test_scan(scan);
    delete scan;   
	
   // PrintInfo(btf);
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();


    //MaxRangeScan
    scan = btf->new_scan(NULL, &hikey);
    std::cout << "\n-------Start MaxRangeScan with hikey = "<<hikey<<"------\n";
    test_scan(scan);
    delete scan;

  //  PrintInfo(btf);
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();

    //MinRangeScan;
    scan = btf->new_scan(&lokey, NULL);
    std::cout << "\n-----Start MinRangeScan with lokey = "<<lokey<<"------\n";
    test_scan(scan);
    delete scan;   

 //   PrintInfo(btf);
    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();
    
    //ExactMatch
    scan = btf->new_scan(&hikey, &hikey);
    std::cout << "\n--------Start ExactMatch with key = " <<hikey <<"-------\n";
    test_scan(scan);
    delete scan;

    std::cout<<"\n--Hit a return to continue\n"<< std::endl; fflush(stdout);
    getchar();
   
    //MinMaxRangeScan with delete_current()
    scan = btf->new_scan(&lokey, &hikey);
    std::cout << "\n------Start MinMaxRangeScan with lokey = "<<lokey   \
	    << " hikey = "<<hikey<<"------\n";
    std::cout << "Will also perform delete_current()\n";
  
    int count = 0;
    status = OK;
    while (status == OK) {
	    void* temp = new char[scan->keysize()];    // BK
	    if ((status = scan->get_next(rid, temp)) == OK)  {
		count++;
		if ((status = scan->delete_current()) == OK) {
		  std::cout << "Record with [pageNo,slotNo] = ";
		  std::cout << "[" << rid.pageNo<<","<<rid.slotNo<<"] deleted"<< std::endl;
		}
		else
		{
		  std::cout << "Failure to delete record...\n";
		  minibase_errors.show_errors();
                }
             delete [] temp;  // BK
	    }
//	    else if (status != DONE)
//	   	 minibase_errors.show_errors();
    }

    if (status != DONE) {
      std::cout << "Something is wrong in test3\n";
      minibase_errors.show_errors();
    }

    std::cout << "Number of records scanned = " << count << std::endl;
    std::cout << "\n---------------End of scan-----------------------\n";
    delete scan;
    
    PrintInfo(btf);

    delete btf;

    // test destroyFile()
	std::cout << "\n-----------Destroying index----------\n";

    btf = new BTreeFile(status, "BTreeIndex");

    std::cout << "\n-------Start to destroy the index----------" << std::endl;
    status = btf->destroyFile();
    if (status != OK)
        minibase_errors.show_errors();

    delete btf;
}

void BTreeTest::PrintInfo(BTreeFile * btf) {
	while(1) {
		int choice = 0;
		std::cout << "Print Page:\n";
		while(choice<1 || choice >5) {
			std::cout << "[1] Print header page.\n";
			std::cout << "[2] Print root page.\n";
			std::cout << "[3] Print a selected page.\n";
			std::cout << "[4] Print all leaf pages.\n";
			std::cout << "[5] Quit.\n";
			std::cout << "Enter your choice[1-5]:  ";

			std::cin >> choice;
			std::cin.ignore(100, '\n');
		}

		switch(choice) {
			case 1: btf->PrintHeader();
					break;
			case 2: btf->PrintRoot();
					break;
			case 3: int pageno;
					std::cout << "Enter the page no:  ";
					std::cin >> pageno;
					btf->PrintPage(pageno);
					break;
			case 4: btf->PrintLeafPages();
					break;
			case 5: return;
					break;
			default:break;
		}
	}
}
