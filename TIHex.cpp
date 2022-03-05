#include "TIHex.h"

TIHex::TIHex()
{
}

TIHex::~TIHex()
{
}
bool TIHex::append(const std::string &line) {
  //Start code   Byte count   Address   Record type   Data   Checksum
  int lineSize = line.size();
  int p;
  // Discard leading spaces or tabs or ':'
  for (p = 0; line[p] == ' ' || line[p] == '\t' || line[p] == ':'; p++);
  // Check if there is at least characters for 5 bytes.
  if(p == lineSize || (lineSize - p < 5*2)){
    __error = Error::Malformed;
    return false;
  }
  std::string byteCountStr = line.substr(p,2); p+=2;
  std::string addressStr = line.substr(p,4); p+=4;
  std::string recordTypeStr = line.substr(p,2); p+=2;

  uint16_t entryAddress = 0;
  try{
    entryAddress = std::stoul(addressStr,nullptr,16);
  }
  catch(const std::exception& e) {
    __error = Error::Malformed;
    return false;
  }
  if(static_cast<uint16_t>(__addressPointer) != entryAddress){
    // Entry address is different from previous calculation.
    Address newAddressPointer = __addressPointer & ~(0xFFFF); // __addressPointer with cleared lower 16 bits.
    newAddressPointer |= entryAddress; // Define lower 16 bits with new entry.
    __addressPointer = newAddressPointer;
  }
  __entryList.emplace_back(); // Append new element.
  auto& entry = __entryList.back(); // Get reference.

  entry.startCode = ':';

  try{
    entry.byteCount = std::stoul(byteCountStr,nullptr,16);
  }
  catch(const std::exception& e) {
      __entryList.pop_back();
    __error = Error::Malformed;
    return false;
  }
  
  // Check if there is enough data
  if(2*(entry.byteCount+1) > lineSize-p){
      __entryList.pop_back();
    __error = Error::Malformed;
    return false;
  }
  
  entry.address = entryAddress;
  if(entry.address > TIHEX_ADDRESS_MAX_JUMP){
      __entryList.pop_back();
    __error = Error::InvalidJumpSize;
    return false;
  }
  try{
    entry.recordType = std::stoul(recordTypeStr,nullptr,16);
  }
  catch(const std::exception& e) {
      __entryList.pop_back();
    __error = Error::Malformed;
    return false;
  }

  std::string dataStr = line.substr(p,2*entry.byteCount); p+=2*entry.byteCount;
  auto dataByteCount = dataStr.size()/2;
  if(dataByteCount != entry.byteCount){
      __entryList.pop_back();
    __error = Error::InvalidDataSize;
    return false;
  }
  entry.data.reserve(dataByteCount); // Preallocate memory.
  for(int i=0; i<dataStr.size(); i+=2)
  {
    auto s = dataStr.substr(i,2);
    uint8_t value;
    try{
      value = std::stoul(s,nullptr,16);
    }
    catch(const std::exception& e) {
      __entryList.pop_back();
      __error = Error::Malformed;
      return false;
    }
    entry.data.push_back(value);
  }

  std::string checksumStr = line.substr(p,2); p+=2;

  try{
    entry.checksum = std::stoul(checksumStr,nullptr,16);
  }
  catch(const std::exception& e) {
      __entryList.pop_back();
    __error = Error::Malformed;
    return false;
  }

  Address newAddressPointer = __addressPointer;

  // Process record types
  if(entry.recordType == 0x00){
    __entryMap[__addressPointer] = &entry; // Add reference into map.
    newAddressPointer += entry.byteCount;
    if(newAddressPointer < __addressPointer){
      __entryMap.erase(__addressPointer);
      __entryList.pop_back();
      __error = Error::Overflow;
      return false;
    }
  }
  else if(entry.recordType == 0x02){
    // Extended Segment Address
    auto dataSize = entry.data.size();
    uint64_t addressOffset = 0;
    for(int i=0; i < dataSize; i++){
      addressOffset <<= 8;
      addressOffset |= entry.data[i];
    }
    addressOffset <<= 4;
    newAddressPointer = addressOffset;
  }
  else if(entry.recordType == 0x04){
    // Extended Linear Address
    auto dataSize = entry.data.size();
    uint64_t upperAddress = 0;
    for(int i=0; i < dataSize; i++){
      upperAddress <<= 8;
      upperAddress |= entry.data[i];
    }
    upperAddress <<= 32;
    newAddressPointer = upperAddress;
  }
  else{
    // End Of File or Start Segment Address or Start Linear Address
    newAddressPointer += entry.byteCount;
    if(newAddressPointer < __addressPointer){
      __entryList.pop_back();
      __error = Error::Overflow;
      return false;
    }
  }
  __addressPointer = newAddressPointer;
  __error = Error::None;
  return true;
}

bool TIHex::contains(const Address &address){
    try
    {
        __entryMap.at(address);
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}

const std::string TIHex::errorString() {
    switch (__error)
    {
      case Error::None:
        return "None";
      case Error::Malformed:
        return "Malformed";
      case Error::InvalidJumpSize:
        return "InvalidJumpSize";
      case Error::InvalidDataSize:
        return "InvalidDataSize";
      case Error::Checksum:
        return "Checksum";
      case Error::AddressNotFound:
        return "AddressNotFound";
      case Error::LowerAddressNotFound:
        return "LowerAddressNotFound";
      case Error::UpperAddressNotFound:
        return "UpperAddressNotFound";
      case Error::Overflow:
        return "Overflow";

      default:
      return "Unknown";
    }
}

bool TIHex::fixChecksum(Entry &entry) {
  int sum = 
    entry.byteCount+
    (entry.address >> 8)+
    (entry.address & 0xFF)+
    entry.recordType;
  auto dataSize = entry.data.size();
  for(int i = 0; i<dataSize; i++) sum += entry.data[i];
  entry.checksum = (~sum) + 1; // Two's complement
  __error = Error::None;
  return true;
}

TIHex::Address TIHex::lowerAddress(const Address &address) {
    auto it = __entryMap.lower_bound(address);
    if(it == __entryMap.begin()){
      __error = Error::LowerAddressNotFound;
      return std::numeric_limits<Address>::min();
    }
    it--;
    __error = Error::None;
    return it->first;
}

bool TIHex::overwrite(const Address &address, uint8_t &byte,bool calculateChecksum){
  auto it = __entryMap.upper_bound(address);

  // Let's choose the right iterator, if it exists.
  if(it == __entryMap.end()){
    // Over than last address.
    if(__entryMap.size()){
      it--; // Get last entry
    }
    else return false;
  }
  else it--;

  // Checking the offset, if it exists.
  auto offset = address - it->first;
  auto& data = it->second->data;
  if(offset >= data.size()) return false; // There is no data to overwrite.

  // Overwrite data.
  data[offset] = byte;
  if(calculateChecksum) fixChecksum(*it->second);

  return true;
}

TIHex::Address TIHex::upperAddress(const Address &address) {
    auto it = __entryMap.upper_bound(address);
    if(it == __entryMap.end()){
      __error = Error::UpperAddressNotFound;
      return std::numeric_limits<Address>::max();
    }
    __error = Error::None;
    return it->first;
}
