#ifndef NTS_LOGIC_HPP_
#define NTS_LOGIC_HPP_
#pragma once

#include <ostream>
#include <vector>
#include <list>
#include <initializer_list>
#include <memory>
#include "data_types.hpp"
#include "variables.hpp"

namespace nts
{

/*
 * Some concepts
 * 1. Data type of term is fixed.
 *    Please, do not change type of any term,
 *    neither directly (writing to _type field),
 *    nor indirectly (by substitution of subterm).
 */


class Nts;
class Variable;
class Formula;
class FormulaTransitionRule;
class CallTransitionRule;
class QuantifiedType;
class VariableReference;
class ArrayWrite;
class QuantifiedVariableList;
class CallTransitionRule;

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
  int evaluate() { return 0; }

		enum class ParentType
		{
			None,
			Term,
			Formula,
			DataType,
			QuantifiedType,
			CallTransitionRule
		};

		union ParentPtr
		{
			void               * raw;
			Term               * term;
			Formula            * formula;
			DataType           * dtype;
			QuantifiedType     * qtype;
			CallTransitionRule * crule;
		};

		ParentType _parent_type;
		ParentPtr  _parent_ptr;
};

// Formulas have always type Bool
// It can be:
// * Atomic proposition
// * FormulaNot
// * FormulaBop
// * QuantifiedFormula
// Formula is often owned by someone.

class Formula
{
	public:
		enum class Type
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
		Formula ( Type t );
		virtual ~Formula() = default;

		virtual Formula * clone() const = 0;

		Type type() const { return _type; }

		friend std::ostream & operator<< ( std::ostream &, const Formula & );

		// Who is owner of this formula?
		enum class ParentType
		{
			None,

			// This formula makes a transition rule
			FormulaTransitionRule,

			// Another formula - like FormulaNot or QuantifiedFormula
			Formula,

			// Nts - this is a root of initial formula
			NtsInitialFormula,
		};

		// Points to owner
		union ParentPtr
		{
			void                  * raw;
			FormulaTransitionRule * ftr;
			Formula               * formula;
			Nts                   * nts;
		};


		// These variables can be publicly read,
		// but only owner is allowed to write them.
		ParentType _parent_type;
		ParentPtr  _parent_ptr;
};

class FormulaBop : public Formula
{
	private:
		BoolOp _op;
		std::unique_ptr<Formula> _f[2];

		void set_formulas_parent();

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
		BoolOp op () const { return _op; }

		virtual FormulaBop * clone() const override;
};

class FormulaNot : public Formula
{
	private:
		std::unique_ptr<Formula>  _f;

		void set_formula_parent();
		
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

		void set_terms_parent();

		friend class QuantifiedVariableList;
		QuantifiedVariableList * _parent;
		
	public:
		QuantifiedType ( DataType t );
		QuantifiedType ( DataType t,
				std::unique_ptr<Term> from,
				std::unique_ptr<Term> to );

		QuantifiedType ( const QuantifiedType & orig );
		QuantifiedType ( QuantifiedType && old );

		const DataType & type () const { return _t; }
		// may be null
		Term * from() const { return _from.get(); }
		Term * to() const  { return _to.get();   }

		QuantifiedVariableList * parent() const { return _parent; }

		friend std::ostream & operator<< ( std::ostream & o, const QuantifiedType & qt );
};

class QuantifiedFormula;
// Owns all variables inserted in Variable::insert_to()
class QuantifiedVariableList
{
	private:
		QuantifiedType           _qtype;

		// Order matters: this container must be placed
		// before formula, because formula may use (and will use)
		// variables from there.
		VariableContainer        _vars;

		friend class Variable;
		friend class QuantifiedFormula;
		QuantifiedFormula * _parent;

	public:
		Quantifier quantifier;

		QuantifiedVariableList ( Quantifier q, QuantifiedType qtype );
		QuantifiedVariableList ( const QuantifiedVariableList & orig );
		QuantifiedVariableList ( QuantifiedVariableList && old );

		~QuantifiedVariableList() = default;

		QuantifiedFormula * parent() const { return _parent; }

		const QuantifiedType    & qtype()      const { return _qtype; }
		const VariableContainer & variables()  const { return _vars; }
		      VariableContainer & variables()        { return _vars; }


		friend std::ostream & operator<< ( std::ostream & o,
				const QuantifiedVariableList & qvl );
};

class QuantifiedFormula : public Formula
{
	// Order of declaration matters.
	// Formula can use variables
	// owned by list. If so,
	// it should be destroyed before list.
	// For that reason formula must be placed
	// after the list.
	public:
		QuantifiedVariableList list;

	private:
		std::unique_ptr<Formula> _f;

	private:
		void set_formula_parent();

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
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
		Havoc ( const std::initializer_list < Variable * > & );
		Havoc ( const Havoc & orig );
		Havoc ( Havoc && old );
		virtual ~Havoc() = default;

		VariableUseContainer variables;

		virtual Havoc * clone() const override;
};

class BooleanTerm : public AtomicProposition
{
	private:
		std::unique_ptr<Term> _t;

		void set_term_parent();

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

		void set_terms_parent();

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
	
		// Order matters: this should be destroyed last
		VariableUse _arr;

		using Terms = std::vector < Term * >;

		Terms _indices_1;
		Terms _indices_2;
		Terms _values;

		void set_terms_parent();
	
	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		ArrayWrite ( Variable & arr, Terms idxs_1, Terms idxs_2, Terms values );
		ArrayWrite ( const ArrayWrite & orig );
		ArrayWrite ( ArrayWrite && old );
		~ArrayWrite();

		virtual ArrayWrite * clone() const override;

		const Terms & indices_1() const { return _indices_1; }
		const Terms & indices_2() const { return _indices_2; }
		const Terms & values()    const { return _values;    }
		const VariableUse & array_use() const { return _arr; }
		Variable * array() const { return _arr.get(); }
};

class ArithmeticOperation : public Term
{
	private:

		ArithOp _op;
		p_Term  _t1;
		p_Term  _t2;

		void set_terms_parent();

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
		void clear_indices_parent();
		void set_indices_parent();
		void set_terms_parent();

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		ArrayTerm ( p_Term arr, std::vector < Term * > indices );
		ArrayTerm ( const ArrayTerm & orig );
		ArrayTerm ( ArrayTerm && old );

		virtual ~ArrayTerm();

		Term & array() const { return *_array; }
  bool is_size_term() { return _indices.empty(); }
		const std::vector < Term * > & indices() const { return _indices; }

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
		void set_term_parent();

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
			VariableReference,
			BoolConstant
		};

	private:
		LeafType _leaf_type;

	public:
		Leaf ( DataType type, LeafType ltype );
		Leaf ( const Leaf & orig );
		Leaf ( Leaf && old );

		LeafType leaf_type() const { return _leaf_type; }
		virtual Leaf * clone() const override = 0;
};

class Constant : public Leaf
{
	public:
		Constant ( DataType type, LeafType ltype );
		Constant ( const Constant & orig );
		Constant ( Constant && old );

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
		IntConstant ( const IntConstant & orig );
		IntConstant ( IntConstant && old );

		virtual ~IntConstant() = default;

		virtual IntConstant * clone() const override;
  int evaluate() { return _value; }
};

class BoolConstant : public Constant
{
	private:
		bool _value;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		explicit BoolConstant ( bool value );
		BoolConstant ( const BoolConstant & orig );
		BoolConstant ( BoolConstant && old );

		virtual ~BoolConstant() = default;

		virtual BoolConstant * clone() const override;
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
  int evaluate() { return std::stoi( _value.c_str() ); }
};

class VariableReference : public Leaf
{
	private:
		VariableUse _var;
		const bool  _primed;

	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		VariableReference ( Variable & var, bool primed );
		VariableReference ( const VariableReference & orig ) = delete;
		VariableReference ( VariableReference && old ) = delete;
		virtual ~VariableReference() = default;

		bool primed() const { return _primed; }

		const VariableUse & variable () const { return _var; }
		VariableUse & variable() { return _var; }

		void substitute ( Variable & var );

		virtual VariableReference * clone() const override;
};

}

#endif // NTS_LOGIC_HPP_
