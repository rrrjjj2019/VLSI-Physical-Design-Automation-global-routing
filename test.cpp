#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <set>
#include <ctime>
#include <math.h>
#include <assert.h>
#include <cstdlib>
#include <ctime>

using namespace std;

void test(const pair<int, int> &a){
    cout << "a = " << a.first << ", "<< a.second << endl;
}

int main(){
    pair<int, int> a (1, 1);
    pair<int, int> b (1, 0);

    cout << "a = " << a.first << ", "<< a.second << endl;
    cout << "b = " << b.first << ", "<< b.second << endl;

    cout << (a==b) << endl;

    test(a);
}