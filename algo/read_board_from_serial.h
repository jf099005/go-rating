#pragma once

#include<iostream>

#ifndef GO_board_H
#define GO_board_H
#include"Go_board.h"
#endif

#include<fstream>

#include<filesystem>

#include<sstream>

using namespace std;
namespace fs = std::filesystem;

using namespace std;

class board_data_loader{
    public:
        GO_board &game;

        board_data_loader(GO_board & agent, string file_path = "", string difficulty = "D", bool show_detail = false):
            game(agent)
        {
            if(file_path.length()){
                load_file(file_path,difficulty, show_detail);
            };
        };

        const int max_problem_size = 1000;
        string source_txt_path;
        int problem_serials[1000];
        int problem_url[1000];
        string problem_level[1000];
        int problem_size = 0 ,current_problem_idx = -1;

        void load_file( string file_path , string difficulty, bool show_detail = false){
            problem_size = 0;

            source_txt_path = file_path;
            ifstream ifs(file_path, ios::in);
            string s = "";
            while(getline( ifs, s )){
                stringstream ss;
                ss << s;
                string current_difficulty;
                ss >>  current_difficulty;
                if(current_difficulty != difficulty){
                    continue;
                }
                string board_serial;
                ss >> problem_serials[ problem_size ];
                int is_black_first;
                ss >> is_black_first;
                if(is_black_first != 1)
                    problem_serials[ problem_size ] = reverse_stone( problem_serials[problem_size] );
                ss >> problem_url[problem_size];
                ss >> problem_level[problem_size];
                problem_size ++ ;
            }
            ifs.close();
            if(show_detail){
                cout<<"difficulty "<<difficulty<<", loading "<<problem_size<<" problems\n";
            }
            current_problem_idx = 0;
            load_board_from_serial( problem_serials[0] );
        };

        int reverse_stone( int serial ){
            int new_serial = 0;
            for(int i=0, pw=1;i<16;i++, pw*=3){
                int stone_color = serial%3;
                // cout<<"("<<py<<","<<px<<"): "<<board[py][px]<<endl;
                serial /= 3;
                if(stone_color)stone_color = 3 - stone_color;
                new_serial += stone_color * pw;
            }
            return new_serial;
        }

        void next_problem(bool show_problem_id = false){
            current_problem_idx = (current_problem_idx + 1)%max_problem_size;
            load_board_from_serial( problem_serials[current_problem_idx] );
            if(show_problem_id)cout<<"loading problem: "<<current_problem_idx<<endl;
        };

        void previous_problem(bool show_problem_id = false){
            current_problem_idx = (current_problem_idx + max_problem_size - 1)%max_problem_size;
            load_board_from_serial( problem_serials[current_problem_idx] );
            if(show_problem_id)cout<<"loading problem: "<<current_problem_idx<<endl;
        }

        void load_board_from_idx(int idx){
            current_problem_idx = idx;
            load_board_from_serial( problem_serials[ idx ] );
        }

        void load_board_from_serial(int board_serial){
            vector< vector<int> > board(4, vector<int>(4));
            // cout<<"loading "<<board_serial<<endl;
            for(int i=0;i<16;i++){
                int py = i/4, px = i%4;
                board[py][px] = board_serial%3;
                // cout<<"("<<py<<","<<px<<"): "<<board[py][px]<<endl;
                board_serial /= 3;
                board[py][px] = (board[py][px] == 2? -1: board[py][px]);
            }
            game.set_board( board );

            return;
        };
        int get_current_url(){
            return problem_url[current_problem_idx];
        }
        string get_current_level(){
            return problem_level[current_problem_idx];
        }

        int get_current_kyu(){
            int kyu = 0;
            string level_str = get_current_level();
            for(int i=0; i<level_str.length(); i++){
                if(level_str[i]<'0' or level_str[i] >'9'){
                    if(level_str[i] != 'K')return 0;
                    break;
                }
                else kyu = kyu*10+(level_str[i] - '0');
            }
            return kyu;
        }

        int get_current_dan(){
            int kyu = 0;
            string level_str = get_current_level();
            for(int i=0; i<level_str.length(); i++){
                if(level_str[i]<'0' or level_str[i] >'9'){
                    if(level_str[i] != 'D')return 0;
                    break;
                }
                else kyu = kyu*10+(level_str[i] - '0');
            }
            return kyu;
        }
};