#include <cstdlib>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <numeric>
#include <iomanip>
#include <vector>

using namespace std;

int intreg[32] = {0}; // integer registers are global

int loadmemvalue(string value); // sub function for decoding Loads and Stores

int main(int argc, char *argv[])
{
    int add_lat;
    int mult_lat;
    int div_lat;
    int ld_lat;
    int add_stations;
    int mult_stations;
    int ld_stations;
    int st_stations;
    char custom;
    cout << "Custom Y/N";
    cin >> custom;
    if (custom == 'Y')
    {
        cout << " Enter number of cycles for adder";
        cin >> add_lat;
        cout << " Enter number of cycles for Multiplier";
        cin >> mult_lat;
        cout << "Enter number of cycles for Divider";
        cin >> div_lat;
        cout << " Enter number of cycles for Load/Store";
        cin >> ld_lat;
        cout << " Enter number of Add reservation stations";
        cin >> add_stations;
        cout << " Enter number of Multiply reservation stations";
        cin >> mult_stations;
        cout << "Enter ld";
        cin >> ld_stations;

        cout << " Enter number of Store reservation stations";

        cin >> st_stations;
    }
    else
    {
        add_lat = 2;
        mult_lat = 10;
        div_lat = 40;
        ld_lat = 2;
        add_stations = 3;
        mult_stations = 2;
        ld_stations = 3;
        st_stations = 3;
    }
    vector<int> add_res;

    for (int i = 0; i < add_stations; i++)
    {

        add_res.push_back(0);
    }

    vector<int> mult_res;

    for (int i = 0; i < mult_stations; i++)
    {

        mult_res.push_back(0);
    }

    vector<int> st_res;

    for (int i = 0; i < st_stations; i++)
    {
        st_res.push_back(0);
    }

    vector<int> ld_res;

    for (int i = 0; i < ld_stations; i++)
    {

        ld_res.push_back(0);
    }

    string instruction; // instruction read before for parsing
    string dest_s;
    string S1_s;
    string S2_s;
    string b;
    string buffer;
    size_t i = 1;
    int dest = 0; // dest and source registers
    int S1 = 0;
    int S2 = 0;
    int memory[19] = {45, 12, 0, 0, 10, 135, 254, 127, 18, 4, 55, 8, 2, 98, 13, 5, 233, 158, 167}; // memory locations
    float FPreg[32] = {0};                                                                         // Floating point registers
    int FPreg_busy[32] = {0};                                                                      // Buffer holding value of when Floating Point Registers are Free
    int smallest;

    int index;

    int intreg_busy[32] = {0}; // Buffer holding value of when Integer point Registers are free
    vector<int> write_vector;
    int mem_buffer = 0;
    int done = 0;
    int issue = 0; // buffer holding issue value
    int execution; // execution cycle
    //int writeback_buff[32]={0};  // buffer holding value of when operand get is per register to avoid write after read hazard
    int ready = 0;
    int operand = 0; // operand cycle
    int write = 0;   // writeback cycle
    /*int LD_busy=0;    // holds when LD is free
    int ADD_busy=0; // holds when the adder is free
    int MULT_busy=0; // holds when the multiplier is free
    int DIV_busy=0; */
    // holds when the divider is free
    bool immediate = false; // flag for immediate or not immediate so the program knows how to read in the data
    int mem_busy[19];

    ifstream read; // reading object
    read.open(argv[1]);

    if (!read.is_open())
    {
        cout << "File did not open"; // error message if file does not open
        return -1;
    }

    cout << endl
         << "Scoreboard:" << endl;

    while (done == 0)
    {

        read >> instruction;
        read >> dest_s;

        if (instruction == "ADDI" || instruction == "SUBI")
        { // checks if immediate or not

            immediate = true;
        }

        else
        {

            immediate = false;
        }

        while (dest_s[i] != ',')
        { // Reads in the values and parses the values it reads in,
            //                                                              ***FROM HERE*****

            b.push_back(dest_s[i]);
            i++;
        }

        dest = stoi(b, nullptr, 10);
        i = 1;
        b.clear();

        if (instruction == "L.D")
        {

            read >> buffer;
        }
        else if (instruction == "S.D")
        {
            read >> buffer;
        }
        else
        {

            read >> S1_s;
            while (S1_s[i] != ',')
            {
                b.push_back(S1_s[i]);
                i++;
            }

            S1 = stoi(b, nullptr, 10);
            //cout<<S1;
            i = 1;
            b.clear();

            if (immediate == false)
            {
                read >> S2_s;
                while (dest_s[i] != string::npos)
                {
                    b.push_back(S2_s[i]);
                    i++;
                }

                S2 = stoi(b, nullptr, 10);
                i = 1;
                b.clear();
            }
            else
            {
                read >> S2;
            }
        } //                                               *** TO HERE****

        if (issue >= ready)
        { // checks issue buffer
            ready = issue;
        }

        if (instruction == "L.D")
        { //         IF LOAD FUNCTIONALITY

            smallest = ld_res[0];
            index = 0;

            mem_buffer = loadmemvalue(buffer); // CALLS SUBFUNCTION loadmemvalue(); to read which memory location to load from

            for (int i = 0; i < ld_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (ld_res[i] < smallest)
                {

                    smallest = ld_res[i];
                    index = i;
                }
            }

            if (issue < smallest)
            {
                issue = smallest;
            }
            ::intreg[4] = 10;
            ready = issue;

            /*if(LD_busy>ready){     // If the LD is busy after the destination reg is busy, the issue is the clock cycle of the LD is freed
                        
                        ready=LD_busy;
                        }*/

            ready++; // increment issue;

            cout << setw(2) << ready << " "; // print out issue for that instruction

            issue = ready;

            operand = issue; // Load can always get operand after issue if only one ALU

            if (mem_busy[mem_buffer] >= operand)
            {
                operand = mem_busy[mem_buffer];
            }

            /* FIX */ //writeback_buff_int[S1]=operand; // hold when operand get was for source registers to make sure they dont overwrite before the operand get

            execution = operand + ld_lat; // execution is operand + 1;

            cout << setw(2) << execution << " ";

            write = execution + 1;

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);

            ld_res[index] = write;
            //if(writeback_buff[dest]>write){  // checks to make sure no Write after read hazard exists

            //  write=writeback_buff[dest]+1;

            //}

            cout << setw(2) << write << " "; // print out writeback

            FPreg_busy[dest] = write; // update Register busy buffer

            //LD_busy=write;

            FPreg[dest] = memory[mem_buffer]; // Load register value
        }

        else if (instruction == "ADD")
        {

            ready = issue;

            smallest = add_res[0];
            index = 0;

            for (int i = 0; i < add_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (add_res[i] < smallest)
                {

                    smallest = add_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;
            //THISSSSSSSSSSSSSSSSSSSSSSSSSSSSSS

            if (intreg_busy[S1] > issue)
            {
                operand = intreg_busy[S1];
            }
            else if (intreg_busy[S2] > issue && intreg_busy[S2] > intreg_busy[S1])
            { //checks is source registers are ready for opperand
                operand = intreg_busy[S2];
            }
            else
            {
                operand = issue;
            }
            ////THISSSSSSSSS

            execution = operand + add_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            add_res[index] = write;

            cout << setw(2) << write << " ";

            // THISSSSSSSSS

            intreg_busy[dest] = write;

            ::intreg[dest] = ::intreg[S1] + ::intreg[S2];
        }

        else if (instruction == "ADDI")
        { // SAME AS ADD but slight difference with immediate

            ready = issue;

            smallest = add_res[0];
            index = 0;

            for (int i = 0; i < add_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (add_res[i] < smallest)
                {

                    smallest = add_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;
            if (intreg_busy[S1] > issue)
            { // checks if only one source register is ready
                operand = intreg_busy[S1];
            }

            else
            {
                operand = issue;
            }

            execution = operand + add_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            add_res[index] = write;

            cout << setw(2) << write << " ";

            intreg_busy[dest] = write;
            ::intreg[dest] = ::intreg[S1] + S2;
        }

        else if (instruction == "ADD.D")
        {

            ready = issue;

            smallest = add_res[0];
            index = 0;

            for (int i = 0; i < add_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (add_res[i] < smallest)
                {

                    smallest = add_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;

            if (FPreg_busy[S1] > issue)
            {
                operand = FPreg_busy[S1];
            }
            else if (FPreg_busy[S2] > issue && FPreg_busy[S2] > FPreg_busy[S1])
            {

                operand = FPreg_busy[S2]; //checks when source registers are free for operand
            }
            else
            {
                operand = issue;
            }

            execution = operand + add_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            add_res[index] = write;

            cout << setw(2) << write << " ";

            FPreg_busy[dest] = write;

            FPreg[dest] = FPreg[S1] + FPreg[S2];
        }
        else if (instruction == "SUB.D")
        {

            ready = issue;

            smallest = add_res[0];
            index = 0;

            for (int i = 0; i < add_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (add_res[i] < smallest)
                {

                    smallest = add_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;

            if (FPreg_busy[S1] > issue)
            {
                operand = FPreg_busy[S1];
            }
            else if (FPreg_busy[S2] > issue && FPreg_busy[S2] > FPreg_busy[S1])
            { // checks source registers
                operand = FPreg_busy[S2];
            }
            else
            {
                operand = issue;
            }

            execution = operand + add_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            add_res[index] = write;

            cout << setw(2) << write << " ";

            FPreg_busy[dest] = write;

            //ADD_busy=write;

            FPreg[dest] = FPreg[S1] - FPreg[S2]; // update value
        }

        else if (instruction == "SUBI")
        {
            // SUB IMMEDIATE

            ready = issue;

            smallest = add_res[0];
            index = 0;

            for (int i = 0; i < add_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (add_res[i] < smallest)
                {

                    smallest = add_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;

            if (intreg_busy[S1] > issue)
            {
                operand = intreg_busy[S1];
            }

            else
            {
                operand = issue;
            }

            execution = operand + add_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            add_res[index] = write;

            cout << setw(2) << write << " ";

            intreg_busy[dest] = write;

            ::intreg[dest] = ::intreg[S1] - S2;
        }

        else if (instruction == "MULT.D")
        { // MULTIPLICATION

            ready = issue;

            smallest = mult_res[0];
            index = 0;

            for (int i = 0; i < mult_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (mult_res[i] < smallest)
                {

                    smallest = mult_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;
            if (FPreg_busy[S1] > issue)
            {
                operand = FPreg_busy[S1];
            }
            else if (FPreg_busy[S2] > issue && FPreg_busy[S2] > FPreg_busy[S1])
            {
                operand = FPreg_busy[S2];
            }
            else
            {
                operand = issue;
            }
            execution = operand + mult_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            mult_res[index] = write;

            cout << setw(2) << write << " ";

            FPreg_busy[dest] = write;

            FPreg[dest] = FPreg[S1] * FPreg[S2];
        }

        else if (instruction == "DIV.D")
        {

            ready = issue;

            smallest = mult_res[0];
            index = 0;

            for (int i = 0; i < mult_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (mult_res[i] < smallest)
                {

                    smallest = mult_res[i];
                    index = i;
                }
            }

            if (ready < smallest)
            {
                ready = smallest;
            }

            ready++;

            cout << setw(2) << ready << " ";
            issue = ready;

            if (FPreg_busy[S1] > issue)
            {
                operand = FPreg_busy[S1];
            }

            else if (FPreg_busy[S2] > issue && FPreg_busy[S2] > FPreg_busy[S1])
            {
                operand = FPreg_busy[S2];
            }
            else
            {
                operand = issue;
            }
            execution = operand + div_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;
            /*if(writeback_buff_int[dest]>write){ // checks for writeback hazard
                    write=writeback_buff_int[dest]+1;

                    }
*/

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }

            write_vector.push_back(write);
            mult_res[index] = write;

            cout << setw(2) << write << " ";

            FPreg_busy[dest] = write;

            FPreg[dest] = FPreg[S1] / FPreg[S2];
        }

        else if (instruction == "S.D")
        {
            mem_buffer = loadmemvalue(buffer); // CALLS SUBFUNCTION loadmemvalue(); to read which memory location to load from

            smallest = st_res[0];

            for (int i = 0; i < st_res.size(); i++)
            { // IF the destination register is busy after the last issue, issue is the cycle it is freed;

                if (st_res[i] < smallest)
                {

                    smallest = st_res[i];
                    index = i;
                }
            }

            if (issue < smallest)
            {
                issue = smallest;
            }

            ready = issue;

            ready++;
            issue = ready;
            cout << issue << " ";

            if (FPreg_busy[dest] > issue)
            { // checks if value is ready for operand get
                operand = FPreg_busy[dest];
            }
            else
            {
                operand = issue;
            }

            //writeback_buff[S1]=operand;

            execution = operand + ld_lat;

            cout << setw(2) << execution << " ";

            write = execution + 1;

            cout << setw(2) << write << " ";

            for (int i = 0; i < write_vector.size(); i++)
            {
                if (write_vector[i] == write)
                { // checks that write back value is unique
                    write++;
                    i = -1;
                }
            }                                 //LD_busy=write;
            memory[mem_buffer] = FPreg[dest]; // stores in mem;

            mem_busy[mem_buffer] = write;

            st_res[index] = write;
        }

        if (!read.good())
        { // checks if file is done
            done = 1;
        }

        cout << endl;
    }
    //prints out register value
    cout << endl
         << "Register Values:" << endl;

    for (i = 0; i < 32; i++)
    {

        cout << "Integer Register " << i << ": " << intreg[i] << endl;
    }

    cout << endl;
    for (i = 0; i < 32; i++)
    {

        cout << "FP Register " << i << ": " << FPreg[i] << endl;
    }

    return 0;
}

int loadmemvalue(string value)
{ // definition of loadmemvalue function

    string offset;
    string mem;

    int offval;

    size_t i = 0;
    size_t t = 0;
    int memval;

    while (value[i] != '(')
    {
        offset.push_back(value[i]); // calculates offset
        i++;
        t++;
    }
    //cout << offset;
    offval = stoi(offset, nullptr, 10);
    i++; // removes parenthesis
    if (value[i] == '$')
    {
        i++;

        while (value[i] != ')')
        {
            mem.push_back(value[i]);
            i++;
        }
        memval = stoi(mem, nullptr, 10);

        offval = offval / 8;
        memval = ::intreg[memval] + offval;
    }
    else
    {

        while (value[i] != ')')
        {

            mem.push_back(value[i]);
            i++;
        }

        offval = offval / 8;
        memval = stoi(mem, nullptr, 10); // applies offset to memory
        memval = memval + offval;
    }

    return memval;
} // returns memory location