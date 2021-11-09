// inserting into a vector
#include <iostream>
#include <vector>


struct element {
  unsigned long long num;
  unsigned pos;
  unsigned len;
};

int main ()
{
  std::vector<int> myvector(3, 100);
  std::vector<int>::iterator it;

  //int a[32][32];


  it = myvector.begin();
  it = myvector.insert ( it , 200 );
  it = myvector.insert (it,1,9487);

//  myvector.insert (it,1,9487);

  // "it" no longer valid, get a new one:
  
  std::vector<int> anothervector (2,400);
  myvector.insert (it+2,anothervector.begin(),anothervector.end());

  int myarray [] = { 501,502,503 };
  myvector.insert (myvector.begin(), myarray, myarray+3);

  myvector.push_back(101);
  myvector.push_back(102);

  int temp;
  std::cout << "myvector contains:";
  for (it=myvector.begin(); it<myvector.end(); it++) {
    std::cout << ' ' << (*it);
    temp += *it;
  }
  std::cout << '\n';

  return 0;
}