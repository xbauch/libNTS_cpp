#include <utility>
#include "logic.hpp"
#include "data_types.hpp"

using std::move;
using std::vector;

namespace nts
{

ScalarType::ScalarType ( Type t, unsigned int bw) :
	_type ( t ),
	_bitwidth ( bw )
{
	;
}

ScalarType::ScalarType ( ) :
	_type     ( Type::None ),
	_bitwidth ( 0 )
{
	;
}

ScalarType ScalarType::Integer()
{
	return ScalarType ( Type::Integer, 0 );
}

ScalarType ScalarType::Real()
{
	return ScalarType ( Type::Real, 0 );
}

ScalarType ScalarType::Integral()
{
	return ScalarType ( Type::Integral, 0 );
}

ScalarType ScalarType::BitVector ( unsigned int bw )
{
	return ScalarType ( Type::BitVector, bw );
}

ScalarType ScalarType::Bool ()
{
	return BitVector ( 1 );
}

bool ScalarType::is_integral() const
{
	return _type != Type::Real;
}

bool ScalarType::is_bitvector() const
{
	return _type == Type::BitVector;
}

bool ScalarType::operator== ( const ScalarType & rhs ) const
{
	return _type == rhs._type && _bitwidth == rhs._bitwidth;
}

bool ScalarType::operator!= ( const ScalarType & rhs ) const
{
	return ! (*this == rhs);
}

void ScalarType::print ( std::ostream &o ) const
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

bool coerce ( const ScalarType & t1, const ScalarType & t2, ScalarType & result ) noexcept
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
	if ( t1 == ScalarType::Integral() && t2.is_integral() )
	{
		result = t2;
		return true;
	}

	// Commutatively
	if ( t2 == ScalarType::Integral() && t1.is_integral() )
	{
		result = t1;
		return true;
	}

	// Both are BitVectors, but have different size (because t1 != t2)
	if ( t1.is_bitvector() && t2.is_bitvector() )
	{
		result = ScalarType::BitVector ( std::max ( t1.bitwidth(), t2.bitwidth() ) );
		return true;
	}

	return false;
}

ScalarType coerce ( const ScalarType & t1, const ScalarType & t2 )
{
	// ScalarType does not have zero parameter constructor
	// (and imho ScalarType should not have it)
	ScalarType t = ScalarType::Integer();

	if ( coerce ( t1, t2, t ) )
		return t;

	throw TypeError();
}

bool coercible_ne ( const ScalarType & from, const ScalarType & to ) noexcept
{
	if ( from == to )
		return true;

	/* 
	 * If 'from :: Integral a => a' and
	 * ( 'to :: BitVector' or 'to :: Integer' or 'to :: Integral' )
	 */
	if ( from == ScalarType::Integral() && to.is_integral() )
		return true;

	if ( from.is_bitvector() && to.is_bitvector() && from.bitwidth() <= to.bitwidth() )
		return true;

	return false;
}

void coercible ( const ScalarType & from, const ScalarType & to )
{
	if ( ! coercible_ne ( from, to ) )
		throw TypeError();
}

void DataType::set_term_parent()
{
	for ( Term *t : _arr_size )
	{
		t->_parent_ptr.dtype = this;
		t->_parent_type = Term::ParentType::DataType;
	}
}

DataType::DataType () :
	_type ( ScalarType() ),
	_dim_ref ( 0 ),
	_arr_size ( {} )
{
	;
}

DataType::DataType ( ScalarType t,
		unsigned int dim_ref,
		vector < Term * > arr_size ) :
	_type ( t ),
	_dim_ref ( dim_ref ),
	_arr_size ( move ( arr_size ) )
{
	set_term_parent();
}

DataType::DataType ( const DataType & orig ) :
	_type    ( orig._type    ),
	_dim_ref ( orig._dim_ref )
{
	for ( const Term * t : orig._arr_size )
	{
		_arr_size.push_back ( t->clone() );
	}

	set_term_parent();
}

DataType::DataType ( DataType && old ) :
	_type     ( move ( old._type     ) ),
	_dim_ref  ( move ( old._dim_ref  ) ),
	_arr_size ( move ( old._arr_size ) )
{
	set_term_parent();
}

DataType::~DataType ()
{
	for ( Term *t : _arr_size )
	{
		delete t;
	}
}

DataType & DataType::operator= ( const DataType  & orig )
{
	// Delete old content
	for ( Term * t : _arr_size )
	{
		delete t;
	}

	_type = orig._type;
	_dim_ref = orig._dim_ref;
	for ( const Term * t : orig._arr_size )
	{
		_arr_size.push_back ( t->clone() );
	}

	set_term_parent();

	return *this;
}

DataType & DataType::operator= ( DataType && old )
{
	// Delete old content
	for ( Term * t : _arr_size )
	{
		delete t;
	}

	_arr_size = move ( old._arr_size );
	_type     = old._type;
	_dim_ref  = old._dim_ref;

	set_term_parent();

	return *this;
}

bool DataType::operator== ( const DataType & t ) const
{
	if ( _arr_size.size() > 0 || t._arr_size.size() > 0 )
		return false;

	return ( _type == t._type ) && ( _dim_ref == t._dim_ref ) ;
}

bool DataType::operator!= ( const DataType & t ) const
{
	return ! this->operator== ( t );
}

bool DataType::is_scalar() const noexcept
{
	return ( _arr_size.size() == 0 ) && ( _dim_ref == 0 );
}

void DataType::print_arr ( std::ostream & o ) const
{
	for ( const Term * t : _arr_size )
	{
		o << "[" << *t << "]";
	}

	for ( unsigned int i = 0; i < _dim_ref; i++ )
	{
		o << "[]";
	}
}

bool DataType::can_index_array() const
{
	return is_scalar() && _type.is_integral();
}

bool coerce ( const DataType & t1, const DataType & t2, DataType & result ) noexcept
{
	if ( !t1.is_scalar() || !t2.is_scalar() )
		return false;

	ScalarType st;
	if ( ! coerce ( t1.scalar_type(), t2.scalar_type(), st ) )
		return false;

	result = DataType ( st );
	return true;
}

DataType coerce ( const DataType & t1, const DataType & t2 )
{
	DataType t;
	if ( ! coerce ( t1, t2, t ) )
		throw TypeError();

	return t;
}

bool coercible_ne ( const DataType & from, const DataType & to ) noexcept
{
	if ( !from.is_scalar() || !to.is_scalar() )
		return false;
	return true;
}

void coercible ( const DataType & from, const DataType & to )
{
	if ( ! coercible_ne ( from, to ) )
		throw TypeError();
}

} // namespace nts
