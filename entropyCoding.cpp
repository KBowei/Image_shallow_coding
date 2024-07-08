#include <fstream>
#include <string>
#include <math.h>
#include "entropyCoding.h"

using namespace std;

void BitOutputStream::write(char b)
{
	//cout << int(b) << " ";
	countBit++;
	//std::cout << b;
	if (b != 0 and b != 1) throw "Argument must be 0 or 1";
	m_currentbyte = (m_currentbyte << 1) | b;
	m_numbitsfilled += 1;
	if (m_numbitsfilled == 8) {
		const char* towrite = &m_currentbyte;
		m_bit_out.write(towrite, 1);
		m_currentbyte = 0;
		m_numbitsfilled = 0;
	}
}

void BitOutputStream::close()
{
	while (m_numbitsfilled != 0)
		write(0);

	m_bit_out.close();
}

int BitInputStream::read()
{
	countBit++;
	if (m_currentbyte == -1) return -1;

	if (m_numbitsremaining == 0)
	{
		unsigned char temp;
		if (m_bit_in.eof())
		{
			m_currentbyte = -1;
			return -1;
		}
		m_bit_in.read((char*)&temp, 1);
		m_currentbyte = (int)temp;
		m_numbitsremaining = 8;
	}
	assert(m_numbitsremaining > 0);
	m_numbitsremaining -= 1;
	return (m_currentbyte >> m_numbitsremaining) & 1;
}

int BitInputStream::read_no_eof()
{
	int result = read();
	if (result != -1){
		return result;
	}
	else {
		throw("EOFError");
	}

}

void BitInputStream::close()
{
	m_bit_in.close();
	m_currentbyte = -1;
	m_numbitsremaining = 0;
}

void PositionCoding::expGolombCoding(int number, int kOrder)
{
	int codingNumber = number + pow(2, kOrder) - 1;

	// zero order exp golomb coding
	int m = floor(log2(codingNumber + 1));
	for (int i = kOrder; i < m; i++) {
		m_c_bit_out.write(0);
	}
	m_c_bit_out.write(1);

	int offset = codingNumber + 1 - pow(2, m);
	for (int i = m - 1; i >= 0; i--) {
		char bit = ((offset & (1 << i))) != 0;
		m_c_bit_out.write(bit);
	}
}

void PositionCoding::write(char bit, char sign)
{
	assert (bit == 0 || bit == 1);
	if (m_count0 > 1 && bit == 0) {
		m_count0++;
	}
	if (m_count0 > 1 && bit == 1) {
		expGolombCoding(m_count0 - 2, 1);
		m_count0 = 0;
		m_c_bit_out.write(sign);
	}
	else {
		if (m_count0 < 2) {
			m_c_bit_out.write(bit);
			if (bit == 0) {
				m_count0++;
			}
			else {
				m_c_bit_out.write(sign);
				m_count0 = 0;
			}
		}
	}
}

void PositionCoding::tailCoding()
{
	if (m_count0 > 2)
	{
		expGolombCoding(m_count0 - 2, 1);
		m_count0 = 0;
	}
	else if (m_count0 <= 2)
	{
		m_count0 = 0;
	}
}

int PositionDecoding::expGolombDecoding(int kOrder)
{
	int m = 0;
	while (!m_c_bit_in.read()) {
		m++;
	}
	m += kOrder;

	int offset = 0;
	for (int i = m - 1; i >= 0; i--) {
		int bit = m_c_bit_in.read();
		offset += bit << i;
	}

	return pow(2, m) + offset -  pow(2, kOrder);
}

void PositionDecoding::read(int32_t& bit, char& sign)
{
	if (m_count0 < 2) {
		bit = m_c_bit_in.read();
		if (bit == 0) {
			m_count0++;
		}
		else {
			sign = m_c_bit_in.read();
			m_count0 = 0;
		}
	}
	else {
		if (m_remain0 == 0 && m_count0 <= 2) {
			m_remain0 = expGolombDecoding(1);
		}
		if (m_remain0 == 0) {
			bit = 1;
			sign = m_c_bit_in.read();
			m_count0 = 0;
		}
		if (m_remain0 > 0) {
			m_remain0--;
			bit = 0;
			m_count0++;
		}
	}
	
}

void PositionDecoding::tailDecoding()
{
	m_count0 = 0;
	m_remain0 = 0;
}