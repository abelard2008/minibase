#ifndef _LH_TESTS_
#define _LH_TESTS_





class LinearHashTests {

 public: 
  LinearHashTests();
  
 private:
  void gen_rec(char * rec, int key, int len);
  
  void test0();
  void test1();
  void test2();
  void test3();
  void test4();
  void test5();
  
 public:
  Status runTests();
};

#endif


