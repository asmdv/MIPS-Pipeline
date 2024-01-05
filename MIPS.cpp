#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR  (5)
#define NOR (7)


#define MemSize (65536)

enum OpCode {
    BEQ,
    LW,
    SW,
    ADDIU,
    J,
    JAL,
    RTYPE
};

void removeWinTrail(string& line){
  if (!line.empty() && line[line.size() - 1] == '\r')
    line.erase(line.size() - 1);
}

// doing this as I avoided using <map> STL
class OpCodeAdapter{
  public:
      static string GetAsString(OpCode code) {
        switch (code) {
          case BEQ:
            return "000100";
          case ADDIU:
            return "001001";
          case LW:
            return "100011";
          case SW:
            return "101011";
          case J:
            return "000010";
          case JAL:
            return "000011";
          case RTYPE:
            return "000000";
        }
      }
};

class RF
{
  public:
    bitset<32> ReadData1, ReadData2; 
    RF()
    { 
      Registers.resize(32);  
      Registers[0] = bitset<32> (0);  
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {    
      if(WrtEnable.to_ulong()==1)
      {
        //Write wrtData to wrtRegister
        int wrtAddr = WrtReg.to_ulong();
        Registers[wrtAddr] = WrtData;
      }         
      else {
        int reg1addr = RdReg1.to_ulong();
        ReadData1 = Registers[reg1addr]; 
        int reg2addr = RdReg2.to_ulong();
        ReadData2 = Registers[reg2addr]; 
      }  
    }

    void OutputRF()
    {
      ofstream rfout;
      rfout.open("RFresult.txt",std::ios_base::app);
      if (rfout.is_open())
      {
        rfout<<"A state of RF:"<<endl;
        for (int j = 0; j<32; j++)
        {        
          rfout << Registers[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      rfout.close();

    }     
  private:
    vector<bitset<32>> Registers;
};



class ALU
{
  private:

    bitset<32> ALUOperation(int aluOpr,bitset<32> oprand1, bitset<32> oprand2) {
      int opr1 = (int)(oprand1.to_ulong());
      int opr2 = (int)(oprand2.to_ulong());
      switch (aluOpr)
      {
      case ADDU:
        return bitset<32>(opr1+opr2);
      case SUBU:
        return bitset<32>(opr1-opr2);
      case AND:
        return oprand1&oprand2;
      case OR:
        return oprand1|oprand2;
      case NOR:
        return ~(oprand1|oprand2);
      default:
        return bitset<32>();
      }
    }
  public:

    bitset<32> ALUresult;
    
    bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {   
      int aluOpr = (int)(ALUOP.to_ulong());
      ALUresult = ALUOperation(aluOpr,oprand1,oprand2);
      cout<<"ALU RESULT IS : "<<ALUresult<<"\n";
      return ALUresult;
    }            
};


class INSMem
{
  public:
    bitset<32> Instruction;
    INSMem()
    {      
      IMem.resize(MemSize); 
      ifstream imem;
      string line;
      int i=0;
      imem.open("imem.txt");
      if (imem.is_open())
      {
        while (getline(imem,line))
        {
          removeWinTrail(line);
          IMem[i] = bitset<8>(line);
          i++;
        }

      }
      else cout<<"Unable to open file";
      imem.close();

    }

    bitset<32> ReadMemory (bitset<32> ReadAddress) 
    {    
      int currentAddress = ReadAddress.to_ulong();
      string instr = "";
      for(int i=currentAddress;i<currentAddress+4 && i<IMem.size(); i++) {
        instr+=IMem[i].to_string();
      }
      return bitset<32>(instr);     
    }     

  private:
    vector<bitset<8> > IMem;

};

class DataMem    
{
  public:
    bitset<32> readdata;  
    DataMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("dmem.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {
          removeWinTrail(line);
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open file";
      dmem.close();

    }  
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) 
    {    
      int addressValue = (int)Address.to_ulong();
      if(writemem.to_ulong()==1) { 
        
        string writedataStr = WriteData.to_string();
        cout<<"data "<<writedataStr<<"\n";
        int st = 0;
        for(int i=addressValue;i<addressValue+4 && i<DMem.size();i++, st+=8) {
           bitset<8> res(writedataStr.substr(st,8));
           DMem[i] = res;
        }  
      }
      else if(readmem.to_ulong()==1) {
        string readInst = "";
        for(int i = addressValue;i<addressValue+4 && i<DMem.size();i++) {
          readInst+=DMem[i].to_string();
        }
        readdata = bitset<32>(readInst);
      }
      return readdata;     
    }   

    void OutputDataMem()
    {
      ofstream dmemout;
      dmemout.open("dmemresult.txt");
      if (dmemout.is_open())
      {
        for (int j = 0; j< 1000; j++)
        {     
          dmemout << DMem[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      dmemout.close();

    }             

  private:
    vector<bitset<8>> DMem;

};  

class Decoder {
  public :
    bool isStore(bitset<32> Instruction) {
      //if isLoad is true, read from register and store to memory
      string instructionStr = Instruction.to_string();
      return (instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(SW));
    }

    bool isLoad(bitset<32> Instruction) {
      //if isLoad is true, read from Memory and store to register
      string instructionStr = Instruction.to_string();
      return (instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(LW));
    }

    bool isRType(bitset<32> Instruction) {
      string instructionStr = Instruction.to_string();
      return (instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(RTYPE));
    }

    bool isJType(bitset<32> Instruction) {
      string instructionStr = Instruction.to_string();
      return (
              instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(J) ||
              instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(JAL)
              );
    }

    bool isIType(bitset<32> Instruction) {
      string instructionStr = Instruction.to_string();
      return (
              instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(BEQ) ||
              instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(LW) ||
              instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(SW) ||
              instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(ADDIU)
      );
    }

    bool isBranch(bitset<32> Instruction) { 
      //Only consider beq for the lab
      string instructionStr = Instruction.to_string();
      return (instructionStr.substr(0,6) == OpCodeAdapter::GetAsString(BEQ));
    }

    bool wrtEnable(bitset<32> Instruction) {
      //This tracks if we need to write to register or not
      //We write only when ALU operations results need to be stored to RF
      //We donot write if we are branching or jumping or isStore is enabled (isStore means writing register to memory)
      if(isStore(Instruction) || isJType(Instruction) || isBranch(Instruction))
        return false;
      return true;  
    }

    bool isBranchEqual(bitset<32> Instruction, RF &rg) {
      //Assuming the instrunction is I-type
      string instructionStr = Instruction.to_string();
      bitset<5> reg1(instructionStr.substr(6,5));
      bitset<5> reg2(instructionStr.substr(11,5));
      //if data at register stored at operand 1 == operand 2
      rg.ReadWrite(reg1, reg2, bitset<5>(), bitset<32>(), bitset<1>("0"));

      return rg.ReadData1 == rg.ReadData2;
    }

    bitset<32> calculateBranchingPC(bitset<32> Instruction, bitset<32> PC) { 
      //{SignExtend, imm, 00} 
      int branchAddress = bitset<18>(Instruction.to_string().substr(16,16) + "00").to_ulong();
      int PCvalue = calculateNextPC(PC).to_ulong();
      return bitset<32>(branchAddress+PCvalue);
    }

    bitset<32> calculateJumpPC(bitset<32> Instruction, bitset<32> PC) {
      //PC+4[31:28], address, 2’b0
      //take 4 most significant bits from PC+4 + address(26) + 00(*4 to make it byte compatible)
      string PCstr = calculateNextPC(PC).to_string().substr(0,4);
      string address = Instruction.to_string().substr(6,26);
      return bitset<32>(PCstr+address+"00");
    }

    bitset<32> calculateNextPC(bitset<32> PC) { 
      //This corresponds to every 8 bit line in Imem 
      int pcValue = (int)PC.to_ulong();
      pcValue += 4;
      return bitset<32>(pcValue);
    }

    bitset<32> nextPC(bitset<32> Instruction, bitset<32> PC, RF rg) {
      //3 cases - branch/jump/nextPC
      if(isBranch(Instruction) && isBranchEqual(Instruction,rg))
        return calculateBranchingPC(Instruction,PC);
      if(isJType(Instruction))
        return calculateJumpPC(Instruction,PC);
      return calculateNextPC(PC);
    }

    string decodeInstruction(bitset<32> Instruction) {
      //decode the type of Instruction
      if(isLoad(Instruction))
        return "lw";
      if(isStore(Instruction))
        return "sw";
      if(isBranch(Instruction))
        return "beq";
      if(isJType(Instruction))
        return "j";
      return "alu";
    } 

    void loadword(bitset<32> Instruction, RF &myRF, DataMem &myDataMem, bitset<5> rt, bitset<5> rs, bitset<16> immediate) {
       bitset<32> targetAddress(immediate.to_ulong() + myRF.ReadData1.to_ulong());
       bitset<32> writeData = myDataMem.MemoryAccess(targetAddress, bitset<32>(), bitset<1>(1), bitset<1>(0));
       myRF.ReadWrite(rs, rt, rt, writeData, bitset<1>(1));
    }

    void storeword(bitset<32> Instruction, RF &myRF, DataMem &myDataMem, bitset<5> rt, bitset<5> rs, bitset<16> immediate) {
       bitset<32> targetAddress(immediate.to_ulong() + myRF.ReadData1.to_ulong());
       bitset<32> writeData = myDataMem.MemoryAccess(targetAddress, myRF.ReadData2, bitset<1>(0), bitset<1>(1));
    }

};


int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;
  Decoder decoder;
  //empty contents of result files - 
  remove("RFResult.txt");
  remove("dmemresult.txt");
  bitset<32> PC(0);
  int cycle=1;
  while (1)  
  {
    printf("Cycle %d: ", cycle++);
    //stage 1 - fetch instruction from myInsMem
    bitset<32> instruction = myInsMem.ReadMemory(PC);
    if(instruction.to_string()=="11111111111111111111111111111111"){
      printf("Exit");
      //myRF.OutputRF();
      break;
    }

    //stage 2 - decode the instruction and read/write to register
    string instructionType = decoder.decodeInstruction(instruction);
    printf("Type %s\n", &instructionType);
    if(decoder.isIType(instruction))
    {
      bitset<5> rt(instruction.to_string().substr(11,5));
      bitset<5> rs(instruction.to_string().substr(6,5));
      bitset<16> immediate(instruction.to_string().substr(16,16));
      myRF.ReadWrite(rs, rt, bitset<5>(), bitset<32>(), bitset<1>(0));
      if(instructionType == "lw")
        decoder.loadword(instruction,myRF,myDataMem,rt,rs,immediate);
      else if(instructionType == "sw")
        decoder.storeword(instruction,myRF,myDataMem,rt,rs,immediate);
      else if (instructionType == "alu") {
        bitset<3> ALUop(instruction.to_string().substr(3,3));
        bitset<32> oprand2data(immediate.to_ulong());
        if(immediate.to_string()[0]=='1') {
          oprand2data = bitset<32>("1111111111111111" + immediate.to_string());
        }
        myALU.ALUOperation(ALUop, myRF.ReadData1, oprand2data);
        //write alu result to rf
        myRF.ReadWrite(rs,rt,rt,myALU.ALUresult,bitset<1>(1));
      }
    }
    else if(decoder.isRType(instruction)) {
      bitset<3> ALUop(instruction.to_string().substr(29,3));
      bitset<5> rd(instruction.to_string().substr(16,5));
      bitset<5> rt(instruction.to_string().substr(11,5));
      bitset<5> rs(instruction.to_string().substr(6,5));
      myRF.ReadWrite(rs, rt, bitset<5>(), bitset<32>(), bitset<1>(0));
      myALU.ALUOperation(ALUop, myRF.ReadData1, myRF.ReadData2);
      myRF.ReadWrite(rs,rt,rd,myALU.ALUresult,bitset<1>(1));
    }
    //if it is jump or beq, we just increment PC - no operation
    PC = decoder.nextPC(instruction, PC, myRF);

    /**** You don't need to modify the following lines. ****/
    myRF.OutputRF(); // dump RF;    
  }
  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
