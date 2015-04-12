#include <algorithm>
#include <utility>

#include "nts.hpp"
#include "logic.hpp"
#include "to_csv.hpp"

using namespace nts;
using std::unique_ptr;
using std::move;
using std::ostream;

const char * to_str ( BoolOp op )
{
	switch ( op )
	{
		case BoolOp::And:
			return "&&";

		case BoolOp::Or:
			return "||";

		case BoolOp::Imply:
			return "=>";

		case BoolOp::Equiv:
			return "<=>";

		default:
			throw std::domain_error ( "Unknown value of BoolOp" );
	}
}

const char * to_str ( ArithOp op )
{
	switch ( op )
	{
		case ArithOp::Add:
			return "+";

		case ArithOp::Sub:
			return "-";

		case ArithOp::Mul:
			return "*";

		case ArithOp::Div:
			return "/";

		case ArithOp::Mod:
			return "%";

		default:
			throw std::domain_error ( "Unknown ArithOp" );
	}
}

const char * to_str ( RelationOp op )
{
	switch ( op )
	{
		case RelationOp::eq:
			return "=";

		case RelationOp::neq:
			return "!=";

		case RelationOp::lt:
			return "<";

		case RelationOp::leq:
			return "<=";

		case RelationOp::gt:
			return ">";

		case RelationOp::geq:
			return ">=";

		default:
			throw std::domain_error ( "Unknown RelationOp" );
	}
}

const char * to_str ( Quantifier q  )
{
	switch ( q )
	{
		case Quantifier::Forall:
			return "forall";

		case Quantifier::Exists:
			return "exists";

		default:
			throw std::domain_error ( "Unknown quantifier" );
	}
}

//------------------------------------//
// Term                               //
//------------------------------------//

Term::Term ( bool minus, DataType t ) :
	_minus ( minus ),
	_type  ( t     )
{
	;
}

Term::Term ( const Term & orig ) :
	_minus ( orig._minus ),
	_type  ( orig._type  )
{
	;
}

ostream & nts::operator<< ( ostream & o, const Term & t )
{
	t.print ( o );
	return o;
}

//------------------------------------//
// Formula                            //
//------------------------------------//

ostream & nts::operator<< ( ostream & o, const Formula & f )
{
	f.print ( o );
	return o;
}

//------------------------------------//
// FormulaBop                         //
//------------------------------------//

FormulaBop::FormulaBop ( BoolOp op,
		unique_ptr<Formula> f1,
		unique_ptr<Formula> f2 ) :
	_op ( op ),
	_f { move ( f1 ), move ( f2 ) }
{
	;
}

FormulaBop::FormulaBop ( const FormulaBop & orig ) :
	_op ( orig._op )
{
	_f[0] = unique_ptr<Formula> ( orig._f[0]->clone() );
	_f[1] = unique_ptr<Formula> ( orig._f[1]->clone() );
}

FormulaBop::FormulaBop ( FormulaBop && old ) :
	_op ( old._op )
{
	_f[0] = move ( old._f[0] );
	_f[1] = move ( old._f[1] );
}

const Formula & FormulaBop::formula_1() const
{
	return *_f[0];
}

const Formula & FormulaBop::formula_2() const
{
	return *_f[1];
}

FormulaBop * FormulaBop::clone() const
{
	return new FormulaBop ( *this );
}

void FormulaBop::print ( ostream & o ) const
{
	o << "( " << *_f[0] << " " << to_str ( _op ) << " " << *_f[1] << " )";
}

//------------------------------------//
// FormulaNot                         //
//------------------------------------//

FormulaNot::FormulaNot ( unique_ptr<Formula> f ) :
	_f ( move(f) )
{
	;
}

FormulaNot::FormulaNot ( const FormulaNot & orig )
{
	_f = unique_ptr<Formula> ( orig._f->clone() );
}

FormulaNot::FormulaNot ( FormulaNot && old )
{
	_f = move ( old._f );
}

const Formula & FormulaNot::formula() const
{
	return *_f;
}

FormulaNot * FormulaNot::clone() const
{
	return new FormulaNot ( *this );
}

void FormulaNot::print ( ostream & o ) const
{
	o << "not " << *_f;
}

//------------------------------------//
// QuantifiedType                     //
//------------------------------------//

QuantifiedType::QuantifiedType ( DataType t ) :
	_t    ( t ),
	_from ( nullptr ),
	_to   ( nullptr )
{
	;
}

QuantifiedType::QuantifiedType ( DataType t,
		unique_ptr<Term> from,
		unique_ptr<Term> to  ) :
	_t    ( t                  ),
	_from ( move ( from ) ),
	_to   ( move ( to   ) )
{
	if ( from->type() != t || to->type() != t )
		throw TypeError();
}

QuantifiedType::QuantifiedType ( const QuantifiedType & orig ) :
	_t    ( orig._t ),
	_from ( nullptr ),
	_to   ( nullptr )
{
	if ( orig._from )
	{
		_from = unique_ptr<Term> ( orig._from->clone() );
		_to   = unique_ptr<Term> ( orig._to->clone() );
	}
}

QuantifiedType::QuantifiedType ( QuantifiedType && old ) :
	_t ( move ( old._t ) )
{
	_from = move ( old._from );
	_to   = move ( old._to   );
}

ostream & nts::operator<< ( ostream & o, const QuantifiedType & qt )
{
	qt._t.print ( o );
	if ( qt._from )
	{
		o << "[" << *qt._from << ", " << *qt._to << "]";
	}

	return o;
}

//------------------------------------//
// QuantifiedVariableList             //
//------------------------------------//

QuantifiedVariableList::QuantifiedVariableList
(
		Quantifier             q,
		const QuantifiedType & type
):
	_q     ( q    ),
	_qtype ( type )

{
	;
}

QuantifiedVariableList::QuantifiedVariableList
(
		Quantifier              q,
		const QuantifiedType && type
):
	_q     ( q    ),
	_qtype ( type )

{
	;
}

QuantifiedVariableList::QuantifiedVariableList ( const QuantifiedVariableList & orig ) :
	_q     ( orig._q     ),
	_qtype ( orig._qtype )
{
	for ( auto * v : orig._vars )
	{
		auto * clone = v->clone();
		_vars.push_back ( clone );
	}
}

QuantifiedVariableList::QuantifiedVariableList ( QuantifiedVariableList && old ) :
	_q     ( move ( old._q     ) ),
	_qtype ( move ( old._qtype ) )
{
	_vars = move ( old._vars );
	old._vars.clear();

	for ( auto *v : _vars )
	{
		v->remove_from_parent();
		v->insert_to ( *this );
	}
}

QuantifiedVariableList::~QuantifiedVariableList ()
{
	for ( auto *v : _vars )
	{
		delete v;
	}
}

ostream & nts::operator<< ( ostream & o, const QuantifiedVariableList & qvl )
{
	auto print_name = [] ( ostream & o, const Variable *v ) 
	{
		o << v->name();
	};

	o << to_str ( qvl._q ) << " ";
	to_csv ( o, qvl._vars.cbegin(), qvl._vars.cend(), print_name, ", " );
	o << " : " << qvl._qtype;


	return o;
}


//------------------------------------//
// QuantifiedFormula                  //
//------------------------------------//

QuantifiedFormula::QuantifiedFormula (
				Quantifier              q,
				const QuantifiedType &  type,
				unique_ptr<Formula>     f    ) :
	_qvlist ( q, type         ),
	_f      ( move ( f ) )
{
	;
}

QuantifiedFormula::QuantifiedFormula (
				Quantifier              q,
				const QuantifiedType && type,
				unique_ptr<Formula>     f    ) :
	_qvlist ( q, type         ),
	_f      ( move ( f ) )
{
	;
}

QuantifiedFormula::QuantifiedFormula ( const QuantifiedFormula & orig ) :
	_qvlist ( orig._qvlist )
{
	_f = unique_ptr<Formula> ( orig._f->clone() );
}

QuantifiedFormula::QuantifiedFormula ( QuantifiedFormula && old ) :
	_qvlist ( move ( old._qvlist ) ),
	_f      ( move ( old._f      ) )
{
	;
}

QuantifiedFormula * QuantifiedFormula::clone() const
{
	return new QuantifiedFormula ( *this );
}

void QuantifiedFormula::print ( ostream & o ) const
{
	o << _qvlist << " . " << *_f;
}

//------------------------------------//
// Havoc                              //
//------------------------------------//

Havoc::Havoc ( const std::initializer_list < const Variable *> & vars ) :
	_vars ( vars )
{
	;
}

Havoc::Havoc ( const Havoc & orig ) :
	_vars ( orig._vars )
{
	;
}

Havoc::Havoc ( Havoc && old ) :
	_vars ( move ( old._vars ) )
{
	;
}

Havoc * Havoc::clone() const
{
	return new Havoc ( *this );
}

void Havoc::print ( ostream & o ) const
{
	o << "havoc ( ";
	to_csv ( o, _vars.cbegin(), _vars.cend(),
			[] ( ostream & o, const Variable *var ) {
				o << var->name();
			}, ", " );
	o << " )";
}

//------------------------------------//
// BooleanTerm                        //
//------------------------------------//

BooleanTerm::BooleanTerm ( unique_ptr<Term> t )
{
	if ( t->type() != DataType::Bool() )
		throw TypeError();

	_t = move(t);
}

BooleanTerm::BooleanTerm ( const BooleanTerm & orig )
{
	_t = unique_ptr<Term> ( orig._t->clone() );
}

BooleanTerm::BooleanTerm ( BooleanTerm && old )
{
	_t = move ( old._t );
}

BooleanTerm * BooleanTerm::clone() const
{
	return new BooleanTerm ( *this );
}

void BooleanTerm::print ( std::ostream & o ) const
{
	o << *_t;
}

//------------------------------------//
// Relation                           //
//------------------------------------//

void Relation::check_type ( const DataType &t1, const DataType &t2 )
{
	if ( t1.is_bitvector() && t2.is_bitvector() )
		return;

	if ( t1 != t2 )
		throw TypeError();
}

Relation::Relation ( RelationOp op, unique_ptr<Term> t1, unique_ptr<Term> t2 ) :
	_op ( op )
{
	check_type( t1->type(), t2->type() );
	_t1 = move ( t1 );
	_t2 = move ( t2 );
}

Relation::Relation ( const Relation & orig ) :
	_op ( orig._op )
{
	_t1 = unique_ptr<Term> ( orig._t1->clone() );
	_t2 = unique_ptr<Term> ( orig._t2->clone() );
}

Relation::Relation ( Relation && old ) :
	_op ( move ( old._op ) ),
	_t1 ( move ( old._t1 ) ),
	_t2 ( move ( old._t2 ) )
{
	;
}

Relation * Relation::clone() const
{
	return new Relation ( *this );
}

void Relation::print ( std::ostream & o ) const
{
	o << "( " << *_t1 << " " << to_str ( _op ) << " " << *_t2 << " )";
}

//------------------------------------//
// ArithmeticOperation                //
//------------------------------------//

ArithmeticOperation::ArithmeticOperation ( ArithOp op,
				unique_ptr < Term > t1,
				unique_ptr < Term > t2 ) :
	Term ( false, calc_type ( t1.get(), t2.get() ) ),
	_op ( op ),
	_t1 ( move ( t1 ) ),
	_t2 ( move ( t2 ) )
{
	;
}

ArithmeticOperation::ArithmeticOperation ( const ArithmeticOperation & orig ) :
	Term ( false, orig.type() ),
	_op  ( orig._op )
{
	_t1 = unique_ptr<Term> ( orig._t1->clone() );
	_t2 = unique_ptr<Term> ( orig._t2->clone() );
}

ArithmeticOperation::ArithmeticOperation ( ArithmeticOperation && old ) :
	Term ( false, old.type() ),
	_op  ( std::move ( old._op ) ),
	_t1  ( std::move ( old._t1 ) ),
	_t2  ( std::move ( old._t2 ) )
{
	;
}

DataType ArithmeticOperation::calc_type ( const Term * term1, const Term *term2 )
{
	const DataType &t1 = term1->type();
	const DataType &t2 = term2->type();

	// Same types
	if ( t1 == t2 )
		return t1;

	// t1 can be whatever type of class Integral (probably constant)
	// t2 is some concrete type of class Integral,
	// or whatever type of class Integral
	if ( t1 == DataType::Integral() && t2.is_integral() )
		return t2;

	// Commutatively
	if ( t2 == DataType::Integral() && t1.is_integral() )
		return t1;

	// Both are BitVectors, but have different size (because t1 != t2)
	if ( t1.is_bitvector() && t2.is_bitvector() )
	{
		return DataType::BitVector ( std::max( t1.bitwidth(), t2.bitwidth() ) );
	}

	throw TypeError();
}

const ArithOp & ArithmeticOperation::operation() const
{
	return _op;
}

const Term & ArithmeticOperation::term1() const
{
	return *_t1;
}

const Term & ArithmeticOperation::term2() const
{
	return *_t2;
}

ArithmeticOperation * ArithmeticOperation::clone() const
{
	return new ArithmeticOperation ( *this );
}

void ArithmeticOperation::print ( ostream & o ) const
{
	o << "( " << *_t1 << " " << to_str ( _op ) << " " << *_t2 << " )";
}

//------------------------------------//
// IntConstant                        //
//------------------------------------//

IntConstant::IntConstant ( int value ) :
	Constant ( DataType::Integral() ),
	_value   ( value )
{
	;
}

IntConstant * IntConstant::clone() const
{
	return new IntConstant ( _value );
}

void IntConstant::print ( ostream & o ) const
{
	o << _value;
}

//------------------------------------//
// VariableReference n                //
//------------------------------------//

VariableReference::VariableReference ( const Variable &var, bool primed ) :
	Leaf    ( var.type() ),
	_var    ( &var       ),
	_primed ( primed     )
{
	;
}

VariableReference * VariableReference::clone() const
{
	return new VariableReference ( *_var, _primed );
}

void VariableReference::print ( ostream & o ) const
{
	o << _var->name();
	if ( _primed )
		o << "'";
}


