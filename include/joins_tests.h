#include "minirel.h"

#include "iterator.h"

class JoinsTests {

public:

JoinsTests();

Status runTests();
void Query1();
void Query2();
void Query3();
void Query4();
void Query5();

private:
void Query1_CondExpr(CondExpr **&expr);
void Query2_CondExpr(CondExpr **&expr, CondExpr **&expr2);
void Query3_CondExpr(CondExpr **&expr);
void Query5_CondExpr(CondExpr **&expr2);

void Disclaimer(void);

};

// class RelnScan;





