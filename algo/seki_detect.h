#pragma once
#include"capture_to_win.h"
using namespace std;

class seki_detector: public capture_win_detect{
    public:
    seki_detector(int n):
        capture_win_detect(n)
    {};

    bool can_win(int color){
        return kill_within_one_move(color);

        // return kill_with_depth(color, 3, 0) or win_within_one_move(color);
    }

    bool dead_after_n_move(int color, int depth=2){
        if(depth<=0)return 0;
        if(kill_within_one_move(-color))return 1;
        // if(kill_with_depth(-color,3))return 1;
        for(int py=1;py<=get_board_size(); py++){
            for(int px=1; px<=get_board_size(); px++){
                if(is_legal_movement(color, {py,px})){
                    add_stone(color, {py,px});
                    if(!dead_after_n_move(color, depth-1)){
                        undo();
                        return 0;
                    }
                    undo();
                }
            }
        }
        return 1;
    };

    bool is_fake_eye(int color, pair<int,int> pt){
        int py=pt.first, px=pt.second;
        if(get_board(py,px) != 0 or !is_legal_movement(color, pt))return 0;
        for(int d=0;d<4;d++){
            if(!out_of_edge({py+dir[d][0],px+dir[d][1]}) and get_board(py+dir[d][0], px+dir[d][1]) != color)return 0;
        }
        int occupied_corner = 0;
        for(int dy=-1;dy<=1;dy+=2){
            for(int dx=-1;dx<=1;dx+=2){
                if(!out_of_edge({py+dy,px+dx}) and get_board(py+dy, px+dx) == -color)
                    occupied_corner++;
            }
        }
        return occupied_corner>1 or (at_edge(pt) and occupied_corner>0);
    }

    bool is_seki(int color = 1, int depth = 2, bool show_detail = false){
        // return dead_after_n_move(1) and dead_after_n_move(-1);
        int append_stone_cnt = 0;
        for(int py=1; py<=get_board_size(); py++){
            for(int px=1; px<=get_board_size(); px++){
                if(is_fake_eye(1, {py,px})){
                    add_stone(1, {py,px});
                    append_stone_cnt++;
                }
                else if(is_fake_eye(-1,{py,px})){
                    add_stone(-1, {py,px});
                    append_stone_cnt++;
                }
            }
        }
        bool solution = 1;
        if(show_detail)print_board();
        for(int py=1; py<=get_board_size() and solution; py++){
            for(int px=1; px<=get_board_size()and solution; px++){
                if(is_legal_movement(1, {py,px})){
                    if(show_detail)cout<<"try put black on "<<py<<","<<px<<endl;
                    add_stone(1, {py,px});
                    bool dead_black = dead_after_n_move(1, depth);
                    undo();
                    if(!dead_black)solution = 0;
                }
                if(is_legal_movement(-1, {py,px})){
                    if(show_detail)cout<<"try put white on "<<py<<","<<px<<endl;
                    add_stone(-1, {py,px});
                    bool dead_white = dead_after_n_move(-1, depth);
                    undo();
                    if(!dead_white)solution = 0;
                }
                
            }
        }
        while(append_stone_cnt--)
            undo();

        return solution;
    }

    bool defend_by_seki(int color, bool show_detail = false){

        if( is_seki() )
            return 1;

        bool opponent_win = can_win(-color);
        bool seki_defend = 0;
        // cout<<"start\n";
        for(int py=1; py<=get_board_size() and opponent_win; py++){
            for(int px=1; px<=get_board_size() and opponent_win; px++){
                // cout<<"#";
                if(is_legal_movement(color, {py,px})){
                    if(show_detail) cout<<"visit "<<py<<","<<px<<",color:"<<color<<endl;
                    add_stone(color, {py,px});
                    // print_board();
                    opponent_win = can_win(-color);
                    if(!opponent_win){
                        if(show_detail) cout<<"\t white does not win\n";
                        if( is_seki() ){
                            if(show_detail)cout<<"\t\t defended by seki\n"<<endl;
                            opponent_win = 1;
                            seki_defend = 1;
                        }
                    }
                    undo();
                }
            }
        }
        return opponent_win and seki_defend;
    }
};
