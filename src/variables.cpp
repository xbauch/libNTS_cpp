#include <utility>
#include "nts.hpp"
#include "variables.hpp"

using std::move;

namespace nts
{

//------------------------------------//
// VariableUse                        //
//------------------------------------//

VariableUse::VariableUse ( VariableReference & vref, bool modify ) :
	modifying ( modify ),
	user_type ( UserType::VariableReference )
{
	_var = nullptr;
	user_ptr.vref = & vref;
}

VariableUse::VariableUse ( ArrayWrite & awr ) :
	modifying ( true ),
	user_type ( UserType::ArrayWrite )
{
	_var = nullptr;
	user_ptr.arr_wr = & awr;
}

VariableUse::VariableUse ( CallTransitionRule & ctr ) :
	modifying ( true ),
	user_type ( UserType::CallTransitionRule )
{
	_var = nullptr;
	user_ptr.ctr = & ctr;
}

VariableUse::VariableUse ( UserType type, UserPtr ptr, bool modify ) :
	modifying ( modify ),
	user_type ( type   ),
	user_ptr  ( ptr    )
{
	_var = nullptr;
}

#if 0
VariableUse::VariableUse ( const VariableUse & orig ) :
	user_type ( orig.user_type )
{
	user_ptr.raw = nullptr;
	// set() can call release()
	_var = nullptr;
	set ( orig.get() );
}
#endif

VariableUse::VariableUse ( VariableUse && old ) :
	modifying ( old.modifying ),
	user_type ( old.user_type ),
	user_ptr  ( old.user_ptr  )
{
	_var = nullptr;
	set ( old.release() );

	old.user_ptr.raw = nullptr;
}

VariableUse::~VariableUse()
{
	release();
}

void VariableUse::set ( Variable * v )
{
	release();
	if ( v )
	{
		_pos = v->_uses.insert ( v->_uses.cend(), this );
		_var = v;
	}
}

Variable * VariableUse::release()
{
	Variable * v = _var;
	if ( _var )
	{

		_var->_uses.erase ( _pos );
		_var = nullptr;
	}
	return v;
}

VariableUse & VariableUse::operator= ( Variable * var )
{
	set ( var );
	return *this;
}

VariableUse & VariableUse::operator= ( const VariableUse & orig )
{
	set ( orig.get() );
	return *this;
}

VariableUse & VariableUse::operator= ( VariableUse && old )
{
	set ( old.release() );
	return *this;
}

//------------------------------------//
// VariableUseContainer               //
//------------------------------------//

VariableUseContainer :: VariableUseContainer ( VariableReference & vref, bool modify ) :
	modifying ( modify )
{
	_type = VariableUse::UserType::VariableReference;
	_ptr.vref = & vref;
}

VariableUseContainer :: VariableUseContainer ( ArrayWrite & arr_wr ) :
	modifying ( true )
{
	_type = VariableUse::UserType::ArrayWrite;
	_ptr.arr_wr = & arr_wr;
}

VariableUseContainer :: VariableUseContainer ( CallTransitionRule & ctr ) :
	modifying ( true )
{
	_type = VariableUse::UserType::CallTransitionRule;
	_ptr.ctr = & ctr;
}

VariableUseContainer :: VariableUseContainer ( Havoc & hvc ) :
	modifying ( true )
{
	_type = VariableUse::UserType::Havoc;
	_ptr.hvc = & hvc;
}

void VariableUseContainer :: push_back ( Variable * v )
{
	std::vector < VariableUse > :: push_back ( VariableUse ( _type, _ptr, modifying ) );
	std::vector < VariableUse > :: back() = v;
}

VariableUseContainer & VariableUseContainer::operator= (
		const VariableUseContainer & orig )
{
	clear();

	for ( const VariableUse & u : orig )
		push_back ( u.get() );
	return *this;
}

VariableUseContainer & VariableUseContainer::operator= (
		VariableUseContainer && orig )
{
	clear();

	for ( VariableUse & u : orig )
		push_back ( u.release() );

	orig.clear();

	return *this;
}

//------------------------------------//
// VariableContainer                  //
//------------------------------------//

VariableContainer::VariableContainer ( list < Variable * > l ) :
	list < Variable * > ( l )
{
	;
}

VariableContainer::~VariableContainer()
{
	for ( Variable * v : *this )
	{
		delete v;
	}
	clear();
}

VariableContainer & VariableContainer::operator= ( VariableContainer && old )
{
	list < Variable * > :: operator= ( move ( old ) );
	return *this;
}

}; // namespace nts;
