# 程式架構說明
以下為此程式架構

## algo
各類演算法
- GO_board.h\
棋盤實作，包括落子(同時進行題子、紀錄)、找連通塊、列印棋盤等
- experience_method.h\
簡單的概念判斷、必死子判斷
- endgame_detect.h\
大眼、角落眼判斷，有眼殺瞎判斷，
- capture_to_win.h\
D級minimax解題函數、D級一步獲勝判斷函數，知識控制、複雜棋型判斷
- seki_detect.h\
雙活判斷

- memorize_search.h\
同形表實作

- normal_end_detect.h\
官子終局狀態判斷、官子勝利判斷

- possible_point_statistic.h\
統計有效分支(目前僅一層)

- read_board_from_serial.h\
讀取檔案

- win_with_depth.h\
C級以上minimax解題函數、C級以上一步獲勝判斷函數，知識控制

## record
實驗結果紀錄

## 其他

- accuracy_calculate.cpp\
    實驗用，使用win_with_depth中的解題函數對張栩300題執行，列印錯題、執行結果並產生紀錄存於./record\
使用例如下
    - ./accuracy_calculate.exe [your file name]\
    
    執行後檔案將存於./record/[your file name].txt

- load_error_data.exe\
讀取錯題用，使用方式為\
./load_error_data.exe [file_path]\
如此將會列出[file_path]檔案中所有錯題及棋盤

- output_compare.exe\
比較輸出用，若想比較兩個模型的紀錄，使用\
output_compare.exe [file1_path] [file2_path]\
即可輸出那些題目file1中答對而file2中錯，以及那些題目file1中錯而file2中對。
