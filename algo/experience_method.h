#pragma once

#ifndef GO_board
#include"Go_board.h"
#endif

class basic_technique_detector: public GO_board{
    public:
        basic_technique_detector(int n):
            GO_board(n)
        {};
        inline bool is_capture(int color, pair<int,int> pt);
        inline bool is_atari(int color, pair<int,int> pt);
        inline int  is_extend_liberty(int color, pair<int,int> pt);
        inline bool is_an_eye( int color, pair<int,int> pt );

        inline bool is_rational_movement( int color, pair<int,int> pt ){
            if(!is_legal_movement(color, pt))return 0;
            add_stone(color, pt);
            bool is_rational = (calculate_liberty(pt) > 1) or is_ko or game_recorder[record_length-1].ate_stone;
            undo();
            return is_rational;
        };

        inline int find_board_area(int color ,pair<int,int> root, pair<int,int> recorder[]);

};

bool basic_technique_detector::is_capture(int color, pair<int,int> pt){
    int py = pt.first, px = pt.second;
    if(get_board(py,px) != 0)return 0;
    add_stone(color, {py,px});
    bool is_ate = game_recorder[record_length-1].ate_stone;
    undo();
    return is_ate;
}


bool basic_technique_detector::is_atari(int color, pair<int,int> pt){
    int py = pt.first, px = pt.second;
    if(get_board(py,px) != 0)return 0;
    add_stone(color, {py,px});
    for(int d=0; d<4; d++){
        int ny = py+dir[d][0], nx=px+dir[d][1];
        if( get_board(ny,nx) != -color )continue;
        if( calculate_liberty({ny,nx}) == 1 ){
            undo();
            return 1;
        }
    }
    undo();
    return 0;
}

int basic_technique_detector::is_extend_liberty(int color, pair<int,int> pt){
    int py = pt.first, px = pt.second;
    if(get_board(py,px) != 0)return 0;
    int max_liberty_diff = 0;
    for(int d=0;d<4;d++){
        int ny = py+dir[d][0], nx=px+dir[d][1];
        if( get_board(ny,nx) != color )continue;
        
        int original_liberty = calculate_liberty( {ny,nx} );
        if(original_liberty>1)continue;
        add_stone(color, {py,px});
        int liberty_diff = calculate_liberty( {py,px} ) - original_liberty;
        undo();
        max_liberty_diff = max( liberty_diff, max_liberty_diff );
    }

    return max_liberty_diff;
}

bool basic_technique_detector::is_an_eye( int color, pair<int,int> pt ){
    if( get_board(pt) != 0 )
        return 0;
    int edge_num = 0;
    for(int d=0;d<4;d++){
        int ny = pt.first + dir[d][0], nx = pt.second + dir[d][1];
        if(out_of_edge({ny,nx})){
            edge_num++;
        }
        else if(get_board(ny,nx) != color){
            return 0;
        }
    }
    int danger_corner = 0;

    for(int dx=-1;dx<=1; dx+=2){
        for(int dy=-1;dy<=1;dy+=2){
            pair<int,int> nx_pt = { pt.first + dy, pt.second + dx };
            if( out_of_edge(nx_pt) )
                continue;

            if( get_board(pt.first+dy, pt.second+dx) != color ){
                if( get_board(pt.first+dy, pt.second+dx) == -color or is_rational_movement( -color, nx_pt ) ){
                    danger_corner++;
                }
            }
        }
    }

    if( edge_num >= 1 and danger_corner >=1 )return 0;
    else if(edge_num == 0 and danger_corner>=2)return 0;
    return 1;
}

inline int basic_technique_detector::find_board_area(int color, pair<int,int> root, pair<int,int> recorder[]){
    int area_size = 0;
    pair<int,int> st[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int size=0;
    bool is_visited[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    memset(is_visited,0,sizeof(is_visited));
    st[size++] = root;
    is_visited[root.first][root.second] = 1;
    while(size){
        recorder[area_size++] = st[--size];
        int py=recorder[area_size-1].first, px=recorder[area_size-1].second;
        for(int d=0;d<4;d++){
            int nx_py = py+dir[d][0], nx_px = px+dir[d][1];
            if(out_of_edge({nx_py, nx_px}))
                continue;
            if(get_board(nx_py,nx_px)== -color or is_visited[nx_py][nx_px])// or !is_rational_movement(color, {nx_py,nx_px}))
                continue;
            st[size++] = {nx_py, nx_px};
            is_visited[nx_py][nx_px] = 1;
        }
    }
    return area_size;
};
