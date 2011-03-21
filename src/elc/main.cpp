/* Copyright NICTA, 2011
 */

#include <wnic.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace net;
using namespace std;

int
main(int ac, char **av)
{
   if(2 != ac) {
      cerr << "usage: elc device" << endl;
      exit(EXIT_FAILURE);
   }

   const char *what = *++av;
   wmic_sptr s(wnic::open(what));

   // ToDo: compute ELC

   exit(EXIT_SUCCESS);
}
