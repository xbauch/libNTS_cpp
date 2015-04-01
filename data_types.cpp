#include "data_types.hpp"

using namespace nts;

DataType::DataType ( Type t, unsigned int bw) :
	_type ( t ),
	_bitwidth ( bw )
{
	;
}

DataType DataType::Integer()
{
	return DataType ( Type::Integer, 0 );
}

DataType DataType::Real()
{
	return DataType ( Type::Real, 0 );
}

DataType DataType::Integral()
{
	return DataType ( Type::Integral, 0 );
}

DataType DataType::BitVector ( unsigned int bw )
{
	return DataType ( Type::BitVector, bw );
}

DataType DataType::Bool ()
{
	return BitVector ( 1 );
}

bool DataType::is_integral() const
{
	return _type != Type::Real;
}

bool DataType::is_bitvector() const
{
	return _type == Type::BitVector;
}

bool DataType::operator== ( const DataType & rhs ) const
{
	return _type == rhs._type && _bitwidth == rhs._bitwidth;
}

bool DataType::operator!= ( const DataType & rhs ) const
{
	return ! (*this == rhs);
}


