#include <iostream>
#include <vector>
#include <string>
using namespace std;

class net { 
public:
    net(int id, int x1, int y1, int x2, int y2)
        :id(id),
        pin_1(pair<int, int>(x1, y1)),
        pin_2(pair<int, int>(x2, y2))
    {}
    
    int id;
    pair<int, int> pin_1;
    pair<int, int> pin_2;

    vector<pair<int, int>> route_path_vec;
};