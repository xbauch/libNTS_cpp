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

void DataType::print ( std::ostream &o ) const
{
	switch ( _type )
	{
		case Type::Integer:
			o << "Int";
			break;

		case Type::Real:
			o << "Real";
			break;

		case Type::BitVector:
			o << "BitVector<" << _bitwidth << '>';
			break;

		// Probably not used - there should be no variable of type Integral
		case Type::Integral:
			o << "Integral";
			break;

		default:
			throw TypeError();
	}
}

bool nts::coerce ( const DataType & t1, const DataType & t2, DataType & result ) noexcept
{
	// Same types
	if ( t1 == t2 )
	{
		result = t1;
		return true;
	}

	// t1 can be whatever type of class Integral (probably constant)
	// t2 is some concrete type of class Integral,
	// or whatever type of class Integral
	if ( t1 == DataType::Integral() && t2.is_integral() )
	{
		result = t2;
		return true;
	}

	// Commutatively
	if ( t2 == DataType::Integral() && t1.is_integral() )
	{
		result = t1;
		return true;
	}

	// Both are BitVectors, but have different size (because t1 != t2)
	if ( t1.is_bitvector() && t2.is_bitvector() )
	{
		result = DataType::BitVector ( std::max ( t1.bitwidth(), t2.bitwidth() ) );
		return true;
	}

	return false;
}

DataType nts::coerce ( const DataType & t1, const DataType & t2 )
{
	// DataType does not have zero parameter constructor
	// (and imho DataType should not have it)
	DataType t = DataType::Integer();

	if ( coerce ( t1, t2, t ) )
		return t;

	throw TypeError();
}

bool nts::coercible_ne ( const DataType & from, const DataType & to ) noexcept
{
	if ( from == to )
		return true;

	/* 
	 * If 'from :: Integral a => a' and
	 * ( 'to :: BitVector' or 'to :: Integer' or 'to :: Integral' )
	 */
	if ( from == DataType::Integral() && to.is_integral() )
		return true;

	if ( from.is_bitvector() && to.is_bitvector() && from.bitwidth() <= to.bitwidth() )
		return true;

	return false;
}

void nts::coercible ( const DataType & from, const DataType & to )
{
	if ( ! coercible_ne ( from, to ) )
		throw TypeError();
}
