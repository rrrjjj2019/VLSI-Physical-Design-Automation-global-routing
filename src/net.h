#include <iostream>
#include <vector>
#include <string>
using namespace std;

class net { 
public:
    net(int id, int x1, int y1, int x2, int y2){
        this->id = id;
        this->pin_1 = pair<int, int>(x1, y1);
        this->pin_2 = pair<int, int>(x2, y2);
        this->length = abs(pin_1.first - pin_2.first) + abs(pin_1.second - pin_2.second);
    }

    int length;

    int id;
    pair<int, int> pin_1;
    pair<int, int> pin_2;

    vector<pair<int, int>> route_path_vec;
};