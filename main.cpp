#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "TIHex.h"

#define TEMP_BUFFER_SIZE 1024

void showHelp();
void showVersion();

int main(int argc, char *argv[]){
  if(argc>-1){
    bool stdinEnabled = false;
    bool stdoutEnabled = false;
    std::string filename = "";
    std::list<std::pair<TIHex::Address,std::vector<uint8_t>>> newDataList; // list of pairs <address,bytes list>
    TIHex::Address lastAddress = 0;
    bool addressSet = false;
    for (int i = 1; i < argc; i++)
    {
      std::string arg = argv[i];
      if(arg == "-i" || arg == "--stdin") stdinEnabled = true;
      else if(arg == "-o" || arg == "--stdout") stdoutEnabled = true;
      else if(arg == "-a" || arg == "--address"){
        if(i+1 < argc){
          try
          {
            lastAddress = std::stoull(argv[i+1],nullptr,16);
            addressSet = true;
          }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << ": on " << argv[i+1] << '\n';
            return -1;
          }
          
          i++; // Move forward on arguments.
        }
        else{
          std::cout << "Address switch must have a hexadecimal value as following argument." << std::endl;
          showHelp();
          return -1;
        }
      }
      else if(arg == "-d" || arg == "--data"){
        if(i+1 < argc){
          std::istringstream values(argv[i+1]);
          newDataList.emplace_back();
          auto& newData = newDataList.back();
          newData.first = lastAddress;
          std::vector<std::string> valuesStrings;
          std::string s;
          // Split values on strings according to separator ','
          while (std::getline(values, s, ',')) {
              valuesStrings.push_back(s);
              lastAddress++;
          }
          //std::cout << "Appending data:";
          for(auto& s : valuesStrings){
            try
            {
              auto v = std::stoul(s,nullptr,16);
              if(v > 255){
                std::cout << s << "is greater than 0xFF. Use byte values only." << std::endl;
                return -1;
              }
              newData.second.push_back(v);
              //std::cout << v << " ";
            }
            catch(const std::exception& e)
            {
              std::cerr << e.what() << ": on " << s << '\n';
              return -1;
            }
          }
          //std::cout << '\n';
          i++; // Move forward on arguments
        }
        else{
          std::cerr << "Data switch must have a comma separated list as following argument." << std::endl;
          showHelp();
          return -1;
        }
      }
      else if(arg == "-h" || arg == "--help"){
        showHelp();
        return 0;
      }
      else if(arg == "-v" || arg == "--version"){
        showVersion();
        return 0;
      }
      else{
        filename = arg;
      }
    }
    //std::cout << std::endl;
    
    // Get HEX data
    TIHex hex;
    if(stdinEnabled){
      std::string line;
      uint64_t lineNumber = 0;
      while(!std::cin.eof()){
        lineNumber++;
        std::getline(std::cin,line);
        if( (line.size() && (line[0] == '\n' || line[0] == '\r')) || !line.size()) continue; // Skips empty lines
        if(!hex.append(line))
        {
          std::cerr << "Error '" << hex.errorString() << "' while parsing line " << lineNumber << ": '" << line << "'\n";
          return -1;
        }
      }
      //std::cout << std::endl;
    }
    else if(filename > ""){
      std::fstream file(filename,file.in);
      if(!file.is_open()){
        std::cerr << "Error '" << std::strerror(errno) << "' while opening file: " << filename << std::endl;
        return errno;
      }
      else{
        std::string line;
        uint64_t lineNumber = 0;
        while(!file.eof()){
          lineNumber++;
          std::getline(file,line);
          if( (line.size() && (line[0] == '\n' || line[0] == '\r')) || !line.size()) continue; // Skips empty lines
          if(!hex.append(line))
          {
            std::cerr << "Error '" << hex.errorString() << "' while parsing line " << lineNumber << ": '" << line << "'\n";
            return -1;
          }
        }
        //std::cout << std::endl;
        file.close();
      }
    }

    // Process HEX
    if(addressSet){
      for(auto&pair : newDataList){
        auto address = pair.first;
        for(auto &v : pair.second){
          if(!hex.overwrite(address,v)){
            std::cerr << "Data address " << std::hex << address << " could not be overwritten." << std::endl;
            return -1;
          }
          address++;
        }
      }
    }

    // Send to stdout?
    if(stdoutEnabled)
    {
      char temp[8];
      for(auto it = hex.begin(); it != hex.end(); it++){
        //hex.fixChecksum(it);
        snprintf(temp,sizeof(temp),"%.2X",it->byteCount);
        std::cout << it->startCode << temp;
        snprintf(temp,sizeof(temp),"%.4X",it->address);
        std::cout << temp;
        snprintf(temp,sizeof(temp),"%.2X",it->recordType);
        std::cout << temp;
        auto dataSize = it->data.size();
        for(int i=0; i<dataSize; i++){
          snprintf(temp,sizeof(temp),"%.2X",it->data[i]);
          std::cout << temp;
        }
        snprintf(temp,sizeof(temp),"%.2X",it->checksum);
        std::cout << temp << '\n';
      }
    }
  }
  else{
    showHelp();
  }
  return 0;
}

void showHelp(){
  std::cout << "Textual Intel Hex Editor" << '\n';
  std::string ar = "@";
  std::cout << "Original author: Fabricio Ribeiro Toloczko (fabriciotoloczko" << ar << "gmail.com)" << '\n';
  std::cout << "file processing:\ttihex [options] [input filename]" << '\n';
  std::cout << "stdin:\t\t\ttihex [options] -i " << '\n';
  std::cout << "--help or -h: this help message." << '\n';
  std::cout << "--stdin or -i: process stdin." << '\n';
  std::cout << "--stdout or -o: show final data on stdout." << '\n';
  std::cout << "--address or -a: set address to overwrite, hexadecimal 0 to FFFFFFFFFFFFFFFF. E.g. \"-a EAF00F1\"." << '\n';
  std::cout << "--data or -d: define data, hex values comma separated. E.g. \"-d 0,0,1a,95,AB\"." << '\n';
  std::cout << "--version or -v: show version." << std::endl;
}

void showVersion(){
  std::cout << "Textual Intel Hex Editor" << '\n';
  std::cout << "Version: " << GIT_COMMIT_DATE << '\n';
  std::cout << "Hash: " << GIT_COMMIT_HASH << '\n';
  std::cout << "Branch: " << GIT_BRANCH << std::endl;
}
