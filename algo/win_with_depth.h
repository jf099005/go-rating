#pragma once
#include"normal_end_detect.h"
#include"memorize_search.h"
using namespace std;

class minimax_detector: public normal_end_detector{
    public:
    minimax_detector(int n):
        normal_end_detector(n)
    {};

    bool capture_knowledge  = true;
    bool atari_knowledge = true;
    bool connect_knowledge = true;
    bool cut_knowledge = true;
    bool make_eye_knowledge = true;
    bool break_eye_knowledge = true;
    bool escape_knowledge = true;
    bool stop_escape_knowledge = true;

    void set_knowledge(bool capture, bool atari, bool connect, bool cut, bool make_eye,\
                 bool break_eye, bool escape, bool stop_escape){
        capture_knowledge = capture;
        atari_knowledge = atari;
        connect_knowledge = connect;
        cut_knowledge = cut;
        make_eye_knowledge = make_eye;
        break_eye_knowledge = break_eye;
        escape_knowledge = escape;
        stop_escape_knowledge = stop_escape;
    };

    bool restriction = 1;
    void free_restriction(){
        restriction = 0;
    }
    void enable_restriction(){
        restriction = 1;
    }

    inline bool knowledge_check(int color, pair<int,int> pt, int current_step=2, bool restrict_first_step = false, bool show_detail = false){
        if(show_detail){
            cout<<is_capture(color, pt)<<"/"<<is_strictly_atari(color, pt)<<"/"<<is_connect(color, pt)<<"/"<<is_make_eye(color, pt);
            cout<<"/"<<is_break_eye(color,pt)<<"/"<<is_cut(color, pt)<<"/"\
            <<is_escape(color, pt)<<"/"<<is_stop_escape(color, pt)<<endl;
        }
        if(!restriction)return 1;
        if(current_step <= 1 and restrict_first_step){
            return is_capture(color, pt) or is_strictly_atari(color, pt)\
                    or is_escape(color,pt) or is_stop_escape(color, pt);
        }
        bool result = capture_knowledge and is_capture(color, pt);
        result |= atari_knowledge and is_strictly_atari(color, pt);
        result |= connect_knowledge and is_connect(color, pt);
        result |= make_eye_knowledge and is_make_eye(color, pt);
        result |= break_eye_knowledge and is_break_eye(color, pt);
        result |= cut_knowledge and is_cut(color, pt);
        result |= escape_knowledge and is_escape(color, pt);
        result |= stop_escape_knowledge and is_stop_escape(color, pt);
        // result |= is_generalized_connect(color, pt);
        // result |= is_generalized_cut(color, pt);
        return result;
    };

    search_solution_record recorder;
    int optimal_path[10];
    int depth_cnt = 0;
    bool win_with_depth(int color, int depth, bool show_detail=0, int current_color=0,int current_step=1, bool can_win_by_territory = 0){
        if(current_color==0)current_color = color;
        if(show_detail)cout<<"calling "<<color<<", "<<depth<<", "<<current_color<<endl;
        if(show_detail)print_board();
        int query_solution = recorder.query_minimax_solution(*this, color, current_color, depth);
        depth_cnt++;
        if(show_detail)cout<<depth_cnt<<endl;
        if(query_solution != query_fail and 1){
            if(query_solution != lower_depth){
                return query_solution;
            }
        }
        else{
        // if(1){
            bool is_win = 0;
            if((color == current_color or depth <=0 or 1) and capture_with_eyes(color)){
                if(show_detail){
                    cout<<"win at--------------------------------------------------\n";
                    print_board();
                    cout<<"-------------------------------------------------------\n";
                }
                is_win = 1;
            }
            else if(win_by_kill_all(color)){
                if(show_detail){
                    cout<<"kill_all at--------------------------------------------------\n";
                    print_board();
                    cout<<"-------------------------------------------------------\n";
                }
                //cout<<"return of "<<color<<depth<<current_color<<endl;
                is_win = 1;
            }
            else{
                int end_state = is_end_2(color); 
                if(can_win_by_territory and (end_state == true_end )){
                    if(win_by_territory(color, 0)){
                        if(show_detail){
                            cout<<"win by territory---------------------\n";
                            print_board();
                            cout<<"-------------------------------------\n";
                        }
                        is_win = 1;
                    }
                    else is_win = 0;
                }
                else if(can_win_by_territory and end_state == pseudo_end and depth<=0){
                    is_win = win_by_territory(color, 0);
                }
            }
            recorder.record_minimax_solution(*this, color, current_color, is_win?100:0,is_win );
            if(is_win)return 1;
        }

        if(depth <= 0){
            return 0;
        }
        for(int py=1;py<=get_board_size();py++){
            for(int px=1;px<=get_board_size();px++){
                if(!is_legal_movement(current_color, {py,px})){
                    if(show_detail)cout<<"skip "<<py<<","<<px<<endl;
                    continue;
                }

                if(color == current_color){
                    if(!is_rational_movement( current_color, {py,px} ) and restriction){
                        if(show_detail)cout<<"skip by rational move:: "<< py<<","<<px<<endl;
                        continue;
                    }
                    if(!knowledge_check(current_color, {py,px}, current_step)){
                        if(show_detail)cout<<"skip by non-common move:: "<<py<<","<<px<<endl;
                        continue;
                    }
                }

                bool is_atari_move = is_atari(current_color, {py,px});
                
                if(show_detail)cout<<"search "<<py<<","<<px<<endl;
                add_stone( current_color, {py,px} );

                if(show_detail){
                    cout<<"at depth "<<depth <<" search ("<<py<<","<<px<<") :\n";
                    print_board();
                }

                bool is_winning = win_with_depth(color, depth-1, 0, -current_color, current_step+1, can_win_by_territory);
                if(show_detail)cout<<"search solution:"<<is_winning<<endl;
                
                if( !(is_winning^(current_color == color)) \
                            and calculate_liberty({py,px}) == 1){
                    if(show_detail)cout<<"suicide detected, continue\n";

                    pair<int,int> capture_pt = get_liberty_position({py,px});
                    if(is_legal_movement(-current_color, capture_pt)){
                        add_stone(-current_color, capture_pt);
                        is_winning = win_with_depth(color, depth, 0, current_color, current_step + 2, can_win_by_territory);
                        undo();
                    }
                }
                // if(show_detail)cout<<"\t atari:"<<is_atari_move<<endl;
                if( !(is_winning^(current_color == color)) \
                        and is_atari_move and depth<=1){
                    if(show_detail)cout<<"atari detected, continue\n";

                    for(int d=0;d<4;d++){
                        pair<int,int> nx(py+dir[d][0], px+dir[d][1]);
                        if(out_of_edge(nx))continue;
                        if(calculate_liberty( nx ) == 1){
                            pair<int,int> intuitive_defend = get_liberty_position(nx);
                            if(!is_legal_movement(-current_color, intuitive_defend))
                                continue;
                            add_stone(-current_color, intuitive_defend);
                            is_winning = win_with_depth(color, depth, 0, current_color, current_step +2, can_win_by_territory);
                            undo();
                        }
                    }
                }

                undo();
                
                if( !( is_winning ^ (current_color == color) )){
                    if(show_detail)cout<<"ok, return "<<is_winning<<endl;
                    // search_record[to_serial(color, current_color)] = is_winning;
                    // depth_record[to_serial(color, current_color)] = depth;
                    recorder.record_minimax_solution(*this, color, current_color, depth, is_winning);
                    return is_winning;
                };
            }
        }
        // cout<<"pass\n";
        bool solution = 0;
        if(current_color != color){
            bool pass_res;
            if(depth>1)
                pass_res = win_with_depth(color, depth-1, 0, -current_color, current_step+1, can_win_by_territory );
            else
                pass_res = win_with_depth(color, depth, 0, -current_color, current_step+1, can_win_by_territory);
            if(show_detail)cout<<"pass:"<<pass_res<<endl;
            solution = pass_res;
        }
        else solution = (color != current_color);//win_with_depth(color, depth-1, 0, -current_color);//!(color == current_color);
        
        recorder.record_minimax_solution(*this, color, current_color, depth, solution);
        return solution;
    }

    bool win_within_one_move(int color, int show_detail = 0){
        bool is_winning = capture_with_eyes(color) or win_by_kill_all(color);
        for(int py=1; py<=get_board_size() and !is_winning; py++){
            for(int px=1; px<=get_board_size() and !is_winning; px++){
                if(is_rational_movement(color, {py,px}) ){
                    if(show_detail)cout<<"visit "<<py<<","<<px<<endl;
                    add_stone(color, {py,px});
                    is_winning |= win_with_depth(color, 1, 0, -color) || capture_with_eyes(color);
                    undo();
                }
                if(is_winning){
                    if(show_detail){
                        cout<<"win at "<<py<<","<<px<<endl;
                        add_stone(color, {py,px});
                        cout<<win_with_depth(color, 1, 0, -color)<<"/"<<win_by_kill_all(color)<<endl;
                        undo();
                    }
                    return true;
                }
            }
        }
        return is_winning;
    }
};
