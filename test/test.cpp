#include <vector>
#include <iostream>
#include <memory>

#include "nts.hpp"
#include "logic.hpp"
#include "sugar.hpp"

using namespace std;
using namespace nts;
using namespace nts::sugar;


struct Example
{
	struct Nts2
	{
		BasicNts *basic;
		BitVectorVariable *arg_1;
		BitVectorVariable *arg_2;
		BitVectorVariable *arg_ret;


		Nts2()
		{
			basic = new BasicNts ( "nts_2" );
			arg_1 = new BitVectorVariable ( "arg_1", 4 );
			arg_2 = new BitVectorVariable ( "arg_2", 8 );
			arg_ret = new BitVectorVariable ( "arg_ret", 16 );

			arg_1->insert_param_in_to    ( *basic );
			arg_2->insert_param_in_to    ( *basic );
			arg_ret->insert_param_out_to ( *basic );
		}

		~Nts2()
		{
			delete basic;
			basic = nullptr;
		}
	};

	// This should call Nts2
	struct Nts1
	{
		BasicNts *basic;
		BitVectorVariable *var_1;
		BitVectorVariable *var_2;
		BitVectorVariable *var_3;

		TransitionRule *ctr1;

		State * st_1;
		State * st_3;

		Variable * arr;
		//Transition *t1;
		
		void add_arr()
		{
			arr = new Variable (
					DataType (
						ScalarType::Integer(),
						3,
						{ new IntConstant ( 5 ) }
					),
					"my_array"
			);

			arr->insert_to ( *basic );
		}

		void add_arr_read ()
		{
			VariableReference * aref = new VariableReference ( *arr, false );
			std::vector < Term * > idx_terms_1;
			std::vector < Term * > idx_terms_2;

			int i = 0;
			for ( ; i < 2; i++ )
			{
				idx_terms_1.push_back ( new IntConstant ( 2 + i ) );
			}

			for ( ; i < 4; i++ )
			{
				idx_terms_2.push_back ( new IntConstant ( 3 + i ) );
			}

			ArrayTerm *at_1 = new ArrayTerm (
					unique_ptr < VariableReference > ( aref ),
					move ( idx_terms_1 )
			);

			ArrayTerm * at_2 = new ArrayTerm (
					unique_ptr < Term > ( at_1 ),
					move ( idx_terms_2 )
			);
	
			Relation * r = new Relation (
					RelationOp::gt,
					unique_ptr < Term > ( at_2 ),
					unique_ptr < IntConstant> ( new IntConstant ( 9 ) )
			);

			Transition * tra = new Transition (
					unique_ptr < FormulaTransitionRule > (
						new FormulaTransitionRule ( 
							unique_ptr < Formula > ( r )
						)
					),
					*st_1,
					*st_3
			);

			tra->insert_to ( *basic );	
		}

		void add_arr_write ()
		{
			std::vector < Term * > idx_terms_1;
			for ( int i = 0; i < 3; i++ )
			{
				idx_terms_1.push_back ( new IntConstant ( 2 + i ) );
			}

			std::vector < Term * > idx_terms_2;
			std::vector < Term * > value_terms;

			for ( int i = 0; i < 7; i++ )
			{
				idx_terms_2.push_back ( new IntConstant ( 3 * i ) );
				value_terms.push_back ( new IntConstant ( 2 * i ) );
			}

			auto wr = new ArrayWrite (
					*arr,
					move ( idx_terms_1 ),
					move ( idx_terms_2 ),
					move ( value_terms )
			);

			Transition * tra = new Transition (
					unique_ptr < FormulaTransitionRule > (
						new FormulaTransitionRule ( 
							unique_ptr < Formula > ( wr )
						)
					),
					*st_1,
					*st_3
			);

			tra->insert_to ( *basic );	
		}

		Nts1 ( struct Nts2 *n )
		{
			basic = new BasicNts ( "nts_1" );
			var_1 = new BitVectorVariable ( "var_1",  4 );
			var_2 = new BitVectorVariable ( "var_2",  8 );
			var_3 = new BitVectorVariable ( "var_3", 16 );

			var_1->insert_to ( *basic );
			var_2->insert_to ( *basic );
			var_3->insert_to ( *basic );

			auto s1 = new State ( "s1" );
			auto s2 = new State ( "s2" );
			auto s3 = new State ( "s3" );

			s1->insert_to ( *basic );	
			s2->insert_to ( *basic );
			s3->insert_to ( *basic );

			Annotation * a1 = new AnnotString ( "origin", "f1::f2::foo" );
			a1->insert_to ( s1->annotations );

			Annotation * a2 = new AnnotString ( "blah", "foo(bar)" );
			a2->insert_to ( s1->annotations );
			
			Annotation * a3 = new AnnotString ( "meta", "< author name='unknown' />" );
			a3->insert_to ( s2->annotations );


			this->st_1 = s1;
			this->st_3 = s3;

			ctr1 = new CallTransitionRule ( *n->basic, {
					new VariableReference ( *var_1, false ),
					new VariableReference ( *var_2, false ) },
					{ var_3 } );
			auto t = new Transition ( unique_ptr<TransitionRule> ( ctr1 ), *s1, *s2 );
			t->insert_to ( *basic );

			add_arr();
			add_arr_read();
			add_arr_write();
		}

		~Nts1()
		{
			delete basic;
		}

	};


	void examples()
	{
		Nts2 n2;
		Nts1 n1 ( &n2 );
		std::cout << *n2.basic;
		std::cout << *n1.basic;
	}

};

struct Example_callees_callers
{
	BitVectorVariable *bvvar[4];

	BasicNts *nb[2];
	Nts toplevel_nts;

	vector < Transition * > tr;


	Example_callees_callers() :
		toplevel_nts ( "namedNts" )
	{	
		bvvar[0] = new BitVectorVariable ( "var1",  8 );
		bvvar[1] = new BitVectorVariable ( "var2", 16 );
		bvvar[2] = new BitVectorVariable ( "var3",  1 );
		bvvar[3] = new BitVectorVariable ( "var4",  4 );

		nb[0] = new BasicNts ( "nb0" );
		nb[1] = new BasicNts ( "nb1" );

		auto s1 = new State ( "s1" );
		auto s2 = new State ( "s2" );
		auto s3 = new State ( "s3" );
		auto s4 = new State ( "s4" );

		for ( auto s : { s1, s2, s3, s4 } )
		{
			s->insert_to ( *nb[0] );
		}

		// Formulas
		auto *gt = new Relation ( RelationOp::gt,
				unique_ptr<Term>(new VariableReference ( *bvvar[0], false )),
				unique_ptr<Term>(new VariableReference ( *bvvar[1], false )) );

		auto *bf = new FormulaBop ( BoolOp::And,
				unique_ptr<Formula> ( gt ),
				unique_ptr<Formula> ( gt->clone() ) );

		
		// It is not wise to call before BasicNts has all parameters

		// tr[0] - call transition
	
		unique_ptr < TransitionRule > rule;
		rule = unique_ptr < TransitionRule > ( new CallTransitionRule ( *nb[1], {}, {} ) );
		tr.push_back ( new Transition ( move ( rule ), *s1, *s2 ) );
		tr.back()->insert_to ( *nb[0] );

		// tr[1] - call transition
		rule = unique_ptr < TransitionRule > ( new CallTransitionRule ( *nb[1], {}, {} ) );
		tr.push_back ( new Transition ( move ( rule ), *s1, *s3 ) );
		tr.back()->insert_to ( *nb[0] );

		// tr[2] - formula transition
		rule = unique_ptr < TransitionRule > (
				new FormulaTransitionRule (
					unique_ptr<Formula> ( bf ) ) );
		tr.push_back ( new Transition ( move ( rule ), *s1, *s4 ) );
		tr.back()->insert_to ( *nb[0] );

		// tr[3] - formula transition
		rule = unique_ptr < TransitionRule > (
				new FormulaTransitionRule (
					unique_ptr<Formula> ( bf->clone() ) ) );
		tr.push_back ( new Transition ( move ( rule ), *s2, *s3 ) );
		tr.back()->insert_to ( *nb[0] );


		// After this block toplevel_nts owns all BasicNtses
		nb[0]->insert_to ( toplevel_nts );
		nb[1]->insert_to ( toplevel_nts );

		bvvar[0]->insert_to ( toplevel_nts );
		bvvar[1]->insert_param_in_to (  *nb[1] );
		bvvar[2]->insert_before ( *bvvar[1] );

		// Find all uses of bvvar[0]
		printf ( "Users of bvvar[0]\n");
		for ( auto & u : bvvar[0]->users() )
		{
			printf ( "user: %p\n", u.user_ptr.raw );
		}

	}

	// It will automatically destroy everything
	~Example_callees_callers()
	{
		delete bvvar[3];
	}


	void try_callees()
	{
		auto i = nb[0]->callees().begin();

		printf ( "%p == %p\n", tr[0], (*i).transition() );
		i++;
		printf ( "%p == %p\n", tr[1], (*i).transition() );
		i++;
		printf ( "end? %s\n", i == nb[0]->callees().end() ? "yes" : "no" );
	}

	void try_callers()
	{
		auto i = nb[1]->callers().begin();
		auto e = nb[1]->callers().end();
		for (int j = 0; j < 5 && i != e; ++j, ++i )
		{
			printf( "%d: %p\n", j, (*i).transition() );
		}
	}

	void print()
	{
		cout << *nb[0];
		cout << *nb[1];
	}

};

int main ( void )
{
	printf ( "Hello world\n" );

	Example e1;
	cout << "e1.examples()\n";
	e1.examples();

	Example_callees_callers e2;
	cout << "e2.callees()\n";
	e2.try_callees();
	cout << "e2.callers()\n";
	e2.try_callers();
	cout << "e2.print()\n";
	e2.print();

	return 0;
}
