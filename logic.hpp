#ifndef NTS_LOGIC_HPP_
#define NTS_LOGIC_HPP_
#pragma once

#include <ostream>
#include <vector>
#include <list>
#include <initializer_list>
#include <memory>
#include "data_types.hpp"


namespace nts
{

/*
 * Some concepts
 * 1. Data type of term is fixed.
 *    Please, do not change type of any term,
 *    neither directly (writing to _type field),
 *    nor indirectly (by substitution of subterm).
 */


// Forward declaration (not to depend on nts.hpp)
class Variable;

enum class BoolOp
{
	And,
	Or,
	Imply,
	Equiv
};

enum class ArithOp
{
	Add,
	Sub,
	Mul,
	Div,
	Mod
};

enum class RelationOp
{
	eq,  // =  Equal
	neq, // != Not equal
	leq, // <= Less than or equal to
	lt,  // <  Less than
	geq, // >= Greather than or equal to
	gt   // >  Greater than
};

enum class Quantifier
{
	Forall,
	Exists
};

class Term
{
	public:
		enum TermType
		{
			ArithmeticOperation,
			ArrayTerm,
			MinusTerm,
			Leaf
		};

	private:
		DataType _type;
		TermType _term_type;

	protected:
		using p_Term = std::unique_ptr < Term >;
		virtual void print ( std::ostream & o ) const = 0;

	public:
		// type can be whatever type
		Term ( DataType type, TermType ttype ); 
		Term ( const Term & orig );
		virtual ~Term() = default;

		const DataType & type () const { return _type; }
		TermType term_type() const { return _term_type; }

		// Caller is responsible to manage this
		// (can not use unique_ptr, they are not covariant )
		// Result is guaranteed to be non-null
		// Every child should override this function
		// and return its own type.
		virtual Term * clone() const = 0;

		friend std::ostream & operator<< ( std::ostream & o, const Term & t );
};

// Formulas have always type Bool
// It can be:
// * Atomic proposition
// * FormulaNot
// * FormulaBop
// * QuantifiedFormula
class Formula
{
	public:
		enum Type
		{
			AtomicProposition,
			FormulaNot,
			FormulaBop,
			QuantifiedFormula
		};

	private:
		Type _type;

	protected:
		virtual void print ( std::ostream & o ) const = 0;

	public:
		Formula ( Type t ) : _type ( t ) { ; }
		virtual ~Formula() = default;

		virtual Formula * clone() const = 0;

		Type type() const { return _type; }

		friend std::ostream & operator<< ( std::ostream &, const Formula & );
};

class FormulaBop : public Formula
{
	private:
		BoolOp _op;
		std::unique_ptr<Formula> _f[2];

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		FormulaBop ( BoolOp op,
				std::unique_ptr<Formula> f1,
				std::unique_ptr<Formula> f2 );
		
		FormulaBop ( const FormulaBop & orig );
		FormulaBop ( FormulaBop && old );
		virtual ~FormulaBop() = default;

		Formula & formula_1 () const;
		Formula & formula_2 () const;

		virtual FormulaBop * clone() const override;
};

class FormulaNot : public Formula
{
	private:
		std::unique_ptr<Formula>  _f;
		
	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit FormulaNot ( std::unique_ptr<Formula> f );
		FormulaNot ( const FormulaNot & orig );
		FormulaNot ( FormulaNot && old );
		virtual ~FormulaNot() = default;

		Formula & formula() const;

		virtual FormulaNot * clone() const override;
};

class QuantifiedType
{
	private:
		DataType     _t;

		std::unique_ptr<Term> _from;
		std::unique_ptr<Term> _to;
		
	public:
		QuantifiedType ( DataType t );
		QuantifiedType ( DataType && t );
		QuantifiedType ( DataType t,
				std::unique_ptr<Term> from,
				std::unique_ptr<Term> to );

		QuantifiedType ( const QuantifiedType & orig );
		QuantifiedType ( QuantifiedType && old );

		const DataType & type () const { return _t; }
		// may be null
		const Term * from() { return _from.get(); }
		const Term * to()   { return _to.get();   }

		friend std::ostream & operator<< ( std::ostream & o, const QuantifiedType & qt );
};

// Owns all variables inserted in Variable::insert_to()
class QuantifiedVariableList
{
	private:
		QuantifiedType           _qtype;
		std::list < Variable * > _vars;

		friend class Variable;

	public:
		Quantifier quantifier;

		QuantifiedVariableList ( Quantifier q, QuantifiedType qtype );
		QuantifiedVariableList ( const QuantifiedVariableList & orig );
		QuantifiedVariableList ( QuantifiedVariableList && old );

		~QuantifiedVariableList();

		const QuantifiedType        & qtype()      const { return _qtype; }
		const std::list<Variable *> & variables()  const { return _vars; }

		friend std::ostream & operator<< ( std::ostream & o,
				const QuantifiedVariableList & qvl );
};

class QuantifiedFormula : public Formula
{
	private:
		std::unique_ptr<Formula> _f;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		QuantifiedVariableList list;

		QuantifiedFormula (
				Quantifier               q,
				QuantifiedType           type,
				std::unique_ptr<Formula> f    );

		QuantifiedFormula ( const QuantifiedFormula & orig );
		QuantifiedFormula ( QuantifiedFormula && old );

		virtual ~QuantifiedFormula() = default;

		Formula & formula() const { return *_f; }

		virtual QuantifiedFormula * clone() const override;
};

// Atomic proposition is a:
// * BooleanTerm
// * Havoc
// * Relation
// * ArrayWrite
class AtomicProposition : public Formula
{
	public:
		enum APType
		{
			BooleanTerm,
			Havoc,
			Relation,
			ArrayWrite
		};

	private:
		APType _aptype;

	protected:
		AtomicProposition ( APType t );

	public:
		virtual AtomicProposition * clone() const override = 0;
		APType aptype() const { return _aptype; }
};

class Havoc : public AtomicProposition
{
	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		Havoc ();
		explicit Havoc ( std::vector < Variable * > list );
		Havoc ( const Havoc & orig );
		Havoc ( Havoc && old );
		virtual ~Havoc() = default;

		std::vector < Variable * > variables;

		virtual Havoc * clone() const override;
};

class BooleanTerm : public AtomicProposition
{
	private:
		std::unique_ptr<Term> _t;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit BooleanTerm ( std::unique_ptr<Term> t);
		BooleanTerm ( const BooleanTerm & orig );
		BooleanTerm ( BooleanTerm && old );
		virtual ~BooleanTerm() = default;

		Term & term () const { return *_t; }

		virtual BooleanTerm * clone() const override;
};

class Relation : public AtomicProposition
{
	private:
		RelationOp            _op;
		std::unique_ptr<Term> _t1;
		std::unique_ptr<Term> _t2;
		// Both _t1 and _t2 are coerced to _type
		DataType              _type;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		Relation ( RelationOp op,
				std::unique_ptr<Term> t1,
				std::unique_ptr<Term> t2 );

		Relation ( const Relation & orig );
		Relation ( Relation && old );
		virtual ~Relation() = default;

		const RelationOp & operation() const { return _op; }
		Term & term1() const { return *_t1; }
		Term & term2() const { return *_t2; }

		const DataType & type() const { return _type; }

		virtual Relation * clone() const override;
};

class ArrayWrite : public AtomicProposition
{
	private:
		// a'[1][x+4][1,2,3]
		//    ^^^^^^  ^^^^^
		//    \=+=/   \   |
		//     \+/     \  |
		// _indices_1   \ |
		//                |
		// _indices_2 ----+
		//
		const Variable * _arr;

		using Terms = std::vector < Term * >;

		Terms _indices_1;
		Terms _indices_2;
		Terms _values;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		ArrayWrite ( const Variable & arr, Terms idxs_1, Terms idxs_2, Terms values );
		ArrayWrite ( const ArrayWrite & orig );
		ArrayWrite ( ArrayWrite && old );
		~ArrayWrite();

		virtual ArrayWrite * clone() const override;

		const Terms & indices_1() const { return _indices_1; }
		const Terms & indices_2() const { return _indices_2; }
		const Terms & values()    const { return _values;    }
};

class ArithmeticOperation : public Term
{
	private:

		ArithOp _op;
		p_Term  _t1;
		p_Term  _t2;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:

		ArithmeticOperation ( ArithOp op,
				std::unique_ptr < Term > t1,
				std::unique_ptr < Term > t2 );

		ArithmeticOperation ( const ArithmeticOperation & orig );
		ArithmeticOperation ( ArithmeticOperation && old );
		virtual ~ArithmeticOperation() = default;

		const ArithOp & operation() const { return _op; }
		Term & term1() const { return *_t1; }
		Term & term2() const { return *_t2; }

		virtual ArithmeticOperation * clone() const override;
};

class ArrayTerm : public Term
{
	private:
		p_Term _array;
		std::vector < Term * > _indices;

		// Type after application of 'n' indices
		static DataType after ( const DataType & a_type, unsigned int n );

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		ArrayTerm ( p_Term arr, std::vector < Term * > indices );
		ArrayTerm ( const ArrayTerm & orig );
		ArrayTerm ( ArrayTerm && old );

		virtual ~ArrayTerm();

		Term & array() const { return *_array; }

		// A function which becomes an owner of given term
		// and gives caller a new term to use
		using IdxTransFunc = std::function< p_Term ( p_Term ) >;

		/**
		 * Transforms indices with given function.
		 * @post for each position in _indices,
		 *       _indices[i] is set to f ( old ( _indices[i] ) )
		 */
		void transform_indices ( IdxTransFunc f );

		virtual ArrayTerm * clone() const override;
};

#if 0
class ArraySize : public Term
{
	private:
		p_Term _array;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit ArraySize ( p_Term arr );
		ArraySize ( const ArraySize & orig );
		ArraySize ( ArraySize && old );

		virtual ~ArraySize() = default;

		virtual ArraySize * clone() const override;
};
#endif

class MinusTerm : public Term
{
	private:
		std::unique_ptr < Term > _term;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		MinusTerm ( std::unique_ptr < Term > term );
		MinusTerm ( const MinusTerm & orig );

		Term & term() const { return *_term; }

		virtual MinusTerm * clone() const override;
};

class Leaf : public Term
{
	public:
		enum LeafType
		{
			ThreadID,
			IntConstant,
			UserConstant,
			VariableReference
		};

	private:
		LeafType _leaf_type;

	public:
		Leaf ( DataType type, LeafType ltype ) :
			Term ( std::move ( type ), TermType::Leaf ),
			_leaf_type ( ltype )
		{ ; }

		LeafType leaf_type() const { return _leaf_type; }
		virtual Leaf * clone() const override = 0;
};

class Constant : public Leaf
{
	public:
		Constant ( DataType type, LeafType ltype ) :
			Leaf ( std::move ( type ), ltype )
		{ ; }

		virtual Constant * clone() const override = 0;
};

class ThreadID : public Constant
{
	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		ThreadID();
		~ThreadID() = default;

		virtual ThreadID * clone() const override;
};

class IntConstant : public Constant
{
	private:
		int _value;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit IntConstant ( int value );
		virtual ~IntConstant() = default;

		virtual IntConstant * clone() const override;
};

// Constant of arbitrary type, stored in string
class UserConstant : public Constant
{
	private:
		std::string _value;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		UserConstant ( DataType type, std::string value );
		UserConstant ( const UserConstant & orig );
		UserConstant ( UserConstant && old );
		virtual ~UserConstant() = default;

		virtual UserConstant * clone() const override;
};

class VariableReference : public Leaf
{
	private:
		const Variable * _var;
		const bool       _primed;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		VariableReference ( const Variable &var, bool primed );
		virtual ~VariableReference() = default;

		bool primed() const { return _primed; }
		const Variable & variable () const { return *_var; }

		void substitute ( const Variable & var );

		virtual VariableReference * clone() const override;
};

#if 0
class Formula
{
	public:
		Formula();

		class Variables;
		class PrimedVariables;

		Variables variables() const;
		PrimedVariables primed_variables() const;
};

class Formula::Variables
{
	public:
		class iterator;

		iterator begin();
		iterator end();
};

class Formula::PrimedVariables
{
	public:
		class iterator;

		iterator begin();
		iterator end();
};
#endif

}

#endif // NTS_LOGIC_HPP_
