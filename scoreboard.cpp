#include <cstdlib>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <numeric>
#include <iomanip>

using namespace std;

int intreg[32]={0}; // integer registers are global

int loadmemvalue(string value); // sub function for decoding Loads and Stores 


    int main(int argc, char * argv[]){
    ::intreg[3]=17; // initialize int reg 2 with memory location 17 
    string instruction;   // instruction read before for parsing 
    string dest_s;
    string S1_s;
    string S2_s;
    string b;
    string buffer;
    size_t i=1;
    int dest=0;     // dest and source registers 
    int S1=0;
    int S2=0;
    int memory[19]={45,12,0,0,10,135,254,127,18,4,55,8,2,98,13,5,233,158,167}; // memory locations
    float FPreg[32]={0}; // Floating point registers
    int FPreg_busy[32]={0}; // Buffer holding value of when Floating Point Registers are Free


    int intreg_busy[32]={0}; // Buffer holding value of when Integer point Registers are free
    int mem_buffer=0;  
    int done=0;
    int issue=0;    // buffer holding issue value
    int execution; // execution cycle 
    int writeback_buff[32]={0};  // buffer holding value of when operand get is per register to avoid write after read hazard
    int ready=0;  
    int operand=0;   // operand cycle
    int write=0;     // writeback cycle 
    int LD_busy=0;    // holds when LD is free
    int ADD_busy=0; // holds when the adder is free
    int MULT_busy=0; // holds when the multiplier is free
    int DIV_busy=0; // holds when the divider is free
    bool immediate= false;   // flag for immediate or not immediate so the program knows how to read in the data 
    int writeback_buff_int[32]={0};

    ifstream read; // reading object 
    read.open(argv[1]);

    if(!read.is_open()){
        cout<< "File did not open"; // error message if file does not open
        return -1;
    }
    
    cout<<endl<< "Scoreboard:"<<endl; 

            while(done==0){

                read >> instruction;
                read >> dest_s;

                if(instruction=="ADDI"||instruction== "SUBI"){ // checks if immediate or not
                
                immediate=true;

                 }    
                
                else{
                
                immediate=false;
                }

                while(dest_s[i]!=string::npos){                // Reads in the values and parses the values it reads in, 
                //                                                              ***FROM HERE*****  
                
                    b.push_back(dest_s[i]);
                i++;
                }
            
                dest=stoi(b,nullptr,10);
                i=1;
                b.clear();

                if(instruction=="L.D"){

                read >> buffer;

                }
                else if(instruction=="S.D"){
                    read>>buffer;
                }
                else{

                read >> S1_s;
                    while(S1_s[i]!=string::npos){
                    b.push_back(S1_s[i]);
                    i++;
                    }
                
                    S1=stoi(b,nullptr,10);
                    //cout<<S1;
                    i=1;
                    b.clear();

                if(immediate==false){
                read >> S2_s;
                    while(dest_s[i]!=string::npos){
                    b.push_back(S2_s[i]);
                    i++;}

                    S2=stoi(b,nullptr,10);
                    i=1;
                    b.clear();
                }
                else{
                    read>>S2;
                }
                } //                                               *** TO HERE****

                
                if(issue >= ready){  // checks issue buffer
                    ready=issue;
                }

                    if(instruction=="L.D"){ //         IF LOAD FUNCTIONALITY

                        mem_buffer=loadmemvalue(buffer);    // CALLS SUBFUNCTION loadmemvalue(); to read which memory location to load from
                    
                        if(FPreg_busy[dest]>issue){        // IF the destination register is busy after the last issue, issue is the cycle it is freed;
                            
                            ready=FPreg_busy[dest];
                        
                        }
                        
                        if(LD_busy>ready){     // If the LD is busy after the destination reg is busy, the issue is the clock cycle of the LD is freed
                        
                        ready=LD_busy;
                        }
                    
                    ready++; // increment issue;
                    
                    cout<<setw(2)<<ready<<" ";      // print out issue for that instruction
                    
                    issue=ready;

                    operand=1+ issue;    // Load can always get operand after issue if only one ALU  

                    cout<<setw(2)<< operand<<" ";  

                    writeback_buff[S1]=operand; // hold when operand get was for source registers to make sure they dont overwrite before the operand get 

                    execution= operand + 1;  // execution is operand + 1;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1; 
                    
                    if(writeback_buff[dest]>write){  // checks to make sure no Write after read hazard exists 
                        
                        write=writeback_buff[dest]+1;

                    }

                    cout<<setw(2)<<write<<" "; // print out writeback 

                    FPreg_busy[dest]=write; // update Register busy buffer 
                
                    LD_busy=write;
                
                    FPreg[dest]=memory[mem_buffer]; // Load register value

                }


                else if(instruction=="ADD"){

                    if(intreg_busy[dest]>issue){  // checks for structural hazard
                    ready=intreg_busy[dest];
                     }

                    if(ADD_busy>ready){  // checks is add is busy
                        ready=ADD_busy;
                    }
                    ready++;
                    cout<<setw(2)<<ready<<" ";
                    issue=ready;

                    if(intreg_busy[S1]> issue){
                    operand =intreg_busy[S1]+1;
                    }
                    else if(intreg_busy[S2]> issue && intreg_busy[S2]> intreg_busy[S1]){ //checks is source registers are ready for opperand 
                        operand=intreg_busy[S2]+1;
                    }
                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<< operand<<" "; 
                    writeback_buff_int[S2]=operand; // writes back when operand get was for write after read hazard
                    writeback_buff_int[S1]=operand;

                    execution= operand + 2;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;
                    if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }

                    cout<<setw(2)<<write <<" "; 

                    intreg_busy[dest]=write;
                    ADD_busy=write;
                    ::intreg[dest]=::intreg[S1]+::intreg[S2];

                    }

                else if(instruction=="ADDI"){ // SAME AS ADD but slight difference with immediate
                
                    if(intreg_busy[dest]>issue){
                        ready=intreg_busy[dest];
                    }
                    if(ADD_busy>ready){
                            ready=ADD_busy;
                            }
                        ready++;
                        cout<<setw(2)<<ready<<" ";
                        issue=ready;

                    if(intreg_busy[S1]> issue){ // checks if only one source register is ready
                    operand =intreg_busy[S1]+1;
                    }

                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<<operand <<" ";
                    //writeback_buff[S2]=operand;
                    writeback_buff_int[S1]=operand;

                    execution= operand + 2;   // delay 2 cycled


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;
                    if(writeback_buff_int[dest]>write){
                    write=writeback_buff_int[dest]+1; //updates writeback buffer

                    }

                    cout<<setw(2)<<write<<" "; 

                    intreg_busy[dest]=write;
                    ADD_busy=write;
                    ::intreg[dest]=::intreg[S1]+S2;


                    }

                else if(instruction=="ADD.D"){
                    
                    if(FPreg_busy[dest]>issue){
                        ready=FPreg_busy[dest];
                    }
                    if(ADD_busy>ready){ //checks for hazards
                            ready=ADD_busy;
                            }
                        ready++;
                        cout<<setw(2)<<ready<<" ";
                        issue=ready;

                    if(FPreg_busy[S1]> issue){
                    operand =FPreg_busy[S1]+1;
                    }
                    else if(FPreg_busy[S2]> issue && FPreg_busy[S2]> FPreg_busy[S1]){  
            
                        operand=FPreg_busy[S2]+1; //checks when source registers are free for operand 
                    }
                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<<operand<<" ";

                    if(operand>writeback_buff[S1]){
                    writeback_buff[S1]=operand; //updates writeback buffer
                    

                    }
                    if(operand>writeback_buff[S2]){
                    writeback_buff[S2]=operand;
                    }

                    execution= operand + 2;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;

                    if(writeback_buff[dest]>write){
                    write=writeback_buff[dest]+1;

                    }

                    cout<<setw(2)<<write<<" ";

                    FPreg_busy[dest]=write;
                    ADD_busy=write;
                    FPreg[dest]=FPreg[S1]+FPreg[S2];

                    }
                else if(instruction=="SUB.D"){ 


                    if(FPreg_busy[dest]>issue){
                        ready=FPreg_busy[dest];
                    }
                    if(ADD_busy>ready){  // checks for hazard
                            ready=ADD_busy;
                            }
                        ready++;
                        cout<<setw(2)<<ready<<" ";
                        issue=ready;

                    if(FPreg_busy[S1]> issue){
                    operand =FPreg_busy[S1]+1;
                    }
                    else if(FPreg_busy[S2]> issue && FPreg_busy[S2]> FPreg_busy[S1]){ // checks source registers 
                        operand=FPreg_busy[S2]+1;
                    }
                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<<operand<<" ";
                    writeback_buff[S2]=operand;
                    writeback_buff[S1]=operand;

                    execution= operand + 2;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;
                    if(writeback_buff[dest]>write){ // writeback buffer
                    write=writeback_buff[dest]+1;

                    }

                    cout<<setw(2)<<write<<" "; 

                    FPreg_busy[dest]=write;
                    ADD_busy=write;
                    FPreg[dest]=FPreg[S1]-FPreg[S2]; // update value

                    }

                else if(instruction=="SUBI"){ 
                                                 // SUB IMMEDIATE
                    if(intreg_busy[dest]>issue){
                        ready=intreg_busy[dest];
                    }
                    if(ADD_busy>ready){
                            ready=ADD_busy;
                            }
                        ready++;
                        cout<<setw(2)<<ready<<" ";
                        issue=ready;

                    if(intreg_busy[S1]> issue){
                    operand =intreg_busy[S1]+1;
                    }

                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<<operand<<" ";
                    
                    writeback_buff_int[S1]=operand;

                    execution= operand + 2;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;
                    if(writeback_buff_int[dest]>write){
                    write=writeback_buff_int[dest]+1;

                    }

                    cout<<setw(2)<<write<< " "; 

                    intreg_busy[dest]=write;
                    ADD_busy=write;
                    ::intreg[dest]=::intreg[S1]-S2;
                    }

                else if(instruction=="MULT.D"){  // MULTIPLICATION
                    if(FPreg_busy[dest]>issue){
                        ready=FPreg_busy[dest];
                    }
                    if(MULT_busy>ready){
                            ready=MULT_busy;
                            }
                        
                        ready++;
                        cout<<setw(2)<<ready<<" ";
                        issue=ready;

                    if(FPreg_busy[S1] > issue){
                    operand =FPreg_busy[S1]+1;
                    }
                    else if(FPreg_busy[S2]> issue && FPreg_busy[S2]> FPreg_busy[S1]){
                        operand=FPreg_busy[S2]+1;
                    }
                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<<operand<<" ";
                    writeback_buff[S2]=operand;
                    writeback_buff[S1]=operand;

                    execution= operand + 10;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;
                    if(writeback_buff[dest]>write){
                    write=writeback_buff[dest]+1;

                    }

                    cout<<setw(2)<<write<<" "; 

                    FPreg_busy[dest]=write;
                    MULT_busy=write;
                    FPreg[dest]=FPreg[S1] * FPreg[S2];
                }

                else if(instruction=="DIV.D"){  
                //                               ***DIVISION 
                    if(FPreg_busy[dest]>issue){
                        ready=FPreg_busy[dest];
                    }
                    if(DIV_busy>ready){
                            ready=DIV_busy;
                            }
                        ready++;
                        cout<<setw(2)<<ready<<" ";
                        issue=ready;

                    if(FPreg_busy[S1]> issue){
                    operand =FPreg_busy[S1]+1;
                    }


                    else if(FPreg_busy[S2]> issue && FPreg_busy[S2]> FPreg_busy[S1]){
                        operand=FPreg_busy[S2]+1;
                    }
                    else{
                    operand=1+ issue;

                    }
                    cout<<setw(2)<<operand<<" ";
                    writeback_buff[S2]=operand;
                    writeback_buff[S1]=operand;

                    execution= operand + 40;


                    cout<<setw(2)<<execution;


                    cout<<" ";
                    write=execution+1;
                    if(writeback_buff[dest]>write){
                    write=writeback_buff[dest]+1;

                    }

                    cout<<setw(2)<<write<<" "; 

                    FPreg_busy[dest]=write;
                    DIV_busy=write;
                    FPreg[dest]=FPreg[S1]/FPreg[S2];
                }

                else if(instruction=="S.D"){
                
                    if(LD_busy > issue){ // checks for structural hazard
                        ready=LD_busy;
                    }
                    mem_buffer=loadmemvalue(buffer);  // parses memory location 

                    ready++;
                    issue=ready;
                    cout<<issue<<" ";
                        
                        if(FPreg_busy[dest]> issue){ // checks if value is ready for operand get 
                            operand =FPreg_busy[dest]+1;
                        }
                        else{
                            operand=issue+1;
                        }

                    cout<<setw(2)<<operand<<" ";

                    writeback_buff[S1]=operand;

                    execution= operand + 1;


                    cout<<setw(2)<<execution<<" ";

                    write=execution+1;


                    cout<< setw(2)<<write<<" "; 


                    LD_busy=write;
                    memory[mem_buffer]=FPreg[dest];// stores in mem;
                    }

                    if(!read.good()){ // checks if file is done
                        done=1;
                    }

                    cout<<endl;

                }
//prints out register value
    cout<<endl<< "Register Values:"<<endl;
    
        for(i=0;i<32;i++){     

            cout<<"Integer Register "<< i<< ": "<< intreg[i]<<endl;
        
        }

    cout<<endl;
        for(i=0;i<32;i++){

            cout<<"FP Register "<< i<< ": "<< FPreg[i]<<endl;
        }

    return 0;
    
    }

    int loadmemvalue(string value){  // definition of loadmemvalue function 

        string offset;
        string mem;
        
        int offval;

        size_t i = 0;
        size_t t= 0;
        int memval;

            while(value[i]!='('){  
                offset.push_back(value[i]);  // calculates offset
                i++;
                t++;
            } 
        //cout << offset;
        offval=stoi(offset,nullptr,10);
        i++;// removes parenthesis 
        if(value[i]=='$'){
               i++;
        
        while(value[i]!=')'){
            mem.push_back(value[i]);
            i++;
        }
        memval=stoi(mem,nullptr,10);

        offval=offval/8;
        memval=::intreg[memval]+offval;

        
        }
        else{
        
        while(value[i]!=')'){

            mem.push_back(value[i]);
        i++;


         }
       
        offval=offval/8;
        memval=stoi(mem,nullptr,10); // applies offset to memory
        memval=memval+offval;
        

        }

        return memval;
    } // returns memory location