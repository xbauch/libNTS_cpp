#include <list>
#include <cstddef>

using std::list;
using std::size_t;

// Forward declarations
class nts;
class nts_basic;
class instance;
class call_rule;

class nts
{
	private:
		list <instance *> _instances;
		friend class instance;

		list <nts_basic *> _basics;
		friend class nts_basic;

	public:
		class folder
		{
			// Processes call
			void call ( call_rule *r );
			
			// All calls to B were performed
			void called_never_more ( nts_basic *b );
		};

		/**
		 * @brief Performs bottom-up tree folding
		 * @param initial - 
		 */
		template < typename T >
		T call_tree_fold( T initial );

};

class instance
{
	private:
		nts * _parent;
		// Valid if parent is not null
		using Instances = decltype(nts::_instances);
		Instances::iterator _pos;

		nts_basic * function;
		size_t n;

	public:	
		instance ( nts_basic *basic, size_t n ) :
			function ( basic ), n ( n )
		{
			;
		}

		void remove_from_parent();
		void insert_to_nts ( nts * parent );
		void insert_before ( const instance & i );
};




class transition;
class nts_basic
{
	private:
		using Basics = decltype(nts::_basics);

		nts * _parent;
		Basics::iterator _pos;

		list < transition *> _transitions;
		friend class transition;

	public:
		class Callers;
		class Callees;

		// transitions
		Callers callers();
		Callees callees();

		const Callers callers() const;
		const Callees callees() const;

		void remove_from_parent();
		void insert_to_nts ( nts * parent );

};

class call_rule
{
	public:
		nts_basic * dest;

	public:
		call_rule ( nts_basic * dest ) :
			dest ( dest )
		{
			;
		}
};

class formula_rule
{

};

class transition
{
	private:
		using Transitions = decltype(nts_basic::_transitions);
		nts_basic * _parent;
		Transitions::iterator _pos;

		call_rule * _call_rule;
		formula_rule * _formula_rule;

	public:
		transition ( call_rule *rule ) :
			_call_rule    ( rule    ),
			_formula_rule ( nullptr )
		{
			;
		}

		transition ( formula_rule * rule ) :
			_call_rule    ( nullptr ),
			_formula_rule ( rule    )
		{
			;
		}

		void remove_from_parent();

		// Order does not matter
		void insert_to_nts_basic ( nts_basic * parent );

		bool has_call_rule() const;
		const call_rule * get_call_rule () const
		{
			return _call_rule;
		}
};


// yields call transitions
class nts_basic::Callees
{
	public:
		using Transitions = decltype(nts_basic::_transitions);

	private:
		Transitions & _transitions;

	public:

		Callees ( Transitions & transitions) :
			_transitions ( transitions )
		{ ; }

		class iterator
		{
			private:
				Transitions::iterator _it;
				const Transitions & _t;

				void skip()
				{
					while ( _it != _t.end() && !(*_it)->has_call_rule() )
						_it++;					
				}

			public:

				// 'it' must be related to 't'
				iterator ( const Transitions::iterator &it,
						const Transitions & t) :
					_it ( it ), _t ( t )
				{ 
					skip();
				}

				iterator ( const iterator & it ) = default;

				// prefix incrementation
				iterator & operator++ ()
				{
					if ( _it != _t.end() )
					{
						_it++;
						skip();
					}
					return *this;
				}

				// postfix incrementation
				iterator operator++ ( int )
				{
					iterator old ( *this );
					operator++();
					return old;
				}

				bool operator== ( const iterator & rhs ) const
				{
					return _it == rhs._it;
				}

				bool operator!= ( const iterator & rhs ) const
				{
					return *this != rhs;
				}

				// can not be end
				transition * & operator* ()
				{
					return *_it;
				}
		};

		iterator begin()
		{
			return iterator ( _transitions.begin(), _transitions );
		}

		iterator end()
		{
			return iterator ( _transitions.end(), _transitions );
		}
};

class nts_basic::Callers
{
	private:
		nts_basic * _callee;
		Basics    & _basics;

	public:

		Callers ( nts_basic *callee, Basics &basics ) :
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

class nts_basic::Callers::iterator
{
	private:
		using Transitions = decltype(nts_basic::_transitions);

		const Basics & _basics;
		const nts_basic * _callee;
		// Current caller
		Basics::iterator      _nts_basic;
		Transitions::iterator _transition; // never has end value

		void skip();

	public:
		iterator ( const Basics::iterator & it,
				const Basics & basics,
				const nts_basic *callee ) :
			_basics    ( basics ),
			_callee    ( callee ),
			_nts_basic ( it )
		{
			if ( _nts_basic != _basics.end() )
			{
				_transition = (*_nts_basic)->_transitions.begin();
				skip();
			}
		}

		iterator operator++ ( int );
		iterator & operator++ ();
		bool operator== ( const iterator & rhs ) const;
		bool operator!= ( const iterator & rhs ) const;
		transition * & operator* ();

};

nts_basic::Callers::iterator nts_basic::Callers::begin()
{
	return iterator ( _basics.begin(), _basics, _callee );
}

nts_basic::Callers::iterator nts_basic::Callers::end()
{
	return iterator ( _basics.end(), _basics, _callee );
}

void nts_basic::Callers::iterator::skip()
{
	while ( _nts_basic != _basics.end() )
	{
		auto e = (*_nts_basic)->_transitions.end();
		while ( _transition != e )
		{
			if ( (*_transition)->has_call_rule() )
			{
				const call_rule *r = (*_transition)->get_call_rule();
				if ( r->dest == this->_callee)
					return;
			}
			_transition++;
		}

		_nts_basic++;

		if ( _nts_basic != _basics.end() )
			_transition = (*_nts_basic)->_transitions.begin();
	}
}

nts_basic::Callers::iterator & nts_basic::Callers::iterator::operator++()
{
	if ( _nts_basic != _basics.end() )
	{
		_transition++;
		skip();
	}

	return *this;
}

nts_basic::Callers::iterator nts_basic::Callers::iterator::operator++(int)
{
	iterator old = *this;
	operator++();
	return old;
}

transition * & nts_basic::Callers::iterator::operator*()
{
	return *_transition;
}

bool nts_basic::Callers::iterator::operator== ( const iterator &rhs ) const
{
	return _nts_basic == rhs._nts_basic &&
		( _nts_basic == _basics.end() || _transition == rhs._transition);
}

bool nts_basic::Callers::iterator::operator!= ( const iterator &rhs) const
{
	return !(*this == rhs);
}
