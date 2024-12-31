#include"./algo/win_with_depth.h"
#include"./algo/read_board_from_serial.h"
#include"./algo/possible_point_statistic.h"

#include<cstdlib>

#include<io.h>

#include <iostream>
#include <fstream>
//using namespace std;

int main(int argc, char* argv[]){
    minimax_detector game(4);

    board_data_loader loader( game );
    point_calculator pt_calculate(game);

    string file_path = "../../test_data/data/test.txt";
    string record_path = "./record/record.txt";
    
    int record_idx = 0;
    while(_access(record_path.c_str(), F_OK) != -1 ){
            record_path = "./record/record"+ to_string(record_idx) + ".txt";
            record_idx++;
    };

    if(argc >=2){
        string file_name( argv[1] );
        record_path = "./record/"+file_name +".txt";
    }

    cout<<"record path: "<<record_path<<endl;
    
    int total = 0, correct = 0;
    int depth_limit = 6;
    int Dres_cnt = 0, Dfail_cnt = 0;
    bool f = 0;
    
    // game.enable_restriction();
    // game.set_capture_knowledge(1,1,0,0,1,1,1,1);
    game.free_restriction();

    ofstream record(record_path, ios::out);
    int start_time = time(0);

    loader.load_file( file_path , "D" );

    int correct_level_statistic[30];
    int fail_level_statistic[30];
    memset(correct_level_statistic,0,sizeof(correct_level_statistic));
    memset(fail_level_statistic,0,sizeof(fail_level_statistic));
    
    string difficulty[5] = {"D","C", "B", "A", "S"};

    for(int i0=1;i0<5;i0++){
        cout<<difficulty[i0]<<":"<<endl;
        loader.load_file(file_path, difficulty[i0]);

        int res_cnt = 0, fail_cnt = 0;
        bool f = 0;
        bool correct_ans = (difficulty[i0] == "C");
        for(int i=0;i<loader.problem_size;i++){
            cout<<"\r"<<i;
            game.alive_shape_recorder.reset();
            game.recorder.reset();
            bool res = game.win_with_depth(1, 6, 0, 0, 1, 0) or game.defend_by_seki(1) or game.win_within_one_move(1);
            if(!res){
                int start_time = time(0);
                game.recorder.reset();
                res = game.win_with_depth(1, 6, 0,0, 1, 1);
                cout<<"\rproblem "<<difficulty[i0]<<"-"<<i<<" spend "<<time(0)-start_time<<"seconds\n";
            }
            
            res_cnt += res;
            fail_cnt += !res;
            if(res!=correct_ans){
                cout<<"\rERROR: "<<difficulty[i0] <<"-" << i <<", url:"<< loader.get_current_url() <<" "<<\
                      loader.get_current_level()<<" "<<pt_calculate.calculate_possible_point(1)<<endl; 
                game.print_board();
                record <<difficulty[i0] <<" " << i <<" , url: "<< loader.get_current_url()\
                         <<" "<<loader.get_current_level()<<" "<<pt_calculate.calculate_possible_point(1)<<endl;  
            };
            if(res)correct_level_statistic[loader.get_current_kyu()] ++ ;
            else fail_level_statistic[loader.get_current_kyu()]++;
            //cout<<"================================\n\n\n";
            if(i<loader.problem_size-1)loader.next_problem();
        }
        cout<<endl;
        if(difficulty[i0] == "D")cout<<res_cnt<<"/"<<fail_cnt<<": "<<float(res_cnt)/(res_cnt+fail_cnt)<<endl;
        else cout<<fail_cnt<<"/"<<res_cnt<<": "<<float(fail_cnt)/(res_cnt+fail_cnt)<<endl;
        record<<"---"<<difficulty[i0]<<": "<<fail_cnt<<"/"<<res_cnt<<"---"<<endl;
        cout<<endl;
        total += fail_cnt + res_cnt;
        correct += difficulty[i0] == "D"? res_cnt:fail_cnt;
    }
    cout<<correct<<"/"<<total<<":  "<<double(correct)/double(total)*100<<"\%"<<endl;
    record<<correct<<"/"<<total<<":  "<<double(correct)/double(total)*100<<"\%"<<endl;
    cout<<"end time:"<<time(0) - start_time<<endl;
    record<<"end time:"<<time(0) - start_time<<endl;

    cout<<"level statistic:"<<endl;

    cout<<setw(10)<<" :"<<endl;
    for(int i=15;i>=0;i--)
        cout<<setw(3)<<i;
    cout<<endl;

    cout<<setw(10)<<"correct: ";
    for(int i=15;i>=0;i--){
        cout<<setw(3)<<correct_level_statistic[i];
    }
    cout<<endl;
    cout<<setw(10)<<"fail: ";
    for(int i=15;i>=0;i--){
        cout<<setw(3)<<fail_level_statistic[i];
    }

    record<<"level statistic:"<<endl;

    record<<setw(10)<<" :";
    for(int i=15;i>=0;i--)
        record<<setw(3)<<i;
    record<<endl;

    record<<setw(10)<<"correct: ";
    for(int i=15;i>=0;i--){
        record<<setw(3)<<correct_level_statistic[i];
    }
    record<<endl;
    record<<setw(10)<<"fail: ";
    for(int i=15;i>=0;i--){
        record<<setw(3)<<fail_level_statistic[i];
    }

    record.close();
    return 0;
}
