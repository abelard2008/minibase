#include <ostream>
#include "page.h"

// Methods of the class Page
//***********************************************************

// Constructor
Page::Page()
{
#ifdef PAGE_DEBUG
    std::cout << "constructing page"<<endl;
#endif
}

//*************************************************************

// Destructor
Page::~Page()
{
#ifdef PAGE_DEBUG
    std::cout << "Destroying page"<<endl;
#endif
}

//************************************************************
