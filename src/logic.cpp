#include <algorithm>
#include <utility>

#include "nts.hpp"
#include "logic.hpp"
#include "to_csv.hpp"

using namespace nts;
using std::unique_ptr;
using std::move;
using std::ostream;
using std::string;
using std::vector;
using std::transform;

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

// What happens, if we apply 'n' terms to an array term?
bool array_type_aply_terms ( const DataType & a_type, unsigned int n, DataType & out )
{
	unsigned int tot = a_type.arr_dimension() + a_type.ref_dimension();

	if ( tot < n )
		return false;

	unsigned int arr;
	unsigned int ref;
	if ( a_type.arr_dimension() >= n )
	{
		arr = a_type.arr_dimension() - n;
		ref = a_type.ref_dimension();
	} else {
		arr = 0;
		ref = tot - n;
	}

	std::vector < Term * > index_terms;
	index_terms.reserve ( arr );
	for ( unsigned int i = 0; i < arr; i++ )
	{
		index_terms.push_back ( a_type.idx_terms()[ n + i ]->clone() ); 
	}

	out = DataType ( a_type.scalar_type(), ref, move ( index_terms ) );
	return true;
}

DataType array_type_apply_terms ( const DataType & a_type, unsigned int n )
{
	DataType t;
	if ( array_type_aply_terms ( a_type, n, t ) )
		return t;

	throw TypeError();
}


//------------------------------------//
// Term                               //
//------------------------------------//

Term::Term ( DataType t, TermType tt ) :
	_type  ( move ( t ) ),
	_term_type ( tt )
{
	_parent_type = Term::ParentType::None;
	_parent_ptr.raw = nullptr;
}

Term::Term ( const Term & orig ) :
	_type  ( orig._type  )
{
	_parent_type = Term::ParentType::None;
	_parent_ptr.raw = nullptr;
}

ostream & nts::operator<< ( ostream & o, const Term & t )
{
	t.print ( o );
	return o;
}

//------------------------------------//
// Formula                            //
//------------------------------------//

Formula::Formula ( Type t ) :
	_type ( t )
{
	_parent_type    = ParentType::None;
	_parent_ptr.raw = nullptr;
}

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
	Formula ( Type::FormulaBop ),
	_op ( op ),
	_f { move ( f1 ), move ( f2 ) }
{
	set_formulas_parent();
}

FormulaBop::FormulaBop ( const FormulaBop & orig ) :
	Formula ( Type::FormulaBop ),
	_op     ( orig._op    )
{
	_f[0] = unique_ptr<Formula> ( orig._f[0]->clone() );
	_f[1] = unique_ptr<Formula> ( orig._f[1]->clone() );
	set_formulas_parent();
}

FormulaBop::FormulaBop ( FormulaBop && old ) :
	Formula ( Type::FormulaBop ),
	_op     ( old._op    )
{
	_f[0] = move ( old._f[0] );
	_f[1] = move ( old._f[1] );
	set_formulas_parent();
}

void FormulaBop::set_formulas_parent()
{
	for ( unsigned int i = 0; i < 2; i++ )
	{
		_f[i]->_parent_ptr.formula = this;
		_f[i]->_parent_type = ParentType::Formula;
	}
}

Formula & FormulaBop::formula_1() const
{
	return *_f[0];
}

Formula & FormulaBop::formula_2() const
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
	Formula ( Type::FormulaNot ),
	_f ( move(f) )
{
	set_formula_parent();
}

FormulaNot::FormulaNot ( const FormulaNot & orig ) :
	Formula ( Type::FormulaNot )
{
	_f = unique_ptr<Formula> ( orig._f->clone() );
	set_formula_parent();
}

FormulaNot::FormulaNot ( FormulaNot && old ) : 
	Formula ( Type::FormulaNot )
{
	_f = move ( old._f );
	set_formula_parent();
}

Formula & FormulaNot::formula() const
{
	return *_f;
}

FormulaNot * FormulaNot::clone() const
{
	return new FormulaNot ( *this );
}

void FormulaNot::set_formula_parent()
{
	_f->_parent_ptr.formula = this;
	_f->_parent_type = Formula::ParentType::Formula;
}

void FormulaNot::print ( ostream & o ) const
{
	o << "not " << *_f;
}

//------------------------------------//
// QuantifiedType                     //
//------------------------------------//

QuantifiedType::QuantifiedType ( DataType t ) :
	_from ( nullptr    ),
	_to   ( nullptr    )
{
	// quantification is supported only over scalar types
	if ( ! t.is_scalar() )
		throw TypeError(); 
	_t = move ( t );
}

QuantifiedType::QuantifiedType ( DataType t,
		unique_ptr<Term> from,
		unique_ptr<Term> to  )
{
	if ( ! t.is_scalar() )
		throw TypeError();

	if ( from->type() != _t || to->type() != _t )
		throw TypeError();

	_t    = move ( t    );
	_from = move ( from );
	_to   = move ( to   );
	set_terms_parent();
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
		set_terms_parent();
	}
}

QuantifiedType::QuantifiedType ( QuantifiedType && old ) :
	_t ( move ( old._t ) )
{
	_from = move ( old._from );
	_to   = move ( old._to   );
	set_terms_parent();
}

void QuantifiedType::set_terms_parent()
{
	if ( _from )
	{
		_from->_parent_type = Term::ParentType::QuantifiedType;
		_from->_parent_ptr.qtype = this;
	}

	if ( _to )
	{
		_to->_parent_type = Term::ParentType::QuantifiedType;
		_to->_parent_ptr.qtype = this;
	}
}

ostream & nts::operator<< ( ostream & o, const QuantifiedType & qt )
{
	// we have only scalar types
	qt._t.scalar_type().print ( o );
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
		Quantifier     q,
		QuantifiedType type
):
	_qtype     ( move ( type ) ),
	quantifier ( q )

{
	;
}
QuantifiedVariableList::QuantifiedVariableList ( const QuantifiedVariableList & orig ) :
	_qtype     ( orig._qtype     ),
	quantifier ( orig.quantifier )
{
	for ( auto * v : orig._vars )
	{
		auto * clone = v->clone();
		_vars.push_back ( clone );
	}
}

QuantifiedVariableList::QuantifiedVariableList ( QuantifiedVariableList && old ) :
	_qtype     ( move ( old._qtype     ) ),
	quantifier ( move ( old.quantifier ) )
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
		o << v->name;
	};

	o << to_str ( qvl.quantifier ) << " ";
	to_csv ( o, qvl._vars.cbegin(), qvl._vars.cend(), print_name, ", " );
	o << " : " << qvl._qtype;


	return o;
}


//------------------------------------//
// QuantifiedFormula                  //
//------------------------------------//

QuantifiedFormula::QuantifiedFormula (
				Quantifier          q,
				QuantifiedType      type,
				unique_ptr<Formula> f    ) :
	Formula ( Type::QuantifiedFormula ),
	_f      ( move ( f ) ),
	list    ( q, move ( type ) )

{
	set_formula_parent();
}

QuantifiedFormula::QuantifiedFormula ( const QuantifiedFormula & orig ) :
	Formula ( Type::QuantifiedFormula ),
	list ( orig.list )
{
	_f = unique_ptr<Formula> ( orig._f->clone() );
	set_formula_parent();
}

QuantifiedFormula::QuantifiedFormula ( QuantifiedFormula && old ) :
	Formula ( Type::QuantifiedFormula ),
	_f   ( move ( old._f   ) ),
	list ( move ( old.list ) )
{
	set_formula_parent();
}

void QuantifiedFormula::set_formula_parent()
{
	_f->_parent_ptr.formula = this;
	_f->_parent_type = Formula::ParentType::Formula;
}

QuantifiedFormula * QuantifiedFormula::clone() const
{
	return new QuantifiedFormula ( *this );
}

void QuantifiedFormula::print ( ostream & o ) const
{
	o << list << " . " << *_f;
}

//------------------------------------//
// AtomicProposition                  //
//------------------------------------//

AtomicProposition::AtomicProposition ( APType t ) :
	Formula ( Formula::Type::AtomicProposition ),
	_aptype ( t )
{
	;
}

//------------------------------------//
// Havoc                              //
//------------------------------------//

Havoc::Havoc () :
	AtomicProposition ( APType::Havoc )
{
	;
}

Havoc::Havoc ( std::vector < Variable * > list ) :
	AtomicProposition ( APType::Havoc ),
	variables ( move ( list ) )
{
	;
}


Havoc::Havoc ( const Havoc & orig ) :
	AtomicProposition ( APType::Havoc ),
	variables ( orig.variables )
{
	;
}

Havoc::Havoc ( Havoc && old ) :
	AtomicProposition ( APType::Havoc ),
	variables ( move ( old.variables ) )
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
	to_csv ( o, variables.cbegin(), variables.cend(),
			[] ( ostream & o, const Variable *var ) {
				o << var->name;
			}, ", " );
	o << " )";
}

//------------------------------------//
// BooleanTerm                        //
//------------------------------------//

BooleanTerm::BooleanTerm ( unique_ptr<Term> t ) :
	AtomicProposition ( APType::BooleanTerm )
{
	if ( !t->type().is_scalar() )
		throw TypeError();
	if ( t->type().scalar_type() != ScalarType::Bool() )
		throw TypeError();

	_t = move(t);
	set_term_parent();
}

BooleanTerm::BooleanTerm ( const BooleanTerm & orig ) :
	AtomicProposition ( APType::BooleanTerm )
{
	_t = unique_ptr<Term> ( orig._t->clone() );
	set_term_parent();
}

BooleanTerm::BooleanTerm ( BooleanTerm && old ) :
	AtomicProposition ( APType::BooleanTerm )
{
	_t = move ( old._t );
	set_term_parent();
}

void BooleanTerm::set_term_parent()
{
	_t->_parent_type = Term::ParentType::Formula;
	_t->_parent_ptr.formula = this;
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

Relation::Relation ( RelationOp op, unique_ptr<Term> t1, unique_ptr<Term> t2 ) :
	AtomicProposition ( APType::Relation ),
	_op   ( op ),
	_type ( coerce ( t1->type(), t2->type() ) )
{
	_t1 = move ( t1 );
	_t2 = move ( t2 );

	set_terms_parent();
}

Relation::Relation ( const Relation & orig ) :
	AtomicProposition ( APType::Relation ),
	_op   ( orig._op   ),
	_type ( orig._type )
{
	_t1 = unique_ptr<Term> ( orig._t1->clone() );
	_t2 = unique_ptr<Term> ( orig._t2->clone() );

	set_terms_parent();
}

Relation::Relation ( Relation && old ) :
	AtomicProposition ( APType::Relation ),
	_op   ( move ( old._op   ) ),
	_t1   ( move ( old._t1   ) ),
	_t2   ( move ( old._t2   ) ),
	_type ( move ( old._type ) )
{
	set_terms_parent();
}

Relation * Relation::clone() const
{
	return new Relation ( *this );
}

void Relation::print ( std::ostream & o ) const
{
	o << "( " << *_t1 << " " << to_str ( _op ) << " " << *_t2 << " )";
}

void Relation::set_terms_parent()
{
	_t1->_parent_type = Term::ParentType::Formula;
	_t1->_parent_ptr.formula = this;
	_t2->_parent_type = Term::ParentType::Formula;
	_t2->_parent_ptr.formula = this;
}

//------------------------------------//
// ArrayWrite                         //
//------------------------------------//

ArrayWrite::ArrayWrite (
		const Variable & arr,
		Terms idxs_1,
		Terms idxs_2,
		Terms values ):
	AtomicProposition ( APType::ArrayWrite )
{
	if ( idxs_2.size() != values.size() )
		throw TypeError();

	for ( const Term *t : idxs_1 )
	{
		if ( ! t->type().can_index_array() )
			throw TypeError();
	}

	for ( const Term *t : idxs_1 )
	{
		if ( ! t->type().can_index_array() )
			throw TypeError();
	}

	DataType value_type = array_type_apply_terms ( arr.type(), idxs_1.size() + 1 );
	for ( const Term * t : values )
	{
		if ( ! coercible_ne ( t->type(), value_type ) )
			throw TypeError();
	}

	_arr       = & arr;
	_indices_1 = move ( idxs_1 );
	_indices_2 = move ( idxs_2 );
	_values    = move ( values );

	set_terms_parent();
}

ArrayWrite::ArrayWrite ( const ArrayWrite & orig ) :
	AtomicProposition ( APType::ArrayWrite )
{
	_arr = orig._arr->clone();
	_indices_1.reserve ( orig._indices_1.size() );
	_indices_2.reserve ( orig._indices_2.size() );
	_values   .reserve ( orig._values   .size() );

	for ( auto * x : orig._indices_1 )
		_indices_1.push_back ( x->clone() );

	for ( auto *x : orig._indices_2 )
		_indices_2.push_back ( x->clone() );

	for ( auto *x : orig._values )
		_values.push_back ( x -> clone() );

	set_terms_parent();
}

ArrayWrite::ArrayWrite ( ArrayWrite && old ) :
	AtomicProposition ( APType::ArrayWrite )
{
	_arr       = move ( old._arr       );
	_indices_1 = move ( old._indices_1 );
	_indices_2 = move ( old._indices_2 );
	_values    = move ( old._values    );

	set_terms_parent();
}

ArrayWrite::~ArrayWrite()
{
	for ( const Term * t : _indices_1 )
		delete t;

	for ( const Term * t : _indices_2 )
		delete t;

	for ( const Term * t : _values )
		delete t;
}

ArrayWrite * ArrayWrite::clone() const
{
	return new ArrayWrite ( *this );
}

void ArrayWrite::print ( ostream & o ) const
{
	o << _arr->name << "'";
	for ( const Term * t : _indices_1 )
		o << "[" << *t << "]";

	o << "[ ";
	to_csv ( o,
			_indices_2.cbegin(),
			_indices_2.cend(),
			ptr_print_function < Term >,
			", "
	);
	o << " ] = [";
	to_csv ( o,
			_values.cbegin(),
			_values.cend(),
			ptr_print_function < Term >,
			", "
	);
	o << "]";
}

void ArrayWrite::set_terms_parent()
{
	for ( Term * t : _values )
	{
		t->_parent_type = Term::ParentType::Formula;
		t->_parent_ptr.formula = this;
	}

	for ( Term * t : _indices_1 )
	{
		t->_parent_type = Term::ParentType::Formula;
		t->_parent_ptr.formula = this;
	}

	for ( Term * t : _indices_2 )
	{
		t->_parent_type = Term::ParentType::Formula;
		t->_parent_ptr.formula = this;
	}
}

//------------------------------------//
// ArithmeticOperation                //
//------------------------------------//

ArithmeticOperation::ArithmeticOperation ( ArithOp op,
				unique_ptr < Term > t1,
				unique_ptr < Term > t2 ) :
	Term ( coerce ( t1->type(), t2->type() ), TermType::ArithmeticOperation ),
	_op ( op ),
	_t1 ( move ( t1 ) ),
	_t2 ( move ( t2 ) )
{
	set_terms_parent();
}

ArithmeticOperation::ArithmeticOperation ( const ArithmeticOperation & orig ) :
	Term ( orig.type(), TermType::ArithmeticOperation ),
	_op  ( orig._op )
{
	_t1 = unique_ptr<Term> ( orig._t1->clone() );
	_t2 = unique_ptr<Term> ( orig._t2->clone() );

	set_terms_parent();
}

ArithmeticOperation::ArithmeticOperation ( ArithmeticOperation && old ) :
	Term ( old.type(), TermType::ArithmeticOperation ),
	_op  ( std::move ( old._op ) ),
	_t1  ( std::move ( old._t1 ) ),
	_t2  ( std::move ( old._t2 ) )
{
	set_terms_parent();
}

ArithmeticOperation * ArithmeticOperation::clone() const
{
	return new ArithmeticOperation ( *this );
}

void ArithmeticOperation::print ( ostream & o ) const
{
	o << "( " << *_t1 << " " << to_str ( _op ) << " " << *_t2 << " )";
}

void ArithmeticOperation::set_terms_parent()
{
	_t1->_parent_type = Term::ParentType::Term;
	_t2->_parent_ptr.term = this;
}


//------------------------------------//
// ArrayTerm                          //
//------------------------------------//

void ArrayTerm::clear_indices_parent()
{
	for ( Term * t : _indices )
	{
		t->_parent_type = Term::ParentType::None;
		t->_parent_ptr.term = nullptr;
	}
}

void ArrayTerm::set_indices_parent()
{
	for ( Term * t : _indices )
	{
		t->_parent_type = Term::ParentType::Term;
		t->_parent_ptr.term = this;
	}
}

void ArrayTerm::set_terms_parent()
{
	set_indices_parent();
	_array->_parent_type = Term::ParentType::Term;
	_array->_parent_ptr.term = this;
}

DataType ArrayTerm::after ( const DataType & a_type, unsigned int n )
{
	unsigned int tot = a_type.arr_dimension() + a_type.ref_dimension();

	if ( tot < n )
		throw TypeError();

	unsigned int arr;
	unsigned int ref;
	if ( a_type.arr_dimension() >= n )
	{
		arr = a_type.arr_dimension() - n;
		ref = a_type.ref_dimension();
	} else {
		arr = 0;
		ref = tot - n;
	}

	std::vector < Term * > index_terms;
	index_terms.reserve ( arr );
	for ( unsigned int i = 0; i < arr; i++ )
	{
		index_terms.push_back ( a_type.idx_terms()[ n + i ]->clone() ); 
	}

	return DataType ( a_type.scalar_type(), ref, move ( index_terms ) );
}

ArrayTerm::ArrayTerm ( p_Term arr, vector < Term * > indices ) :
	Term (
		array_type_apply_terms ( arr->type(), indices.size() ),
		TermType::ArrayTerm
	)	
{
	_array = move ( arr );
	_indices = move ( indices );
	set_terms_parent();
}

ArrayTerm::ArrayTerm ( const ArrayTerm & orig ) :
	Term ( orig.type(), TermType::ArrayTerm )
{
	_array = unique_ptr < Term > (orig._array->clone() );
	_indices.reserve ( orig._indices.size() );
	for ( const Term *t : orig._indices )
	{
		_indices.push_back ( t->clone() );
	}

	set_terms_parent();
}


ArrayTerm::~ArrayTerm()
{
	for ( const Term * t : _indices )
	{
		delete t;
	}
}

void ArrayTerm::print ( ostream & o ) const
{
	o << *_array;
	for ( const Term * t : _indices )
	{
		o << "[" << *t << "]";
	}
}

ArrayTerm * ArrayTerm::clone() const
{
	return new ArrayTerm ( *this );
}

void ArrayTerm::transform_indices ( IdxTransFunc f )
{
	clear_indices_parent();

	transform (
		_indices.cbegin(),
		_indices.cend(),
		_indices.begin(),
		[ &f ] ( Term * t ) {
			return f ( p_Term ( t ) ).release();
		}
	);

	set_indices_parent();
}

//------------------------------------//
// MinusTerm                          //
//------------------------------------//

MinusTerm::MinusTerm ( unique_ptr < Term > term ) :
	Term  ( term->type(), TermType::MinusTerm  ),
	_term ( move ( term ) )
{
	set_term_parent();
}

MinusTerm::MinusTerm ( const MinusTerm & orig ) :
	Term ( orig.type(), TermType::MinusTerm ),
	_term ( unique_ptr < Term > ( orig._term->clone() ) )
{
	set_term_parent();
}

void MinusTerm::set_term_parent()
{
	_term->_parent_type = Term::ParentType::Term;
	_term->_parent_ptr.term = this;
}

void MinusTerm::print ( ostream & o ) const
{
	o << "-" << *_term;
}

MinusTerm * MinusTerm::clone() const
{
	return new MinusTerm ( *this );
}

//------------------------------------//
// ThreadID                           //
//------------------------------------//
ThreadID::ThreadID() :
	Constant ( DataType ( ScalarType::Integral() ), LeafType::ThreadID )
{
	;
}

ThreadID * ThreadID::clone() const
{
	return new ThreadID();
}

void ThreadID::print ( ostream & o ) const
{
	o << "tid";
}


//------------------------------------//
// IntConstant                        //
//------------------------------------//

IntConstant::IntConstant ( int value ) :
	Constant ( DataType ( ScalarType::Integral() ), LeafType::IntConstant ),
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
// UserConstant                       //
//------------------------------------//

UserConstant::UserConstant ( DataType type, string value ) :
	Constant ( move ( type  ), LeafType::UserConstant ),
	_value   ( move ( value ) )
{
	;
}


UserConstant::UserConstant ( const UserConstant & orig ) :
	Constant ( orig ),
	_value   ( orig._value )
{
	;
}

UserConstant::UserConstant ( UserConstant && old ) :
	Constant ( move ( old.type() ), LeafType::UserConstant ),
	_value   ( move ( old._value ) )
{
	;
}

void UserConstant::print ( ostream & o ) const
{
	o << _value;
}

UserConstant * UserConstant::clone() const
{
	return new UserConstant ( *this );
}

//------------------------------------//
// VariableReference                  //
//------------------------------------//

VariableReference::VariableReference ( const Variable &var, bool primed ) :
	Leaf    ( var.type(), LeafType::VariableReference ),
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
	o << _var->name;
	if ( _primed )
		o << "'";
}

void VariableReference::substitute ( const Variable & var )
{
	// It is enough for var to be coercible to _var
	if ( var.type() != _var->type() )
		throw TypeError();
	_var = &var;
}

