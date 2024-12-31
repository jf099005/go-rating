#pragma once

#ifndef EXP
#define EXP
#include"GO_property.h"
#endif

#include<iostream>

using namespace std;
class endgame_detect: public Go_property{
    public:
        endgame_detect(int n):
            Go_property(n){
        };

        bool capture_with_eyes(int color , bool show_detail = false, bool include_2_eyes = true, pair<int,int> root = {0,0});

        bool is_all_connected( int color );

        //return 1 if pt in a big eye and pt have the smallest lexicographically order
        bool is_big_eye(int color, pair<int,int> pt_start, int depth = 4, bool exclude_corner = true);

        //return 1 if pt is_big_eye() and the big eye which contain pt is a eye of the given block
        bool valid_big_eye(int color, pair<int,int> pt_start , pair<int,int> block[], int block_size, int depth = 4, bool exclude_corner = true);

        int get_connected_part( int color, pair<int,int> pt, pair<int,int> record[] );

        bool is_connect(int color, pair<int,int> pt);
        bool is_cut(int color, pair<int,int> pt);

        // '.' : 空，'O':我方子，'!':我方子或空，'?':don't care
        string eye_shape[4] = {
            "..!?",
            "OO!?",
            "????",
            "????"
        };
        bool corner_eye_shape_detect(int color, pair<int,int> pt);
};

//檢查pt位置是否符合角落眼型(符合eye_shape的眼型)
bool endgame_detect::corner_eye_shape_detect(int color, pair<int,int> pt){
            
    if(pt.first !=1 and pt.first !=get_board_size())
        return 0;
    if(pt.second !=1 and pt.second !=get_board_size())
        return 0;
    
    //dx,dy為x軸/y軸前進方向，e.g.當pt在左上角時dx,dy是正常的1,1，但在右下角時dx,dy都需變反向，也就是-1,-1
    int dy=1, dx=1;
    if( pt.first+dy <= 0 or pt.first+dy > get_board_size() )
        dy = -dy;
    if( pt.second+dx <= 0 or pt.second+dx > get_board_size() )
        dx = -dx;

    //當rev=1時,xy軸會顛倒
    int rev = 0;
    
    for(int _=0; _<2;_++){
        bool is_same = 1;
        // cout<<dy<<","<<dx<<endl;
        for(int i1=0;i1<get_board_size() and is_same; i1++){
            for(int i2=0; i2<get_board_size() and is_same; i2++){
                //cur為狀態轉換後座標下的board(i1,i2)
                int cur = get_board( pt.first+((!rev)*i1+rev*i2)*dy, pt.second + ((rev)*i1+(!rev)*i2)*dx );
                if( eye_shape[i1][i2] == '.' )
                    is_same &= (cur == 0);
                else if(eye_shape[i1][i2] == 'O')
                    is_same &= (cur == color);
                else if(eye_shape[i1][i2] == '!')
                    is_same &= (cur == 0 or cur == color);
                // if(!is_same)cout<<"fail at pt "<<i1<<","<<i2<<"/ ("<<pt.first+i1*dy<<
                //         ","<<pt.second+i2*dx<<")"<<cur<<":"<<eye_shape[i1][i2]<<endl;
            }
        }
        if(is_same)return 1;
        // swap(dy,dx);
        rev = !rev;
    }
    return 0;
}

//用DFS尋找pt所在位置的(weakly) connected part
int endgame_detect:: get_connected_part( int color, pair<int,int> pt, pair<int,int> res[] ){
    if(get_board(pt) != color){
        return 0;
    }
    
    bool is_visited[get_board_size()+2][get_board_size()+2];
    memset(is_visited, 0, sizeof(is_visited));
    pair<int,int> DFS_stack[get_board_size()*get_board_size()];

    int stack_size = 0;
    DFS_stack[stack_size++] = pt;

    int result_size = 0;

    while(stack_size){
        pair<int,int> cur = DFS_stack[--stack_size];
        is_visited[ cur.first ][ cur.second ] = 1;
        res[result_size++] = cur;
        for(int d=0;d<4;d++){
            pair<int,int> nx( cur.first+dir[d][0], cur.second+dir[d][1] );
            
            if( out_of_edge({nx}) )
                continue;
            if( is_visited[nx.first][nx.second])
                continue;

            if(get_board(nx) != color){
                if(!is_big_eye(color, nx))
                    continue;
            }
            
            DFS_stack[stack_size++] = nx;
            is_visited[nx.first][nx.second] = 1;
        }
        
        //檢查斜角方向是否連接，若有連接則同樣加入stack搜尋
        for(int dy=-1; dy<=1; dy+=2){    // 列 斜 方
            for(int dx=-1; dx<=1; dx+=2){// 舉 角 向
                pair<int,int> nx( cur.first+dy, cur.second+dx );

                if( out_of_edge(nx) )
                    continue;
                if( is_visited[nx.first][nx.second])
                    continue;
                if(get_board(nx) != color){
                    continue;
                }

                //corner1, corner2為兩個眼角，若其中一個已被對手佔領
                //且另一個對對手來說非禁著點，此時對手是有機會將cur與nx分斷，因此不將該點看做有連接
                pair<int,int> corner1 = {cur.first+dy, cur.second}, corner2 = {cur.first, cur.second+dx};

                if( get_board(corner1) == -color  or get_board(corner2) == -color ){
                    bool empty_1 = (get_board(corner1) == 0), 
                                empty_2 = (get_board(corner2)  == 0);
                    if(!empty_1 and !empty_2)continue;

                    //將對手棋子放置在斷點處，另外即使corner1/2為禁著點，add_stone一樣可以正確運作，且下方can_capture會return true，因此不會產生bug
                    if(empty_1)add_stone(-color, corner1);
                    if(empty_2)add_stone(-color, corner2);
                    
                    //若我方可直接吃掉對手棋子，則代表該斷點對黑棋來說是送死，因此該斷點通常無意義，cur與nx同樣可視為連接
                    bool still_connect = can_capture( color, corner1 ) or can_capture( color, corner2);

                    if(empty_1)undo();
                    if(empty_2)undo();
                    if(!still_connect)continue;
                }
                DFS_stack[stack_size++] = nx;
                is_visited[nx.first][nx.second] = 1;
            }
        }
    }
    return result_size;
}
//當color下在pt點可連接兩塊棋，則return true
//此函數定義過於廣泛，且不會包含斜角連接，因此不會調用。若需要較嚴格且包含斜角的函數，可調用is_escape
bool endgame_detect::is_connect(int color, pair<int,int> pt){
    int py = pt.first, px = pt.second;
    if(get_board(py,px) != 0)return 0;
    
    pair<int,int> connected_component[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int component_size = 0;
    for(int d=0;d<4;d++){
        int ny = py+dir[d][0], nx=px+dir[d][1];
        if( get_board(ny,nx) != color )continue;
        bool unconnect = 1;
        for(auto it: connected_component){
            if(it == pair<int,int>(ny,nx)){
                unconnect = 0;
                break;
            };
        }
        if(unconnect){
            if(component_size == 0){
                component_size = get_connected_part(color, {ny,nx}, connected_component );
            }
            else return 1;
        }
    }
    return 0;
};

//若color下在pt能夠切斷兩塊棋，則return true(由於判斷草率因此同樣棄用)
bool endgame_detect::is_cut(int color, pair<int,int> pt){
    if( !is_rational_movement(color, pt) or !is_connect(-color, pt))return 0;
    return 1;    
}

//當color所有棋子連成一塊時，return true
bool endgame_detect::is_all_connected(int color){
    int connected_part_size = 0;
    pair<int,int> connected_part[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    for(int py=1;py<=get_board_size();py++){
        for(int px=1;px<=get_board_size();px++){
            if( get_board(py,px)== color ){
                connected_part_size = get_connected_part( color, {py,px},connected_part );
                break;
            }
        }
        if(connected_part_size)break;
    }

    bool is_visited[get_board_size()+2][get_board_size()+2];
    memset(is_visited, 0, sizeof(is_visited));
    for(int i=0;i<connected_part_size;i++){
        pair<int,int> it = connected_part[i];
        is_visited[it.first][it.second] = 1;
    }
    
    for(int dy=1; dy<=get_board_size(); dy++){
        for(int dx=1; dx<=get_board_size(); dx++){
            if( is_visited[dy][dx] != 1 and get_board(dy,dx) == color ){
                return 0;
            }
        }
    }
    return 1;
}

//當pt所在連通塊對color來說為大眼時，return true
//使用遞迴判斷，耗時較長，若要加速可以嘗試設計新演算法
//為避免耗時，僅判斷大小<=4的大眼
//為避免重複計算眼位，僅當pt為整個大眼中Lexicographic order最小的點時才return true
//為避免與corner_eye_detect做出重複判斷，因此加入exclude_corner參數，當其為1時，只要包含corner eye就會直接return false
bool endgame_detect:: is_big_eye(int color, pair<int,int> pt, int depth, bool exclude_corner){
    if(depth<=0)return 0;
    if( (pt.first == 1 or pt.first == get_board_size()) and 
                (pt.second == 1 or pt.second == get_board_size()) )
        exclude_corner = 0;
    
    //處理pt非空的case
    if( get_board(pt) != 0){
        //若pt本身非空，則只有在pt為敵方子且為必死子時才有可能會是大眼
        if(get_board(pt) == -color and !can_escape(-color, pt)  ){
                pair<int,int> tmp_removed_stone[MAX_BOARD_SIZE*MAX_BOARD_SIZE];// = get_connected_component( pt )[0];
                int removed_cnt = find_connected_component( pt,  tmp_removed_stone);
                for(int i=0; i<removed_cnt; i++){
                    for(int d=0;d<4;d++){
                        pair<int,int> neighbor( tmp_removed_stone[i].first+dir[d][0],\
                                                        tmp_removed_stone[i].second + dir[d][1] );
                        if(out_of_edge(neighbor))
                            continue;
                        if( (neighbor.first %get_board_size()<=1) and (neighbor.second %get_board_size())<=1 ){
                            if(!corner_eye_shape_detect(color, neighbor)){
                                exclude_corner = 0;
                            }
                        }
                    }
                }
                //移除必死子後再重新偵測是否為大眼
                remove_stones(tmp_removed_stone, removed_cnt);
                bool is_eye = is_big_eye(color, pt, depth, exclude_corner);//is_big_eye( color, get_minimal( component_after_remove ) );

                for(int i=0;i<removed_cnt;i++){
                    add_stone(-color, tmp_removed_stone[i], false);
                }

                if(is_eye)return 1;
        }
        return 0;
    }
    //此後假設pt為空
    
    
    if( is_an_eye( color, pt ) ){
        return 1;
    }

    //interior: 大眼內部
    //external: 大眼邊緣
    pair<int,int> interior[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    pair<int,int> external[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int interior_size = find_connected_component(pt, interior);
    int external_size = find_connected_component_edge(0, interior,interior_size, external);
    // int external_size = find_connected_component_edge(pt, external);
        // cout<<"HAHA\n";

    //判斷是否pt為整個區塊中字典序最小者，以及
    //判斷是否內含corner eye
    for(int i_int = 0; i_int<interior_size; i_int++){
        pair<int,int> adj_pt = interior[i_int];
        if( adj_pt< pt )
            return 0;
        if( (adj_pt.first == 1 or adj_pt.first == get_board_size()) and
                     (adj_pt.second == 1 or adj_pt.second == get_board_size()) ){
            // cout<<"visit adj corner/"<<exclude_corner<<endl;
            if(corner_eye_shape_detect(color, adj_pt) and exclude_corner){
                // cout<<"return 0 by corner\n";
               return 0;
            }
        }
    }

    //尋訪大眼邊界，若有對手棋子則檢測是否為必死子，
        //若不是則return false
        //若是則移除該必死子，重跑大眼判斷
    for(int i_ext=0; i_ext<external_size; i_ext++){
        pair<int,int> e = external[i_ext];
        if( get_board(e)!= color ){
            if( get_board(e) != -color ){
                cout<<"ERROR occur at is_big_eye: ["<<color<<"] / "<<e<<endl;
                print_board();
                cout<<"calling at "<<pt<<","<<endl;
                system("pause");
                return 0;
            }
            // cout<<e<<":\n";
            if( !can_escape(-color, e) ){
                pair<int,int> tmp_removed_stone[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
                int removed_cnt = find_connected_component(e, tmp_removed_stone);
                remove_stones(tmp_removed_stone, removed_cnt);
                bool is_eye;
                is_eye = is_big_eye( color, pt, depth, 0 );

                for(int i=0;i<removed_cnt;i++){
                    add_stone(-color, tmp_removed_stone[i], false);
                }
                if(is_eye)return 1;
            }
            return 0;
        }
    }
    //以下假設pt所在大眼內部皆空，且外部皆與color同色

    //去除過大大眼
    int maximal_size = 4;
    if(interior_size>maximal_size or interior_size <= 1)return 0;

    //找出大眼周邊區塊
    bool adjacent[MAX_BOARD_SIZE+1][MAX_BOARD_SIZE+1];
    for(int i=0;i<=MAX_BOARD_SIZE;i++){
        for(int j=0;j<MAX_BOARD_SIZE;j++){
            adjacent[i][j] = 0;
        }
    }
    for(int i=0;i<interior_size;i++){
        for(int d=0;d<4;d++){
            if(out_of_edge(interior[i].first+dir[d][0], interior[i].second+dir[d][1]))
                continue;
            adjacent[ interior[i].first+dir[d][0] ][ interior[i].second+dir[d][1] ] = 1;
        }
    }

    //對手嘗試在大眼周邊落子，看是否能破掉大眼
    for(int py=1;py<=get_board_size();py++){
    for(int px=1;px<=get_board_size();px++){
        pair<int,int> pt_opponent(py,px);
        if(!adjacent[py][px])
            continue;

        if( is_legal_movement( -color, pt_opponent ) ){
            add_stone( -color, pt_opponent );

            bool break_the_eye = 1;
            

            //color嘗試在大眼周邊落子防禦，看受對手攻擊後是否還能在大眼內部做出眼
            for(int py2=1;py2<=get_board_size();py2++){
            for(int px2=1;px2<=get_board_size();px2++){
                pair<int,int> pt_self(py2,px2);
                
                if( !is_adjacent( pt_self, interior, interior_size ) )
                    continue;

                if( is_legal_movement( color, pt_self ) ){
                    add_stone( color, pt_self);
                    for(int ip=0;ip<interior_size;ip++){
                        if( is_big_eye( color, interior[ip], depth-1, 0) ){
                            break_the_eye = 0;
                            break;
                        }
                    }
                    undo();
                }
            }
            }
            undo();

            if(break_the_eye)return 0;
        }
    }
    }
    //執行至此代表對手破眼失敗，也就是pt所在位置為大眼，return true
    return 1;
}

//判斷是否有眼殺瞎

bool endgame_detect::capture_with_eyes(int color, bool show_detail, bool include_2_eyes , pair<int,int> root){
    //若未給定root則遍歷所有可能的root
    if(root.first == 0 and root.second == 0){
        bool is_captured = false;
        for(int py=1;!is_captured and py<=get_board_size();py++){
            for(int px=1;!is_captured and px<=get_board_size();px++){
                if(get_board(py,px) != color)
                    continue;
                is_captured |= capture_with_eyes( color, show_detail, include_2_eyes, {py,px} );
            }
        }
        return is_captured;
    }

    if(show_detail)cout<<"start:"<<root<<endl;
    if(show_detail)print_board();


    //求出root所在弱連通塊
    int eyes_cnt = 0;
    pair<int,int> connected_part[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int connected_part_size = get_connected_part( color, root, connected_part );

    if(show_detail)cout<<"connected part size: "<<connected_part_size<<endl;

    //計算connected_part中真眼的數量
    for(int py=1;py<=get_board_size();py++){
        for(int px=1;px<=get_board_size();px++){
            if(is_adjacent( {py,px}, connected_part, connected_part_size) and is_an_eye(color, {py,px})){
                eyes_cnt++;
                if(show_detail)cout<<"there's an eye at ("<<py<<","<<px<<")"<<endl;
            }
            else if( is_big_eye( color, {py,px} ) and is_adjacent({py,px}, connected_part, connected_part_size)){
                    eyes_cnt++;
                    if(show_detail)cout<<"there's a big eye at ("<<py<<","<<px<<")"<<endl;
            }
            else if(valid_big_eye(color,{py,px}, connected_part, connected_part_size)){
                eyes_cnt++;
                if(show_detail)cout<<"there's a adjacent big eye at ("<<py<<","<<px<<")"<<endl;
            }
            else{
                if(corner_eye_shape_detect(color, {py, px})){
                    eyes_cnt++;
                    if(show_detail)cout<<"there's an corner eye at ("<<py<<","<<px<<")"<<endl;
                }
            }
        }
    }

    //若無眼則不可能達成有眼殺瞎，return false
    if(eyes_cnt == 0){
        if(show_detail)cout<<"no eye\n";
        return 0;
    };

    //記錄所有敵方可能領地
    bool possible_opponent_field[get_board_size()+2][get_board_size()+2];
    // memset(possible_opponent_field, 0, sizeof(possible_opponent_field));
    
    for(int py=0;py<=get_board_size();py++){
        for(int px=0;px<=get_board_size();px++){
            possible_opponent_field[py][px] = 1;
        }
    }

    //將connected_part及其周遭一格排除在敵方領地外
    for(int i=0;i<connected_part_size;i++){
        possible_opponent_field[connected_part[i].first][connected_part[i].second] = 0;
        for(int d=0;d<4;d++){
            int nx_py = connected_part[i].first+dir[d][0];
            int nx_px = connected_part[i].second+dir[d][1];
            if(!out_of_edge(nx_py,nx_px))
                possible_opponent_field[nx_py][nx_px] = 0;
        }
    }

    //若某塊棋可被敵方吃死或我方未佔領，則將其也視作敵方領地
    for(int py=1;py<=get_board_size();py++){
        for(int px=1;px<=get_board_size();px++){
            if( get_board(py,px) != color and !is_occupied(color, {py,px})){
                possible_opponent_field[py][px] = 1;
            }
            if(get_board(py,px) == color and can_capture(-color, {py,px}))
                possible_opponent_field[py][px] = 1;
        }
    }

    if(show_detail){
        cout<<"region:---------------------------------"<<endl;
        for(int py=1;py<=4;py++){
            for(int px=1;px<=4;px++){
                cout<< (possible_opponent_field[py][px] ? 'X':'O')<<"   ";
            }
            cout<<endl;
        }
    }

    //紀錄敵方可能眼位
    pair<int,int> opponent_possible_eyes[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int opponent_possible_eyes_size = 0;

    for(int py=1;py<=get_board_size();py++){
        for(int px=1;px<=get_board_size();px++){
            //以下確認敵方是否有機會在(py,px)做眼
            if(get_board(py,px) == -color or possible_opponent_field[py][px] == 0)
                continue;
            
            bool flag = 1;
            
            //若相鄰四格內任何一格被黑棋佔領，則該點不可能做出眼
            for(int d=0;d<4 and flag;d++){
                if( !out_of_edge({py+dir[d][0], px+dir[d][1]}) and possible_opponent_field[ py+dir[d][0] ][ px+dir[d][1] ] == 0 )
                    flag = 0;
            }

            //計算鄰近8個方位被黑子佔據的數量，若不多則代表(py,px)有機會做出眼
            int occupied_corner = 0;
            for(int dy = -1; dy<=1; dy+=2){
                for(int dx=-1; dx<=1;dx+=2){
                    if((dy or dx) and !out_of_edge({py+dy,px+dx}) and possible_opponent_field[ py+dy ][px+dx] == 0 )
                        occupied_corner += 1;
                }
            }
            
            flag &= ( at_edge({py,px}) and occupied_corner ==0 ) or ( !at_edge({py,px}) and occupied_corner<2);

            if(flag){
                opponent_possible_eyes[opponent_possible_eyes_size++] = {py,px};
                if(show_detail)cout<<"there's a white eye at ("<<py<<","<<px<<")"<<endl;
            }
        }
    }

    int opponent_eyes_cnt = (opponent_possible_eyes_size>0);
    //檢查是否有不相鄰的兩個眼位(若相鄰只能構成一個大眼)
    if( opponent_possible_eyes_size >1){
        for(int i=0;i<opponent_possible_eyes_size;i++){
            for(int j=0;j<opponent_possible_eyes_size;j++){
                pair<int,int> pi = opponent_possible_eyes[i], pj = opponent_possible_eyes[j];
                if( abs( pi.first-pj.first )+abs(pi.second-pj.second) >1 ){
                    opponent_eyes_cnt = 2;
                }
            }
        }
    }
    if(show_detail)cout<<"black eyes:"<< eyes_cnt<<"/"<<"white eyes:"<<opponent_eyes_cnt<<endl;

    //兩眼殺一眼判斷
    if((opponent_eyes_cnt==1 and eyes_cnt>=2) and include_2_eyes)
        return 1;
    //一般有眼殺瞎判斷
    return opponent_eyes_cnt == 0;
}

//觀察pt所在大眼是否與block相鄰
bool endgame_detect::valid_big_eye(int color, pair<int,int> pt, pair<int,int> block[], int block_size, int depth, bool exclude_corner){
    if(depth<=0)return 0;
    if( (pt.first == 1 or pt.first == get_board_size()) and 
                (pt.second == 1 or pt.second == get_board_size()) )
        exclude_corner = 0;
    // cout<<pt<<":"<<pt<<" corner:"<<exclude_corner<<endl;
    if( get_board(pt) != 0){
        if(get_board(pt) == -color and !can_escape(-color, pt)  ){
                pair<int,int> tmp_removed_stone[MAX_BOARD_SIZE*MAX_BOARD_SIZE];// = get_connected_component( pt )[0];
                int removed_cnt = find_connected_component( pt,  tmp_removed_stone);
                for(int i=0; i<removed_cnt; i++){
                    for(int d=0;d<4;d++){
                        pair<int,int> neighbor( tmp_removed_stone[i].first+dir[d][0],\
                                                        tmp_removed_stone[i].second + dir[d][1] );
                        if(out_of_edge(neighbor))
                            continue;
                        if( (neighbor.first %get_board_size()<=1) and (neighbor.second %get_board_size())<=1 ){
                            if(!corner_eye_shape_detect(color, neighbor)){
                                exclude_corner = 0;
                            }
                        }
                    }
                }
                remove_connected_component( pt );
                bool is_eye = is_big_eye(color, pt, depth, exclude_corner);//is_big_eye( color, get_minimal( component_after_remove ) );

                for(int i=0;i<removed_cnt;i++){
                    add_stone(-color, tmp_removed_stone[i], false);
                }

                if(is_eye)return 1;
        }
        return 0;
    }
    if( is_an_eye( color, pt ) ){
        return 1;
    }

    pair<int,int> interior[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    pair<int,int> external[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int interior_size = find_connected_component(pt, interior);
    int external_size = find_connected_component_edge(pt, external);
    
    bool is_valid = false;
    for(int i_int = 0; i_int<interior_size; i_int++){
        pair<int,int> adj_pt = interior[i_int];
        if( adj_pt< pt )
            return 0;
        if( (adj_pt.first == 1 or adj_pt.first == get_board_size()) and
                     (adj_pt.second == 1 or adj_pt.second == get_board_size()) ){
            // cout<<"visit adj corner/"<<exclude_corner<<endl;
            if(corner_eye_shape_detect(color, adj_pt) and exclude_corner){
                // cout<<"return 0\n";
               return 0;
            }
        }
        if(is_adjacent(adj_pt, block, block_size))
            is_valid = true;
    }
    if(!is_valid)return 0;
    // cout<<"--"<<depth<<endl;
    for(int i_ext=0; i_ext<external_size; i_ext++){
        pair<int,int> e = external[i_ext];
        if( get_board(e)!= color ){
            if( get_board(e) != -color ){
                cout<<"ERROR occur at is_big_eye: ["<<color<<"] / "<<e<<endl;
                print_board();
                return 0;
            }
            // cout<<e<<":\n";
            if( !can_escape(-color, e) ){
                pair<int,int> tmp_removed_stone[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
                int removed_cnt = find_connected_component(e, tmp_removed_stone);
                remove_connected_component(e);

                for(int i=0;i<removed_cnt;i++){
                    pair<int,int> rm_pt = tmp_removed_stone[i];
                    if( rm_pt< pt ){
                        for(int j=0;j<removed_cnt;j++){
                            add_stone(-color, tmp_removed_stone[j], false);
                        }
                        return 0;
                    }
                }
                bool is_eye;
                is_eye = is_big_eye( color, pt, depth, 0 );

                for(int i=0;i<removed_cnt;i++){
                    add_stone(-color, tmp_removed_stone[i], false);
                }
                if(is_eye)return 1;
            }
            return 0;
        }
    }


    int maximal_size = 4;
    if(interior_size>maximal_size or interior_size <= 1)return 0;

    for(int py=1;py<=get_board_size();py++){
    for(int px=1;px<=get_board_size();px++){
       pair<int,int> pt_opponent(py,px);
        if(!is_adjacent( pt_opponent, interior, interior_size ))
            continue;
        
        if( is_legal_movement( -color, pt_opponent ) ){
            add_stone( -color, pt_opponent );

            bool break_the_eye = 1;
            
            // for(auto pt_self: interior){
            for(int py2=1;py2<=get_board_size();py2++){
            for(int px2=1;px2<=get_board_size();px2++){
                pair<int,int> pt_self(py2,px2);
                
                if( !is_adjacent( pt_self, interior, interior_size ) )
                    continue;

                if( is_legal_movement( color, pt_self ) ){
                    add_stone( color, pt_self);
                    // cout<<"at layer "<<depth<<", search "<<pt_opponent<<"/"<<pt_self<<endl;
                    // print_board();
                    // system("pause");
                    
                    for(auto possible_eye: interior){
                        if( is_big_eye( color, possible_eye, depth-1, 0) ){
                            break_the_eye = 0;
                            break;
                        }
                    }
                    undo();
                }
            }
            }
            undo();

            if(break_the_eye)return 0;
        }
    }
    }
    return 1;
}