#pragma once
#include<random>
#include"Go_board.h"
//rand_max=32767
// const int record_size = 10007;
const int record_size = (1<<25)-1;
const int empty_sym = -1;
const int query_fail = -2;
const int lower_depth = -3;

class search_solution_record{
    public:
        bool* solution_record;
        int *depth_record;
        int *key_value_record;

        search_solution_record(int n = MAX_BOARD_SIZE){
            solution_record = (bool*)malloc(record_size*sizeof(bool));//new bool[record_size];
            depth_record = (int*)malloc(record_size*sizeof(int));//new int[record_size];
            key_value_record = (int*)malloc(record_size*sizeof(int));//new int[record_size];
            
            random_device rd;   
            default_random_engine gen = std::default_random_engine(rd());
            uniform_int_distribution<int> dist(0, record_size-1);

            for(int py=0;py<=n;py++){
                for(int px=0; px<=n; px++){
                    transposition_table_black[py][px] = dist(gen);
                    transposition_table_white[py][px] = dist(gen);
                    position_hash[py][px] = dist(gen);
                }
            }
            ko_hash = dist(gen);
            for(int i=0;i<record_size;i++){
                solution_record[i] = 0;
                depth_record[i] = 0;
                key_value_record[i] = empty_sym;
            }
            black_hash_value = dist(gen);
            white_hash_value = dist(gen);
            black_move_value = dist(gen);
            white_move_value = dist(gen);
            for(int i=0;i<MAX_BOARD_SIZE*MAX_BOARD_SIZE;i++){
                black_ate_hash[i] = dist(gen);
                white_ate_hash[i] = dist(gen);
            }
        }
        ~ search_solution_record(){
            delete solution_record;
            solution_record = nullptr;
            delete depth_record;
            depth_record = nullptr;
            delete key_value_record;
            key_value_record = nullptr;
        }
    int transposition_table_black[MAX_BOARD_SIZE+2][MAX_BOARD_SIZE+2];
    int transposition_table_white[MAX_BOARD_SIZE+2][MAX_BOARD_SIZE+2];
    int ko_hash;
    int position_hash[MAX_BOARD_SIZE+2][MAX_BOARD_SIZE+2];
    int black_hash_value, white_hash_value;
    int black_move_value, white_move_value;
    int black_ate_hash[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int white_ate_hash[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
    int record_cnt = 0;
    inline int board_to_hash(GO_board& game){
        int hash_value = 0;
        for(int py=1; py<=game.get_board_size(); py++){
            for(int px=1; px<=game.get_board_size(); px++){
                if(game.get_board(py,px) == 1)
                    hash_value ^= transposition_table_black[py][px];
                else if(game.get_board(py,px) == -1)
                    hash_value ^= transposition_table_white[py][px];
            }
        }
        if(game.is_ko){
            hash_value ^= ko_hash;
            hash_value ^= position_hash[game.ko_position.first][game.ko_position.second];
        }
        hash_value ^= black_ate_hash[game.black_ate_stone]^white_ate_hash[game.white_ate_stone];
        return hash_value;
    }

    int state_to_hash(GO_board& game, int color, int current_color){
        int hash_value = board_to_hash(game);
        if(color==1)hash_value ^= black_hash_value;
        else hash_value ^= white_hash_value;
        if(current_color) hash_value ^= black_move_value;
        else hash_value ^= white_move_value;
        return hash_value%record_size;
    }

    inline int get_new_hash(int old_hash_value){
        return old_hash_value*2%record_size;
    }

    int collision_occur = 0;
    int addicional_cost = 0;
    int max_chain_length = 0;

    bool record_solution(int hash_value, int state_serial, int depth, bool solution){
        hash_value %= record_size;
        if(hash_value<0 or hash_value >=record_size){
            cout<<"fatal error: illegal hash value:"<<hash_value<<endl;
            return 0;
        }
        int cnt=0;
        while(key_value_record[hash_value] != empty_sym and cnt<1000){
            hash_value = get_new_hash(hash_value);//(hash_value*hash_value)%record_size;
            cnt++;
            addicional_cost++;
        }
        if(cnt)collision_occur++;
        max_chain_length = max(max_chain_length, cnt);
        if(cnt>=1000){
            std::cout<<"ERROR: fail when recording solution\n";
            cout<<hash_value<<"/"<<get_new_hash(hash_value)<<endl;
            cout<<record_cnt<<endl;
            return 0;
        }
        if(key_value_record[hash_value]!=empty_sym){
            if(depth_record[hash_value] > depth){
                return 0;
            }
        }
        solution_record[hash_value] = solution;
        depth_record[hash_value] = depth;
        key_value_record[hash_value] = state_serial;
        record_cnt++;
        return 1;
    }

    int query(int hash_value,int board_serial, int depth){
        hash_value %= record_size;
        int cnt=0;
        while( cnt<1000 and key_value_record[hash_value] != empty_sym and key_value_record[hash_value] != board_serial ){
            hash_value = get_new_hash(hash_value);
            cnt++;
        }
        if(cnt>=1000){
            std::cout<<"ERROR: fail when reading solution\n";
            cout<<hash_value<<"/"<<get_new_hash(hash_value)<<endl;
            cout<<record_cnt<<endl;
            return query_fail;
        }
        if( key_value_record[hash_value] == board_serial){
            if( depth_record[hash_value] >= depth or solution_record[hash_value] == 1)
            // if(solution_record[hash_value] == 1)
                return solution_record[hash_value];
            return lower_depth;
        }
        else return query_fail;

    }

    bool record_minimax_solution(GO_board& game, int color, int current_color, int depth, bool solution){
        int hash_value = state_to_hash(game, color, current_color);
        return record_solution(hash_value, game.to_serial(color, current_color), depth, solution);
    }
    int query_minimax_solution(GO_board& game, int color, int current_color, int depth){
        int hash_value = state_to_hash(game,color, current_color);
        int board_serial = game.to_serial(color, current_color);
        return query(hash_value, board_serial, depth);
    }

    bool record_alive_state(GO_board& game, int color, pair<int,int> pt, bool solution){
        int hash_value = board_to_hash((game));
        hash_value ^= (color==1?black_hash_value:white_hash_value);
        if(pt.first!=0 and pt.second !=0)
            hash_value ^= position_hash[pt.first][pt.second];
        return record_solution(hash_value, game.to_serial(color, color), 100, solution);
    }

    int query_alive_state(GO_board& game, int color, pair<int,int> pt){
        int hash_value = board_to_hash(game);
        hash_value ^= (color==1?black_hash_value:white_hash_value);
        if(pt.first!=0 and pt.second !=0)
            hash_value ^= position_hash[pt.first][pt.second];
        return query(hash_value, game.to_serial(color,color), 100);
    }

    void reset(){
        for(int i=0;i<record_size;i++){
            solution_record[i] = empty_sym;
            depth_record[i] = empty_sym;
            key_value_record[i] = empty_sym;
        }
        record_cnt = 0;
    }
    int size(){
        return record_cnt;
    }

};