#pragma once

#include<iostream>
#include<iomanip>
#include<algorithm>
#include<cmath>

#ifndef map
#include<map>
#endif

#ifndef MAX_BOARD_SIZE
const int MAX_BOARD_SIZE = 6;
#endif

#ifndef EDGE_VALUE
const int EDGE_VALUE = 10;
#endif

using namespace std;

ostream &operator <<(ostream& ofs, pair<int,int> pt){
    ofs<<"("<<pt.first<<","<<pt.second<<")";
    return ofs;
}

//定義棋盤歷史紀錄格式，每步
struct go_record{
    int color;
    pair<int,int> point;
    bool ate_stone = 0;//是否吃子
    int ate_stone_cnt = 0;//若有吃子，則紀錄吃子數量
    vector<int> ate_stone_direction;//若有吃子，則記錄所有該子提吃子的方位，將用於undo時還原棋盤
    bool make_ko = false;//是否產生打劫

    go_record():
        color(0), point(0,0), ate_stone_direction(0), ate_stone(0)
    {}

    go_record(int color, int py,int px):
        color(color), point(py,px), ate_stone_direction(0), ate_stone(0)
    {}

};

//判斷connected part是否與pt相鄰(或包含)
inline bool is_adjacent( pair<int,int> pt, pair<int,int> connected_part[], int connected_part_size ){
    for(int i=0;i<connected_part_size;i++){
        pair<int,int> it = connected_part[i];
        if( abs( pt.first-it.first ) + abs(pt.second - it.second) <=1 )
            return true;
    }
    return false;
}

inline bool is_inside(pair<int,int> pt, pair<int,int> component[], int component_size){
    for(int i=0; i<component_size; i++){
        if( component[i] == pt )return true;
    }
    return false;
}

class GO_board{
    protected:
        //棋盤
        int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
        //棋盤大小
        int board_size;
        //落子紀錄
        go_record game_recorder[400];
        int record_length = 0;



    public:
        //雙方目前吃子數
        int black_ate_stone = 0, white_ate_stone = 0;

        void print_board(){
            cout<<"==========\n";
            cout<<setw(3)<<"\\";
            for(int i=1;i<=board_size;i++)cout<<setw(3)<<i;
            cout<<endl;
            for(int i=1; i<=board_size; i++){
                cout<<setw(3)<<i;
                for(int j=1; j<=board_size; j++){
                    cout<<setw(3)<<( board[i][j]==0?".": (board[i][j]==1? "O":"X") );
                }
                cout<<"\n";
            }
            cout<<"==========\n";

        };

        GO_board(int n):
            board_size(n), is_ko(0)
        {
            for(int i=0;i<=n;i++){
                board[i][0] = EDGE_VALUE;
                board[0][i] = EDGE_VALUE;
                board[n][0] = EDGE_VALUE;
                board[0][n] = EDGE_VALUE;
            }
            record_length = 0;
        };

        //紀錄目前是否有劫
        bool is_ko = 0;
        //紀錄劫位置
        pair<int,int> ko_position;

        //在棋盤上落子，包括提子等操作。請勿用此函數以外的方式在盤面落子
        //若落子失敗則return 0
        inline bool add_stone(int color, pair<int,int> position, bool make_record = true){
            int py = position.first, px = position.second;

            //判斷落子點是否合法
            if( out_of_edge(position) ){
                cout<<"ERROR: add stone on edge\n";
                return 0;
            }
            else if(board[py][px] != 0){
                if(min(py,px)<=0 or max(py,px)>board_size)cout<<"ERROR: put a stone at edge\n";
                else cout<<"ERROR: add a stone at non-empty position\n";
                cout<<"ERROR PT: "<<py<<","<<px<<endl;
                return 0;
            }

            //進行記錄
            go_record current_record(color, py, px);

            board[py][px] = color;

            //紀錄吃子方位並移除死子
            int removed_stone_cnt = 0;
            for(int d=0;d<4;d++){
                if( board[ py+dir[d][0] ][ px+dir[d][1] ] != -color )continue;
                if(  calculate_liberty( {py+dir[d][0], px+dir[d][1]} )==0){
                    current_record.ate_stone = 1;
                    current_record.ate_stone_direction.push_back(d);
                    removed_stone_cnt += remove_connected_component( {py+dir[d][0], px+dir[d][1]} );
                }
            }

            //判斷是否產生劫
            if(removed_stone_cnt == 1 and calculate_liberty( position ) == 1 and
                                    get_component_size(position) == 1){
                is_ko = 1;
                ko_position = position;
                current_record.make_ko = 1;
            }
            current_record.ate_stone_cnt = removed_stone_cnt;

            //記錄
            if(make_record){
                game_recorder[record_length] = current_record;
                record_length++;
            }

            //更新吃子數量
            if(color == 1)black_ate_stone+=removed_stone_cnt;
            else white_ate_stone+=removed_stone_cnt;

            return 1;
        };

        //取消最近一次落子
        void undo(){
            go_record undo_rec = game_recorder[record_length-1];
            int py = undo_rec.point.first, px = undo_rec.point.second;
            
            //若有吃子，則將被吃的子放回
            if(undo_rec.ate_stone){
                for(auto d: undo_rec.ate_stone_direction){
                    if( board[ py+dir[d][0] ][ px+dir[d][1] ] == 0 ){
                        pair<int,int> removed_stones[MAX_BOARD_SIZE*MAX_BOARD_SIZE];// = get_connected_component( {py+dir[d][0], px+dir[d][1]} )[0];
                        int removed_size = find_connected_component( {py+dir[d][0], px+dir[d][1]}, removed_stones );
                        for(int i=0;i<removed_size;i++){
                            pair<int,int> recover_pt = removed_stones[i];
                            board[ recover_pt.first ][ recover_pt.second ] = - undo_rec.color;
                        }
                    }
                }
            }
            
            //回復吃子數量
            if(undo_rec.color == 1)black_ate_stone -= undo_rec.ate_stone_cnt;
            else white_ate_stone -= undo_rec.ate_stone_cnt;
            
            record_length--;

            //回復打劫狀態
            is_ko = 0;

            if(record_length>0){
                is_ko = game_recorder[record_length-1].make_ko;
                if(is_ko)ko_position = game_recorder[record_length-1].point;
            }
            board[py][px] = 0;
        }

        //清除棋盤上某位置
        void clean_position(int py,int px){
            board[py][px] = 0;
        }
        void clean_position(pair<int,int> pt){
            board[pt.first][pt.second] = 0;
        }
        
        void set_board( vector<string> board_str ){
            board_size = board_str.size();
            for(int i=0;i<=board_size+1;i++){
                board[i][0] = EDGE_VALUE;
                board[0][i] = EDGE_VALUE;
                board[board_size+1][0] = EDGE_VALUE;
                board[0][board_size+1] = EDGE_VALUE;
            }
            map<char,int> trans = { {'.',0}, {'O',1}, {'X',-1} };
            for(int py=1;py<=board_size;py++){
                for(int px=1;px<=board_size;px++){
                    board[py][px] = trans[ board_str[py-1][px-1] ];
                }
            }
        }
                
        void set_board( vector< vector<int> > inp_board ){
            board_size = inp_board.size();
            for(int i=0;i<=board_size+1;i++){
                board[i][0] = EDGE_VALUE;
                board[0][i] = EDGE_VALUE;
                board[board_size+1][0] = EDGE_VALUE;
                board[0][board_size+1] = EDGE_VALUE;
            }
            for(int py=1;py<=board_size;py++){
                for(int px=1;px<=board_size;px++){
                    board[py][px] = inp_board[py-1][px-1];
                }
            }
        }

        //判斷pt是否出界
        inline bool out_of_edge( pair<int,int> pt ){
            return min(pt.first, pt.second)<=0 or max(pt.first,pt.second)>board_size;
        }
        inline bool out_of_edge( int py,int px ){
            return min(py,px)<=0 or max(py,px)>board_size;
        }

        //判斷pt是否在死亡線上
        inline bool at_edge( pair<int,int> pt){
            return pt.first == 1 or pt.second == 1 or pt.first == board_size or pt.second == board_size;
        }

        //判斷pt是否位於四角
        inline bool at_corner( pair<int,int> pt){
            return (pt.first == 1 or pt.first == board_size) and (pt.second == 1 or pt.second == board_size);
        }
        
        //找出pt所在連通塊
        int find_connected_component( pair<int,int> pt , pair<int,int>* all_elements);

        //找出pt所在連通塊之邊界
        inline int find_connected_component_edge(pair<int,int> pt, pair<int,int> edge_record[]){
            pair<int,int> component[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int component_size = find_connected_component(pt, component);
            int edge_size = 0;
            for(int py=1;py<=board_size;py++){
                for(int px=1;px<=board_size;px++){
                    if( is_adjacent({py,px}, component, component_size) and board[py][px] != get_board(pt) ){
                        edge_record[edge_size++] = {py,px};
                    }
                }
            }
            return edge_size;
        }

        inline int find_connected_component_edge(int color, pair<int,int>* component, int component_size, pair<int,int> edge_record[]){
            int edge_size = 0;
            bool is_visited[MAX_BOARD_SIZE+1][MAX_BOARD_SIZE+1];
            for(int i=0;i<=get_board_size();i++){
                for(int j=0;j<=get_board_size();j++){
                    is_visited[i][j] = 0;
                }
            }
            for(int i=0;i<component_size;i++){
                for(int d=0;d<4;d++){
                    if(out_of_edge(component[i].first+dir[d][0], component[i].second+dir[d][1]))
                        continue;
                    if(get_board(component[i].first+dir[d][0], component[i].second+dir[d][1]) != color\
                            and !is_visited[ component[i].first+dir[d][0] ][ component[i].second+dir[d][1] ]){
                        edge_record[edge_size++] = {component[i].first+dir[d][0], component[i].second+dir[d][1]};
                        is_visited[ component[i].first+dir[d][0] ][ component[i].second+dir[d][1] ] = 1;
                    }
                }
            }
            return edge_size;
        }

        inline int get_component_size(pair<int,int> pt){
            pair<int,int>  recorder[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            return find_connected_component( pt,recorder );            ;
        }

        //計算pt所在連通塊的氣數
        inline int calculate_liberty(pair<int,int> pt);// calculate the liberty of the connected component with pt

        //若pt剩下一氣，則此函數回傳剩下的一氣著點位置
        inline pair<int,int> get_liberty_position( pair<int,int> pt );
        int dir[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

        //移除root所在連通塊
        int remove_connected_component( pair<int,int> root ){
            pair<int,int> removed_stones[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int component_size = find_connected_component( root , removed_stones);
            for(int i=0;i<component_size;i++){
                pair<int,int> rm_pt = removed_stones[i];
                board[rm_pt.first][rm_pt.second] = 0;
            }
            return component_size;
        }

        //移除給定著點序列
        void remove_stones(pair<int,int> removed_stones[], int stone_size ){
            for(int i=0;i<stone_size;i++){
                pair<int,int> rm_pt = removed_stones[i];
                board[rm_pt.first][rm_pt.second] = 0;
            }
        }

        //判斷color落子於pt是否合法
        inline bool is_legal_movement(int color, pair<int,int> pt){
            if(out_of_edge(pt))return 0;
            if( board[pt.first][pt.second] != 0 )
                return 0;

            if(is_ko){
                if( color != board[pt.first][pt.second] and
                         abs(pt.first-ko_position.first)+ abs( pt.second -ko_position.second ) <= 1){
                    return 0;
                }
            }

            add_stone( color, pt );
            bool is_legal = calculate_liberty( pt );
            undo();
            return is_legal;
        }
        

        inline int get_board(int py,int px){
            return board[py][px];
        }

        inline int get_board(pair<int,int> pt){
            return board[pt.first][pt.second];
        }

        inline int get_board_size(){
            return board_size;
        }

        //used for kill_with_depth
        inline int to_serial(int color, int current_color){
            int serial = 0;
            for(int i=0;i<16;i++){
                int py=4-i/4, px=4-i%4;
                if(color == 1)serial = serial*3+((board[py][px]+3)%3);
                else serial = serial*3+(3-board[py][px])%3;
            }
            serial = serial*4+is_ko*2+(current_color == color);
            return serial;
        }

        //used for is_big_eye
        inline long long to_serial(int color, pair<int,int> pt){
            long long serial = 0;
            for(int i=0;i<16;i++){
                int py=4-i/4, px=4-i%4;
                if(color == 1)serial = serial*3+((board[py][px]+3)%3);
                else serial = serial*3+(3-board[py][px])%3;
            }
            serial =serial*16+4*pt.first+pt.second;
            return serial;
        }
        void reset_ate_stone(){
            black_ate_stone = 0;
            white_ate_stone = 0;
        }

};

inline int GO_board::calculate_liberty( pair<int,int> pt ){
    int color = board[pt.first][pt.second];
    queue< pair<int,int> > BFS;
    BFS.push(pt);
    bool visited[ board_size+1 ][ board_size+1 ];
    memset(visited,0,sizeof(visited));
    visited[pt.first][pt.second] = 1;
    int liberty = 0;
    while(BFS.size()){
        pair<int, int> cur = BFS.front();
        BFS.pop();
        int py = cur.first, px = cur.second;
        for(int d=0;d<4;d++){
            int nx_py = py+dir[d][0], nx_px = px+dir[d][1];
            
            if( min(nx_py,nx_px)<=0 or max(nx_py, nx_px)>board_size )
                continue;
            if( visited[nx_py][nx_px] or board[nx_py][nx_px] == -color)
                continue;
                
            if( board[nx_py][nx_px] == 0){
                liberty++;
            }
            else{
                BFS.push({nx_py, nx_px});
            }
            visited[nx_py][nx_px] = 1;
        }
    }
    return liberty;
}

int GO_board::find_connected_component( pair<int,int> pt , pair<int,int>* all_elements){
    int color = board[pt.first][pt.second];
            
    queue< pair<int,int> > BFS;
    BFS.push(pt);

    bool visited[ board_size+1 ][ board_size+1 ];
    memset(visited,0,sizeof(visited));
    visited[pt.first][pt.second] = 1;
    int component_size = 0;
    while(BFS.size()){
        pair<int, int> cur = BFS.front();
        BFS.pop();

        all_elements[component_size ++ ] = cur;
        int py = cur.first, px = cur.second;
        for(int d=0;d<4;d++){
            int nx_py = py+dir[d][0], nx_px = px+dir[d][1];
                    
            if( min(nx_py,nx_px)<=0 or max(nx_py, nx_px)>board_size )
                continue;
            if( visited[nx_py][nx_px])
                continue;
            if( board[nx_py][nx_px] != color ){// cur reach an edge, so it's an edge element
                // edge_elements.push_back({nx_py,nx_px});
                visited[nx_py][nx_px] = 1;
                continue;
            }
            BFS.push({nx_py, nx_px});
            visited[nx_py][nx_px] = 1;
        }
    }
    return component_size;
};

inline pair<int,int> GO_board::get_liberty_position( pair<int,int> pt ){
    int color = board[pt.first][pt.second];
    queue< pair<int,int> > BFS;
    BFS.push(pt);
    bool visited[ board_size+1 ][ board_size+1 ];
    memset(visited,0,sizeof(visited));
    visited[pt.first][pt.second] = 1;
        
    while(BFS.size()){
        pair<int, int> cur = BFS.front();
        BFS.pop();
        int py = cur.first, px = cur.second;
        for(int d=0;d<4;d++){
            int nx_py = py+dir[d][0], nx_px = px+dir[d][1];
            
            if( min(nx_py,nx_px)<=0 or max(nx_py, nx_px)>board_size )
                continue;
            if( visited[nx_py][nx_px] or board[nx_py][nx_px] + color == 0 )
                continue;
                
            if( board[nx_py][nx_px] == 0){
                return  {nx_py,nx_px};
            }
            else{
                BFS.push({nx_py, nx_px});
            }
            visited[nx_py][nx_px] = 1;
        }
    }
    return {-1,-1};
}