#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "ripemd_160.h"

string Ripemd160::encode(const string &input) {
    vector<byte> inputBytes(0);
    for (char i : input) {
        inputBytes.push_back(i);
    }
    vector<byte> messageBytes(inputBytes);
    messageBytes = expanseInput(inputBytes);
    vector<uint> message(messageBytes.size() / UINT_BYTE_SIZE);
    for (int i = 0; i < messageBytes.size(); i++) {
        message[i / UINT_BYTE_SIZE] |= ((uint) messageBytes[i])
                << (i % 4) * 8;
    }
    vector<uint> resultUint = hash(message);
    std::stringstream stream;
    for (uint i = 0; i < resultUint.size() * 4; i++) {
        byte byte_value = static_cast<int8_t>(uRightShift(resultUint[i / 4], i % 4 * 8));
        stream << std::setfill('0') << std::setw(2) << std::hex << (byte_value & 0xff);
    }
    return stream.str();
}

vector<byte> Ripemd160::expanseInput(const vector<byte>& input) {
    vector<byte> bytes(input);
    bytes.push_back(0x80);
    uint32_t padding = BLOCK_LENGTH - (bytes.size() % BLOCK_LENGTH);
    if (padding < 8) {
        padding += BLOCK_LENGTH;
    }
    bytes.resize(bytes.size() + padding - 8, uint(0x0));

    const uint64_t bit_length = 8 * input.size();
    for (uint32_t i = 0; i < 8; ++i) {
        bytes.push_back(bit_length >> (8 * i));
    }
    return bytes;
}

uint Ripemd160::cycleRolLeft(uint word, uint amount) {
    while (amount > sizeof(uint) * 8) {
        amount -= sizeof(uint) * 8;
    }
    while (amount < 0) {
        amount += sizeof(uint) * 8;
    }
    uint helper = (word >> (sizeof(uint) * 8 - amount));
    word <<= amount;
    word |= helper;
    return word;
}

uint Ripemd160::uRightShift(const uint &base, uint shift) {
    while (shift > sizeof(uint) * 8) {
        shift -= sizeof(uint) * 8;
    }
    while (shift < 0) {
        shift += sizeof(uint) * 8;
    }
    return (uint) base >> shift;
}

uint Ripemd160::f(uint j, uint x, uint y, uint z) {
    uint result;
    switch (j) {
        case 1:
            result = x ^ y ^ z;
            break;
        case 2:
            result = (x & y) | (~x & z);
            break;
        case 3:
            result = (x | ~y) ^ z;
            break;
        case 4:
            result = (x & z) | (y & ~z);
            break;
        case 5:
            result = x ^ (y | ~z);
            break;
        default:
            throw std::invalid_argument("Unexpected argument: " + std::to_string(j));
    }
    return result;
}


vector<uint> Ripemd160::hash(vector<uint> input) {
    uint T;
    uint _h0 = h0, _h1 = h1, _h2 = h2, _h3 = h3, _h4 = h4;
    for (int i = 0; i < input.size(); i += BLOCK_WORDS_LENGTH) {
        uint A = _h0, A_ = _h0;
        uint B = _h1, B_ = _h1;
        uint C = _h2, C_ = _h2;
        uint D = _h3, D_ = _h3;
        uint E = _h4, E_ = _h4;
        for (int j = 0; j < 80; j++) {
            T = cycleRolLeft(A + f(j / BLOCK_WORDS_LENGTH + 1, B, C, D) + input[i * BLOCK_WORDS_LENGTH + r[j]] + k[j / BLOCK_WORDS_LENGTH], s[j]) + E;
            A = E, E = D, D = cycleRolLeft(C, 10), C = B, B = T;
            T = cycleRolLeft(A_ + f(5 - j / BLOCK_WORDS_LENGTH, B_, C_, D_) + input[i * BLOCK_WORDS_LENGTH + r_[j]] + k_[j / BLOCK_WORDS_LENGTH], s_[j]) + E_;
            A_ = E_, E_ = D_, D_ = cycleRolLeft(C_, 10), C_ = B_, B_ = T;
        }
        T = _h1 + C + D_;
        _h1 = _h2 + D + E_;
        _h2 = _h3 + E + A_;
        _h3 = _h4 + A + B_;
        _h4 = _h0 + B + C_;
        _h0 = T;
    }
    return vector<uint>({_h0, _h1, _h2, _h3, _h4});
}