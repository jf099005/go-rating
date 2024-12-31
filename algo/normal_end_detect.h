#pragma once
#include"D_detect.h"
#include"memorize_search.h"
class normal_end_detector:public D_classifier{
    public:
        normal_end_detector(int n):
            D_classifier(n)
        {};

        inline int remove_dead_stones(int dead_color, pair<int,int>* dead_stone_record){
            int dead_stone_cnt = 0;
            for(int py=1; py<=get_board_size(); py++){
                for(int px=1; px<=get_board_size(); px++){
                    if( get_board(py,px) == dead_color and is_dead(dead_color, {py,px}) ){
                        int removed_component_size = \
                                find_connected_component({py,px}, dead_stone_record+dead_stone_cnt);
                        remove_stones(dead_stone_record+dead_stone_cnt, removed_component_size);
                        dead_stone_cnt += removed_component_size;
                    }
                }
            }
            return dead_stone_cnt;
        };

        int alive_with_eyes_cnt = 0;

        search_solution_record alive_shape_recorder;
        inline int alive_with_eyes(int color, bool show_detail = false , pair<int,int> root = {0,0}){
            int query_solution = alive_shape_recorder.query_alive_state(*this, color, root);
            if(query_solution != query_fail){
                return query_solution;
            }

            alive_with_eyes_cnt++;
            if(root.first == 0 and root.second == 0){
                bool is_alive = false;
                for(int py=1;!is_alive and py<=get_board_size();py++){
                    for(int px=1;!is_alive and px<=get_board_size();px++){
                        if(get_board(py,px) != color)
                            continue;
                        is_alive |= alive_with_eyes( color, show_detail, {py,px} );
                    }
                }
                return is_alive;
            }

            if(show_detail)cout<<"start:"<<root<<endl;
            if(show_detail)print_board();

            int eyes_cnt = 0;
            pair<int,int> connected_part[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int connected_part_size = get_connected_part( color, root, connected_part );

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

            if(eyes_cnt == 0){
                if(show_detail)cout<<"no eye\n";
                alive_shape_recorder.record_alive_state(*this, color, root, 0);
                return 0;
            };

            bool possible_opponent_field[get_board_size()+2][get_board_size()+2];
            memset(possible_opponent_field, 0, sizeof(possible_opponent_field));
            
            for(int py=1;py<=get_board_size();py++){
                for(int px=1;px<=get_board_size();px++){
                    if( !is_adjacent({py,px}, connected_part, connected_part_size)\
                                 or ( get_board(py,px) != color and !is_occupied(color, {py,px}) ) ){
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

            pair<int,int> opponent_possible_eyes[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int opponent_possible_eyes_size = 0;

            for(int py=1;py<=get_board_size();py++){
                for(int px=1;px<=get_board_size();px++){

                    if(get_board(py,px) == -color or possible_opponent_field[py][px] == 0)
                        continue;
                    
                    bool flag = 1;
                    
                    for(int d=0;d<4 and flag;d++){
                        if( !out_of_edge({py+dir[d][0], px+dir[d][1]}) and possible_opponent_field[ py+dir[d][0] ][ px+dir[d][1] ] == 0 )
                            flag = 0;
                    }

                    int occupied_corner = 0;
                    for(int dy=-1; dy<=1; dy+=2){
                        for(int dx=-1; dx<=1;dx+=2){
                            if(!out_of_edge({py+dy,px+dx}) and possible_opponent_field[ py+dy ][px+dx] == 0 )
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
            if( opponent_possible_eyes_size >1 ){
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
            bool solution =  (opponent_eyes_cnt<=1 and eyes_cnt>=1) or eyes_cnt >=2;
            alive_shape_recorder.record_alive_state(*this, color, root, solution);
            return solution;
        }

        int alive_wov_cnt = 0;
        inline bool alive_within_one_move(int color, pair<int,int> root = {0,0}){
            alive_wov_cnt++;
            bool can_alive = alive_with_eyes(color, false, root);
            for(int py=1; py<=get_board_size() and !can_alive; py++){
                for(int px=1; px<=get_board_size() and !can_alive; px++){
                    if(is_legal_movement(color, {py,px})){
                        // bool atari = is_atari(color, {py,px});
                        // bool successfully_atari = false;
                        add_stone(color, {py,px});
                        can_alive |= alive_with_eyes(color, 0, root);                
                        undo();
                    }
                }
            }
            return can_alive;
        }

        //return true if the connected-part which contain "root" is alive (have 2 eyes, or have equal eyes with opponent)
        int is_alive_cnt = 0;
        inline bool is_alive(int color, pair<int,int> root){
            is_alive_cnt++;
            if(get_board(root)!=color){
                cout<<"ERROR, calling is_alive with wrong color:"<<color<<"/"<<root<<endl;    
                return 0;
            }
            bool alive = alive_within_one_move(color);
            for(int py=1; py<=get_board_size() and alive; py++){
                for(int px=1; px<=get_board_size() and alive; px++){
                    if( is_legal_movement(-color, {py,px}) ){
                        add_stone(-color, {py,px});

                        alive &= alive_within_one_move(color, root);

                        undo();
                        if(!alive)return 0;
                    }
                }
            }
            return alive;
        }

        int end_cnt = 0;
        bool is_end(bool remove_stone = true){
            end_cnt++;
            pair<int,int> dead_black[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            pair<int,int> dead_white[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int black_size = 0, white_size=0;
            bool is_ended = 1;
            bool is_visited[MAX_BOARD_SIZE+1][MAX_BOARD_SIZE+1];
            memset(is_visited, 0, sizeof(is_visited));
            for(int py=1; py<=get_board_size() and is_ended; py++){
                for(int px=1; px<=get_board_size() and is_ended; px++){
                    if(get_board(py,px) != 0){
                        if(is_visited[py][px])
                            continue;
                        int color = get_board(py,px);
                        if( !is_dead(color, {py,px}) and !is_alive(color, {py,px}) )
                            is_ended = 0;//,cout<<"fail at pt"<<py<<","<<px<<endl;
                        else{
                            pair<int,int> alive_block[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
                            int alive_block_size = get_connected_part(color, {py,px}, alive_block);
                            for(int i=0;i<alive_block_size;i++){
                                is_visited[ alive_block[i].first ][ alive_block[i].second ] = 1;
                            }
                        }
                    }
                }
            }
            return is_ended;
        }
        
        const int true_end = 1;
        const int pseudo_end = 2;
        bool is_end_2(int color, bool remove_stone = true, bool check_seki = false){
            // if(is_seki()){
            //     return true_end;
            // }
            end_cnt++;

            pair<int,int> dead_black[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            pair<int,int> dead_white[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int dead_black_size = 0, dead_white_size=0;
            if(remove_stone){
                dead_black_size = remove_dead_stones(1, dead_black);
                dead_white_size = remove_dead_stones(-1, dead_white);
            }

            pair<int,int> black_component[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            pair<int,int> white_component[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int black_component_size=0, white_component_size=0;
            for(int py=1; py<=get_board_size(); py++){
                for(int px=1; px<=get_board_size(); px++){
                    if(get_board(py,px) == 1 and !black_component_size){
                        black_component_size = get_connected_part(1, {py,px}, black_component);
                    }
                    else if(get_board(py,px)==-1 and !white_component_size){
                        white_component_size = get_connected_part(-1, {py,px}, white_component);
                    }
                }
            }

            bool is_visited[MAX_BOARD_SIZE+2][MAX_BOARD_SIZE+2];
            memset(is_visited, 0, sizeof(is_visited));

            for(int i=0;i<black_component_size;i++)
                is_visited[ black_component[i].first ][ black_component[i].second ] = 1;
            for(int i=0;i<white_component_size;i++)
                is_visited[ white_component[i].first ][ white_component[i].second ] = 1;
            bool end = 1;
            for(int py=1; py<=get_board_size() and end; py++){
                for(int px=1; px<=get_board_size() and end; px++){
                    if(is_visited[py][px])continue;
                    if(get_board(py,px) != 0){
                        // cout<<"error occur at "<<py<<","<<px<<endl;
                        end = 0;
                    }
                    else{
                        bool inside_field = 1;
                        pair<int,int>edge[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
                        int edge_size = find_connected_component_edge({py,px}, edge);
                        for(int i=0;i<edge_size-1;i++)
                            inside_field &= get_board(edge[i]) == get_board(edge[i+1]);

                        if(inside_field)continue;

                        bool adj_b=0, adj_w=0;
                        for(int dy=-1; dy<=1;dy++){
                            for(int dx=-1;dx<=1;dx++){
                                if(!out_of_edge({py+dy, px+dx})){
                                    adj_b |= get_board(py+dy, px+dx) == 1;
                                    adj_w |= get_board(py+dy, px+dx) == -1;
                                }
                            }
                        }
                        
                        if(!adj_b or !adj_w)end = 0;
                        // ,cout<<"error occur at"<<py<<","<<px<<endl;
                    }
                }
            }
            if(remove_stone){
                // cout<<"return stones\n";
                for(int b=0;b<dead_black_size;b++){
                    // cout<<"add back "<<dead_black[b]<<endl;
                    add_stone( 1, dead_black[b], 0);
                }
                for(int w=0;w<dead_white_size;w++){
                    add_stone(-1, dead_white[w], 0);
                }
            }
            if(end){
                // cout<<"check\n";
                bool state_self = alive_with_eyes(color);
                bool state_opponent = alive_with_eyes(-color);
                if(state_self and state_opponent)
                    return true_end;
                else{
                    if(state_self and alive_within_one_move(-color)){
                        return pseudo_end;
                    }
                    else{
                        if(is_seki()){
                            return true_end;
                        }
                    }
                }
            }
            return 0;
        }

        inline int calculate_territory(int color, bool show_detail = 0){
            int territory = 0;
            bool is_visited[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
            memset(is_visited,0,sizeof(is_visited));
            for(int py=1; py<=get_board_size(); py++){
                for(int px=1; px<=get_board_size(); px++){
                    if(is_visited[py][px])continue;
                    if(get_board(py,px) == 0){
                        if(show_detail)cout<<"visit "<<py<<","<<px<<endl;
                        pair<int,int> interior[MAX_BOARD_SIZE*MAX_BOARD_SIZE],
                            edge[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
                        int interior_size = find_connected_component({py,px}, interior);
                        int edge_size = find_connected_component_edge({py,px}, edge);
                        bool is_territory = true;
                        for(int e=0;e<edge_size and is_territory;e++){
                            if(get_board(edge[e]) != color)
                                is_territory = 0;
                        }
                        if(!is_territory)continue;

                        //calculate the territory of the interior point
                        for(int i=0;i<interior_size;i++){
                            int occupied_corner = 0;
                            for(int dy=-1;dy<=1;dy+=2){
                                for(int dx=-1;dx<=1;dx+=2){
                                    if(!out_of_edge({interior[i].first+dy, interior[i].second+dx})){
                                        occupied_corner += get_board(interior[i].first+dy, interior[i].second+dx) == -color;
                                    }
                                }
                            }
                            territory += occupied_corner == 0 or (!at_edge(interior[i]) and occupied_corner <=1);
                        }
                        if(show_detail)cout<<"\t territory:"<<territory<<endl;
                        for(int i=0;i<interior_size;i++)
                            is_visited[ interior[i].first ][ interior[i].second ] = 1;
                        for(int e=0;e<edge_size;e++){
                            is_visited[ edge[e].first ][ edge[e].second ] = 1;
                        }
                    }
                }
            }
            return territory;
        }
        //only available in end-game state
        bool win_by_territory(int color=1, bool check_endgame = true, bool show_detail = false){
            if(check_endgame){
                if(!is_seki() and !is_end()){
                    if(show_detail)cout<<"the game did'n end yet\n";
                    return 0;
                }
            }
            pair<int,int> removed_black_stones[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int removed_black_size = remove_dead_stones( 1, removed_black_stones);
            pair<int,int> removed_white_stones[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int removed_white_size = remove_dead_stones(-1, removed_white_stones);
            
            int total_black_territory = calculate_territory(1) + removed_white_size + black_ate_stone;
            int total_white_territory = calculate_territory(-1)+ removed_black_size + white_ate_stone;

            for(int i=0;i<removed_black_size;i++){
                add_stone(1, removed_black_stones[i], 0);
            }
            for(int i=0;i<removed_white_size;i++){
                add_stone(-1, removed_white_stones[i], 0);
            }
            if(show_detail){
                cout<<"black territory:"<<total_black_territory<<endl;
                cout<<"white_territory:"<<total_white_territory<<endl;
            }
            if(color==1)return total_black_territory>total_white_territory;
            else return total_white_territory>total_black_territory;
        }

};