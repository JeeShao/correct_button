#include<fstream>
#include<iostream>
#include<string>

using namespace std;

int main1(){
    string a="../20180706/params.txt";
    ifstream fileinput;
    try {
        fileinput.open(a.c_str());
        cout<<"20180706/params.txt"<<endl;

    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
    }
    if (!fileinput.is_open())
    {
        cerr<<"打开文件失败！"<<endl;
        exit(0);
    }
}