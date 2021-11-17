#include <iostream>
#include <fstream>


int main() {

    std::fstream infile;
    infile.open(".hookID.txt", std::ios::in | std::ios::out);
    unsigned int hookID;
    if (infile.eof())
        hookID = 0;
    else 
        infile >> hookID;
    infile >> hookID;

    std::cout << hookID << std::endl;
    infile.close();


    std::fstream outfile;
    outfile.open(".hookID.txt", std::ios::out | std::ios::trunc);
    
    outfile << hookID+1;
    outfile.close();
    return 0;
}