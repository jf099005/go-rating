#include<iostream>
#include<fstream>
#include<sstream>
using namespace std;



void cmp_record(string path1, string path2){
    cout<<"compare file, this time:"<<path1<<" ,\n\t\t last time:"<<path2<<endl;
    ifstream this_time(path1, ios::in);
    ifstream last_time(path2, ios::in);
    string s1, s2;
    string difficulty[5] = {"D","C","B","A","S"};
    for(int d=0;d<5;d++){
        cout<<"difficulty "<< difficulty[d]<<": -----"<<endl;
        vector<int> rec1, rec2;
        vector<int>url1, url2;
        while(getline(this_time, s1)){
            if(s1[1] != ' ')break;
            stringstream ss;
            ss << s1;
            string tmp;
            ss>>tmp;
            int id1;
            ss >> id1;
            rec1.push_back(id1);
            ss>>tmp;
            ss>>tmp;
            int url;
            ss>>url;
            url1.push_back(url);
        }
        while(getline(last_time, s1)){
            if(s1[1] != ' ')break;
            stringstream ss;
            ss << s1;
            string tmp;
            ss>>tmp;
            int id1;
            ss >> id1;
            rec2.push_back(id1);
            ss>>tmp;
            ss>>tmp;
            int url;
            ss>>url;
            url2.push_back(url);
        }
        // for(int i=0;i<rec1.size();i++)cout<<rec1[i]<<" ";
        // cout<<endl;
        // for(int i=0;i<rec2.size();i++)cout<<rec2[i]<<" ";
        // cout<<endl;
        

        cout<<"\t new solve:\n";
        for(int i2=0;i2<rec2.size();i2++){
            bool solved = 1;
            for(int i1=0;solved and i1<rec1.size();i1++){
                if(rec1[i1] == rec2[i2])solved = 0;
            }
            if(solved){
                cout<<"\t\t "<<difficulty[d]<<"-"<<rec2[i2]<<": "<<url2[i2]<<endl;
            }
        }
        
        cout<<"\t new mistake:\n";
        for(int i1=0;i1<rec1.size();i1++){
            bool solved = 1;
            for(int i2=0;solved and i2<rec2.size();i2++){
                if(rec1[i1] == rec2[i2])solved = 0;
            }
            if(solved){
                cout<<"\t\t "<<difficulty[d]<<"-"<<rec1[i1]<<": "<<url1[i1]<<endl;
            }
        }

        cout<<"------ end ------"<<endl<<endl;
    }

    return;
}

int main(int argc, char* argv[]){
    if(argc<3){
        cout<<"ERROR\n";
        return 0;
    }
    string root = "./record/";
    string path1 = string(argv[1]);
    string path2 = string(argv[2]);
    
    cmp_record(path1, path2);
    return 0;
}