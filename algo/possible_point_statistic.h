#pragma once

#include"D_detect.h"
using namespace std;
class point_calculator{
    public:
        D_classifier& game;
        int dir[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        point_calculator(D_classifier& game_test):
            game(game_test)
        {};


        bool alive_with_eyes(int color, bool show_detail = false , pair<int,int> root = {0,0}){
            if(root.first == 0 and root.second == 0){
                bool is_alive = false;
                for(int py=1;!is_alive and py<=game.get_board_size();py++){
                    for(int px=1;!is_alive and px<=game.get_board_size();px++){
                        if(game.get_board(py,px) != color)
                            continue;
                        is_alive |= alive_with_eyes( color, show_detail, {py,px} );
                    }
                }
                return is_alive;
            }

            if(show_detail)cout<<"start:"<<root<<endl;
            if(show_detail)game.print_board();

            int eyes_cnt = 0;
            pair<int,int> connected_part[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
            int connected_part_size = game.get_connected_part( color, root, connected_part );

            for(int py=1;py<=game.get_board_size();py++){
                for(int px=1;px<=game.get_board_size();px++){
                    if(is_adjacent( {py,px}, connected_part, connected_part_size) and game.is_an_eye(color, {py,px})){
                        eyes_cnt++;
                        if(show_detail)cout<<"there's an eye at ("<<py<<","<<px<<")"<<endl;
                    }
                    else if( game.is_big_eye( color, {py,px} ) and is_adjacent({py,px}, connected_part, connected_part_size)){
                            eyes_cnt++;
                            if(show_detail)cout<<"there's a big eye at ("<<py<<","<<px<<")"<<endl;
                    }
                    else if(game.valid_big_eye(color,{py,px}, connected_part, connected_part_size)){
                        eyes_cnt++;
                        if(show_detail)cout<<"there's a adjacent big eye at ("<<py<<","<<px<<")"<<endl;
                    }
                    else{
                        if(game.corner_eye_shape_detect(color, {py, px})){
                            eyes_cnt++;
                            if(show_detail)cout<<"there's an corner eye at ("<<py<<","<<px<<")"<<endl;
                        }
                    }
                }
            }

            if(eyes_cnt == 0){
                if(show_detail)cout<<"no eye\n";
                return 0;
            };

            bool possible_opponent_field[game.get_board_size()+2][game.get_board_size()+2];
            memset(possible_opponent_field, 0, sizeof(possible_opponent_field));
            
            for(int py=1;py<=game.get_board_size();py++){
                for(int px=1;px<=game.get_board_size();px++){
                    if( !is_adjacent({py,px}, connected_part, connected_part_size) or ( game.get_board(py,px) != color and !game.is_occupied(color, {py,px}) ) ){
                        possible_opponent_field[py][px] = 1;
                    }
                    if(game.get_board(py,px) == color and game.can_capture(-color, {py,px}))
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

            for(int py=1;py<=game.get_board_size();py++){
                for(int px=1;px<=game.get_board_size();px++){

                    if(game.get_board(py,px) == -color or possible_opponent_field[py][px] == 0)
                        continue;
                    
                    bool flag = 1;
                    
                    for(int d=0;d<4 and flag;d++){
                        if( !game.out_of_edge({py+dir[d][0], px+dir[d][1]}) and possible_opponent_field[ py+dir[d][0] ][ px+dir[d][1] ] == 0 )
                            flag = 0;
                    }

                    int occupied_corner = 0;
                    for(int dy=-1; dy<=1; dy+=2){
                        for(int dx=-1; dx<=1;dx+=2){
                            if(!game.out_of_edge({py+dy,px+dx}) and possible_opponent_field[ py+dy ][px+dx] == 0 )
                                occupied_corner += 1;
                        } 
                    }
                    
                    flag &= ( game.at_edge({py,px}) and occupied_corner ==0 ) or ( !game.at_edge({py,px}) and occupied_corner<2);

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
            return (opponent_eyes_cnt<=1 and eyes_cnt>=1) or eyes_cnt >=2;
        }

        inline bool alive_within_one_move(int color){
            bool can_alive = 0;
            for(int py=1; py<=game.get_board_size() and !can_alive; py++){
                for(int px=1; px<=game.get_board_size() and !can_alive; px++){
                    if(game.is_legal_movement(color, {py,px}) and game.capture_stone_position_check(color, {py,px})){
                        bool atari = game.is_atari(color, {py,px});
                        bool successfully_atari = false;
                        game.add_stone(color, {py,px});
                        can_alive |= alive_with_eyes(color);
                        if(atari){
                            pair<int,int> atari_stone(py,px);
                            for(int d=0;d<4;d++){
                                pair<int,int> nx_pt(atari_stone.first+dir[d][0],\
                                                            atari_stone.second+dir[d][1]);
                                if(game.get_board(nx_pt)!= -color)
                                    continue;
                                if(game.calculate_liberty(nx_pt) == 1){
                                    atari_stone = game.get_liberty_position(nx_pt);
                                    successfully_atari |= game.is_dead(-color, nx_pt);
                                    break;
                                }
                            }
                            if((atari_stone.first!=py or atari_stone.second != px) and successfully_atari){
                                game.add_stone(color, atari_stone);
                                can_alive |= alive_with_eyes(color);
                                game.undo();
                            }
                            // if(can_alive)cout<<"alive by put stone at "<<py<<","<<px<<endl;
                        }
                        
                        game.undo();
                    }
                }
            }
            return can_alive;
        }

        bool capture_huge_stone(int color, int number= 6){
            for(int py=1; py<=game.get_board_size(); py++){
                for(int px=1; px<=game.get_board_size(); px++){
                    if( game.get_board(py,px) != -color )continue;
                    if(game.calculate_liberty({py,px})){
                        if(game.get_component_size({py,px}) >= number)
                            return 1;
                    }
                }
            }
            return 0;
        };

        int calculate_possible_point(int color,int depth = 1, bool show_detail = 0){
            if(depth <= 0 or game.kill_within_one_move(color))return 1;// or win_within_one_move(-color))return 1;
            int cnt = 0;
            for(int py=1; py<=game.get_board_size(); py++){
                for(int px=1; px<=game.get_board_size(); px++){
                    if(game.is_legal_movement(color,{py,px})){
                        if(!game.capture_stone_position_check(color, {py,px}))continue;
                        game.add_stone(color,{py,px});
                        if(!game.kill_with_depth(-color,1) and !game.defend_by_seki(-color)\
                                and !alive_within_one_move(-color) and !capture_huge_stone(-color)){
                            if(show_detail)cout<<"search ("<<py<<","<<px<<")..."<<endl;
                            cnt += calculate_possible_point(-color, depth-1);
                        }
                        game.undo();
                    }
                    if(cnt>=50)return cnt;
                }
            }
            return cnt;
        }
};