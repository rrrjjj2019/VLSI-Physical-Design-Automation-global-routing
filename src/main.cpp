#define NDEBUG 
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
#include <list>
#include "net.h"
#include "globalEdge.h"
#define NUMPIN 2
#define INF 99999
using namespace std;

string input_fileName;
string output_fileName;

int horizontal_grids;
int vertical_grids;
int horizontal_capacity;
int vertical_capacity;
int num_net;

vector<net*> net_vec;

map<pair<int, int>, globalEdge*> horizontal_global_edge;
map<pair<int, int>, globalEdge*> vertical_global_edge;
map<pair<int, int>, double> maze_block_map;

vector<globalEdge*> overflow_horizontal_edges_vec;
vector<globalEdge*> overflow_vertical_edges_vec;

bool reroute_maze(const pair<int, int> &start, const pair<int, int> &end, const int &id);
void init_maze_block_map();
void reroute_far_away(net* reroute_net);

void reroute_L_shape(net*);

bool net_length_compare(const net* p1, const net* p2){
    return p1->length < p2->length;
}

void read_input(){
    fstream file;
    char buffer[60];
    string result;
    vector<string> buffer_without_space;

    file.open(input_fileName, ios::in);
    
    if(!file){
        cout << "Error opening input file" << endl;
    }else{
        do{
            file.getline(buffer, sizeof(buffer));
            stringstream input(buffer);
            buffer_without_space.clear();
            while(input >> result){
                buffer_without_space.push_back(result);
            }
            

            if(buffer_without_space.size() != 0){
                if(buffer_without_space[0] == "grid"){
                    horizontal_grids = stoi(buffer_without_space[1]);
                    vertical_grids = stoi(buffer_without_space[2]);
                }else if(buffer_without_space[0] == "vertical"){
                    vertical_capacity = stoi(buffer_without_space[2]);
                }else if(buffer_without_space[0] == "horizontal"){
                    horizontal_capacity = stoi(buffer_without_space[2]);
                }else if(buffer_without_space[0] == "num"){
                    num_net = stoi(buffer_without_space[2]);
                    net_vec.resize(num_net);
                }else if(buffer_without_space[0][0] == 'n' && buffer_without_space[0][1] == 'e' && buffer_without_space[0][2] == 't'){
                    int net_id = stoi(buffer_without_space[1]);
                    int x1;
                    int y1;
                    int x2;
                    int y2;

                    for(int i = 0; i < NUMPIN; i++){
                        file.getline(buffer, sizeof(buffer));
                        stringstream input(buffer);
                        buffer_without_space.clear();
                        while(input >> result){
                            buffer_without_space.push_back(result);
                        }

                        if(i == 0){
                            x1 = stoi(buffer_without_space[0]);
                            y1 = stoi(buffer_without_space[1]);
                        }else if(i == 1){
                            x2 = stoi(buffer_without_space[0]);
                            y2 = stoi(buffer_without_space[1]);
                        }

                        net_vec[net_id] = new net(net_id, x1, y1, x2, y2);
                    }
                }
            }
            
        }while(!file.eof());
        file.close();

        //sort(net_vec.begin(), net_vec.end(), net_length_compare);
    }
}

void debug_read_input(){
    for(int i=0; i<net_vec.size(); i++){
        cout << "net" << net_vec[i]->id << endl;
        cout << net_vec[i]->pin_1.first << " " << net_vec[i]->pin_1.second << endl;
        cout << net_vec[i]->pin_2.first << " " << net_vec[i]->pin_2.second << endl;
        cout << net_vec[i]->length << endl;
    }
}

void construct_global_edge_map(){
    //horizontal
    for(int i=0; i<horizontal_grids - 1; i++){
        for(int j=0; j<vertical_grids; j++){
            horizontal_global_edge[pair<int, int>(i, j)] = new globalEdge(i, j, horizontal_capacity, 0);
        }
    }

    //vertical
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            vertical_global_edge[pair<int, int>(i, j)] = new globalEdge(i, j, vertical_capacity, 0);
        }
    }

}

void debug_global_edge_map(){
    cout << " @@@@@@@@@@@@@@@@@@ debug_global_edge_map() @@@@@@@@@@@@@@@@@ " << endl;
    cout << "horizontal " << endl;
    for(int i=0; i<horizontal_grids - 1; i++){
        for(int j=0; j<vertical_grids; j++){
            cout << "row = " << horizontal_global_edge[pair<int, int>(i, j)]->row << ", col = " << horizontal_global_edge[pair<int, int>(i, j)]->col << 
                ", supply = " << horizontal_global_edge[pair<int, int>(i, j)]->supply << ", demand = " << horizontal_global_edge[pair<int, int>(i, j)]->demand << endl;
        }
    }

    cout << "vertical " << endl;
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            cout << "row = " << vertical_global_edge[pair<int, int>(i, j)]->row << ", col = " << vertical_global_edge[pair<int, int>(i, j)]->col << 
                ", supply = " << vertical_global_edge[pair<int, int>(i, j)]->supply << ", demand = " << vertical_global_edge[pair<int, int>(i, j)]->demand << endl;
        }
    }
}

void rounting_L_shape_or_maze(){
    srand(1);
    int random_var  = rand() % 2;

    for(int i=0; i<net_vec.size(); i++){
        int x1 = net_vec[i]->pin_1.first;
        int y1 = net_vec[i]->pin_1.second;
        int x2 = net_vec[i]->pin_2.first;
        int y2 = net_vec[i]->pin_2.second;

        int small_x;
        int large_x;
        int small_y;
        int large_y;

        if(x1 == x2){ //route with a straight vertical line

            if(y1 > y2){
                large_y = y1;
                small_y = y2;
            }else{
                large_y = y2;
                small_y = y1;
            }

            //update routing resources begin
            bool ov_flow = true;
            bool maze_route_success = false;
            for(int j = small_y; j < large_y; j++){
                if(vertical_global_edge[pair<int, int>(x1 , j)]->overflow()){
                    ov_flow = true;
                    break;
                }
            }

            if(ov_flow){
                maze_route_success = reroute_maze(net_vec[i]->pin_1, net_vec[i]->pin_2, i);
                if(!maze_route_success){
                    cout << "maze routing fail !!!, net id = " << net_vec[i]->id << endl;
                }
            }

            if(!maze_route_success){
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

                for(int j = small_y; j < large_y; j++){
                    vertical_global_edge[pair<int, int>(x1 , j)]->demand++;
                    vertical_global_edge[pair<int, int>(x1 , j)]->nets_pass_through_list.push_back(net_vec[i]);
                }
            }
            
            //update routing resources end

        }else if(y1 == y2){ //route with a straight horizontal line
            if(x1 > x2){
                large_x = x1;
                small_x = x2;
            }else{
                large_x = x2;
                small_x = x1;
            }

            //update routing resources begin
            bool ov_flow = true;
            bool maze_route_success = false;
            for(int j = small_x; j < large_x; j++){
                if(horizontal_global_edge[pair<int, int>(j , y1)]->overflow()){
                    ov_flow = true;
                    break;
                }
            }

            if(ov_flow){
                maze_route_success = reroute_maze(net_vec[i]->pin_1, net_vec[i]->pin_2, i);
                if(!maze_route_success){
                    cout << "maze routing fail !!!, net id = " << net_vec[i]->id << endl;
                }
            }

            if(!maze_route_success){
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

                for(int j = small_x; j < large_x; j++){
                    horizontal_global_edge[pair<int, int>(j , y1)]->demand++;
                    horizontal_global_edge[pair<int, int>(j , y1)]->nets_pass_through_list.push_back(net_vec[i]);
                }
            }
            
            //update routing resources end

        }else{ //route with a L-shaped line
            
            //random_var == 0 -> upper L;
            //random_var == 1 -> lower L;
            if(random_var == 0){
                

                if(x1 > x2){
                    large_x = x1;
                    small_x = x2;
                }else{
                    large_x = x2;
                    small_x = x1;
                }
                
                if(y1 > y2){
                    large_y = y1;
                    small_y = y2;
                }else{
                    large_y = y2;
                    small_y = y1;
                }

                //update routing resources begin
                bool ov_flow = true;
                bool maze_route_success = false;

                for(int j = small_y; j < large_y; j++){
                    if(vertical_global_edge[pair<int, int>(x1 , j)]->overflow()){
                        ov_flow = true;
                        break;
                    }
                }

                for(int j = small_x; j < large_x; j++){
                    if(horizontal_global_edge[pair<int, int>(j, y2)]->overflow()){
                        ov_flow = true;
                        break;
                    }
                }

                if(ov_flow){
                    maze_route_success = reroute_maze(net_vec[i]->pin_1, net_vec[i]->pin_2, i);
                    if(!maze_route_success){
                        cout << "maze routing fail !!!, net id = " << net_vec[i]->id << endl;
                    }
                }

                if(!maze_route_success){
                    net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
                    net_vec[i]->route_path_vec.push_back(pair<int, int>(x1,
                                                                        y2));
                    net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

                    for(int j = small_y; j < large_y; j++){
                        vertical_global_edge[pair<int, int>(x1 , j)]->demand++;
                        vertical_global_edge[pair<int, int>(x1 , j)]->nets_pass_through_list.push_back(net_vec[i]);
                    }

                    for(int j = small_x; j < large_x; j++){
                        horizontal_global_edge[pair<int, int>(j, y2)]->demand++;
                        horizontal_global_edge[pair<int, int>(j, y2)]->nets_pass_through_list.push_back(net_vec[i]);
                    }
                }
                
                //update routing resources end
                
            }else if(random_var == 1){

                if(x1 > x2){
                    large_x = x1;
                    small_x = x2;
                }else{
                    large_x = x2;
                    small_x = x1;
                }
                
                if(y1 > y2){
                    large_y = y1;
                    small_y = y2;
                }else{
                    large_y = y2;
                    small_y = y1;
                }

                //update routing resources begin
                bool ov_flow = true;
                bool maze_route_success = false;

                for(int j = small_x; j < large_x; j++){
                    if(horizontal_global_edge[pair<int, int>(j, y1)]->overflow()){
                        ov_flow = true;
                        break;
                    }
                }

                for(int j = small_y; j < large_y; j++){
                    if(vertical_global_edge[pair<int, int>(x2 , j)]->overflow()){
                        ov_flow = true;
                        break;
                    }
                }

                if(ov_flow){
                    maze_route_success = reroute_maze(net_vec[i]->pin_1, net_vec[i]->pin_2, i);
                    if(!maze_route_success){
                        cout << "maze routing fail !!!, net id = " << net_vec[i]->id << endl;
                    }
                }

                if(!maze_route_success){
                    net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
                    net_vec[i]->route_path_vec.push_back(pair<int, int>(x2, 
                                                                        y1));
                    net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

                    for(int j = small_x; j < large_x; j++){
                        horizontal_global_edge[pair<int, int>(j, y1)]->demand++;
                        horizontal_global_edge[pair<int, int>(j, y1)]->nets_pass_through_list.push_back(net_vec[i]);
                    }

                    for(int j = small_y; j < large_y; j++){
                        vertical_global_edge[pair<int, int>(x2 , j)]->demand++;
                        vertical_global_edge[pair<int, int>(x2 , j)]->nets_pass_through_list.push_back(net_vec[i]);
                    }
                }
                
                //update routing resources end
            }
        }

        if(net_vec[i]->route_path_vec.size() == 0){
            cout << "AT rounting_L_shape_or_maze(),  net" << net_vec[i]->id << " size == 0" <<endl;
        }

        cout << "AT routing stage, net = " << i << ", route_path_vec" << endl;
        for(int j = 0; j < net_vec[i]->route_path_vec.size(); j++){
            cout << net_vec[i]->route_path_vec[j].first << ", " << net_vec[i]->route_path_vec[j].second << endl;
        }
    }
    
}

void output_file(){
    ofstream file;
    file.open(output_fileName);

    for(int i=0; i<net_vec.size(); i++){
        file << "net" << net_vec[i]->id << " " << net_vec[i]->id << endl;
        for(int j=0; j<net_vec[i]->route_path_vec.size() - 1; j++){
            file <<"(" << net_vec[i]->route_path_vec[j].first << ", " <<  net_vec[i]->route_path_vec[j].second << ", 1)";
            file << "-";
            file <<"(" << net_vec[i]->route_path_vec[j+1].first << ", " <<  net_vec[i]->route_path_vec[j+1].second << ", 1)" << endl;
        }
        file << "!" << endl;
    }
}

void debug_net_route_path_vec(){
    for(int i=0; i<net_vec.size(); i++){
        if(net_vec[i]->route_path_vec.size() == 0){
            cout << "net" << net_vec[i]->id << " size == 0" <<endl;
        }
    }
}


void construct_overflow_global_edges_vec(){
    // horizontal
    overflow_horizontal_edges_vec.clear();
    overflow_vertical_edges_vec.clear();
    overflow_horizontal_edges_vec.resize((horizontal_grids - 1) * vertical_grids);
    overflow_vertical_edges_vec.resize((vertical_grids - 1) * horizontal_grids);

    int counter_horizontal = 0;
    int counter_vertical = 0;
    
    //horizontal
    for(int i=0; i<horizontal_grids - 1; i++){
        for(int j=0; j<vertical_grids; j++){
            if(horizontal_global_edge[pair<int, int>(i, j)]->overflow()){
                overflow_horizontal_edges_vec[counter_horizontal] = horizontal_global_edge[pair<int, int>(i, j)];
                assert(overflow_horizontal_edges_vec[counter_horizontal]->demand == overflow_horizontal_edges_vec[counter_horizontal]->nets_pass_through_list.size());
                if(i == 74 && j == 39){
                    cout << "i == 74 && j == 39" << endl;
                    cout << "overflow_horizontal_edges_vec[counter_vertical]->demand = " << overflow_horizontal_edges_vec[counter_vertical]->demand << endl;
                    cout << "overflow_horizontal_edges_vec[counter_vertical]->nets_pass_through_list.size() = " <<overflow_horizontal_edges_vec[counter_vertical]->nets_pass_through_list.size() << endl;
                    for(auto &k : overflow_horizontal_edges_vec[counter_vertical]->nets_pass_through_list){
                        cout << k->id << ", ";
                    }
                    cout << endl;
                }
                counter_horizontal++;
            }
        }
    }

    //vertical
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            if(vertical_global_edge[pair<int, int>(i, j)]->overflow()){
                overflow_vertical_edges_vec[counter_vertical] = vertical_global_edge[pair<int, int>(i, j)];
                assert(overflow_vertical_edges_vec[counter_vertical]->demand == overflow_vertical_edges_vec[counter_vertical]->nets_pass_through_list.size());
                if(i == 89 && j == 20){
                    cout << "i == 89 && j == 20" << endl;
                    cout << "overflow_vertical_edges_vec[counter_vertical]->demand = " << overflow_vertical_edges_vec[counter_vertical]->demand << endl;
                    cout << "overflow_vertical_edges_vec[counter_vertical]->nets_pass_through_list.size() = " <<overflow_vertical_edges_vec[counter_vertical]->nets_pass_through_list.size() << endl;
                    for(auto &k : overflow_vertical_edges_vec[counter_vertical]->nets_pass_through_list){
                        cout << k->id << ", ";
                    }
                    cout << endl;
                }
                counter_vertical++;
            }
        }
    }

    overflow_horizontal_edges_vec.resize(counter_horizontal);
    overflow_vertical_edges_vec.resize(counter_vertical);
}

void debug_overflow_global_edges_vec(){
    cout << "========== HORIZONTAL BEGIN ==========" << endl;
    for(int i=0; i<overflow_horizontal_edges_vec.size(); i++){
        cout << "row = " << overflow_horizontal_edges_vec[i]->row << ", col = " << overflow_horizontal_edges_vec[i]->col
            << ", congestion_value = " << overflow_horizontal_edges_vec[i]->get_congestion_value() << ", demand = " << overflow_horizontal_edges_vec[i]->demand
            << ", supply = " << overflow_horizontal_edges_vec[i]->supply << endl;
    }

    cout << "========== VERTICAL BEGIN ==========" << endl;
    for(int i=0; i<overflow_vertical_edges_vec.size(); i++){
        cout << "row = " << overflow_vertical_edges_vec[i]->row << ", col = " << overflow_vertical_edges_vec[i]->col
            << ", congestion_value = " << overflow_vertical_edges_vec[i]->get_congestion_value() << ", demand = " << overflow_vertical_edges_vec[i]->demand
            << ", supply = " << overflow_vertical_edges_vec[i]->supply << endl;
    }
}

void debug_demand_supply(){
    cout << endl << endl << endl << endl << endl;
    cout <<"############## DEMAND & SUPPLY ##############" << endl;
    cout << "========== HORIZONTAL ==========" << endl;
    int total_overflow = 0;
    for(int i=0; i<horizontal_grids - 1; i++){
        for(int j=0; j<vertical_grids; j++){
            // cout << "row = " << horizontal_global_edge[pair<int, int>(i, j)]->row << ", col = " << horizontal_global_edge[pair<int, int>(i, j)]->col
            // << ", congestion_value = " << horizontal_global_edge[pair<int, int>(i, j)]->get_congestion_value() << ", demand = " << horizontal_global_edge[pair<int, int>(i, j)]->demand
            // << ", supply = " << horizontal_global_edge[pair<int, int>(i, j)]->supply << endl;

            if(horizontal_global_edge[pair<int, int>(i, j)]->overflow()){
                total_overflow += horizontal_global_edge[pair<int, int>(i, j)]->demand - horizontal_global_edge[pair<int, int>(i, j)]->supply;
            }
        }
    }

    cout << "========== VERTICAL ==========" << endl;
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            // cout << "row = " << vertical_global_edge[pair<int, int>(i, j)]->row << ", col = " << vertical_global_edge[pair<int, int>(i, j)]->col
            // << ", congestion_value = " << vertical_global_edge[pair<int, int>(i, j)]->get_congestion_value() << ", demand = " << vertical_global_edge[pair<int, int>(i, j)]->demand
            // << ", supply = " << vertical_global_edge[pair<int, int>(i, j)]->supply << endl;

            if(vertical_global_edge[pair<int, int>(i, j)]->overflow()){
                total_overflow += vertical_global_edge[pair<int, int>(i, j)]->demand - vertical_global_edge[pair<int, int>(i, j)]->supply;
            }
        }
    }

    cout << "TOTAL OVERFLOW = " <<  total_overflow << endl;
}

void ripUp_reroute(double congestion_threshold){
    // demand--;
    // delete nets_pass_through_list element;

    set<net*> ripped_nets_set;

    
    globalEdge* temp_global_edge;
    net* ripped_up_net;
    int small_x;
    int large_x;
    int small_y;
    int large_y;

    ///////////////////////////////////////////////
    ///             horizontal                  ///
    ///////////////////////////////////////////////
    int original_vec_size = overflow_horizontal_edges_vec.size();
    for(int i=original_vec_size - 1; i>=0; i--){
        if(overflow_horizontal_edges_vec[i]->get_congestion_value() >= congestion_threshold){
            temp_global_edge = overflow_horizontal_edges_vec[i];
            overflow_horizontal_edges_vec.pop_back();

            cout << "horizontal temp_global_edge " << temp_global_edge->row << ", " << temp_global_edge->col << ", demand = " << temp_global_edge->demand << endl;
            while(temp_global_edge->demand > temp_global_edge->supply * 0.8){ //rip up some nets until this global edge do not have overflow
                if(temp_global_edge->nets_pass_through_list.empty()){
                    break;
                }
                ripped_up_net = temp_global_edge->nets_pass_through_list.back();
                temp_global_edge->nets_pass_through_list.pop_back();
                temp_global_edge->demand--;

                cout << "horizontal, ripped_up_net->id = " << ripped_up_net->id << endl;
                for(int j=0; j<ripped_up_net->route_path_vec.size() - 1; j++){ //rip up a chosen net
                    cout << "ripped_up_net->route_path_vec[j] = " << ripped_up_net->route_path_vec[j].first << ", " << ripped_up_net->route_path_vec[j].second << endl;
                    cout << "ripped_up_net->route_path_vec[j + 1] = " << ripped_up_net->route_path_vec[j + 1].first << ", " << ripped_up_net->route_path_vec[j + 1].second << endl;
                    if(ripped_up_net->route_path_vec[j].second == ripped_up_net->route_path_vec[j + 1].second){ // same y -> horizontal path
                        if(ripped_up_net->route_path_vec[j].first > ripped_up_net->route_path_vec[j + 1].first){
                            large_x = ripped_up_net->route_path_vec[j].first;
                            small_x = ripped_up_net->route_path_vec[j + 1].first;
                        }else{
                            large_x = ripped_up_net->route_path_vec[j + 1].first;
                            small_x = ripped_up_net->route_path_vec[j].first;
                        }

                        for(int k = small_x; k < large_x; k++){ // demand--, and delete that net from globalEdge->nets_pass_through_list
                            horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand--;
                            cout << "horizontal_global_edge[" << k << ", " << ripped_up_net->route_path_vec[j].second << "], demand -- = " << horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand << endl;
                            int assert_demand = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand;
                            assert(assert_demand >= 0);

                            int size = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.size();
                            cout << " before delete list, size = " << size << endl;

                            horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.remove(ripped_up_net);
                            
                            // bool delete_success = false;
                            // for(auto &l: horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list){
                            //     cout << "net_id = " << l->id << endl;
                            //     if(l == ripped_up_net){
                            //         horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.remove(l);
                            //         delete_success = true;
                            //         break;
                            //     }
                            // }

                            // if(!delete_success){
                            //     horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand++;
                            //     assert_demand = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand;
                            // }

                            horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand =
                                horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.size();

                            size = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.size();
                            assert_demand = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand;
                            cout << " after delete list, assert_demand = " << assert_demand << ", size = " << size << endl;
                            assert(assert_demand == size);
                        }
                    }else if(ripped_up_net->route_path_vec[j].first == ripped_up_net->route_path_vec[j + 1].first){ //same x -> vertical path
                        if(ripped_up_net->route_path_vec[j].second > ripped_up_net->route_path_vec[j + 1].second){
                            large_y = ripped_up_net->route_path_vec[j].second;
                            small_y = ripped_up_net->route_path_vec[j + 1].second;
                        }else{
                            large_y = ripped_up_net->route_path_vec[j + 1].second;
                            small_y = ripped_up_net->route_path_vec[j].second;
                        }

                        for(int k = small_y; k < large_y; k++){ // demand--, and delete that net from globalEdge->nets_pass_through_list
                            vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand--;
                            cout << "vertical_global_edge[" << ripped_up_net->route_path_vec[j].first << ", " << k << "], demand -- = " << vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand << endl;
                            int assert_demand = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand;
                            assert(assert_demand >= 0);

                            vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.remove(ripped_up_net);

                            // bool delete_success = false;
                            // for(auto &l : vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list){
                            //     if(l == ripped_up_net){
                            //         vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.remove(l);
                            //         delete_success = true;
                            //         break;
                            //     }
                            // }

                            // if(!delete_success){
                            //     vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand++;
                            //     assert_demand = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand;
                            // }
                            
                            vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand = 
                                vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.size();

                            int size = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.size();
                            assert_demand = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand;
                            cout << "assert_demand = " << assert_demand << ", size = " << size << endl;
                            assert(assert_demand == size);
                        }
                    }
                    
                    if(ripped_up_net->route_path_vec[j] == ripped_up_net->pin_1){
                        break;
                    }

                }


                ripped_up_net->route_path_vec.clear();
                ripped_nets_set.insert(ripped_up_net);

                
                
                // cout << "reroute net = " << ripped_up_net->id << ", pin 1 = (" << ripped_up_net->pin_1.first << ", " << ripped_up_net->pin_1.second << ") "
                //  << "pin 2 = ("  << ripped_up_net->pin_2.first << ", " << ripped_up_net->pin_2.second << ") " << endl;
                
                // if(!reroute_maze(ripped_up_net->pin_1, ripped_up_net->pin_2, ripped_up_net->id)){
                //     reroute_L_shape(ripped_up_net);
                // }
            }
        }else{
            overflow_horizontal_edges_vec.pop_back();
        }
    }


    ///////////////////////////////////////////////
    ///             vertical                    ///
    ///////////////////////////////////////////////
    original_vec_size = overflow_vertical_edges_vec.size();
    for(int i=original_vec_size - 1; i>=0; i--){
        if(overflow_vertical_edges_vec[i]->get_congestion_value() >= congestion_threshold){
            temp_global_edge = overflow_vertical_edges_vec[i];
            overflow_vertical_edges_vec.pop_back();

            while(temp_global_edge->demand > temp_global_edge->supply * 0.8){ //rip up some nets until this global edge do not have overflow
                if(temp_global_edge->nets_pass_through_list.empty()){
                    break;
                }
                ripped_up_net = temp_global_edge->nets_pass_through_list.back();
                temp_global_edge->nets_pass_through_list.pop_back();
                temp_global_edge->demand--;
                cout << "vertical, ripped_up_net->id = " << ripped_up_net->id << endl;

                for(int j=0; j<ripped_up_net->route_path_vec.size() - 1; j++){ //rip up a chosen net
                    cout << "ripped_up_net->route_path_vec[j] = " << ripped_up_net->route_path_vec[j].first << ", " << ripped_up_net->route_path_vec[j].second << endl;
                    cout << "ripped_up_net->route_path_vec[j + 1] = " << ripped_up_net->route_path_vec[j + 1].first << ", " << ripped_up_net->route_path_vec[j + 1].second << endl;
                    
                    if(ripped_up_net->route_path_vec[j].second == ripped_up_net->route_path_vec[j + 1].second){ // same y -> horizontal path
                        if(ripped_up_net->route_path_vec[j].first > ripped_up_net->route_path_vec[j + 1].first){
                            large_x = ripped_up_net->route_path_vec[j].first;
                            small_x = ripped_up_net->route_path_vec[j + 1].first;
                        }else{
                            large_x = ripped_up_net->route_path_vec[j + 1].first;
                            small_x = ripped_up_net->route_path_vec[j].first;
                        }

                        for(int k = small_x; k < large_x; k++){ // demand--, and delete that net from globalEdge->nets_pass_through_list
                            horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand--;
                            cout << "horizontal_global_edge[" << k << ", " << ripped_up_net->route_path_vec[j].second << "], demand -- = " << horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand << endl;
                            int assert_demand = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand;
                            assert(assert_demand >= 0);

                            horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.remove(ripped_up_net);

                            // bool delete_success = false;
                            // for(auto &l : horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list){
                            //     if(l == ripped_up_net){
                            //         horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.remove(l);
                            //         delete_success = true;
                            //         break;
                            //     }
                            // }

                            // if(!delete_success){
                            //     horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand++;
                            //     assert_demand = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand;
                            // }

                            horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand = 
                                horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.size();

                            int size = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.size();
                            assert_demand = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->demand;
                            cout << "assert_demand = " << assert_demand << ", size = " << size << endl;
                            assert(assert_demand == size);
                        }
                    }else if(ripped_up_net->route_path_vec[j].first == ripped_up_net->route_path_vec[j + 1].first){ //same x -> vertical path
                        if(ripped_up_net->route_path_vec[j].second > ripped_up_net->route_path_vec[j + 1].second){
                            large_y = ripped_up_net->route_path_vec[j].second;
                            small_y = ripped_up_net->route_path_vec[j + 1].second;
                        }else{
                            large_y = ripped_up_net->route_path_vec[j + 1].second;
                            small_y = ripped_up_net->route_path_vec[j].second;
                        }

                        for(int k = small_y; k < large_y; k++){ // demand--, and delete that net from globalEdge->nets_pass_through_list
                            vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand--;
                            cout << "vertical_global_edge[" << ripped_up_net->route_path_vec[j].first << ", " << k << "], demand -- = " << vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand << endl;
                            int assert_demand = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand;
                            assert(assert_demand >= 0);

                            int size = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.size();
                            cout << " before delete list, size = " << size << endl;

                            vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.remove(ripped_up_net);

                            // bool delete_success = false;
                            // for(auto &l : vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list){
                            //     cout << "net_id = " << l->id << endl;
                            //     if(l == ripped_up_net){
                            //         cout << "before erase" << endl;
                            //         vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.remove(l);
                            //         cout << "end erase" << endl;
                            //         delete_success = true;
                            //         break;
                            //     }
                            // }

                            // if(!delete_success){
                            //     vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand++;
                            //     assert_demand = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand;
                            // }
                            
                            vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand = 
                                vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.size();

                            size = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.size();
                            assert_demand = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->demand;
                            cout << " after delete list, assert_demand = " << assert_demand << ", size = " << size << endl;
                            assert(assert_demand == size);
                        }
                    }

                    if(ripped_up_net->route_path_vec[j] == ripped_up_net->pin_1){
                        break;
                    }
                }
                
                ripped_up_net->route_path_vec.clear();
                ripped_nets_set.insert(ripped_up_net);
                
                // cout << "reroute net = " << ripped_up_net->id << ", pin 1 = (" << ripped_up_net->pin_1.first << ", " << ripped_up_net->pin_1.second << ") "
                //  << "pin 2 = ("  << ripped_up_net->pin_2.first << ", " << ripped_up_net->pin_2.second << ") " << endl;
                
                // if(!reroute_maze(ripped_up_net->pin_1, ripped_up_net->pin_2, ripped_up_net->id)){
                //     reroute_L_shape(ripped_up_net);
                // }
            }
        }else{
            overflow_vertical_edges_vec.pop_back();
        }
    }

    ///////////////////////////////////////////////
    ///            reroute                      ///
    ///////////////////////////////////////////////
    cout << "ripped_nets_set.size() = " << ripped_nets_set.size() << endl;
    for(set<net*>::iterator itr = ripped_nets_set.begin(); itr != ripped_nets_set.end(); itr++){
        if(!reroute_maze((*itr)->pin_1, (*itr)->pin_2, (*itr)->id)){
            cout << "REROUTE, maze route fail, net->id = " << (*itr)->id << endl;
            reroute_L_shape((*itr));
        }
    }
}

void reroute_far_away(net* reroute_net){
    int x1 = reroute_net->pin_1.first;
    int y1 = reroute_net->pin_1.second;
    int x2 = reroute_net->pin_2.first;
    int y2 = reroute_net->pin_2.second;

    int small_x;
    int large_x;
    int small_y;
    int large_y;

    vector <int> vertical_demands(horizontal_grids);
    for(int i=0; i<horizontal_grids; i++){
        vertical_demands[i] = 0;
    }

    for(int i = 0; i<horizontal_grids; i++){
        for(int j = 0; j<vertical_grids - 1; j++){
            vertical_demands[i] += vertical_global_edge[pair<int, int>(i, j)]->demand;
        }
    }

    int min = INF;
    int min_demand_x = INF;
    for(int i=0; i<horizontal_grids; i++){
        if(min > vertical_demands[i]){
            min = vertical_demands[i];
            min_demand_x = i;
        }
    }


    

    reroute_net->route_path_vec.push_back(reroute_net->pin_1);
    reroute_net->route_path_vec.push_back(pair<int, int>(min_demand_x, y1));
    if(x1 > min_demand_x){
        large_x = x1;
        small_x = min_demand_x;
    }else{
        large_x = min_demand_x;
        small_x = x1;
    }

    for(int i = small_x; i < large_x; i++){
        horizontal_global_edge[pair<int, int>(i , y1)]->demand++;
        horizontal_global_edge[pair<int, int>(i , y1)]->nets_pass_through_list.push_back(reroute_net);
    }

    reroute_net->route_path_vec.push_back(pair<int, int>(min_demand_x, y2));
    if(y1 > y2){
        large_y = y1;
        small_y = y2;
    }else{
        large_y = y2;
        small_y = y1;
    }
    for(int i = small_y; i < large_y; i++){
        vertical_global_edge[pair<int, int>(min_demand_x , i)]->demand++;
        vertical_global_edge[pair<int, int>(min_demand_x , i)]->nets_pass_through_list.push_back(reroute_net);
    }

    reroute_net->route_path_vec.push_back(pair<int, int>(reroute_net->pin_2));
    if(x2 > min_demand_x){
        large_x = x2;
        small_x = min_demand_x;
    }else{
        large_x = min_demand_x;
        small_x = x2;
    }

    for(int i = small_x; i < large_x; i++){
        horizontal_global_edge[pair<int, int>(i , y2)]->demand++;
        horizontal_global_edge[pair<int, int>(i , y2)]->nets_pass_through_list.push_back(reroute_net);
    }

    cout << "AT reroute far away" << endl;
    for(int i=0; i<reroute_net->route_path_vec.size(); i++){
        cout << reroute_net->route_path_vec[i].first << ", " << reroute_net->route_path_vec[i].second << endl;
    }

}

void reroute_L_shape(net* reroute_net){
    srand(1);
    int random_var  = rand() % 2;

    int x1 = reroute_net->pin_1.first;
    int y1 = reroute_net->pin_1.second;
    int x2 = reroute_net->pin_2.first;
    int y2 = reroute_net->pin_2.second;

    int small_x;
    int large_x;
    int small_y;
    int large_y;

    if(x1 == x2){ //route with a straight vertical line
        reroute_net->route_path_vec.push_back(reroute_net->pin_1);
        reroute_net->route_path_vec.push_back(reroute_net->pin_2);

        if(y1 > y2){
            large_y = y1;
            small_y = y2;
        }else{
            large_y = y2;
            small_y = y1;
        }

        //update routing resources begin
        for(int j = small_y; j < large_y; j++){
            vertical_global_edge[pair<int, int>(x1 , j)]->demand++;
            vertical_global_edge[pair<int, int>(x1 , j)]->nets_pass_through_list.push_back(reroute_net);
        }
        //update routing resources end

    }else if(y1 == y2){ //route with a straight horizontal line
        reroute_net->route_path_vec.push_back(reroute_net->pin_1);
        reroute_net->route_path_vec.push_back(reroute_net->pin_2);

        if(x1 > x2){
            large_x = x1;
            small_x = x2;
        }else{
            large_x = x2;
            small_x = x1;
        }
        //update routing resources begin
        for(int j = small_x; j < large_x; j++){
            horizontal_global_edge[pair<int, int>(j , y1)]->demand++;
            horizontal_global_edge[pair<int, int>(j , y1)]->nets_pass_through_list.push_back(reroute_net);
        }
        //update routing resources end

    }else{ //route with a L-shaped line
        
        //random_var == 0 -> upper L;
        //random_var == 1 -> lower L;
        if(random_var == 0){
            reroute_net->route_path_vec.push_back(reroute_net->pin_1);
            reroute_net->route_path_vec.push_back(pair<int, int>(x1,
                                                                y2));
            reroute_net->route_path_vec.push_back(reroute_net->pin_2);

            if(x1 > x2){
                large_x = x1;
                small_x = x2;
            }else{
                large_x = x2;
                small_x = x1;
            }
            
            if(y1 > y2){
                large_y = y1;
                small_y = y2;
            }else{
                large_y = y2;
                small_y = y1;
            }

            //update routing resources begin
            for(int j = small_y; j < large_y; j++){
                vertical_global_edge[pair<int, int>(x1 , j)]->demand++;
                vertical_global_edge[pair<int, int>(x1 , j)]->nets_pass_through_list.push_back(reroute_net);
            }

            for(int j = small_x; j < large_x; j++){
                horizontal_global_edge[pair<int, int>(j, y2)]->demand++;
                horizontal_global_edge[pair<int, int>(j, y2)]->nets_pass_through_list.push_back(reroute_net);
            }
            //update routing resources end
            
        }else if(random_var == 1){
            reroute_net->route_path_vec.push_back(reroute_net->pin_1);
            reroute_net->route_path_vec.push_back(pair<int, int>(x2, 
                                                                y1));
            reroute_net->route_path_vec.push_back(reroute_net->pin_2);

            if(x1 > x2){
                large_x = x1;
                small_x = x2;
            }else{
                large_x = x2;
                small_x = x1;
            }
            
            if(y1 > y2){
                large_y = y1;
                small_y = y2;
            }else{
                large_y = y2;
                small_y = y1;
            }

            //update routing resources begin
            for(int j = small_x; j < large_x; j++){
                horizontal_global_edge[pair<int, int>(j, y1)]->demand++;
                horizontal_global_edge[pair<int, int>(j, y1)]->nets_pass_through_list.push_back(reroute_net);
            }

            for(int j = small_y; j < large_y; j++){
                vertical_global_edge[pair<int, int>(x2 , j)]->demand++;
                vertical_global_edge[pair<int, int>(x2 , j)]->nets_pass_through_list.push_back(reroute_net);
            }
            //update routing resources end
        }
    }
}

void init_maze_block_map(){
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids; j++){
            maze_block_map[pair<int, int>(i, j)] = INF;
        }
    }
}

void debug_init_maze_block_map(){
    cout << "@@@@@@ debug maze_block_map @@@@@@" << endl;
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids; j++){
            cout << "i = " << i << ", j = " << j << maze_block_map[pair<int, int>(i, j)] << endl;
        }
    }
}

void update_routing_resources(const int &id){
    int small_x;
    int large_x;
    int small_y; 
    int large_y;
    for(int i=0; i<net_vec[id]->route_path_vec.size() - 1; i++){
        if(net_vec[id]->route_path_vec[i].first == net_vec[id]->route_path_vec[i + 1].first){ // same x, vertical line
            if(net_vec[id]->route_path_vec[i].second > net_vec[id]->route_path_vec[i + 1].second){
                large_y = net_vec[id]->route_path_vec[i].second;
                small_y = net_vec[id]->route_path_vec[i + 1].second;
            }else{
                large_y = net_vec[id]->route_path_vec[i + 1].second;
                small_y = net_vec[id]->route_path_vec[i].second;
            }

            for(int j=small_y; j<large_y; j++){
                vertical_global_edge[pair<int, int>(net_vec[id]->route_path_vec[i].first, j)]->demand++;
                vertical_global_edge[pair<int, int>(net_vec[id]->route_path_vec[i].first, j)]->nets_pass_through_list.push_front(net_vec[id]);
            }
        }else if(net_vec[id]->route_path_vec[i].second == net_vec[id]->route_path_vec[i + 1].second){ //same y, horizontal line
            if(net_vec[id]->route_path_vec[i].first > net_vec[id]->route_path_vec[i + 1].first){
                large_x = net_vec[id]->route_path_vec[i].first;
                small_x = net_vec[id]->route_path_vec[i + 1].first;
            }else{
                large_x = net_vec[id]->route_path_vec[i + 1].first;
                small_x = net_vec[id]->route_path_vec[i].first;
            }

            for(int j=small_x; j<large_x; j++){
                horizontal_global_edge[pair<int, int>(j, net_vec[id]->route_path_vec[i].second)]->demand++;
                horizontal_global_edge[pair<int, int>(j, net_vec[id]->route_path_vec[i].second)]->nets_pass_through_list.push_front(net_vec[id]);
            }
        }
    }
}

vector<pair<int, int>> path_vec;

bool back_trace(const pair<int, int> &back_trace_start, const pair<int, int> &back_trace_end, bool first, const int &id){
    
    vector<pair<int, int>> path_vec_simplified;
    
    if(first){
        path_vec.clear();
    }

    pair<int, int> min_position = back_trace_start;
    int min =  maze_block_map[pair<int, int>(back_trace_start.first, back_trace_start.second)];

    pair<int, int> temp = min_position;

    cout << "back trace...(" << min_position.first << ", " << min_position.second << ")" << endl;

    path_vec.push_back(min_position);
    if(path_vec.size() >= 2){
        if(path_vec[path_vec.size() - 1] == path_vec[path_vec.size() - 2]){
            return false;
        }
    }

    if(min_position == back_trace_end){
        path_vec_simplified.clear();
        if(path_vec.size() == 2){
            net_vec[id]->route_path_vec.clear();
            for(int i = 0; i<path_vec.size(); i++){
                net_vec[id]->route_path_vec.push_back(path_vec[i]);
            }
        }else{
            path_vec.shrink_to_fit();
            path_vec_simplified.push_back(path_vec[0]);
            for(int i=1; i<path_vec.size() - 1; i++){
                if(path_vec[i - 1].first == path_vec[i].first){
                    if(path_vec[i].first != path_vec[i + 1].first){
                        path_vec_simplified.push_back(path_vec[i]);
                    }
                }else if(path_vec[i - 1].second == path_vec[i].second){
                    if(path_vec[i].second != path_vec[i + 1].second){
                        path_vec_simplified.push_back(path_vec[i]);
                    }
                }

                if(path_vec[i] == back_trace_end){
                    break;
                }
            }
            path_vec_simplified.push_back(path_vec[path_vec.size() - 1]);

            net_vec[id]->route_path_vec.clear();
            for(int i=0; i<path_vec_simplified.size(); i++){
                net_vec[id]->route_path_vec.push_back(path_vec_simplified[i]);
                if(path_vec_simplified[i] == back_trace_end){
                    break;
                }
            }
        }
        update_routing_resources(id);
        return true;
    }

    // right
    if(temp.first + 1 < horizontal_grids){
        if(min > maze_block_map[pair<int, int>(temp.first + 1, temp.second)]){
            min = maze_block_map[pair<int, int>(temp.first + 1, temp.second)];
            min_position = pair<int, int>(temp.first + 1, temp.second);
        }
    }

    // left
    if(temp.first - 1 >= 0){
        if(min > maze_block_map[pair<int, int>(temp.first - 1, temp.second)]){
            min = maze_block_map[pair<int, int>(temp.first - 1, temp.second)];
            min_position = pair<int, int>(temp.first - 1, temp.second);
        }
    }
    
    // up
    if(temp.second + 1 < vertical_grids){
        if(min > maze_block_map[pair<int, int>(temp.first, temp.second + 1)]){
            min = maze_block_map[pair<int, int>(temp.first, temp.second + 1)];
            min_position = pair<int, int>(temp.first, temp.second + 1);
        }
    }

    // down
    if(temp.second - 1 >= 0){
        if(min > maze_block_map[pair<int, int>(temp.first, temp.second - 1)]){
            min = maze_block_map[pair<int, int>(temp.first, temp.second - 1)];
            min_position = pair<int, int>(temp.first, temp.second - 1);
        }
    }

    back_trace(min_position, back_trace_end, false, id);
}

bool reroute_maze(const pair<int, int> &start, const pair<int, int> &end, const int &id){
    // cout << "net i = " << id << endl;
    // cout << "start.first = " << start.first << endl;
    // cout << "start.second = " << start.second << endl;
    
    init_maze_block_map();

    // if(id == 0){
    //     debug_init_maze_block_map();
    // }

    maze_block_map[pair<int, int>(start.first, start.second)] = 0;
    int step = 0;
    maze_block_map[start] = step;

    double weight = 60.0;
    
    list<pair<int, int>> queue;
    queue.push_back(start);

    pair<int, int> temp;

    while(!queue.empty()){
        temp = queue.front();
        // cout << "AT route_maze(), (" << temp.first << ", " << temp.second << ")" << endl;
        queue.pop_front();

        if(temp == end){
            if(back_trace(end, start, true, id)){
                path_vec.clear();
                path_vec.resize(0);
                return true;
            }else{
                return false;
            }
        }

        step++;

        // right
        if(temp.first + 1 < horizontal_grids){
            // if(id == 0){
            //     cout << "horizontal_global_edge[pair<int, int>(temp.first, temp.second)]->overflow() = " << horizontal_global_edge[pair<int, int>(temp.first, temp.second)]->overflow() << endl;
            //     cout << "maze_block_map[pair<int, int>(temp.first + 1, temp.second)]  = " << maze_block_map[pair<int, int>(temp.first + 1, temp.second)] << endl;
            // }
            

            if(!horizontal_global_edge[pair<int, int>(temp.first, temp.second)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first + 1, temp.second)] == INF){
                    maze_block_map[pair<int, int>(temp.first + 1, temp.second)] = step + 
                    weight * exp(-(horizontal_global_edge[pair<int, int>(temp.first, temp.second)]->supply - horizontal_global_edge[pair<int, int>(temp.first, temp.second)]->demand));
                    
                    queue.push_back(pair<int, int>(temp.first + 1, temp.second));
                }
            }
        }

        // left
        if(temp.first - 1 >= 0){
            if(!horizontal_global_edge[pair<int, int>(temp.first - 1, temp.second)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first - 1, temp.second)] == INF){
                    maze_block_map[pair<int, int>(temp.first - 1, temp.second)] = step + 
                    weight * exp(-(horizontal_global_edge[pair<int, int>(temp.first - 1, temp.second)]->supply - horizontal_global_edge[pair<int, int>(temp.first - 1, temp.second)]->demand));
                    
                    queue.push_back(pair<int, int>(temp.first - 1, temp.second));
                }
            }
        }
        
        // up
        if(temp.second + 1 < vertical_grids){
            if(!vertical_global_edge[pair<int, int>(temp.first, temp.second)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first, temp.second + 1)] == INF){
                    maze_block_map[pair<int, int>(temp.first, temp.second + 1)] = step + 
                    weight * exp(-(vertical_global_edge[pair<int, int>(temp.first, temp.second)]->supply - vertical_global_edge[pair<int, int>(temp.first, temp.second)]->demand));
                    queue.push_back(pair<int, int>(temp.first, temp.second + 1));
                }
            }
        }

        // down
        if(temp.second - 1 >= 0){
            if(!vertical_global_edge[pair<int, int>(temp.first, temp.second - 1)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first, temp.second - 1)] == INF){
                    maze_block_map[pair<int, int>(temp.first, temp.second - 1)] = step + 
                    weight * exp(-(vertical_global_edge[pair<int, int>(temp.first, temp.second - 1)]->supply - vertical_global_edge[pair<int, int>(temp.first, temp.second - 1)]->demand));
                    queue.push_back(pair<int, int>(temp.first, temp.second - 1));
                }
            }
        }
    }
    
    return false;
}

void zero_all_global_edge_demand(){
    cout << "@@@@@@@@@@@@@@ AT zero_all_global_edge_demand() @@@@@@@@@@@@@@" << endl;
    //horizontal 
    for(int i=0; i<horizontal_grids - 1; i++){
        for(int j=0; j<vertical_grids; j++){
            horizontal_global_edge[pair<int, int>(i, j)]->demand = 0;
            horizontal_global_edge[pair<int, int>(i, j)]->nets_pass_through_list.clear();
        }
    }

    //vertical
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            vertical_global_edge[pair<int, int>(i, j)]->demand = 0;
            vertical_global_edge[pair<int, int>(i, j)]->nets_pass_through_list.clear();
        }
    }
}

void reconstruct_demand(){
    int small_x;
    int large_x;
    int small_y;
    int large_y;
    for(int i=0; i<net_vec.size(); i++){
        for(int j=0; j<net_vec[i]->route_path_vec.size() - 1; j++){
            if(net_vec[i]->route_path_vec[j].first == net_vec[i]->route_path_vec[j+1].first){ // same x, vertical line
                if(net_vec[i]->route_path_vec[j].second > net_vec[i]->route_path_vec[j+1].second){
                    large_y = net_vec[i]->route_path_vec[j].second;
                    small_y = net_vec[i]->route_path_vec[j+1].second;
                }else{
                    large_y = net_vec[i]->route_path_vec[j+1].second;
                    small_y = net_vec[i]->route_path_vec[j].second;
                }

                for(int k=small_y; k<large_y; k++){
                    vertical_global_edge[pair<int, int>(net_vec[i]->route_path_vec[j].first, k)]->demand++;
                    vertical_global_edge[pair<int, int>(net_vec[i]->route_path_vec[j].first, k)]->nets_pass_through_list.push_front(net_vec[i]);
                }
            }else if(net_vec[i]->route_path_vec[j].second == net_vec[i]->route_path_vec[j+1].second){  // same y, horizontal line
                if(net_vec[i]->route_path_vec[j].first > net_vec[i]->route_path_vec[j+1].first){
                    large_x = net_vec[i]->route_path_vec[j].first;
                    small_x = net_vec[i]->route_path_vec[j+1].first;
                }else{
                    large_x = net_vec[i]->route_path_vec[j+1].first;
                    small_x = net_vec[i]->route_path_vec[j].first;
                }

                for(int k=small_x; k<large_x; k++){
                    horizontal_global_edge[pair<int, int>(k, net_vec[i]->route_path_vec[j].second)]->demand++;
                    horizontal_global_edge[pair<int, int>(k, net_vec[i]->route_path_vec[j].second)]->nets_pass_through_list.push_front(net_vec[i]);
                }
            }
        }
    }
}

void congestion_value_color(double value, ofstream &file){
    if(value == 0){
        file << "stroke(255); " << endl;
    }else if(value > 0 && value <= 0.25){
        file << "stroke(0, 0, 255); " << endl;
    }else if(value > 0.25 && value <= 0.5){
        file << "stroke(0, 255, 255); " << endl;
    }else if(value > 0.5 && value <= 0.75){
        file << "stroke(0, 255, 0); " << endl;
    }else if(value > 0.75 && value <= 1.0){
        file << "stroke(255, 255, 0); " << endl;
    }else{
        file << "stroke(255, 0, 0); " << endl;
    }
}

void draw(){
    ofstream file;
    file.open("./draw04.txt");

    file << "void setup(){" << endl;
    file << "size(960, 640);" << endl;
    file << "background(0);" << endl;
    file << "}" << endl;

    file << "void draw(){" << endl;
    file << "draw_half1_H();" << endl;
    file << "draw_half2_H();" << endl;
    file << "draw_half1_V();" << endl;
    file << "draw_half2_V();" << endl;
    file << "draw_half3_V();" << endl;
    file << "}" << endl;

    file << "void draw_half1_H(){" << endl;
    for(int i=0; i<(horizontal_grids - 1) / 2; i++){
        for(int j=0; j<vertical_grids; j++){
            congestion_value_color(horizontal_global_edge[pair<int, int>(i, j)]->get_congestion_value(), file);
            file << "line(" << i*10 + 1 << ", " << j * 10 << ", " << i*10 + 8 << ", " << j * 10 << ");" << endl;
        }
    }
    file << "}" << endl;

    file << "void draw_half2_H(){" << endl;
    for(int i=(horizontal_grids - 1) / 2; i<(horizontal_grids - 1); i++){
        for(int j=0; j<vertical_grids; j++){
            congestion_value_color(horizontal_global_edge[pair<int, int>(i, j)]->get_congestion_value(), file);
            file << "line(" << i*10 + 1 << ", " << j * 10 << ", " << i*10 + 8 << ", " << j * 10 << ");" << endl;
        }
    }
    file << "}" << endl;

    file << "void draw_half1_V(){" << endl;
    for(int i=0; i<(horizontal_grids) / 3; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            congestion_value_color(vertical_global_edge[pair<int, int>(i, j)]->get_congestion_value(), file);
            file << "line(" << i*10 << ", " << j * 10 + 1 << ", " << i*10 << ", " << j * 10 + 8 << ");" << endl;
        }
    }
    file << "}" << endl;

    file << "void draw_half2_V(){" << endl;
    for(int i=(horizontal_grids) / 3; i<(horizontal_grids)*2 / 3; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            congestion_value_color(vertical_global_edge[pair<int, int>(i, j)]->get_congestion_value(), file);
            file << "line(" << i*10 << ", " << j * 10 + 1 << ", " << i*10 << ", " << j * 10 + 8 << ");" << endl;
        }
    }
    file << "}" << endl;

    file << "void draw_half3_V(){" << endl;
    for(int i=(horizontal_grids)*2 / 3; i<(horizontal_grids); i++){
        for(int j=0; j<vertical_grids - 1; j++){
            congestion_value_color(vertical_global_edge[pair<int, int>(i, j)]->get_congestion_value(), file);
            file << "line(" << i*10 << ", " << j * 10 + 1 << ", " << i*10 << ", " << j * 10 + 8 << ");" << endl;
        }
    }
    file << "}" << endl;


}


int main(int argc, char *argv[]){
    input_fileName = argv[1];
    output_fileName = argv[2];

    read_input();
    //debug_read_input();

    construct_global_edge_map();
    //debug_global_edge_map();

    rounting_L_shape_or_maze();

    construct_overflow_global_edges_vec();
    debug_overflow_global_edges_vec();
    debug_demand_supply();

    double congestion_th = 1.7;
    for(int i=0; i<20; i++){
        ripUp_reroute(congestion_th);
        // zero all global edges demand, and reconstruct them
        zero_all_global_edge_demand();
        reconstruct_demand();
        debug_global_edge_map();

        debug_demand_supply();
        construct_overflow_global_edges_vec();

        congestion_th = congestion_th * 0.9;
        if(congestion_th <= 0.85){
            congestion_th = 0.85;
        }
    }
   
    

    //debug_net_route_path_vec();
    output_file();

    draw();
}