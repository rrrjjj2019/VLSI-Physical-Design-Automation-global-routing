#include <iostream>
#include <vector>
#include <string>
using namespace std;

class globalEdge { 
public:
    globalEdge(int row, int col, int supply, int demand)
        :row(row),
        col(col),
        supply(supply),
        demand(demand)
    {}
    
    double get_congestion_value(){
        return (double) demand / supply;
    }

    bool overflow(){
        return supply <= demand;
    }

    int row;
    int col;
    int supply;
    int demand;

    list<net*> nets_pass_through_list;
};