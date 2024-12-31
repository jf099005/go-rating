#pragma once
#include"seki_detect.h"
#include"read_board_from_serial.h"
using namespace std;
class D_classifier: public seki_detector{
    public:
    D_classifier(int n):
        seki_detector(n)
    {};

    void reset_problem(board_data_loader& loader){
        loader.load_board_from_idx(loader.current_problem_idx);
    };

    bool is_D_problem(int color, int depth, bool show_detail = 0, bool show_result = 0){
        bool  win_ov = kill_within_one_move(color),\
                        win_seki=defend_by_seki(color);
        if(win_seki or win_ov)return 1;
        bool win_dp = kill_with_depth(color, depth, show_detail);
        if(show_result) cout<<win_dp<<"/"<<win_ov<<"/"<<win_seki<<endl;
        return win_dp or win_ov or win_seki;
    }
};
