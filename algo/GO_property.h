#pragma once
#include"experience_method.h"
#include<iostream>

using namespace std;
class Go_property: public basic_technique_detector{
    public:
        Go_property(int n):
            basic_technique_detector(n){
        };

        bool is_occupied(int color, pair<int,int> pt);

        //return 1 if attacker can eat the stone at pt with continuously atari
        bool can_capture(int attacker_color, pair<int,int> pt, int depth = 2);
        bool can_escape(int escaper_color, pair<int,int> pt, int depth = 2);

        bool is_dead(int dead_color, pair<int,int> pt, int depth = 2);

        bool equal_to_stone( int color, pair<int,int> pt );

        bool is_connect(int color, pair<int,int> pt);
        bool is_cut(int color, pair<int,int> pt);

        //return 1 if pt1 and pt2 connected directly/by kosumi
        bool is_connected(pair<int,int> pt1, pair<int,int> pt2);
};

bool Go_property::is_occupied(int color, pair<int,int> pt){
    // ?   ?   ?
    // ? [pt]  ?
    // ?   ?   ?
    //if all ? are empty or [color], then [pt] is occupied by color
    //besides, if any one of ? is -color, but [pt] can't connect to such ?, we still see [pt] is being occupied
    if(get_board(pt) != 0){
        if(get_board(pt) == color)return true;
        else return is_dead(-color, pt);
    };
    
    int self_cnt = 0, edge_cnt = 0, empty_cnt = 0;

    for(int dy = -1;dy<=1;dy++){
        for(int dx=-1;dx<=1;dx++){
            if(!(dy|dx))continue;
            pair<int,int> adj( pt.first+dy, pt.second+dx );
            if( get_board(adj) == color )self_cnt++;
            else if(get_board(adj) == EDGE_VALUE or out_of_edge(adj))edge_cnt++;
            else if(get_board(adj)  == 0 and (dy^dx))empty_cnt++;
            else{// it's enemy's stone
                if( dy&dx ){// if dy!=0 and dx != 0
                    pair<int,int> corner1( pt.first+dy , pt.second );
                    pair<int,int> corner2( pt.first , pt.second+dx );
                    if(get_board(corner1) != color or 
                                    get_board(corner2) != color){
                        return 0;
                    }
                }
                else{
                    return 0;
                }
            }
        }
    }

    if( empty_cnt <=2 )return 1;

    pair<int,int> edge_elements[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int edge_elements_cnt = find_connected_component_edge( pt, edge_elements );
    for(auto eg: edge_elements)
        if( get_board(eg) == -color )
            return 0;
    
    return 1;
}

bool Go_property::can_capture(int attacker_color, pair<int,int> pt, int depth){
    if(depth<=0)return 0;
    if( get_board(pt)!= -attacker_color ){
        return 0;
    }
    int current_liberty = calculate_liberty( pt );
    if( current_liberty >2){
        return 0;
    };
    if( current_liberty <=1 ){
        if(!is_ko or ko_position != pt)
            return 1;
    };
    
    pair<int,int> possible_move[MAX_BOARD_SIZE*MAX_BOARD_SIZE];// = get_connected_component( pt )[1];
    int possible_move_cnt = find_connected_component_edge( pt, possible_move );


    for(int i=0; i<possible_move_cnt; i++){
        pair<int,int> attack_pt = possible_move[i];
        if(get_board(attack_pt)!=0)continue;
        if(!is_legal_movement( attacker_color, attack_pt ))
            continue;
        add_stone(attacker_color, attack_pt);

        bool can_defend = false;

        for(auto adj_pt: possible_move){
            if( get_board(adj_pt)!= attacker_color )
                continue;
            if( calculate_liberty(adj_pt) <= 1 ){
                pair<int,int> liberty_position = get_liberty_position( adj_pt );

                if( is_legal_movement( -attacker_color, liberty_position ) ){
                    add_stone( -attacker_color, liberty_position );

                    can_defend |= (!can_capture( attacker_color, pt, depth-1 ));
                    undo();
                }
            }
            if(can_defend){
                break;
            };
        }
        
        if(!can_defend){
            for(auto defend_pt: possible_move){
                if( get_board(defend_pt) != 0 )
                    continue;
                
                if(!is_legal_movement(-attacker_color, defend_pt)){
                    continue;
                }
                add_stone( -attacker_color, defend_pt );

                can_defend |= (!can_capture(attacker_color, pt, depth-1) );
                undo();
                if(can_defend){
                    // cout<<" white defended by "<<defend_pt<<endl;
                    break;
                };
            }
        }
        undo();
        if(!can_defend){
            // cout<<"attack success\n";
            return 1;
        }
        // else cout<<"fail\n";
    }
    return 0;
}

bool Go_property::can_escape(int escaper_color, pair<int,int> pt, int depth){
    if(get_board(pt) == 0){
        cout<<"ERROR: calling can_escape for empty position\n";
        return 1;
    }
    if(!can_capture( -escaper_color, pt ))
        return 1;
    for(int py=1;py<=get_board_size();py++){
        for(int px=1;px<=get_board_size();px++){
            if( is_rational_movement( escaper_color,  {py,px}) ){
                add_stone( escaper_color, {py,px} );
                bool is_escape = !can_capture( -escaper_color, pt, depth );
                undo();
                // if(is_escape)cout<<"escape by "<<py<<","<<px<<endl;
                if( is_escape )return 1;
            }
        }
    }
    return 0;
}

// if a stone can connect to an can-escape block (no matter if it'll dead after the connection), then we see the stone haven't dead yet
bool Go_property::is_dead(int dead_color, pair<int,int> pt, int depth){
    if(can_escape(dead_color, pt, depth) )
        return 0;
    bool is_survive = 0;
    
    pair<int,int> connect_pts[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    
    int connect_pts_size = find_connected_component_edge(pt, connect_pts);
    
    //觀察pt是否能在一手內連到目前未死的棋塊
    for(int i0=0; i0<connect_pts_size;i0++){
        if(get_board(connect_pts[i0]) != 0)
            continue;
        pair<int,int> test_pt = connect_pts[i0];
        // cout<<test_pt<<" ";
        for(int d=0;d<4;d++){
            pair<int,int> nx( test_pt.first+dir[d][0], test_pt.second+dir[d][1] );
            if(get_board(nx) == dead_color and can_escape(dead_color, nx))
                return 0;
        }
    }
    // cout<<"-v-\n";
    for(int py=1; py<=get_board_size(); py++){
        for(int px=1; px<=get_board_size(); px++){
            if(is_legal_movement(dead_color, {py,px})){
                add_stone(dead_color, {py,px});
                bool is_survive = !can_capture(-dead_color, pt);
                if(is_survive){
                    // cout<<"try defend on "<<py<<","<<px<<endl;
                    for(int py1=1; py1<=get_board_size(); py1++){
                        for(int px1=1; px1<=get_board_size(); px1++){
                            if(is_legal_movement(-dead_color, {py1, px1})){
                                add_stone(-dead_color, {py1,px1});
                                is_survive &= can_escape(dead_color, pt);
                                undo();
                            }
                        }
                    }
                    if(is_survive) {
                        undo();
                        return 0;
                    }
                };
                undo();
            }
        }
    }

    return 1;
}

bool Go_property::equal_to_stone( int color, pair<int,int> pt ){
    // auto elements = get_connected_component( pt );
    // auto 
    pair<int,int> interior[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    pair<int,int> edge[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
//    auto edge = elements[1];
    int interior_size = find_connected_component( pt, interior );
    int edge_size = find_connected_component_edge( pt, edge );
    
    for(int i=0;i<edge_size;i++){
        if( get_board(edge[i]) != color )
            return 0;
    }

    for(int i=0;i<interior_size;i++){
        if( !is_legal_movement( -color, interior[i] ) ){
            for(int j=0;j<i;j++)
                undo();

            return 1;
        }
        add_stone( -color, interior[i] );
    }
    for(int i=0;i<interior_size;i++)
        undo();
    return 0;
}

bool Go_property::is_connected(pair<int,int> pt1, pair<int,int> pt2){
    int color = get_board(pt1);
    if(get_board(pt1) != get_board(pt2) or color == 0)
        return 0;
    int dy=pt2.first-pt1.first, dx=pt2.second-pt1.second;
    if(dy>1 or dy<-1 or dx>1 or dx<-1)return 0;
    if(dx==0 or dy==0)return 1;
    pair<int,int> corner1 = {pt1.first+dy, pt1.second}, corner2 = {pt1.first, pt1.second+dx};

    if( get_board(corner1) == -color  or get_board(corner2) == -color ){
        bool empty_1 = (get_board(corner1) == 0), 
                                empty_2 = (get_board(corner2)  == 0);
        if(!empty_1 and !empty_2)return 0;

        if(empty_1)add_stone(-color, corner1);
        if(empty_2)add_stone(-color, corner2);
                    
        bool still_connect = can_capture( color, corner1 ) or can_capture( color, corner2);

        if(empty_1)undo();
        if(empty_2)undo();
        return still_connect;
    }
    else return 0;
}