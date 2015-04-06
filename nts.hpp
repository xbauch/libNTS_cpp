#ifndef NTS_HPP_
#define NTS_HPP_
#pragma once

#include <list>
#include <vector>
#include <string>
#include <iterator>

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
 */


namespace nts
{

// Forward declarations
class BasicNts;
class Instance;
class Variable;

class Nts
{
	private:
		std::list <Instance *> _instances;
		friend class Instance;

		std::list <BasicNts *> _basics;
		friend class BasicNts;

		// Global variables
		std::list <Variable *> _vars;
		friend class Variable;

	public:
		Nts () = default;

		// Copying breaks ownership
		Nts ( const Nts & ) = delete;
		
		Nts ( Nts && ) = default;

		~Nts();

		const std::list <BasicNts *> & basic_ntses() const
		{
			return _basics;
		}

};

class Instance
{
	private:
		using Instances = decltype(Nts::_instances);

		Nts * _parent;
		// Valid if parent is not null
		Instances::iterator _pos;

		BasicNts * function;
		unsigned int n;

	public:	
		Instance ( BasicNts *basic, unsigned int n );
		Instance ( const Instance & ) = delete;
		Instance ( const Instance && ) = delete;

		~Instance() = default;

		void remove_from_parent();
		void insert_to ( Nts * parent );
		void insert_before ( const Instance & i );
};

class Transition;
class State;

/**
 * @brief Represents <nts-basic> 
 */
class BasicNts
{
	private:
		std::string _name;

		using Basics = decltype(Nts::_basics);

		Nts * _parent;
		Basics::iterator _pos;

		friend class Transition;
		std::list < Transition *> _transitions;

		friend class Variable;
		// This must be the same type as in NTS
		using Variables = std::list < Variable * >;
		Variables _variables;
		Variables _params_in;
		Variables _params_out;

		friend class State;
		// Increases after each state_add()
		using States = std::list < State * >;
		States _states;

	public:
		class Callers;
		class Callees;

		explicit BasicNts ( const std::string & name ) : _name(name) {;}
		BasicNts ( const BasicNts &  ) = delete;
		BasicNts ( const BasicNts && ) = delete;

		~BasicNts();

		// Transitions
		Callers callers();
		Callees callees();

		const Callers callers() const;
		const Callees callees() const;

		void remove_from_parent();
		void insert_to ( Nts * parent );

		const Variables & variables()  const { return _variables;  }
		const Variables & params_in()  const { return _params_in;  }
		const Variables & params_out() const { return _params_out; }

		const States & states() const { return _states; }
};

class State
{
	private:
		using States = BasicNts::States;
		States           * _states_list;
		States::iterator   _pos;

		std::string _name;

	public:
		State ( const std::string &  name ) : _name ( name ) { ; }
		State ( const std::string && name ) : _name ( name ) { ; }
		State ( const State &  st  ) = delete;
		State ( const State && old );

		~State() = default;

		const std::string name() const { return _name; }

		bool operator== ( const State & s ) const;
		bool operator!= ( const State & s ) const;

		void insert_to ( BasicNts &n );
		void remove_from_parent ();
};

class Variable
{
	private:
		DataType    _type;
		std::string _name;

		// If variable has a parent, then _pos is valid iterator
		// and points to position of this variable in parent's list
		using Variables = decltype(Nts::_vars);
		Variables         * _parent_list;
		Variables::iterator _pos;


		// If variable is inserted into a parent, default move of variable
		// or parent would break this relation.
		void insert_to ( Variables * parent, const Variables::iterator & before );

	public:
		Variable ( DataType t, const std::string & name );
		Variable ( const Variable &  old ) = delete;
		Variable ( const Variable && old );

		virtual ~Variable() = default;

		// Insert it as normal variable
		void insert_to ( Nts * n );
		void insert_to ( BasicNts *nb );

		// Insert it as input / output parameter
		void insert_param_in_to  ( BasicNts * nb );
		void insert_param_out_to ( BasicNts * nb );

		void insert_before ( const Variable & var );
		void remove_from_parent();


		void insert_copy_to ( const std::string & prefix, BasicNts *nb ) const;

		const DataType & type() const { return _type; }
};

class BitVectorVariable final : public Variable
{
	public:
		BitVectorVariable ( const std::string &name, unsigned int width);

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
		TransitionRule & _rule;

	public:
		explicit Transition ( TransitionRule & rule );
		~Transition();

		void remove_from_parent();

		// Order does not matter
		void insert_to ( BasicNts * parent );

		const TransitionRule & rule() const { return _rule; }
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
		friend Transition::Transition( TransitionRule & rule);
		Transition * _t;

	public:
		TransitionRule ( Kind k ) : _kind ( k ), _t ( nullptr ) { ; }

		// All transition rules can be managed in one container
		// ( virtual destructors allows it )
		virtual ~TransitionRule() = default;

		Kind kind() const { return _kind; }
		const Transition * transition() const { return _t; }
};

class CallTransitionRule : public TransitionRule
{
	public:
		using VariableList = const std::initializer_list < const Variable * > &;
		using Variables     = std::vector < const Variable * >;

	private:
		BasicNts * _dest;
		Variables  _var_in;
		Variables  _var_out;

	public:
		CallTransitionRule ( BasicNts *dest, VariableList in, VariableList out );
		virtual ~CallTransitionRule() = default;

		BasicNts * dest() const { return _dest; }

		const Variables & variables_in()  const { return _var_in;  }
		const Variables & variables_out() const { return _var_out; }
};

class Formula;
class FormulaTransitionRule : public TransitionRule
{
	private:
		const Formula * _f;

	public:
		explicit FormulaTransitionRule ( const Formula * f );
		virtual ~FormulaTransitionRule () = default;

		const Formula * formula() const { return _f; }
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


} // namespace nts

#endif // NTS_HPP_
