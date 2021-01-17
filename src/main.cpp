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
map<pair<int, int>, int> maze_block_map;

vector<globalEdge*> overflow_horizontal_edges_vec;
vector<globalEdge*> overflow_vertical_edges_vec;

bool reroute_maze(const pair<int, int> &start, const pair<int, int> &end, const int &id);
void init_maze_block_map();

void reroute_L_shape(net*);

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
    }
}

void debug_read_input(){
    for(int i=0; i<net_vec.size(); i++){
        cout << "net" << net_vec[i]->id << endl;
        cout << net_vec[i]->pin_1.first << " " << net_vec[i]->pin_1.second << endl;
        cout << net_vec[i]->pin_2.first << " " << net_vec[i]->pin_2.second << endl;
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
    cout << "horizontal " << endl;
    for(int i=0; i<vertical_grids; i++){
        for(int j=0; j<horizontal_grids - 1; j++){
            cout << "row = " << horizontal_global_edge[pair<int, int>(i, j)]->row << ", col = " << horizontal_global_edge[pair<int, int>(i, j)]->col << 
                ", supply = " << horizontal_global_edge[pair<int, int>(i, j)]->supply << ", demand = " << horizontal_global_edge[pair<int, int>(i, j)]->demand << endl;
        }
    }

    cout << "vertical " << endl;
    for(int i=0; i<vertical_grids - 1; i++){
        for(int j=0; j<horizontal_grids; j++){
            cout << "row = " << vertical_global_edge[pair<int, int>(i, j)]->row << ", col = " << vertical_global_edge[pair<int, int>(i, j)]->col << 
                ", supply = " << vertical_global_edge[pair<int, int>(i, j)]->supply << ", demand = " << vertical_global_edge[pair<int, int>(i, j)]->demand << endl;
        }
    }
}

void rounting_L_shape(){
    srand(time(NULL));
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
            net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
            net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

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
                vertical_global_edge[pair<int, int>(x1 , j)]->nets_pass_through_list.push_back(net_vec[i]);
            }
            //update routing resources end

        }else if(y1 == y2){ //route with a straight horizontal line
            net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
            net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

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
                horizontal_global_edge[pair<int, int>(j , y1)]->nets_pass_through_list.push_back(net_vec[i]);
            }
            //update routing resources end

        }else{ //route with a L-shaped line
            
            //random_var == 0 -> upper L;
            //random_var == 1 -> lower L;
            if(random_var == 0){
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
                net_vec[i]->route_path_vec.push_back(pair<int, int>(x1,
                                                                    y2));
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

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
                    vertical_global_edge[pair<int, int>(x1 , j)]->nets_pass_through_list.push_back(net_vec[i]);
                }

                for(int j = small_x; j < large_x; j++){
                    horizontal_global_edge[pair<int, int>(j, y2)]->demand++;
                    horizontal_global_edge[pair<int, int>(j, y2)]->nets_pass_through_list.push_back(net_vec[i]);
                }
                //update routing resources end
                
            }else if(random_var == 1){
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_1);
                net_vec[i]->route_path_vec.push_back(pair<int, int>(x2, 
                                                                    y1));
                net_vec[i]->route_path_vec.push_back(net_vec[i]->pin_2);

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
                    horizontal_global_edge[pair<int, int>(j, y1)]->nets_pass_through_list.push_back(net_vec[i]);
                }

                for(int j = small_y; j < large_y; j++){
                    vertical_global_edge[pair<int, int>(x2 , j)]->demand++;
                    vertical_global_edge[pair<int, int>(x2 , j)]->nets_pass_through_list.push_back(net_vec[i]);
                }
                //update routing resources end
            }
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


void construct_overflow_global_edges_vec(){
    // horizontal
    overflow_horizontal_edges_vec.resize((horizontal_grids - 1) * vertical_grids);
    overflow_vertical_edges_vec.resize((vertical_grids - 1) * horizontal_grids);

    int counter_horizontal = 0;
    int counter_vertical = 0;
    
    //horizontal
    for(int i=0; i<horizontal_grids - 1; i++){
        for(int j=0; j<vertical_grids; j++){
            if(horizontal_global_edge[pair<int, int>(i, j)]->overflow()){
                overflow_horizontal_edges_vec[counter_horizontal] = horizontal_global_edge[pair<int, int>(i, j)];
                counter_horizontal++;
            }
        }
    }

    //vertical
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            if(vertical_global_edge[pair<int, int>(i, j)]->overflow()){
                overflow_vertical_edges_vec[counter_vertical] = vertical_global_edge[pair<int, int>(i, j)];
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
            cout << "row = " << horizontal_global_edge[pair<int, int>(i, j)]->row << ", col = " << horizontal_global_edge[pair<int, int>(i, j)]->col
            << ", congestion_value = " << horizontal_global_edge[pair<int, int>(i, j)]->get_congestion_value() << ", demand = " << horizontal_global_edge[pair<int, int>(i, j)]->demand
            << ", supply = " << horizontal_global_edge[pair<int, int>(i, j)]->supply << endl;

            if(horizontal_global_edge[pair<int, int>(i, j)]->overflow()){
                total_overflow += horizontal_global_edge[pair<int, int>(i, j)]->demand - horizontal_global_edge[pair<int, int>(i, j)]->supply;
            }
        }
    }

    cout << "========== VERTICAL ==========" << endl;
    for(int i=0; i<horizontal_grids; i++){
        for(int j=0; j<vertical_grids - 1; j++){
            cout << "row = " << vertical_global_edge[pair<int, int>(i, j)]->row << ", col = " << vertical_global_edge[pair<int, int>(i, j)]->col
            << ", congestion_value = " << vertical_global_edge[pair<int, int>(i, j)]->get_congestion_value() << ", demand = " << vertical_global_edge[pair<int, int>(i, j)]->demand
            << ", supply = " << vertical_global_edge[pair<int, int>(i, j)]->supply << endl;

            if(vertical_global_edge[pair<int, int>(i, j)]->overflow()){
                total_overflow += vertical_global_edge[pair<int, int>(i, j)]->demand - vertical_global_edge[pair<int, int>(i, j)]->supply;
            }
        }
    }

    cout << "TOTAL OVERFLOW = " <<  total_overflow << endl;
}

void ripUp_reroute(){
    // demand--;
    // delete nets_pass_through_list element;

    //horizontal
    globalEdge* temp_global_edge;
    net* ripped_up_net;
    int original_vec_size = overflow_horizontal_edges_vec.size();
    int small_x;
    int large_x;
    int small_y;
    int large_y;

    int while_loop_cnt = 0;

    for(int i=original_vec_size - 1; i>=0; i--){
        if(overflow_horizontal_edges_vec[i]->overflow()){
            temp_global_edge = overflow_horizontal_edges_vec[i];
            overflow_horizontal_edges_vec.pop_back();

            while_loop_cnt = 0;
            while(temp_global_edge->overflow()){ //rip up some nets until this global edge do not have overflow
                ripped_up_net = temp_global_edge->nets_pass_through_list.back();
                temp_global_edge->nets_pass_through_list.pop_back();

                for(int j=0; j<ripped_up_net->route_path_vec.size() - 1; j++){ //rip up a chosen net
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
                            
                            for(list<net*>::iterator l = horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.end(); 
                                l != horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.begin() ; 
                                l--)
                            {
                                if(*l == ripped_up_net){
                                    horizontal_global_edge[pair<int, int>(k, ripped_up_net->route_path_vec[j].second)]->nets_pass_through_list.erase(l);
                                    break;
                                }
                            }   
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

                            for(list<net*>::iterator l = vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.end();
                                l != vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.begin();
                                l--)
                            {
                                if(*l == ripped_up_net){
                                    vertical_global_edge[pair<int, int>(ripped_up_net->route_path_vec[j].first, k)]->nets_pass_through_list.erase(l);
                                    break;
                                }
                            }
                        }
                    }
                }

                while_loop_cnt++;
                
                cout << "reroute net = " << ripped_up_net->id << ", pin 1 = (" << ripped_up_net->pin_1.first << ", " << ripped_up_net->pin_1.second << ") "
                 << "pin 2 = ("  << ripped_up_net->pin_2.first << ", " << ripped_up_net->pin_2.second << ") " << endl;
                
                if(!reroute_maze(ripped_up_net->pin_1, ripped_up_net->pin_2, ripped_up_net->id)){
                    reroute_L_shape(ripped_up_net);
                }

                if(while_loop_cnt > temp_global_edge->supply){
                    break;
                }
            }
        }else{
            overflow_horizontal_edges_vec.pop_back();
        }
    }
    //vertical


    
}

void reroute_L_shape(net* reroute_net){
    srand(time(NULL));
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
    for(int i=0; i<vertical_grids; i++){
        for(int j=0; j<horizontal_grids; j++){
            maze_block_map[pair<int, int>(i, j)] = INF;
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

bool back_trace(const pair<int, int> &back_trace_start, const pair<int, int> &back_trace_end, bool first, const int &id){
    static vector<pair<int, int>> path_vec;
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
        if(path_vec.size() == 2){
            net_vec[id]->route_path_vec = path_vec;
        }else{
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
            }
            path_vec_simplified.push_back(path_vec[path_vec.size() - 1]);
            net_vec[id]->route_path_vec = path_vec_simplified;
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
    cout << "start.first = " << start.first << endl;
    cout << "start.second = " << start.second << endl;
    
    init_maze_block_map();
    int step = 0;
    maze_block_map[start] = step;
    
    list<pair<int, int>> queue;
    queue.push_back(start);

    pair<int, int> temp;

    while(!queue.empty()){
        temp = queue.front();
        //cout << "(" << temp.first << ", " << temp.second << ")" << endl;
        queue.pop_front();

        if(temp == end){
            if(back_trace(end, start, true, id)){
                return true;
            }else{
                return false;
            }
        }

        step++;

        // right
        if(temp.first + 1 < horizontal_grids){
            if(!horizontal_global_edge[pair<int, int>(temp.first, temp.second)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first + 1, temp.second)] == INF){
                    maze_block_map[pair<int, int>(temp.first + 1, temp.second)] = step;
                    queue.push_back(pair<int, int>(temp.first + 1, temp.second));
                }
            }
        }

        // left
        if(temp.first - 1 >= 0){
            if(!horizontal_global_edge[pair<int, int>(temp.first - 1, temp.second)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first - 1, temp.second)] == INF){
                    maze_block_map[pair<int, int>(temp.first - 1, temp.second)] = step;
                    queue.push_back(pair<int, int>(temp.first - 1, temp.second));
                }
            }
        }
        
        // up
        if(temp.second + 1 < vertical_grids){
            if(!vertical_global_edge[pair<int, int>(temp.first, temp.second)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first, temp.second + 1)] == INF){
                    maze_block_map[pair<int, int>(temp.first, temp.second + 1)] = step;
                    queue.push_back(pair<int, int>(temp.first, temp.second + 1));
                }
            }
        }

        // down
        if(temp.second - 1 >= 0){
            if(!vertical_global_edge[pair<int, int>(temp.first, temp.second - 1)]->overflow()){
                if(maze_block_map[pair<int, int>(temp.first, temp.second - 1)] == INF){
                    maze_block_map[pair<int, int>(temp.first, temp.second - 1)] = step;
                    queue.push_back(pair<int, int>(temp.first, temp.second - 1));
                }
            }
        }
    }
    
    return false;
}

int main(int argc, char *argv[]){
    input_fileName = argv[1];
    output_fileName = argv[2];

    read_input();
    //debug_read_input();

    construct_global_edge_map();
    //debug_global_edge_map();

    rounting_L_shape();

    construct_overflow_global_edges_vec();
    debug_overflow_global_edges_vec();
    debug_demand_supply();

    ripUp_reroute();
    debug_demand_supply();

    output_file();
}