#ifndef NTS_HPP_
#define NTS_HPP_
#pragma once

#include <list>
#include <string>
#include <iterator>

namespace nts
{

using std::list;
using std::string;



// Forward declarations
class BasicNts;
class Instance;
class Variable;

class Nts
{
	private:
		list <Instance *> _Instances;
		friend class Instance;

		list <BasicNts *> _basics;
		friend class BasicNts;

		list <Variable *> _vars;
		friend class Variable;

	public:

		const std::list <BasicNts *> & basic_ntses() const
		{
			return _basics;
		}
};

class Instance
{
	private:
		using Instances = decltype(Nts::_Instances);

		Nts * _parent;
		// Valid if parent is not null
		Instances::iterator _pos;

		BasicNts * function;
		unsigned int n;

	public:	
		Instance ( BasicNts *basic, unsigned int n );
		void remove_from_parent();
		void insert_to_nts ( Nts * parent );
		void insert_before ( const Instance & i );
};

class Transition;

class BasicNts
{
	private:
		using Basics = decltype(Nts::_basics);

		Nts * _parent;
		Basics::iterator _pos;

		list < Transition *> _Transitions;
		friend class Transition;

	public:
		class Callers;
		class Callees;

		// Transitions
		Callers callers();
		Callees callees();

		const Callers callers() const;
		const Callees callees() const;

		void remove_from_parent();
		void insert_to_nts ( Nts * parent );

};

class Variable
{
	public:
		enum class Type
		{
			Integer,
			Bool,
			Real,
			BitVector
		};


	private:
		Type   _type;
		string _name;

		// At most one of them can be non-null
		// If _parent_nts is not null, then this variable
		// is global variable.
		// If _parent_BasicNts is not null, then this variable
		// is local variable;
		Nts       * _parent_nts;
		BasicNts * _parent_BasicNts;

	public:
		Variable ( Type t, const string & name );

		// Does not check names
		void insert_to_nts ( Nts * n );
		void insert_to_BasicNts ( BasicNts *nb );

		void insert_copy_to_BasicNts ( const std::string & prefix, BasicNts *nb ) const;
};

class Transition
{
	public:
		using Transitions = decltype(BasicNts::_Transitions);

		enum class Kind
		{
			Call,
			Formula
		};

	private:
		Kind _kind;

		BasicNts * _parent;
		Transitions::iterator _pos;

	public:
		Transition ( Kind );

		void remove_from_parent();

		// Order does not matter
		void insert_to_BasicNts ( BasicNts * parent );

		Kind kind() const;
};

Transition::Transition ( Transition::Kind k ) :
	_kind ( k )
{
	;
}

Transition::Kind Transition::kind() const
{
	return _kind;
}

class CallTransition : public Transition
{
	private:
		BasicNts * _dest;

	public:
		CallTransition ( BasicNts *dest );
		BasicNts * dest() const;
};

CallTransition::CallTransition ( BasicNts * dest ) :
	Transition ( Kind::Call ),
	_dest      ( dest       )
{
	;
}

BasicNts * CallTransition::dest() const
{
	return _dest;
}

// yields call Transitions
class BasicNts::Callees
{
	public:
		using Transitions = decltype(BasicNts::_Transitions);

	private:
		Transitions & _Transitions;

	public:

		Callees ( Transitions & Transitions) :
			_Transitions ( Transitions )
		{ ; }

		class iterator;
		iterator begin();
		iterator end();
};



class BasicNts::Callees::iterator :
	public std::iterator<std::forward_iterator_tag, Transition *>
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
		Transition * & operator* ();

};

BasicNts::Callees::iterator::iterator (
		const BasicNts::Callees::Transitions::iterator &it,
		const BasicNts::Callees::Transitions &t ) :
	_it ( it ),
	_t  ( t  )
{
	skip();
}

BasicNts::Callees::iterator & BasicNts::Callees::iterator::operator++ ()
{
	if ( _it != _t.end() )
	{
		_it++;
		skip();
	}
	return *this;
}

BasicNts::Callees::iterator BasicNts::Callees::iterator::operator++ ( int )
{
	iterator old ( *this );
	operator++();
	return old;
}

bool BasicNts::Callees::iterator::operator==
	( const BasicNts::Callees::iterator &rhs ) const
{
	return _it == rhs._it;
}

bool BasicNts::Callees::iterator::operator!=
	( const BasicNts::Callees::iterator &rhs ) const
{
	return *this != rhs;
}

Transition * & BasicNts::Callees::iterator::operator* ()
{
	return *_it;
}

void BasicNts::Callees::iterator::skip()
{
	while ( _it != _t.end() &&
			(*_it)->kind() != Transition::Kind::Call)
	{
			_it++;
	}
}

BasicNts::Callees::iterator BasicNts::Callees::begin()
{
	return iterator ( _Transitions.begin(), _Transitions );
}

BasicNts::Callees::iterator BasicNts::Callees::end()
{
	return iterator ( _Transitions.end(), _Transitions );
}


class BasicNts::Callers :
	public std::iterator < std::forward_iterator_tag, CallTransition *>
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
	public std::iterator < std::forward_iterator_tag, CallTransition *>
{
	private:
		using Transitions = decltype(BasicNts::_Transitions);

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
		CallTransition * operator* ();

};

BasicNts::Callers::iterator::iterator (
		const Basics::iterator & it,
		const Basics           & basics,
		const BasicNts        * callee ) :

	_basics    ( basics ),
	_callee    ( callee ),
	_BasicNts ( it )
{
	if ( _BasicNts != _basics.end() )
	{
		_Transition = (*_BasicNts)->_Transitions.begin();
		skip();
	}
}


Instance::Instance ( BasicNts *basic, unsigned int n )  :
	function ( basic ), n ( n )
{
	;
}


BasicNts::Callers::iterator BasicNts::Callers::begin()
{
	return iterator ( _basics.begin(), _basics, _callee );
}

BasicNts::Callers::iterator BasicNts::Callers::end()
{
	return iterator ( _basics.end(), _basics, _callee );
}

void BasicNts::Callers::iterator::skip()
{
	while ( _BasicNts != _basics.end() )
	{
		auto e = (*_BasicNts)->_Transitions.end();
		while ( _Transition != e )
		{
			if ( (*_Transition)->kind() == Transition::Kind::Call )
			{
				const CallTransition *ct = (CallTransition *) (*_Transition);
				if ( ct->dest() == this->_callee)
					return;
			}
			_Transition++;
		}

		_BasicNts++;

		if ( _BasicNts != _basics.end() )
			_Transition = (*_BasicNts)->_Transitions.begin();
	}
}

BasicNts::Callers::iterator & BasicNts::Callers::iterator::operator++()
{
	if ( _BasicNts != _basics.end() )
	{
		_Transition++;
		skip();
	}

	return *this;
}

BasicNts::Callers::iterator BasicNts::Callers::iterator::operator++(int)
{
	iterator old = *this;
	operator++();
	return old;
}

CallTransition * BasicNts::Callers::iterator::operator*()
{
	return (CallTransition *) (*_Transition);
}

bool BasicNts::Callers::iterator::operator== ( const iterator &rhs ) const
{
	return _BasicNts == rhs._BasicNts &&
		( _BasicNts == _basics.end() || _Transition == rhs._Transition);
}

bool BasicNts::Callers::iterator::operator!= ( const iterator &rhs) const
{
	return !(*this == rhs);
}




} // namespace nts

#endif // NTS_HPP_
