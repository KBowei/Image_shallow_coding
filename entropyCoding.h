
#ifndef __ENTROPYCODING__
#define __ENTROPYCODING__

#include<iostream>
#include<string>
#include<fstream>
#include <assert.h>

class BitOutputStream
{
public:
	int countBit;
	BitOutputStream(std::fstream& file) : m_bit_out(file), m_currentbyte(0), m_numbitsfilled(0), countBit(32) {}; // 2 byte�� height width
	void write(char);
	void close();

private:
	char m_currentbyte; // The accumulated bits for the current byte, always in the range [0x00, 0xFF]
	int m_numbitsfilled; // Number of accumulated bits in the current byte, always between 0 and 7 (inclusive)
	std::fstream& m_bit_out;
};


class BitInputStream
{
public:
	int countBit;
	BitInputStream(std::fstream& file) : m_bit_in(file), m_currentbyte(0), m_numbitsremaining(0) {};
	int read();
	int read_no_eof();
	void close();

private:
	int m_currentbyte; // The accumulated bits for the current byte, always in the range [0x00, 0xFF]
	int m_numbitsremaining; // Number of accumulated bits in the current byte, always between 0 and 7 (inclusive)
	std::fstream& m_bit_in;
};	

class PositionCoding
{
public:
	PositionCoding(BitOutputStream& c_bit_out) : m_c_bit_out(c_bit_out), m_count0(0) {};
	void expGolombCoding(int number, int kOrder);
	void write(char bit, char sign);
	void tailCoding();
private:
	int m_count0;
	BitOutputStream& m_c_bit_out;
};

class PositionDecoding
{
public:
	PositionDecoding(BitInputStream& c_bit_in) : m_c_bit_in(c_bit_in), m_count0(0), m_remain0(0) {};
	int expGolombDecoding(int kOrder);
	void read(int32_t& bit, char& sign);
	void tailDecoding();
private:
	int m_count0;
	BitInputStream& m_c_bit_in;
	int m_remain0;
};
#endif