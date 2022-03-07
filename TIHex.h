#ifndef TIHEX_H
#define TIHEX_H

/**
 * @file TIHex.h
 * @author Fabricio Ribeiro Toloczko
 * @brief Edit Intel HEX files trying not to change their layout.
 * @version 1.0
 * @date 2022-03-05
 * 
 * @copyright Copyright (c) 2022
 * License:
 *    Copyright (c) 2022 Fabrício Ribeiro Toloczko
 *
 *    This software is provided 'as-is', without any express or implied warranty. 
 *    In no event will the authors be held liable for any damages arising from the 
 *    use of this software.
 *
 *    Permission is granted to anyone to use this software for any purpose, 
 *    including commercial applications, and to alter it and redistribute it freely,
 *    subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not claim
 *    that you wrote the original software. If you use this software in a product, 
 *    an acknowledgment in the product documentation would be appreciated but is 
 *    not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source distribution.
 * 
 */

#include <cstdint>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <limits>

class TIHex
{
public:
    TIHex();
    ~TIHex();

    /* Maximum jump between two address lines */
    uint16_t TIHEX_ADDRESS_MAX_JUMP = 65535; // 65535: no limit.

    struct Entry
    {
        // Start code   Byte count   Address   Record type   Data   Checksum
        char startCode;
        uint8_t byteCount;
        uint16_t address;
        uint8_t recordType;
        std::vector<uint8_t> data;
        uint8_t checksum;
    };

    typedef uint64_t Address;

    enum class Error
    {
        None,                 // No errors.
        Malformed,            // Line could not be processed.
        InvalidJumpSize,      // Exceeded jump between to lines.
        InvalidDataSize,      // Data size mismatch between line header and actual found size.
        Checksum,             // Checksum failed.
        AddressNotFound,      // Address not found.
        LowerAddressNotFound, // Address lower value not found.
        UpperAddressNotFound, // Address upper value not found.
        Overflow,             // 64 bit addressing overflown. In this case, please, chop the data using two or more TIHex objects.

        Unknown
    };

    typedef std::list<Entry>::iterator iterator;

    /**
     * @brief Append a complete line assuming correct address ordering.
     * No data will be appended if it is not well formed. Check error() to get the error.
     *
     * @param line string containing one complete Intel Hex format line.
     * @return true when everything has gone fine, false to anything else.
     */
    bool append(const std::string &line);

    /**
     * @brief STL iterator. Get first entry iterator.
     * Use iterator->first to get Address value and iterator->second to get Entry data.
     *
     * @return begin iterator.
     */
    iterator begin() { return __entryList.begin(); }

    /**
     * @brief Clear all appended data. Current address is going to reset to zero.
     *
     */
    void clear()
    {
        __addressPointer = 0;
        __programCounter = 0;
        __entryMap.clear();
        __entryList.clear();
    }

    /**
     * @brief Check if it contains an address or not.
     *
     * @param address
     * @return true when address is present, false otherwise.
     */
    bool contains(const Address address);

    /**
     * @brief Address to insert new data.
     *
     * @return Address.
     */
    Address currentAddress() { return __addressPointer; }

    /**
     * @brief Check if entry map is empty.
     *
     * @return true if it is empty, false otherwise.
     */
    bool empty() { return __entryMap.empty(); }

    /**
     * @brief STL iterator. Get end iterator.
     * This indicates the after last entry iterator.
     * Use (--iterator) to get the last entry.
     *
     * @return after last iterator.
     */
    iterator end() { return __entryList.end(); }

    /**
     * @brief Get last error occurred.
     *
     * @return Error code.
     */
    Error error() { return __error; }

    /**
     * @brief Get last error message.
     *
     * @return Error message.
     */
    const std::string errorString();

    /**
     * @brief Calculate the checksum based on entry reference.
     * Check error() if returned false.
     *
     * @param address
     * @return true if checksum was fixed. False if an error occurred.
     */
    bool fixChecksum(Entry &entry);

    /**
     * @brief Get value by address. Use overwrite() to set value.
     * Check error().
     *
     * @param address
     * @return value data.
     */
    uint8_t getValue(Address address);

    /**
     * @brief Get entry address with a value lower than provided.
     * E.g. entry address list = {0x1000,0x1500}, lowerAddress(0x1500) is going to return 0x1000.
     * Check error() to know if it was found.
     *
     * @param address to compare.
     * @return Address lower than provided. If not found, Address minimum value is returned.
     */
    Address lowerAddress(const Address address);

    /**
     * @brief Overwrite data at desired address.
     *
     * @param address to overwrite
     * @param data that will be overwritten
     * @return true when overwrite was done. False otherwise.
     */
    bool overwrite(const Address address, uint8_t &data,bool calculateChecksum = true);

    /**
     * @brief Get entry by address.
     * Use contain() to check if address exists, otherwise it throws an exception.
     *
     * @param address Address to access entry.
     * @return reference to entry.
     */
    Entry& operator[](const Address address){
        if(!contains(address)){
            throw "invalid address";
        }
        return *__entryMap[address];
    }

    /**
     * @brief Get program size. Includes all data entries, i.e. only recordType equals to 0x00.
     * @return program size.
     */
    uint64_t programSize(){return __programCounter;}

    /**
     * @brief Get entries size. Includes all non-data entries, i.e. recordType different from 0x00.
     * @return entries list size.
     */
    uint64_t size(){return __entryList.size();}

    /**
     * @brief Get entry address with a value greater than provided.
     * E.g. entry address list = {0x1000,0x1500}, upperAddress(0x1000) is going to return 0x1500.
     * Check error() to know if it was found.
     *
     * @param address to compare.
     * @return Address greater than provided. If not found, Address maximum value is returned.
     */
    Address upperAddress(const Address address);

private:
    /* __entryMap stores all references to entries, according to each header's address */
    std::map<Address, Entry *> __entryMap;

    /* All entries data are stored in __entryList, including it's original order */
    std::list<Entry> __entryList;

    Error __error;
    Address __addressPointer = 0;
    uint64_t __programCounter = 0;
};

#endif
