#pragma once
#include"endgame_detect.h"
using namespace std;

class capture_win_detect: public endgame_detect{
    public:
    capture_win_detect(int n):
        endgame_detect(n)
    {};
    //判斷color是否已通殺對手的所有棋塊
    bool win_by_kill_all(int color, bool show_detail = false, bool ability_constraint = false){
        int escape_blocks = 0;
        for(int py=1;py<=get_board_size();py++){
            for(int px=1;px<=get_board_size();px++){
                if( get_board(py,px) == -color and !is_dead( -color, {py,px} ) ){
                    
                        if(show_detail)cout<<"there's a live stone at ("<<py<<","<<px<<")"<<endl;
                        return 0;
                    //}
                }
//                is_visited[py][px] = 1;
            }
        }
        //若對手可將此盤面變為複雜盤面，則不會視為已通殺
        if(ability_constraint){
            if(show_detail)cout<<"try to consfuse the state\n";
            for(int py=1; py<=get_board_size(); py++){
                for(int px=1; px<=get_board_size(); px++){
                    if(is_legal_movement(-color, {py,px})){
                        if(show_detail)cout<<"\t try at "<<py<<","<<px<<endl;
                        add_stone( -color, {py,px} );
                        bool complicated = is_complicated(color);
                        undo();
                        if(complicated)return 0;
                    }
                }
            }
        }
        return 1;
    }

    //判斷該落子是否為做眼
    bool is_make_eye(int color, pair<int,int> pt){
        if(!is_legal_movement(color, pt))
            return 0;
        bool make_eye = 0;
        for(int d=0;!make_eye and d<4;d++){
            pair<int,int> nx(pt.first+dir[d][0], pt.second+dir[d][1]);
            if(out_of_edge(nx))continue;
            if( is_big_eye(color, nx, 4, false) )continue;
            add_stone(color, pt);
            make_eye |= is_big_eye( color, nx, 4, false) or corner_eye_shape_detect(color, nx);
            undo();
        }
        return make_eye;
    }

    //判斷該落子是否為破眼且與其他活子連接
    bool is_break_eye(int color, pair<int,int> pt){
        if(!is_make_eye(-color, pt))return 0;
        bool connect_with_others = 0;
        add_stone(color, pt);
        for(int dy=-1; dy<=1 and !connect_with_others; dy++){
            for(int dx=-1; dx<=1 and !connect_with_others; dx++){
                if(!dy and !dx or out_of_edge({pt.first+dy,pt.second+dx}))continue;
                connect_with_others |= (get_board(pt.first+dy, pt.second+dx) == color) and \
                                            is_connected(pt, {pt.first+dy, pt.second+dx});
            }
        }
        undo();
        if(!connect_with_others)return 0;
        return is_rational_movement(color,pt);
    };

    //知識控制變數，設為true時解題者可使用該知識，false時不可
    bool D_capture_knowledge  = true;
    bool D_atari_knowledge = true;
    bool D_connect_knowledge = true;
    bool D_cut_knowledge = true;
    bool D_make_eye_knowledge = true;
    bool D_break_eye_knowledge = true;
    bool D_escape_knowledge = true;
    bool D_stop_escape_knowledge = true;

    //判斷該落子是否為廣義連接(包含斜角)
    bool is_generalized_connect(int color, pair<int,int> pt){
        if(!is_legal_movement(color, pt))return 0;
        for(int d=0; d<4; d++){
            pair<int,int> nx(pt.first+dir[d][0], pt.second+dir[d][1]);
            if(get_board(nx) == color){
                pair<int,int> tmp[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
                int size_before = get_connected_part(color, nx, tmp);
                add_stone(color, pt);
                int size_after = get_connected_part(color, nx, tmp);
                undo();
                if(size_after > size_before + 1)return 1;
                else return 0;
            }
        }
        return 0;
    }

    //判斷該落子除廣義連接外，是否能救活某子(D級以此作為連接定義使用)
    bool is_escape(int color, pair<int,int> pt){
        if(!is_rational_movement(color, pt) or !is_generalized_connect(color, pt))return 0;
        add_stone(color, pt);
        for(int dy=-1; dy<=1; dy++){
            for(int dx=-1; dx<=1; dx++){
                if(!dy and !dx)
                    continue;
                pair<int,int> cur(pt.first+dy, pt.second+dx);
                if(!is_connected(pt, cur) or is_dead(color, cur))
                    continue;
                undo();
                if(is_legal_movement(-color, pt)){
                    add_stone(-color, pt);
                    bool can_kill = get_board(cur) == 0 or is_dead(color, cur);
                    undo();
                    if(can_kill){
                        return 1;
                    }
                }
                add_stone(color, pt);
            }
        }
        undo();
        return 0;
    }

    //判斷該落子是否能阻止廣義連接(D級以此作為切斷定義)
    bool is_stop_escape(int color, pair<int,int> pt){
        return  is_rational_movement(color, pt) and is_escape(-color, pt);
    }

    //設定知識
    void set_capture_knowledge(bool capture, bool atari, bool connect, bool cut, bool make_eye,\
                 bool break_eye, bool escape, bool stop_escape){
        D_capture_knowledge = capture;
        D_atari_knowledge = atari;
        D_connect_knowledge = connect;
        D_cut_knowledge = cut;
        D_make_eye_knowledge = make_eye;
        D_break_eye_knowledge = break_eye;
        D_escape_knowledge = escape;
        D_stop_escape_knowledge = stop_escape;
    };

    //此變數為0時，不執行知識限制
    bool capture_restriction = 1;
    void free_capture_restriction(){
        capture_restriction = 0;
    }
    void enable_capture_restriction(){
        capture_restriction = 1;
    }
    //嚴格定義下的叫吃，叫吃對象須無法以吃掉其他子逃脫
    bool is_strictly_atari(int color, pair<int,int> pt){
        int py = pt.first, px = pt.second;
        if(!is_legal_movement(color, pt))return 0;
        for(int d=0; d<4; d++){
            int ny = py+dir[d][0], nx=px+dir[d][1];
            if( get_board(ny,nx) != -color )continue;
            add_stone(color, pt);
            if(get_board(ny,nx) == 0){
                undo();
                continue;
            }
            if(can_escape_by_capture(-color, {ny,nx})){
                undo();
                continue;
            }
            if( calculate_liberty({ny,nx}) == 1 ){
                undo();
                return 1;
            }
            undo();
        }
        return 0;
    }

    bool can_escape_by_capture(int escape_color, pair<int,int> pt){
        pair<int,int> edge[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
        int edge_size = find_connected_component_edge(pt, edge);
        for(int i=0;i<edge_size;i++){
            if(get_board(edge[i]) != -escape_color)continue;
            if( calculate_liberty(edge[i]) <= 1 )return true;
        }
        return false;
    }

    //判斷落子是否符合目前限制的知識
    inline bool capture_stone_position_check(int color, pair<int,int> pt, int current_step=2, bool show_detail = false){
        if(!capture_restriction)return 1;
        bool result = D_capture_knowledge and is_capture(color, pt);
        result |= D_atari_knowledge and is_strictly_atari(color, pt);
        result |= D_connect_knowledge and is_connect(color, pt);
        result |= D_make_eye_knowledge and is_make_eye(color, pt);
        result |= D_break_eye_knowledge and is_break_eye(color, pt);
        result |= D_cut_knowledge and is_cut(color, pt);
        result |= D_escape_knowledge and is_escape(color, pt);
        result |= D_stop_escape_knowledge and is_stop_escape(color, pt);
        // result |= is_generalized_connect(color, pt);
        // result |= is_generalized_cut(color, pt);
        return result;
    };

    //判斷一個棋塊是否重要且危險(i.e.少了它後對手是否能把我方通殺，且該棋塊氣數<=3)
    bool is_essential_block(int color, pair<int,int> block[], int block_size){
        pair<int,int> removed[block_size];
        int remove_size = 0;
        int liberty = 0;//氣數
        for(int i=0;i<block_size;i++){
            if(get_board( block[i] ) == 0){
                liberty++;
                //若氣數過多則該棋塊不危險，故終止
                if(liberty>3)return 0;
            }
            else if(get_board(block[i]) == color){
                removed[remove_size++] =block[i];
            }
            else{
                cout<<"ERROR: calling is_essential_block but the block have non-consistent stone"<<endl;
                return 0;
            }

        }

        //移除該棋塊後觀察是否敵方可將我方通殺
        remove_stones(removed, remove_size);
        bool is_essential = win_by_kill_all(-color);
        for(int i=0;i<remove_size;i++)
            add_stone(color, removed[i], false);

        return is_essential;
    }

    //判斷一個棋塊是否複雜
    bool is_complicated(int color, bool show_detail = false, bool can_ate = true){
        bool is_visited[MAX_BOARD_SIZE+2][MAX_BOARD_SIZE+2];
        memset(is_visited,0,sizeof(is_visited));
        //紀錄危險&重要棋塊的數量
        int danger_SCC_cnt = 0;
        pair<int,int> danger_block[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
        if(show_detail){
            print_board();
        }
        //尋訪所有棋塊
        for(int py=1; py<=get_board_size(); py++){
            for(int px=1; px<=get_board_size(); px++){
                if(!is_visited[py][px] and get_board(py,px) == color){
                    int block_size = find_board_area(color, {py,px}, danger_block);
                    //取得該棋塊
                    if(show_detail)cout<<"block:"<<py<<","<<px<<endl;
                    if(show_detail)cout<<"\t block size:"<<block_size<<endl;
                    //標記棋塊以避免重複計算
                    for(int i=0;i<block_size;i++){
                        is_visited[ danger_block[i].first ][ danger_block[i].second ] = 1;
                    }
                    //若該棋塊必要&危險，則計數+1
                    if(is_essential_block(color, danger_block, block_size))
                        danger_SCC_cnt++;
                }
            }
        }
        if(show_detail)cout<<"dangerous component: "<<danger_SCC_cnt<<endl;
        
        //若危險棋塊數量<2，則此盤面不複雜
        if(danger_SCC_cnt<2)
            return 0;

        //若危險棋塊>2，則觀察敵方是否有棋筋
        if(can_ate){
            bool solution = true;
            pair<int,int> ate_pt;
            int ate_block_size = 0;
            for(int py=1; py<=get_board_size() and solution; py++){
                for(int px=1; px<=get_board_size() and solution; px++){
                    if(get_board(py,px) == -color and calculate_liberty({py,px}) == 1){
                        int component_size = get_component_size({py,px});
                        if(ate_block_size<= component_size){
                            ate_pt = get_liberty_position({py,px});
                            ate_block_size = component_size;
                        }
                    }
                }
            }
            //若敵方有棋筋且棋筋大小不大，則觀察我方吃掉棋筋後盤面是否依然複雜
            if(ate_block_size>0 and ate_block_size<=4){
                add_stone(color, ate_pt);
                if(show_detail)cout<<"ate the stone at "<<ate_pt<<endl;
                solution = is_complicated(color, 0,false);
                undo();
            }
            return solution;
        }
        return 1;
    }
    
    //展開函數，以minimax觀察是否在限制步數內能解題
    bool kill_with_depth(int color, int depth, bool show_detail = 0, int current_color = 0,int current_step = 1){
        if(current_color==0)current_color = color;
        if(show_detail)cout<<"calling "<<color<<", "<<depth<<", "<<current_color<<endl;
        if(show_detail)print_board();
        
        //若目前非第一步且盤面複雜，則視為解題失敗
        if(current_step>2 and color == current_color and is_complicated(color)){
            if(show_detail)cout<<"the current board is complicated\n";
            return 0;
        }
        
        //若目前輪到我方，且達成有眼殺瞎條件，則解題成功
        if(capture_with_eyes(color) and (color == current_color or depth <=0)){
            if(show_detail){
                cout<<"win at--------------------------------------------------\n";
                print_board();
                cout<<"-------------------------------------------------------\n";
            }
            return 1;
        };

        //若達成通殺條件，則解題成功
        if(win_by_kill_all(color, 0, capture_restriction) and (color == current_color or depth <= 0)){
            if(show_detail){
                cout<<"kill_all at--------------------------------------------------\n";
                print_board();
                cout<<"-------------------------------------------------------\n";
            }
            return 1;
        }

        //若深度過深則解題失敗
        if(depth <= 0){
            return 0;
        }

        //若尚未進入終局，則巡訪所有點:        
        for(int py=1;py<=get_board_size();py++){
            for(int px=1;px<=get_board_size();px++){
                    if(!is_legal_movement(current_color, {py,px})){
                        if(show_detail)cout<<"skip "<<py<<","<<px<<endl;
                        continue;
                    }

                    //跳過所有非知識點
                    if(color == current_color){
                        if(!is_rational_movement( current_color, {py,px} ) and capture_restriction){
                            if(show_detail)cout<<"skip by rational move: "<< py<<","<<px<<endl;
                            continue;
                        }
                        if(!capture_stone_position_check(current_color, {py,px}, current_step)){
                            if(show_detail)cout<<"skip by non-common move: "<<py<<","<<px<<endl;
                            if(show_detail)cout<<capture_stone_position_check(color, {py,px}, current_step, 1)<<endl;
                            continue;
                        }
                    }

                    //判斷該落子是否為叫吃，若是則深度+=1
                    bool is_atari_move = is_atari(current_color, {py,px});
                    
                    if(show_detail)cout<<"search "<<py<<","<<px<<endl;

                    //落子後觀察狀態
                    add_stone( current_color, {py,px} );
                    if(show_detail){
                        cout<<"search ("<<py<<","<<px<<") :\n";
                        print_board();
                    }

                    bool is_winning;
                    //若落子後局面複雜則視為解題失敗
                    if(current_step>=2 and color!=current_color and is_complicated(color)){
                        if(show_detail){
                            cout<<"\tthe state is too complicated\n";
                            if(show_detail)cout<<is_complicated(color, 1)<<"]\n";
                        }
                        is_winning = false;
                    }
                    else{
                        if(show_detail)cout<<"\tsearch\n";
                        //往下搜索
                        is_winning = kill_with_depth(color, depth-1, 0, -current_color, current_step+1);
                        if(show_detail)cout<<"\t normal solution:"<<is_winning<<endl;
                        //檢查是否已達通殺勝利
                        if(color == current_color and win_by_kill_all(color, 0, capture_restriction)){
                            if(show_detail)cout<<"already kill all, end\n";
                            is_winning = true;
                        }
                        //若已達到當層最佳解(若當層為我方則為勝利，敵方則為阻止勝利)且(y,x)點為自殺步，則使當層對手提子並不減深度向下搜索
                        if( !(is_winning^(current_color == color)) \
                                and calculate_liberty({py,px}) == 1){
                            pair<int,int> capture_pt = get_liberty_position({py,px});
                            if(is_legal_movement(-current_color, capture_pt)){
                                add_stone(-current_color, capture_pt);
                                if(show_detail)cout<<"test capture\n";
                                is_winning = kill_with_depth(color, depth, 0, current_color, current_step + 2);
                                if(show_detail){
                                    cout<<"solution of capture:"<<is_winning<<endl;
                                }
                                undo();
                            }
                        }
                        //若已達到當層最佳條件，且(y,x)為叫吃，則使當層對手連回被叫吃子，並不減深度向下搜索
                        if( !(is_winning^(current_color == color)) \
                                and is_atari_move and depth<=1){

                            for(int d=0;d<4;d++){
                                pair<int,int> nx(py+dir[d][0], px+dir[d][1]);
                                if(out_of_edge(nx))continue;
                                if(calculate_liberty( nx ) == 1){
                                    pair<int,int> intuitive_defend = get_liberty_position(nx);
                                    if(!is_legal_movement(-current_color, intuitive_defend))
                                        continue;
                                    if(show_detail)cout<<"test escape at "<<intuitive_defend<<"\n";
                                    add_stone(-current_color, intuitive_defend);                                    
                                    if(show_detail)print_board();
                                    is_winning = kill_with_depth(color, depth, 0, current_color, current_step +2);
                                    undo();
                                    break;
                                }
                            }
                            if(show_detail)cout<<"solution of atari:"<<is_winning<<endl;
                        }
                    }
                    undo();
                    //若上述操作執行完後，該落子依然為當層最佳解，則return true
                    if( !( is_winning ^ (current_color == color) )){
                        if(show_detail)cout<<"return at layer"<<depth<<":  "<<is_winning<<endl;
                        return is_winning;
                    };
                    if(show_detail)cout<<"at layer "<<depth<<", result:"<< is_winning<<endl;
            }
        }
        //若當層為對手，則允許pass
        bool solution;
        if(current_color != color){
            bool pass_res = kill_with_depth(color, depth-1, 0, -current_color, current_step+1 );
            if(show_detail)cout<<"pass:"<<pass_res<<endl;
            solution = pass_res;
        }
        else solution = (color != current_color);//win_with_depth(color, depth-1, 0, -current_color);//!(color == current_color);
        return solution;
    }

    //判斷color是否能用一手棋獲勝
    bool kill_within_one_move(int color, int show_detail = 0){
        bool is_winning = capture_with_eyes(color) or win_by_kill_all(color);
        for(int py=1; py<=get_board_size() and !is_winning; py++){
            for(int px=1; px<=get_board_size() and !is_winning; px++){
                    //若落子後合理，且能達成勝利條件則return true
                if(is_rational_movement(color, {py,px}) ){
                    if(show_detail)cout<<"visit "<<py<<","<<px<<endl;
                    add_stone(color, {py,px});
                    is_winning |= kill_with_depth(color, 1, 0, -color,2) || win_by_kill_all(color, 0, capture_restriction);// || capture_with_eyes(color);
                    undo();
                }
                if(is_winning){
                    if(show_detail){
                        cout<<"win at "<<py<<","<<px<<endl;
                        add_stone(color, {py,px});
                        cout<<kill_with_depth(color, 1, 0, -color)<<"/"<<win_by_kill_all(color)<<endl;
                        undo();
                    }
                    return true;
                }
            }
        }
        return is_winning;
    }
};
