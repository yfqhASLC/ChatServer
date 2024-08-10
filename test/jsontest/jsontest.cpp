#include"json.hpp"
using json = nlohmann::json;
#include<iostream>
using namespace std;


int main()
{
    json js;
    js["id"]={123456};
    js["name"]="zhang san";
    js["msg"]="hello world";
    cout<<js<<endl;





    return 0;
}