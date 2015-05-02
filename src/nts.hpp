#ifndef NTS_HPP_
#define NTS_HPP_
#pragma once

#include <list>
#include <vector>
#include <string>
#include <iterator>
#include <ostream>
#include <memory>

#include "data_types.hpp"

/*
 * Notes about ownership
 *
 * Each entity (Instance, BasicNts, State, Transition, TransitionRule, Variable)
 * can have none or one parent. Parent is set by calling some of _insert() methods.
 * To do so, entity must not have parent. Conversely, parent is removed
 * by calling remove_from_parent() method, and to do so, entity must have a parent.
 *
 * If entity has a parent, then that parent is owner of that entity.
 * If entity does not have a parent, one of following cases holds:
 * a) It is a newly created entity. Then creator is owner of that entity.
 * b) Parent was removed by calling remove_from_parent(). Then the caller is now
 * owner of that entity.
 *
 * Parent calls delete on all its child.
 *
 * Order of declaration matters (read the comments!). Neither
 * local-variable-sized arrays, nor global-variable-sized arrays are not supported,
 * for two reasons:
 * i) Semantics of such array is unclear and ugly.
 * ii) !! Array size would refer to some variable, which could (and probably would)
 *     be destroyed before the array. Destructing the array would then lead to
 *     undefined behavior. !!
 * But it is possible to specify the size of local variable using input parameters,
 * and it is possible to specify the size of output array using all variables.
 *
 * Well, to be honest, the implementation of destructors matters too.
 * 
 */

namespace nts
{

// Forward declarations
class BasicNts;
class Instance;
class Variable;
class Formula;

class Annotation;

class Annotations : public std::list < Annotation * >
{
	public:
		Annotations() { ; }
		Annotations ( const Annotations & orig );
		~Annotations();

		Annotations & operator= ( const Annotations & orig );

		void print ( std::ostream & o ) const;
};

class Nts
{
	private:
		friend class Instance;
		friend class BasicNts;
		friend class Variable;

		using Instances = std::list < Instance * >;
		using BasicNtses = std::list < BasicNts * >;
		using Variables = std::list < Variable * >;


		// Notes about order of declaration:
		// Some of BasicNtses may own an VariableReference
		// to some of global variables or parameters.
		// Because destructor VariableReference needs
		// it to point to valid variable, those references
		// must be destroyed prior to variables.
		// For that reason, variables must be declared
		// before basic ntses.
		//
		// Some of global variables may be arrays with
		// size somehow related to parameters. For the similar
		// reason, parameters must be declared before variables.
		//
		// Instances refer to both BasicNts-es and to parameters
		// (not implemented yet). For the similar reason,
		// instances should be declared after BasicNts-es.

		Variables _pars;
		Variables _vars;

		BasicNtses _basics;
		Instances _instances;

	public:
		explicit Nts ( std::string name );

		// Copying breaks ownership
		Nts ( const Nts & ) = delete;
		
		Nts ( Nts && ) = default;

		~Nts();

		const BasicNtses & basic_ntses() const
		{
			return _basics;
		}

		const Variables & variables() const
		{
			return _vars;
		}

		const Variables & parameters() const
		{
			return _pars;
		}

		const Instances & instances() const
		{
			return _instances;
		}

		friend std::ostream & operator<< ( std::ostream &, const Nts & );

		std::unique_ptr < Formula > initial_formula;
		// FIXME: annotations are not printed
		Annotations annotations;
		std::string name;
};

class Instance
{
	private:
		using Instances = decltype(Nts::_instances);

		Nts * _parent;
		// Valid if parent is not null
		Instances::iterator _pos;

		BasicNts * _bn;


	public:	
		Instance ( BasicNts *basic, unsigned int n );
		Instance ( const Instance & ) = delete;
		Instance ( const Instance && ) = delete;

		~Instance() = default;

		void remove_from_parent();
		void insert_to ( Nts & parent );
		void insert_before ( const Instance & i );

		friend std::ostream & operator<< ( std::ostream &o, const Instance & );

		BasicNts & basic_nts() const { return * _bn; }
		unsigned int n;
};

class Transition;
class State;

/**
 * @brief Represents <nts-basic> 
 */
class BasicNts
{
	private:
		friend class Transition;
		friend class Variable;
		friend class State;

		using Basics = decltype(Nts::_basics);
		using Transitions = std::list < Transition * >;
		using Variables = std::list < Variable * >;
		using States = std::list < State * >;


		// Order of declaration:
		// For detailed commentary, see Nts.
		//
		// Transitions refer to variables, in/out parameters,
		// and 'par' parameters. They should be last.
		//
		// Almost everything can refer to _pars. They should be
		// at the beginning.

		Nts * _parent;
		Basics::iterator _pos;

		States _states;
		Variables _pars;

		// This must be the same type as in NTS
		Variables _params_in;
		Variables _params_out;
		Variables _variables;


		/*
		 * Must be a list. User will want to
		 * iterate over a list of transitions
		 * and remove some of them.
		 * We need that erase() on underlying conteiner
		 * does not invalidate other iterators.
		 */
		Transitions _transitions;

		void print_params_in  ( std::ostream & o ) const;
		void print_params_out ( std::ostream & o ) const;
		void print_variables  ( std::ostream & o ) const;

		void print_states_basic   ( std::ostream & o ) const;
		void print_states_initial ( std::ostream & o ) const;
		void print_states_final   ( std::ostream & o ) const;
		void print_states_error   ( std::ostream & o ) const;

		void print_transitions ( std::ostream & o ) const;

	public:
		class Callers;
		class Callees;

		explicit BasicNts ( std::string name );
		BasicNts ( const BasicNts &  ) = delete;
		BasicNts ( const BasicNts && ) = delete;

		~BasicNts();

		const Transitions & transitions() { return _transitions; }

		Callers callers();
		Callees callees();

		const Callers callers() const;
		const Callees callees() const;

		void remove_from_parent();
		void insert_to ( Nts & parent );
		Nts * parent() const { return _parent; }

		const Variables & variables()  const { return _variables;  }
		const Variables & params_in()  const { return _params_in;  }
		const Variables & params_out() const { return _params_out; }

		const States & states() const { return _states; }

		friend std::ostream & operator<< ( std::ostream &, const BasicNts &);

		Annotations annotations;
		std::string name;
		void * user_data;
};

class State
{
	private:
		BasicNts         * _parent;
		BasicNts::States::iterator   _pos;

		friend class Transition;
		using Transitions = std::list < Transition * >;
		Transitions _incoming_tr;
		Transitions _outgoing_tr;


		bool _initial;
		bool _final;
		bool _error;

	public:
		State ( std::string name );
		State ( const State &  st  ) = delete;
		State ( const State && old ) = delete;

		~State() = default;

		const bool & is_initial() const { return _initial; }
		      bool & is_initial()       { return _initial; }
		
		const bool & is_final() const { return _final; }
		      bool & is_final()       { return _final; }

		const bool & is_error() const { return _error; }
		      bool & is_error()       { return _error; }

		bool operator== ( const State & s ) const;
		bool operator!= ( const State & s ) const;

		// Change parent only of no transition uses this state
		void insert_to ( BasicNts &n );
		void insert_after ( const State & s );
		void remove_from_parent ();

		const Transitions & incoming() const { return _incoming_tr; }
		const Transitions & outgoing() const { return _outgoing_tr; }

		friend std::ostream & operator<< ( std::ostream &, const State & );

		Annotations annotations;
		std::string name;
		void * user_data;
};

class QuantifiedVariableList;
class VariableReference;
class ArrayWrite;

class VariableUser;
class Variable
{
	private:
		DataType    _type;

		// If variable has a parent, then _pos is valid iterator
		// and points to position of this variable in parent's list
		using Variables = decltype(Nts::_vars);
		Variables         * _parent_list;
		Variables::iterator _pos;


		friend class nts::VariableReference;
		friend class nts::ArrayWrite;
		friend class QuantifiedVariableList;

		using Users = std::list < VariableUser >;
		Users _users;


		// If variable is inserted into a parent, default move of variable
		// or parent would break this relation.
		void insert_to ( Variables & parent, const Variables::iterator & before );

	public:
		Variable ( DataType t, std::string name );
		Variable ( const Variable & orig );
		Variable ( const Variable && old );

		virtual ~Variable() = default;

		// Insert it as normal variable
		void insert_to ( Nts & n );
		void insert_to ( BasicNts & nb );

		// Insert it as a parameter of execution
		void insert_par ( Nts & n );
		void insert_par ( BasicNts & bn );

		// Insert it as input / output parameter
		void insert_param_in_to  ( BasicNts & nb );
		void insert_param_out_to ( BasicNts & nb );

		// Make this variable quantified
		void insert_to ( QuantifiedVariableList & ql );

		void insert_before ( const Variable & var );
		void remove_from_parent();

		const Users & users () const { return _users; }
	
		const DataType & type() const { return _type; }

		const Variables * owner_list() const { return _parent_list; }

		Variable * clone() const;

		friend std::ostream & operator<< ( std::ostream &, const Variable & );

		Annotations annotations;
		std::string name;
		void * user_data;
};

class BitVectorVariable final : public Variable
{
	public:
		BitVectorVariable ( std::string name, unsigned int width);

		virtual ~BitVectorVariable() = default;
};

class TransitionRule;

class Transition
{
	public:
		using Transitions = decltype(BasicNts::_transitions);

	private:

		BasicNts * _parent;
		Transitions::iterator _pos;

		std::unique_ptr < TransitionRule > _rule;

		State & _from;
		State & _to;

		// Position of State's list of outgoing / incoming transitions
		// Position in _from._outgoing_tr
		State::Transitions::iterator _st_from_pos;
		// Position in _to._incoming_tr
		State::Transitions::iterator _st_to_pos;

	public:
		// Both states should belong to the same BasicNts (not checked)
		// Transition becomes the owner of 'rule'
		// Both states must belong to some BasicNts
		// and they must belong to the same BasicNts
		Transition ( std::unique_ptr<TransitionRule> rule, State &s1, State &s2 );
		~Transition();

		TransitionRule & rule() const { return *_rule; }
		State & from() const { return _from; }
		State & to() const { return _to; }

		BasicNts * parent() const { return _parent; }
		void insert_to ( BasicNts & bn );
		void remove_from_parent ();

		Annotations annotations;

		friend std::ostream & operator<< ( std::ostream & o, const Transition & );

		void * user_data;
};

class TransitionRule
{
	public:
		enum class Kind
		{
			Call,
			Formula
		};

	private:
		Kind _kind;
		friend class Transition;
		Transition * _t;

		virtual std::ostream & print ( std::ostream & o ) const = 0;
	public:
		TransitionRule ( Kind k ) : _kind ( k ), _t ( nullptr ) { ; }

		// All transition rules can be managed in one container
		// ( virtual destructors allows it )
		virtual ~TransitionRule() = default;

		Kind kind() const { return _kind; }
		Transition * transition() const { return _t; }

		friend std::ostream & operator<< ( std::ostream & o, const TransitionRule &);

		virtual TransitionRule * clone() const = 0;
};

class Term;
class CallTransitionRule : public TransitionRule
{
	public:
		using Variables    = std::vector < const Variable * >;
		using Terms        = std::vector < Term * >;

	private:
		BasicNts & _dest;
		Terms      _term_in;
		Variables  _var_out;

		virtual std::ostream & print ( std::ostream & o ) const override;

		template < typename It_1, typename It_2 >
		static bool coercible (
				It_1 from_begin, It_1 from_end,
				It_2 to_begin,   It_2 to_end    );

		template < typename Cont_1, typename Cont_2 >
		static bool coercible ( const Cont_1 & from, const Cont_2 & to );

		static bool check_args (
				const BasicNts  & dest,
				const Terms     & in,
				const Variables & out
		);

		void set_terms_parent();

	public:
		// Becomes an owner of all terms given in 'in :: ArithList'
		CallTransitionRule ( BasicNts & dest, Terms in, Variables out );
		CallTransitionRule ( const CallTransitionRule & orig );

		virtual ~CallTransitionRule();

		BasicNts & dest() const { return _dest; }

		const Terms & terms_in()  const { return _term_in;  }
		const Variables & variables_out() const { return _var_out; }

		virtual CallTransitionRule * clone() const override;

		using VarTransFunc = std::function < const Variable * ( const Variable * ) >;
		void transform_return_variables ( VarTransFunc f );
};

class FormulaTransitionRule : public TransitionRule
{
	private:
		std::unique_ptr<Formula> _f;

		virtual std::ostream & print ( std::ostream & o ) const override;
		void set_formula_parent();

	public:
		explicit FormulaTransitionRule ( std::unique_ptr<Formula> f );
		FormulaTransitionRule ( const FormulaTransitionRule & orig );

		virtual ~FormulaTransitionRule () = default;

		Formula & formula() const { return *_f; }

		virtual FormulaTransitionRule * clone() const override;
};


// yields call Transitions
class BasicNts::Callees
{
	public:
		using Transitions = decltype(BasicNts::_transitions);

	private:
		Transitions & _transitions;

	public:

		Callees ( Transitions & Transitions) :
			_transitions ( Transitions )
		{ ; }

		class iterator;
		iterator begin();
		iterator end();
};



class BasicNts::Callees::iterator :
	public std::iterator<std::forward_iterator_tag, CallTransitionRule >
{
	private:
		Transitions::iterator _it;
		const Transitions & _t;

		void skip();

	public:

		// 'it' must be related to 't'
		iterator ( const Transitions::iterator &it, const Transitions & t);
		iterator ( const iterator & it ) = default;

		// prefix incrementation
		iterator & operator++ ();
		// postfix incrementation
		iterator operator++ ( int );

		bool operator== ( const iterator & rhs ) const;
		bool operator!= ( const iterator & rhs ) const;


		// can not be end
		const CallTransitionRule & operator* () const;

};

class BasicNts::Callers /*:
	public std::iterator < std::forward_iterator_tag, CallTransitionRule *> */
{
	private:
		BasicNts * _callee;
		Basics    & _basics;

	public:

		Callers ( BasicNts *callee, Basics &basics ) :
			_callee ( callee ),
			_basics ( basics )
		{
			;
		}

		class iterator;
		using const_iterator = const iterator;

		iterator begin();
		iterator end();

		const_iterator begin() const;
		const_iterator end() const;
};

class BasicNts::Callers::iterator :
	public std::iterator < std::forward_iterator_tag, CallTransitionRule >
{
	private:
		using Transitions = decltype(BasicNts::_transitions);

		const Basics & _basics;
		const BasicNts * _callee;
		// Current caller
		Basics::iterator      _BasicNts;
		Transitions::iterator _Transition; // never has end value

		void skip();

	public:
		iterator ( const Basics::iterator & it,
				const Basics & basics,
				const BasicNts *callee );

		iterator operator++ ( int );
		iterator & operator++ ();

		bool operator== ( const iterator & rhs ) const;
		bool operator!= ( const iterator & rhs ) const;

		const CallTransitionRule & operator* () const;
};


class Annotation
{
	public:
		enum class Type
		{
			String
		};

	private:
		Type        _type;

		Annotations           * _parent;
		Annotations::iterator   _pos;

	protected:
		Annotation ( std::string name, Type t );
		virtual void print ( std::ostream & o ) const = 0;

	public:
		virtual ~Annotation() = default;

		void insert_to ( Annotations & ants );
		void remove_from_parent ();

		Annotations * parent() const { return _parent; }

		Type type() const { return _type; }


		friend std::ostream & operator<< ( std::ostream & o, const Annotation & );

		virtual Annotation * clone() const = 0;

		std::string name;
};

class AnnotString : public Annotation
{
	protected:
		virtual void print ( std::ostream & o ) const override;

	public:
		AnnotString ( std::string name, std::string value );
		AnnotString ( const AnnotString & orig );

		virtual ~AnnotString() = default;

		virtual AnnotString * clone() const override;

		std::string value;
};


} // namespace nts

#endif // NTS_HPP_
